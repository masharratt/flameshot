// SPDX-License-Identifier: GPL-3.0-or-later
// SPDX-FileCopyrightText: 2017-2019 Alejandro Sirgo Rica & Contributors

#include "flameshot.h"
#include "core/flameshotdaemon.h"
#if defined(Q_OS_MACOS) || defined(Q_OS_WIN)
#include "qhotkey.h"
#endif

#if defined(Q_OS_MACOS)
#include <QWindow>
#include <objc/message.h>

namespace {

constexpr long NSApplicationActivationPolicyRegular = 0;
constexpr long NSApplicationActivationPolicyAccessory = 1;

void setActivationPolicy(long policy)
{
    auto sharedApp = reinterpret_cast<id (*)(id, SEL)>(objc_msgSend);
    auto setPolicy = reinterpret_cast<void (*)(id, SEL, long)>(objc_msgSend);
    id nsApp = sharedApp(reinterpret_cast<id>(objc_getClass("NSApplication")),
                         sel_registerName("sharedApplication"));
    setPolicy(nsApp, sel_registerName("setActivationPolicy:"), policy);
}

void setActivationPolicyRegular()
{
    setActivationPolicy(NSApplicationActivationPolicyRegular);
}

void setActivationPolicyAccessory()
{
    setActivationPolicy(NSApplicationActivationPolicyAccessory);
}

constexpr const char* visibleInDockProperty = "_visibleInDock";

} // namespace

#include <CoreGraphics/CoreGraphics.h>
#endif

#include "config/cacheutils.h"
#include "config/configresolver.h"
#include "config/configwindow.h"
#include "core/actionrunner.h"
#include "core/capturefirstrequest.h"
#include "core/hotkeyutils.h"
#include "core/qguiappcurrentscreen.h"
#include "utils/abstractlogger.h"
#include "utils/capturehistory.h"
#include "utils/confighandler.h"
#include "utils/imageeffects.h"
#include "utils/screengrabber.h"
#include "utils/screenshotsaver.h"
#include "widgets/capture/capturewidget.h"
#include "widgets/capturehistorywindow.h"
#include "widgets/capturelauncher.h"
#include "widgets/capturetoast.h"
#include "widgets/infowindow.h"

#ifdef ENABLE_IMGUR
#include "tools/imgupload/imguploadermanager.h"
#include "tools/imgupload/storages/imguploaderbase.h"
#include "widgets/imguploaddialog.h"
#include "widgets/uploadhistory.h"
#endif

#include <QApplication>
#include <QDateTime>
#include <QBuffer>
#include <QDebug>
#include <QDesktopServices>
#include <QFile>
#include <QMessageBox>
#include <QPainter>
#include <QScreen>
#include <QStandardPaths>
#include <QThread>
#include <QTimer>
#include <QUrl>
#include <QVersionNumber>

#if defined(Q_OS_MACOS)
#include <QScreen>
#endif

Flameshot::Flameshot()
  : m_haveExternalWidget(false)
  , m_captureWindow(nullptr)
{
    QString StyleSheet = CaptureButton::globalStyleSheet();
    qApp->setStyleSheet(StyleSheet);

    // Every successful save is recorded in the capture history
    connect(this,
            &Flameshot::captureSaved,
            this,
            [](const QString& path, const QPixmap& capture) {
                CaptureHistory history(CaptureHistory::defaultIndexPath());
                HistoryEntry entry;
                entry.timestamp = QDateTime::currentDateTimeUtc();
                entry.path = path;
                entry.size = capture.size();
                entry.kind = QStringLiteral("image");
                if (history.append(entry)) {
                    history.trimTo(ConfigHandler().captureHistoryMax());
                }
            });

#if defined(Q_OS_MACOS)
    // Request Screen Recording permission via the proper CoreGraphics API
    if (!CGPreflightScreenCaptureAccess()) {
        CGRequestScreenCaptureAccess();
    }
#endif
#if (defined(Q_OS_MACOS) || defined(Q_OS_WIN))
    // Global shortcuts for MacOS or Windows. Changes in the config file are
    // applied without a restart; only changed key sequences are touched.
    m_hotkeyDebounce = new QTimer(this);
    m_hotkeyDebounce->setSingleShot(true);
    m_hotkeyDebounce->setInterval(300);
    connect(m_hotkeyDebounce, &QTimer::timeout, this, &Flameshot::syncHotkeys);
    connect(ConfigHandler::getInstance(),
            &ConfigHandler::fileChanged,
            m_hotkeyDebounce,
            qOverload<>(&QTimer::start));
    syncHotkeys();
#endif
}

#if (defined(Q_OS_MACOS) || defined(Q_OS_WIN))
namespace {

// Shortcut names registered as global hotkeys on this platform
QStringList globalHotkeyNames()
{
    QStringList names = ConfigHandler::workflowHotkeys();
#if defined(Q_OS_MACOS)
    names << QStringLiteral("SCREENSHOT_HISTORY");
#endif
    return names;
}

} // namespace

void Flameshot::syncHotkeys()
{
    ConfigHandler config;
    QMap<QString, QString> now;
    for (const QString& name : globalHotkeyNames()) {
        now.insert(name, config.shortcut(name));
    }

    for (const QString& name : changedShortcuts(m_hotkeySequences, now)) {
        if (!now.contains(name)) {
            continue;
        }
        QHotkey* hotkey = m_hotkeys.value(name, nullptr);
        if (hotkey == nullptr) {
            hotkey = new QHotkey(this);
            m_hotkeys.insert(name, hotkey);
            connect(hotkey, &QHotkey::activated, qApp, [this, name]() {
                onHotkeyActivated(name);
            });
        }

        const QKeySequence sequence(now.value(name));
        if (sequence.isEmpty()) {
            hotkey->setRegistered(false);
            continue;
        }
        hotkey->setShortcut(sequence, true);
        if (!hotkey->isRegistered()) {
            QString msg = tr("Could not register the global shortcut %1 for "
                             "%2. Another app may be using it.")
                            .arg(sequence.toString(), name);
#if defined(Q_OS_MACOS)
            if (isOptionOnlyShortcut(sequence)) {
                msg += QLatin1Char(' ') +
                       tr("macOS does not allow shortcuts that only use "
                          "Option or Option+Shift.");
            }
#endif
            AbstractLogger::warning() << msg;
        }
    }
    m_hotkeySequences = now;
}

void Flameshot::onHotkeyActivated(const QString& name)
{
    if (name == QLatin1String("SCREENSHOT_HISTORY")) {
#if ENABLE_IMGUR
        history();
#else
        CaptureHistoryWindow::showWindow();
#endif
        return;
    }

    ConfigHandler config;
    // A hotkey with a workflow always runs its actions; TAKE_SCREENSHOT keeps
    // honouring the capture-first switch.
    if (name == QLatin1String("CAPTURE_AND_EDIT") || config.captureFirst()) {
        gui(buildCaptureFirstRequest(
          name,
          config.savePath(),
          QStandardPaths::writableLocation(QStandardPaths::PicturesLocation)));
    } else {
        gui();
    }
}
#endif

Flameshot* Flameshot::instance()
{
    static Flameshot c;
    return &c;
}

CaptureWidget* Flameshot::gui(const CaptureRequest& req)
{
    if (!resolveAnyConfigErrors()) {
        return nullptr;
    }

    CaptureRequest request = req;
    if (shouldApplyLastRegion(request, ConfigHandler().saveLastRegion())) {
        request.setInitialSelection(getLastRegion());
    }

#if defined(Q_OS_MACOS)
    // This is required on MacOS because of Mission Control. If you'll switch to
    // another Desktop you cannot take a new screenshot from the tray, you have
    // to switch back to the Flameshot Desktop manually. It is not obvious and a
    // large number of users are confused and report a bug.
    if (m_captureWindow != nullptr) {
        m_captureWindow->close();
        delete m_captureWindow;
        m_captureWindow = nullptr;
    }
#endif

    if (nullptr == m_captureWindow) {
        // TODO is this unnecessary now?
        int timeout = 5000; // 5 seconds
        const int delay = 100;
        QWidget* modalWidget = nullptr;
        for (; timeout >= 0; timeout -= delay) {
            modalWidget = qApp->activeModalWidget();
            if (nullptr == modalWidget) {
                break;
            }
            modalWidget->close();
            modalWidget->deleteLater();
            QThread::msleep(delay);
        }
        if (0 == timeout) {
            QMessageBox::warning(
              nullptr, tr("Error"), tr("Unable to close active modal widgets"));
            return nullptr;
        }

        m_captureWindow = new CaptureWidget(request);

#ifdef Q_OS_WIN
        m_captureWindow->show();
#elif defined(Q_OS_MACOS)
        if (shouldUseNativeFullscreen(request,
                                      ConfigHandler().useNativeFullscreen())) {
            m_captureWindow->showFullScreen();
        } else {
            m_captureWindow->show();
        }
        m_captureWindow->activateWindow();
        m_captureWindow->raise();
#else
        m_captureWindow->showFullScreen();
//        m_captureWindow->show(); // For CaptureWidget Debugging under Linux
#endif
        return m_captureWindow;
    } else {
        emit captureFailed();
        return nullptr;
    }
}

void Flameshot::screen(CaptureRequest req, const int screenNumber)
{
    if (!resolveAnyConfigErrors()) {
        return;
    }

    bool ok = false;
    QPixmap p;
    QRect geometry;

    if (screenNumber < 0) {
        ScreenGrabber grabber;
        p = grabber.grabEntireDesktop(ok);
        if (ok) {
            QScreen* selectedScreen = grabber.getSelectedScreen();
            if (selectedScreen) {
                geometry = ScreenGrabber().screenGeometry(selectedScreen);
            } else {
                ok = false;
            }
        }
    } else if (screenNumber >= qApp->screens().count()) {
        AbstractLogger() << QObject::tr(
          "Requested screen exceeds screen count");
        ok = false;
    } else {
        // Specific screen number provided - use grabScreen to bypass selector
        QScreen* screen = qApp->screens()[screenNumber];
        p = ScreenGrabber().grabScreen(screen, ok);
        if (ok) {
            geometry = ScreenGrabber().screenGeometry(screen);
        }
    }

    if (ok) {
        QRect region = req.initialSelection();
        if (region.isNull()) {
            region = geometry;
        } else {
            QRect screenGeom = geometry;
            screenGeom.moveTopLeft({ 0, 0 });
            region = region.intersected(screenGeom);
            p = p.copy(region);
        }
        if (req.tasks() & CaptureRequest::PIN) {
            // change geometry for pin task
            req.addPinTask(region);
        }
        exportCapture(p, geometry, req);
    } else {
        emit captureFailed();
    }
}

void Flameshot::full(const CaptureRequest& req)
{
    if (!resolveAnyConfigErrors()) {
        return;
    }

    bool ok = true;
    QPixmap p(ScreenGrabber().grabFullDesktop(ok));
    if (ok) {
        QRect selection; // `flameshot full` does not support region selection
        exportCapture(p, selection, req);
    } else {
        emit captureFailed();
    }
}

void Flameshot::launcher()
{
    if (!resolveAnyConfigErrors()) {
        return;
    }

    if (m_launcherWindow == nullptr) {
        m_launcherWindow = new CaptureLauncher();
    }
    m_launcherWindow->show();
#if defined(Q_OS_MACOS)
    showDockIcon(m_launcherWindow);
#endif
}

void Flameshot::config()
{
    if (!resolveAnyConfigErrors()) {
        return;
    }

    if (m_configWindow == nullptr) {
        m_configWindow = new ConfigWindow();
        m_configWindow->show();
        // Call show() first, otherwise the correct geometry cannot be fetched
        // for centering the window on the screen
        QRect position = m_configWindow->frameGeometry();
        QScreen* currentScreen = QGuiAppCurrentScreen().currentScreen();
        position.moveCenter(currentScreen->availableGeometry().center());
        m_configWindow->move(position.topLeft());
#if defined(Q_OS_MACOS)
        showDockIcon(m_configWindow);
#endif
    }
}

void Flameshot::info()
{
    if (m_infoWindow == nullptr) {
        m_infoWindow = new InfoWindow();
#if defined(Q_OS_MACOS)
        showDockIcon(m_infoWindow);
#endif
    }
}

#ifdef ENABLE_IMGUR
void Flameshot::history()
{
    static UploadHistory* historyWidget = nullptr;
    if (historyWidget == nullptr) {
        historyWidget = new UploadHistory;
        historyWidget->loadHistory();
        connect(historyWidget, &QObject::destroyed, this, []() {
            historyWidget = nullptr;
        });
    }

    historyWidget->show();
    // Call show() first, otherwise the correct geometry cannot be fetched
    // for centering the window on the screen
    QRect position = historyWidget->frameGeometry();
    QScreen* currentScreen = QGuiAppCurrentScreen().currentScreen();
    position.moveCenter(currentScreen->availableGeometry().center());
    historyWidget->move(position.topLeft());

#if defined(Q_OS_MACOS)
    showDockIcon(historyWidget);
#endif
}
#endif

#if defined(Q_OS_MACOS)
void Flameshot::onWindowVisibilityChanged(QWindow::Visibility newVisibility)
{
    auto* qw = qobject_cast<QWindow*>(sender());
    if (!qw) {
        return;
    }

    if (newVisibility == QWindow::Hidden) {
        qw->setProperty(visibleInDockProperty, false);
        --m_dockIconVisibleCount;
        if (m_dockIconVisibleCount == 0) {
            setActivationPolicyAccessory();
        }
    } else {
        bool windowTrackedInDock = qw->property(visibleInDockProperty).toBool();
        if (!windowTrackedInDock) {
            qw->setProperty(visibleInDockProperty, true);
            ++m_dockIconVisibleCount;
            setActivationPolicyRegular();
        }
    }
}

void Flameshot::showDockIcon(QWidget* w)
{
    QWindow* qw = w->windowHandle();
    if (!qw) {
        return;
    }

    connect(qw,
            &QWindow::visibilityChanged,
            this,
            &Flameshot::onWindowVisibilityChanged);
}
#endif

void Flameshot::openSavePath()
{
    QString savePath = ConfigHandler().savePath();
    if (!savePath.isEmpty()) {
        QDesktopServices::openUrl(QUrl::fromLocalFile(savePath));
    }
}

QVersionNumber Flameshot::getVersion()
{
    return QVersionNumber::fromString(
      QStringLiteral(APP_VERSION).replace("v", ""));
}

void Flameshot::setOrigin(Origin origin)
{
    m_origin = origin;
}

Flameshot::Origin Flameshot::origin()
{
    return m_origin;
}

/**
 * @brief Prompt the user to resolve config errors if necessary.
 * @return Whether errors were resolved.
 */
bool Flameshot::resolveAnyConfigErrors()
{
    bool resolved = true;
    ConfigHandler confighandler;
    if (!confighandler.checkUnrecognizedSettings() ||
        !confighandler.checkSemantics()) {
        auto* resolver = new ConfigResolver();
        QObject::connect(
          resolver, &ConfigResolver::rejected, [resolver, &resolved]() {
              resolved = false;
              resolver->deleteLater();
              if (origin() == CLI) {
                  exit(1);
              }
          });
        QObject::connect(
          resolver, &ConfigResolver::accepted, [resolver, &resolved]() {
              resolved = true;
              resolver->close();
              resolver->deleteLater();
              // Ensure that the dialog is closed before starting capture
              qApp->processEvents();
          });
        resolver->exec();
        qApp->processEvents();
    }
    return resolved;
}

void Flameshot::requestCapture(const CaptureRequest& request)
{
    if (!resolveAnyConfigErrors()) {
        return;
    }

    switch (request.captureMode()) {
        case CaptureRequest::FULLSCREEN_MODE:
            QTimer::singleShot(request.delay(),
                               [this, request] { full(request); });
            break;
        case CaptureRequest::SCREEN_MODE: {
            int&& number = request.data().toInt();
            QTimer::singleShot(request.delay(), [this, request, number]() {
                screen(request, number);
            });
            break;
        }
        case CaptureRequest::GRAPHICAL_MODE: {
            QTimer::singleShot(
              request.delay(), this, [this, request]() { gui(request); });
            break;
        }
        default:
            emit captureFailed();
            break;
    }
}

namespace {

// Real side effects of the after-capture actions
class FlameshotActionSink : public ActionSink
{
public:
    FlameshotActionSink(Flameshot* flameshot,
                        const CaptureRequest& req,
                        const QRect& selection)
      : m_flameshot(flameshot)
      , m_req(req)
      , m_selection(selection)
    {}

    QString save(const QPixmap& pixmap) override
    {
        const QString dir =
          m_req.path().isEmpty()
            ? QStandardPaths::writableLocation(QStandardPaths::PicturesLocation)
            : m_req.path();
        QString savedPath;
        if (!saveToFilesystem(pixmap,
                              dir,
                              QString(),
                              &savedPath,
                              m_req.overwriteExisting(),
                              saveExtensionOverride())) {
            return {};
        }
        // History and other listeners rely on this firing for every save
        emit m_flameshot->captureSaved(
          savedPath, pixmap, m_selection, m_req.capturedGlobalRect());
        return savedPath;
    }

    void copyImage(const QPixmap& pixmap) override
    {
        FlameshotDaemon::copyToClipboard(pixmap);
    }

    void copyText(const QString& text) override
    {
        FlameshotDaemon::copyToClipboard(
          text, QObject::tr("Path copied to clipboard."));
    }

    void openEditor(const QString& path,
                    const QPixmap& pixmap,
                    const QRect& globalRect) override
    {
        // The capture window may still be shutting down when this runs, and
        // gui() refuses to open while it exists, so wait for the event loop.
        QPointer<Flameshot> flameshot(m_flameshot);
        QTimer::singleShot(0, m_flameshot, [flameshot, path, pixmap, globalRect]() {
            if (flameshot) {
                flameshot->editSavedCapture(path, pixmap, globalRect, true);
            }
        });
    }

    void pin(const QPixmap& pixmap, const QRect& selection) override
    {
        FlameshotDaemon::createPin(pixmap, selection);
    }

    void showToast(const QString& path,
                   const QPixmap& pixmap,
                   const QRect& selection,
                   const QRect& globalRect) override
    {
        CaptureToast::showFor(path,
                              pixmap,
                              selection,
                              globalRect,
                              ConfigHandler().captureToastSeconds());
    }

    QPixmap applyEffects(const QPixmap& pixmap) override
    {
        const EffectSettings settings = currentEffectSettings();
        m_effectsNeedAlpha = imageeffects::needsAlpha(settings);
        return QPixmap::fromImage(
          imageeffects::applyEffects(pixmap.toImage(), settings));
    }

private:
    static EffectSettings currentEffectSettings()
    {
        ConfigHandler config;
        EffectSettings settings;
        settings.borderPx = config.effectBorderPx();
        settings.borderColor = config.effectBorderColor();
        settings.cornerRadius = config.effectCornerRadius();
        settings.shadow = config.effectShadow();
        return settings;
    }

    // JPEG cannot hold transparency, so such captures are saved as PNG
    QString saveExtensionOverride() const
    {
        if (!m_effectsNeedAlpha) {
            return {};
        }
        const QString ext = ConfigHandler().saveAsFileExtension().toLower();
        if (ext != QLatin1String("jpg") && ext != QLatin1String("jpeg")) {
            return {};
        }
        AbstractLogger::info()
          << QObject::tr("Effects need transparency, saving this capture as "
                         "PNG instead of JPEG.");
        return QStringLiteral("png");
    }

    Flameshot* m_flameshot;
    const CaptureRequest& m_req;
    QRect m_selection;
    bool m_effectsNeedAlpha = false;
};

} // namespace

void Flameshot::runWorkflow(const QPixmap& capture,
                            const QRect& selection,
                            const CaptureRequest& req)
{
    FlameshotActionSink sink(this, req, selection);
    CaptureResult result;
    result.pixmap = capture;
    result.selection = selection;
    result.globalRect = req.capturedGlobalRect();
    const RunReport report = ActionRunner(sink).run(
      ConfigHandler().workflowActions(req.workflow()), result);
    if (!report.skipped.isEmpty()) {
        QStringList names;
        for (CaptureAction a : report.skipped) {
            names << toName(a);
        }
        AbstractLogger::warning(AbstractLogger::Stderr)
          << tr("Skipped after-capture actions: %1").arg(names.join(", "));
    }
}

void Flameshot::exportCapture(const QPixmap& capture,
                              QRect& selection,
                              const CaptureRequest& req)
{
    using CR = CaptureRequest;
    int tasks = req.tasks(), mode = req.captureMode();
    QString path = req.path();

    if (tasks & CR::PRINT_GEOMETRY) {
        QTextStream(stdout)
          << selection.width() << "x" << selection.height() << "+"
          << selection.x() << "+" << selection.y() << "\n";
    }

    if (tasks & CR::PRINT_RAW) {
        QByteArray byteArray;
        QBuffer buffer(&byteArray);
        capture.save(&buffer, "PNG");
        if (QFile file; file.open(stdout, QIODevice::WriteOnly)) {
            file.write(byteArray);
            file.close();
        }
    }

    const bool hasWorkflow = !req.workflow().isEmpty();
    if (hasWorkflow) {
        // The hotkey's configured actions replace the SAVE and COPY tasks
        runWorkflow(capture, selection, req);
    }

    if ((tasks & CR::SAVE) && !hasWorkflow) {
        if (req.path().isEmpty()) {
            saveToFilesystemGUI(capture);
        } else {
            QString savedPath;
            if (saveToFilesystem(capture,
                                 path,
                                 QString(),
                                 &savedPath,
                                 req.overwriteExisting())) {
                emit captureSaved(
                  savedPath, capture, selection, req.capturedGlobalRect());
            }
        }
    }

    if ((tasks & CR::COPY) && !hasWorkflow) {
        FlameshotDaemon::copyToClipboard(capture);
    }

    if (tasks & CR::PIN) {
        FlameshotDaemon::createPin(capture, selection);
        if (mode == CR::SCREEN_MODE || mode == CR::FULLSCREEN_MODE) {
            AbstractLogger::info()
              << QObject::tr("Full screen screenshot pinned to screen");
        }
    }

#ifdef ENABLE_IMGUR
    if (tasks & CR::UPLOAD) {
        if (!ConfigHandler().uploadWithoutConfirmation()) {
            auto* dialog = new ImgUploadDialog();
            if (dialog->exec() == QDialog::Rejected) {
                return;
            }
        }

        ImgUploaderBase* widget = ImgUploaderManager().uploader(capture);
        widget->show();
        widget->activateWindow();
        // NOTE: lambda can't capture 'this' because it might be destroyed later
        CR::ExportTask tasks = tasks;
        QObject::connect(
          widget, &ImgUploaderBase::uploadOk, [=, this](const QUrl& url) {
              if (ConfigHandler().copyURLAfterUpload()) {
                  if (!(tasks & CR::COPY)) {
                      FlameshotDaemon::copyToClipboard(
                        url.toString(), tr("URL copied to clipboard."));
                  }
                  widget->showPostUploadDialog();
              }
          });
    }
#endif

    if (!(tasks & CR::UPLOAD)) {
        emit captureTaken(capture);
    }
}

void Flameshot::editSavedCapture(const QString& path,
                                 const QPixmap& capture,
                                 const QRect& globalRect,
                                 bool overwrite)
{
    QScreen* screen = QGuiApplication::screenAt(globalRect.center());
    if (!screen) {
        screen = QGuiApplication::primaryScreen();
    }
    const qreal dpr = screen->devicePixelRatio();
    const EditGeometry geo = editGeometry(globalRect, screen->geometry(), dpr);

    // Screen sized backdrop with the captured image drawn where it was taken
    QPixmap backdrop(screen->size() * dpr);
    backdrop.setDevicePixelRatio(dpr);
    backdrop.fill(Qt::black);
    {
        QPainter painter(&backdrop);
        painter.drawPixmap(geo.localLogical, capture);
    }

    CaptureRequest req(CaptureRequest::GRAPHICAL_MODE);
    req.setPresetScreenshot(backdrop);
    req.setInitialSelection(geo.initialSelectionDevice);
    req.addSaveTask(path);
    req.setOverwriteExisting(overwrite);
    gui(req);
}

void Flameshot::setExternalWidget(bool b)
{
    m_haveExternalWidget = b;
}
bool Flameshot::haveExternalWidget()
{
    return m_haveExternalWidget;
}

// STATIC ATTRIBUTES
Flameshot::Origin Flameshot::m_origin = Flameshot::DAEMON;
