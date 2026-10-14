// SPDX-License-Identifier: GPL-3.0-or-later

#include "editedthumbnail.h"
#include "core/flameshot.h"
#include "widgets/thumbnailgesture.h"
#include "widgets/toaststack.h"

#include <QApplication>
#include <QCursor>
#include <QDrag>
#include <QGuiApplication>
#include <QMouseEvent>
#include <QPainter>
#include <QPainterPath>
#include <QScreen>

namespace {
const QSize kThumbMax(200, 140);
const int kBorder = 3;
const int kRadius = 8;
const int kMargin = 16;
} // namespace

QPointer<EditedThumbnail> EditedThumbnail::s_current;

EditedThumbnail::EditedThumbnail(const QString& path,
                                 const QPixmap& image,
                                 int seconds)
  : QWidget(nullptr,
            Qt::Tool | Qt::FramelessWindowHint | Qt::WindowStaysOnTopHint |
              Qt::WindowDoesNotAcceptFocus)
  , m_path(path)
  , m_image(image)
{
    setAttribute(Qt::WA_ShowWithoutActivating);
    setAttribute(Qt::WA_DeleteOnClose);
    setAttribute(Qt::WA_QuitOnClose, false);
    setAttribute(Qt::WA_TranslucentBackground);
    setFocusPolicy(Qt::NoFocus);
    setCursor(Qt::PointingHandCursor);
    setToolTip(tr("Click to edit again, right-click to close, drag to "
                  "drop into another app"));

    const qreal dpr = devicePixelRatioF();
    m_scaled = image.scaled(kThumbMax * dpr,
                            Qt::KeepAspectRatio,
                            Qt::SmoothTransformation);
    m_scaled.setDevicePixelRatio(dpr);
    const QSize logical = m_scaled.deviceIndependentSize().toSize();
    setFixedSize(logical + QSize(kBorder * 2, kBorder * 2));

    m_timer.setSingleShot(true);
    m_timer.setInterval(thumbnailCloseDelayMs(seconds));
    connect(&m_timer, &QTimer::timeout, this, &QWidget::close);
}

void EditedThumbnail::showFor(const QString& path,
                              const QPixmap& image,
                              int seconds)
{
    if (s_current) {
        s_current->close();
    }
    auto* thumb = new EditedThumbnail(path, image, seconds);
    s_current = thumb;

    QScreen* screen = QGuiApplication::screenAt(QCursor::pos());
    if (!screen) {
        screen = QGuiApplication::primaryScreen();
    }
    const QList<QRect> pos =
      toastStackGeometry(screen->availableGeometry(), thumb->size(), 1, kMargin, 0);
    thumb->move(pos.first().topLeft());
    thumb->show();
    if (thumb->m_timer.interval() > 0) {
        thumb->m_timer.start();
    }
}

void EditedThumbnail::enterEvent(QEnterEvent* event)
{
    m_timer.stop();
    QWidget::enterEvent(event);
}

void EditedThumbnail::leaveEvent(QEvent* event)
{
    if (m_timer.interval() > 0) {
        m_timer.start();
    }
    QWidget::leaveEvent(event);
}

void EditedThumbnail::paintEvent(QPaintEvent* event)
{
    Q_UNUSED(event)
    QPainter p(this);
    p.setRenderHint(QPainter::Antialiasing);
    QPainterPath frame;
    frame.addRoundedRect(QRectF(rect()), kRadius, kRadius);
    p.fillPath(frame, QColor(0xee, 0xee, 0xee));
    QPainterPath inner;
    const QRectF imageRect(kBorder,
                           kBorder,
                           width() - kBorder * 2,
                           height() - kBorder * 2);
    inner.addRoundedRect(imageRect, kRadius - kBorder, kRadius - kBorder);
    p.setClipPath(inner);
    p.drawPixmap(imageRect.topLeft(), m_scaled);
}

void EditedThumbnail::mousePressEvent(QMouseEvent* event)
{
    m_pressPos = event->position().toPoint();
    m_pressed = true;
}

void EditedThumbnail::mouseMoveEvent(QMouseEvent* event)
{
    if (m_pressed &&
        thumbnailShouldStartDrag(event->buttons(),
                                 m_pressPos,
                                 event->position().toPoint(),
                                 QApplication::startDragDistance())) {
        m_pressed = false;
        startDrag();
    }
}

void EditedThumbnail::mouseReleaseEvent(QMouseEvent* event)
{
    if (!m_pressed) {
        return; // the press became a drag
    }
    m_pressed = false;
    switch (thumbnailReleaseAction(event->button())) {
        case ThumbnailAction::Open:
            Flameshot::instance()->editSavedCapture(m_path, m_image);
            close();
            break;
        case ThumbnailAction::Close:
            close();
            break;
        case ThumbnailAction::None:
            break;
    }
}

void EditedThumbnail::startDrag()
{
    auto* drag = new QDrag(this);
    drag->setMimeData(thumbnailMimeData(m_path, m_image.toImage()));
    drag->setPixmap(m_scaled);
    drag->setHotSpot(m_pressPos - QPoint(kBorder, kBorder));
    if (drag->exec(Qt::CopyAction) != Qt::IgnoreAction) {
        close();
    }
}
