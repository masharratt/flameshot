// SPDX-License-Identifier: GPL-3.0-or-later

#include "capturehistorywindow.h"
#include "core/capturefirstrequest.h"
#include "core/flameshot.h"
#include "core/flameshotdaemon.h"
#include "platform/videothumbnail.h"

#include <QCursor>
#include <QDesktopServices>
#include <QFileInfo>
#include <QGuiApplication>
#include <QHBoxLayout>
#include <QImageReader>
#include <QListWidget>
#include <QPainter>
#include <QProcess>
#include <QPushButton>
#include <QScreen>
#include <QScrollBar>
#include <QUrl>
#include <QVBoxLayout>

namespace {
const QSize kThumb(200, 150);
const int kChunkSize = 4;
const int kPathRole = Qt::UserRole;
const int kLoadedRole = Qt::UserRole + 1;
const int kMissingRole = Qt::UserRole + 2;
const int kKindRole = Qt::UserRole + 3;

bool isRecordingKind(const QString& kind)
{
    return kind == QLatin1String("mp4") || kind == QLatin1String("gif");
}
} // namespace

QPointer<CaptureHistoryWindow> CaptureHistoryWindow::s_instance;

void CaptureHistoryWindow::showWindow()
{
    if (!s_instance) {
        s_instance = new CaptureHistoryWindow;
    } else {
        s_instance->reload();
    }
    s_instance->show();
    s_instance->raise();
    s_instance->activateWindow();
}

CaptureHistoryWindow::CaptureHistoryWindow(QWidget* parent)
  : QWidget(parent)
  , m_history(CaptureHistory::defaultIndexPath())
{
    setAttribute(Qt::WA_DeleteOnClose);
    setWindowTitle(tr("Capture History"));
    resize(900, 600);

    m_list = new QListWidget(this);
    m_list->setViewMode(QListView::IconMode);
    m_list->setResizeMode(QListView::Adjust);
    m_list->setMovement(QListView::Static);
    m_list->setIconSize(kThumb);
    m_list->setGridSize(QSize(kThumb.width() + 24, kThumb.height() + 48));
    m_list->setWordWrap(true);
    m_list->setUniformItemSizes(true);
    m_list->setSelectionMode(QAbstractItemView::SingleSelection);

    auto makeButton = [this](const QString& text, auto handler) {
        auto* b = new QPushButton(text, this);
        connect(b, &QPushButton::clicked, this, handler);
        return b;
    };
    m_openButton = makeButton(tr("Open"), [this]() { openCurrent(); });
    m_editButton = makeButton(tr("Edit"), [this]() { editCurrent(); });
    m_copyButton = makeButton(tr("Copy"), [this]() { copyCurrent(); });
    m_finderButton =
      makeButton(tr("Show in Finder"), [this]() { showCurrentInFinder(); });
    m_removeButton = makeButton(tr("Remove from history"),
                                [this]() { removeCurrent(); });

    auto* buttons = new QHBoxLayout();
    buttons->addWidget(m_openButton);
    buttons->addWidget(m_editButton);
    buttons->addWidget(m_copyButton);
    buttons->addWidget(m_finderButton);
    buttons->addStretch();
    buttons->addWidget(m_removeButton);

    auto* layout = new QVBoxLayout(this);
    layout->addWidget(m_list);
    layout->addLayout(buttons);

    connect(m_list,
            &QListWidget::itemDoubleClicked,
            this,
            [this]() { openCurrent(); });
    connect(m_list,
            &QListWidget::itemSelectionChanged,
            this,
            [this]() { updateButtons(); });
    connect(m_list->verticalScrollBar(),
            &QScrollBar::valueChanged,
            this,
            [this]() { scheduleLoad(); });

    m_loadTimer.setSingleShot(true);
    m_loadTimer.setInterval(0);
    connect(&m_loadTimer, &QTimer::timeout, this, [this]() {
        loadNextChunk();
    });

    reload();
}

QPixmap CaptureHistoryWindow::placeholder(const QString& label) const
{
    QPixmap pm(kThumb);
    pm.fill(QColor(128, 128, 128));
    QPainter p(&pm);
    p.setPen(Qt::white);
    p.drawText(pm.rect(), Qt::AlignCenter, label);
    return pm;
}

void CaptureHistoryWindow::reload()
{
    m_loadTimer.stop();
    m_list->clear();
    const QList<HistoryEntry> entries = m_history.entries();
    for (const HistoryEntry& e : entries) {
        auto* item = new QListWidgetItem(m_list);
        item->setData(kPathRole, e.path);
        item->setData(kLoadedRole, false);
        item->setData(kKindRole, e.kind);
        item->setToolTip(e.path);
        const QString when =
          e.timestamp.toLocalTime().toString(QStringLiteral("yyyy-MM-dd HH:mm"));
        if (e.exists()) {
            item->setText(when);
            item->setIcon(QIcon(placeholder(tr("loading"))));
            item->setData(kMissingRole, false);
        } else {
            item->setText(when + QStringLiteral("\n") + tr("missing"));
            item->setIcon(QIcon(placeholder(tr("missing"))));
            item->setData(kMissingRole, true);
            item->setData(kLoadedRole, true);
        }
    }
    updateButtons();
    scheduleLoad();
}

void CaptureHistoryWindow::scheduleLoad()
{
    if (!m_loadTimer.isActive()) {
        m_loadTimer.start();
    }
}

// Loads thumbnails for visible items first, then the rest, a few per timer
// tick so the UI thread never blocks on hundreds of decodes.
void CaptureHistoryWindow::loadNextChunk()
{
    const QRect viewport = m_list->viewport()->rect();
    QList<QListWidgetItem*> visible;
    QList<QListWidgetItem*> hidden;
    for (int i = 0; i < m_list->count(); ++i) {
        QListWidgetItem* item = m_list->item(i);
        if (item->data(kLoadedRole).toBool()) {
            continue;
        }
        if (viewport.intersects(m_list->visualItemRect(item))) {
            visible.append(item);
        } else {
            hidden.append(item);
        }
    }
    QList<QListWidgetItem*> todo = visible + hidden;
    if (todo.isEmpty()) {
        return;
    }
    const qreal dpr = devicePixelRatioF();
    for (int n = 0; n < kChunkSize && n < todo.size(); ++n) {
        QListWidgetItem* item = todo[n];
        item->setData(kLoadedRole, true);
        QImage img;
        if (item->data(kKindRole).toString() == QLatin1String("mp4")) {
            // First frame of the video
            img = videoFirstFrame(item->data(kPathRole).toString(),
                                  kThumb * dpr);
        } else {
            // Images and GIFs (a static first frame) decode the same way
            QImageReader reader(item->data(kPathRole).toString());
            reader.setAutoTransform(true);
            const QSize orig = reader.size();
            if (orig.isValid()) {
                reader.setScaledSize(
                  orig.scaled(kThumb * dpr, Qt::KeepAspectRatio));
            }
            img = reader.read();
        }
        if (img.isNull()) {
            item->setIcon(QIcon(placeholder(tr("unreadable"))));
            continue;
        }
        img.setDevicePixelRatio(dpr);
        item->setIcon(QIcon(QPixmap::fromImage(img)));
    }
    if (todo.size() > kChunkSize) {
        scheduleLoad();
    }
}

QListWidgetItem* CaptureHistoryWindow::currentItemOrNull() const
{
    const auto selected = m_list->selectedItems();
    return selected.isEmpty() ? nullptr : selected.first();
}

void CaptureHistoryWindow::updateButtons()
{
    QListWidgetItem* item = currentItemOrNull();
    const bool has = item != nullptr;
    const bool usable = has && !item->data(kMissingRole).toBool();
    m_openButton->setEnabled(usable);
    // Recordings cannot be edited; Copy puts their path on the clipboard
    m_editButton->setEnabled(
      usable && !isRecordingKind(item->data(kKindRole).toString()));
    m_copyButton->setEnabled(usable);
    m_finderButton->setEnabled(usable);
    m_removeButton->setEnabled(has);
}

void CaptureHistoryWindow::openCurrent()
{
    QListWidgetItem* item = currentItemOrNull();
    if (!item || item->data(kMissingRole).toBool()) {
        return;
    }
    QDesktopServices::openUrl(
      QUrl::fromLocalFile(item->data(kPathRole).toString()));
}

// The editor backdrop is screen sized, so an image larger than the screen is
// scaled down to fit (aspect kept) and centered. Saving overwrites the file
// only when no resampling happened; otherwise it saves a numbered copy so the
// original keeps its full resolution. globalRect is in global logical points.
void CaptureHistoryWindow::editCurrent()
{
    QListWidgetItem* item = currentItemOrNull();
    if (!item || item->data(kMissingRole).toBool()) {
        return;
    }
    const QString path = item->data(kPathRole).toString();
    const QPixmap pixmap(path);
    if (pixmap.isNull()) {
        return;
    }
    QScreen* screen = QGuiApplication::screenAt(QCursor::pos());
    if (!screen) {
        screen = QGuiApplication::primaryScreen();
    }
    const QRect geo = screen->geometry();
    const qreal dpr = screen->devicePixelRatio();
    QSize logical(qRound(pixmap.width() / dpr), qRound(pixmap.height() / dpr));
    if (logical.width() > geo.width() || logical.height() > geo.height()) {
        logical = logical.scaled(geo.size(), Qt::KeepAspectRatio);
    }
    QRect rect(QPoint(0, 0), logical);
    rect.moveCenter(geo.center());
    Flameshot::instance()->editSavedCapture(
      path,
      pixmap,
      rect,
      editKeepsOriginalPixels(pixmap.size(), logical, dpr));
    close();
}

void CaptureHistoryWindow::copyCurrent()
{
    QListWidgetItem* item = currentItemOrNull();
    if (!item || item->data(kMissingRole).toBool()) {
        return;
    }
    const QString path = item->data(kPathRole).toString();
    if (isRecordingKind(item->data(kKindRole).toString())) {
        FlameshotDaemon::copyToClipboard(path, tr("Path copied to clipboard."));
        return;
    }
    const QPixmap pixmap(path);
    if (!pixmap.isNull()) {
        FlameshotDaemon::copyToClipboard(pixmap);
    }
}

void CaptureHistoryWindow::showCurrentInFinder()
{
    QListWidgetItem* item = currentItemOrNull();
    if (!item || item->data(kMissingRole).toBool()) {
        return;
    }
    const QString path = item->data(kPathRole).toString();
#if defined(Q_OS_MACOS)
    QProcess::startDetached(QStringLiteral("open"),
                            { QStringLiteral("-R"), path });
#else
    QDesktopServices::openUrl(
      QUrl::fromLocalFile(QFileInfo(path).absolutePath()));
#endif
}

void CaptureHistoryWindow::removeCurrent()
{
    QListWidgetItem* item = currentItemOrNull();
    if (!item) {
        return;
    }
    m_history.remove(item->data(kPathRole).toString());
    delete m_list->takeItem(m_list->row(item));
    updateButtons();
}
