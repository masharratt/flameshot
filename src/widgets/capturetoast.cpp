// SPDX-License-Identifier: GPL-3.0-or-later

#include "capturetoast.h"
#include "core/flameshot.h"
#include "core/flameshotdaemon.h"
#include "platform/videothumbnail.h"
#include "widgets/toaststack.h"

#include <QDesktopServices>
#include <QFileInfo>
#include <QGuiApplication>
#include <QHBoxLayout>
#include <QImageReader>
#include <QLabel>
#include <QProcess>
#include <QPushButton>
#include <QScreen>
#include <QUrl>
#include <QVBoxLayout>

namespace {
const QSize kThumbMax(240, 160);
const int kPadding = 8;
const int kMargin = 16;
const int kSpacing = 8;
} // namespace

QList<QPointer<CaptureToast>> CaptureToast::s_toasts;

CaptureToast::CaptureToast(const QString& path,
                           const QPixmap& capture,
                           const QRect& selection,
                           const QRect& globalRect,
                           int seconds,
                           bool recording)
  : QWidget(nullptr,
            Qt::Tool | Qt::FramelessWindowHint | Qt::WindowStaysOnTopHint |
              Qt::WindowDoesNotAcceptFocus)
  , m_path(path)
  , m_capture(capture)
  , m_selection(selection)
  , m_globalRect(globalRect)
{
    setAttribute(Qt::WA_ShowWithoutActivating);
    setAttribute(Qt::WA_DeleteOnClose);
    setAttribute(Qt::WA_QuitOnClose, false);
    setFocusPolicy(Qt::NoFocus);
    setObjectName(QStringLiteral("CaptureToast"));
    setStyleSheet(QStringLiteral(
      "#CaptureToast { background-color: #2b2b2b; border: 1px solid #555;"
      " border-radius: 8px; }"
      "QPushButton { color: #eee; background: #3c3c3c; border: none;"
      " border-radius: 4px; padding: 3px 8px; }"
      "QPushButton:hover { background: #505050; }"));

    auto* layout = new QVBoxLayout(this);
    layout->setContentsMargins(kPadding, kPadding, kPadding, kPadding);

    auto* thumb = new QLabel(this);
    thumb->setFixedSize(kThumbMax);
    thumb->setAlignment(Qt::AlignCenter);
    QPixmap scaled = capture.scaled(kThumbMax * devicePixelRatioF(),
                                    Qt::KeepAspectRatio,
                                    Qt::SmoothTransformation);
    scaled.setDevicePixelRatio(devicePixelRatioF());
    thumb->setPixmap(scaled);
    layout->addWidget(thumb);

    auto* row = new QHBoxLayout();
    row->setSpacing(4);
    auto addButton = [&](const QString& text, auto handler) {
        auto* b = new QPushButton(text, this);
        b->setFocusPolicy(Qt::NoFocus);
        connect(b, &QPushButton::clicked, this, handler);
        row->addWidget(b);
        return b;
    };
    if (recording) {
        addButton(tr("Copy path"), [this]() {
            FlameshotDaemon::copyToClipboard(
              m_path, tr("Path copied to clipboard."));
            close();
        });
    } else {
        addButton(tr("Edit"), [this]() {
            Flameshot::instance()->editSavedCapture(m_path, m_capture);
            close();
        });
        addButton(tr("Copy"), [this]() {
            FlameshotDaemon::copyToClipboard(m_capture);
            close();
        });
        addButton(tr("Pin"), [this]() {
            FlameshotDaemon::createPin(m_capture, m_selection);
            close();
        });
    }
    addButton(tr("Show in Finder"), [this]() {
#if defined(Q_OS_MACOS)
        QProcess::startDetached(QStringLiteral("open"),
                                { QStringLiteral("-R"), m_path });
#else
        QDesktopServices::openUrl(
          QUrl::fromLocalFile(QFileInfo(m_path).absolutePath()));
#endif
        close();
    });
    addButton(QStringLiteral("x"), [this]() { close(); });
    layout->addLayout(row);

    setFixedSize(sizeHint());

    m_timer.setSingleShot(true);
    m_timer.setInterval(seconds * 1000);
    connect(&m_timer, &QTimer::timeout, this, &QWidget::close);
}

void CaptureToast::showFor(const QString& path,
                           const QPixmap& capture,
                           const QRect& selection,
                           const QRect& globalRect,
                           int seconds)
{
    if (seconds <= 0) {
        return;
    }
    auto* toast = new CaptureToast(path, capture, selection, globalRect, seconds);
    s_toasts.prepend(toast);
    restack();
    toast->show();
    toast->m_timer.start();
}

void CaptureToast::showForRecording(const QString& path,
                                    const QString& kind,
                                    int seconds)
{
    if (seconds <= 0) {
        return;
    }
    const qreal dpr = QGuiApplication::primaryScreen()->devicePixelRatio();
    QImage frame;
    if (kind == QLatin1String("gif")) {
        // The first frame is what QImageReader decodes by default
        QImageReader reader(path);
        const QSize orig = reader.size();
        if (orig.isValid()) {
            reader.setScaledSize(orig.scaled(kThumbMax * dpr, Qt::KeepAspectRatio));
        }
        frame = reader.read();
    } else {
        frame = videoFirstFrame(path, kThumbMax * dpr);
    }
    auto* toast = new CaptureToast(
      path, QPixmap::fromImage(frame), QRect(), QRect(), seconds, true);
    s_toasts.prepend(toast);
    restack();
    toast->show();
    toast->m_timer.start();
}

void CaptureToast::enterEvent(QEnterEvent* event)
{
    m_timer.stop();
    QWidget::enterEvent(event);
}

void CaptureToast::leaveEvent(QEvent* event)
{
    m_timer.start();
    QWidget::leaveEvent(event);
}

void CaptureToast::closeEvent(QCloseEvent* event)
{
    m_timer.stop();
    s_toasts.removeAll(this);
    QWidget::closeEvent(event);
    restack();
}

void CaptureToast::restack()
{
    s_toasts.removeAll(nullptr);

    // Group by the screen the capture was taken on, newest first
    QList<QScreen*> screens;
    for (const auto& t : s_toasts) {
        QScreen* s = QGuiApplication::screenAt(t->m_globalRect.center());
        if (!s) {
            s = QGuiApplication::primaryScreen();
        }
        if (!screens.contains(s)) {
            screens.append(s);
        }
    }
    for (QScreen* screen : screens) {
        QList<CaptureToast*> onScreen;
        for (const auto& t : s_toasts) {
            QScreen* s = QGuiApplication::screenAt(t->m_globalRect.center());
            if (!s) {
                s = QGuiApplication::primaryScreen();
            }
            if (s == screen) {
                onScreen.append(t);
            }
        }
        const QList<QRect> positions = toastStackGeometry(screen->availableGeometry(),
                                                      onScreen.first()->size(),
                                                      onScreen.size(),
                                                      kMargin,
                                                      kSpacing);
        for (int i = 0; i < onScreen.size(); ++i) {
            onScreen[i]->move(positions[i].topLeft());
        }
    }
}
