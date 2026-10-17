// SPDX-License-Identifier: GPL-3.0-or-later

#include "trimtimeline.h"

#include <QMouseEvent>
#include <QPainter>
#include <cstdlib>

namespace {
const int kHandleWidth = 10;
const int kHeight = 36;
// A press this close to a handle grabs it
const int kGrabSlop = 8;
const QColor kTrack(0x3c, 0x3c, 0x3c);
const QColor kKept(0x8B, 0x5C, 0xF6);
const QColor kHandle(0xf2, 0xf2, 0xf2);
const QColor kPlayhead(0xff, 0xd1, 0x4a);
} // namespace

TrimTimeline::TrimTimeline(QWidget* parent)
  : QWidget(parent)
{
    setMinimumHeight(kHeight);
    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    setCursor(Qt::PointingHandCursor);
}

QSize TrimTimeline::sizeHint() const
{
    return { 480, kHeight };
}

void TrimTimeline::setDuration(double seconds)
{
    m_duration = seconds;
    m_range = clampTrim(m_range, m_duration);
    update();
}

void TrimTimeline::setRange(TrimRange range)
{
    m_range = clampTrim(range, m_duration);
    update();
}

TrimRange TrimTimeline::range() const
{
    return m_range;
}

void TrimTimeline::setPosition(double seconds)
{
    m_position = seconds;
    update();
}

QRect TrimTimeline::track() const
{
    // Room for half a handle at each end
    return rect().adjusted(kHandleWidth / 2, 4, -kHandleWidth / 2, -4);
}

int TrimTimeline::xFor(double seconds) const
{
    const QRect t = track();
    return t.left() + xAtTime(seconds, t.width(), m_duration);
}

double TrimTimeline::timeFor(int x) const
{
    const QRect t = track();
    return timeAtX(x - t.left(), t.width(), m_duration);
}

void TrimTimeline::paintEvent(QPaintEvent*)
{
    QPainter p(this);
    p.setRenderHint(QPainter::Antialiasing);
    const QRect t = track();
    p.setPen(Qt::NoPen);
    p.setBrush(kTrack);
    p.drawRoundedRect(t, 4, 4);

    const int x0 = xFor(m_range.start);
    const int x1 = xFor(m_range.end);
    QColor kept = kKept;
    kept.setAlpha(150);
    p.setBrush(kept);
    p.drawRect(QRect(QPoint(x0, t.top()), QPoint(x1, t.bottom())));

    p.setBrush(kHandle);
    for (int x : { x0, x1 }) {
        p.drawRoundedRect(
          QRect(x - kHandleWidth / 2, t.top() - 2, kHandleWidth, t.height() + 4),
          3,
          3);
    }

    p.setPen(QPen(kPlayhead, 2));
    const int px = xFor(m_position);
    p.drawLine(px, 0, px, height());
}

void TrimTimeline::mousePressEvent(QMouseEvent* event)
{
    if (event->button() != Qt::LeftButton || m_duration <= 0.0) {
        return;
    }
    const int x = event->position().toPoint().x();
    const int toStart = std::abs(x - xFor(m_range.start));
    const int toEnd = std::abs(x - xFor(m_range.end));
    if (toStart <= kGrabSlop && toStart <= toEnd) {
        m_drag = Drag::Start;
    } else if (toEnd <= kGrabSlop) {
        m_drag = Drag::End;
    } else {
        m_drag = Drag::Playhead;
    }
    dragTo(x);
}

void TrimTimeline::mouseMoveEvent(QMouseEvent* event)
{
    if (m_drag != Drag::None) {
        dragTo(event->position().toPoint().x());
    }
}

void TrimTimeline::mouseReleaseEvent(QMouseEvent*)
{
    m_drag = Drag::None;
}

void TrimTimeline::dragTo(int x)
{
    const double t = timeFor(x);
    switch (m_drag) {
        case Drag::Start:
            m_range = moveTrimStart(m_range, t, m_duration);
            emit rangeChanged(true);
            break;
        case Drag::End:
            m_range = moveTrimEnd(m_range, t, m_duration);
            emit rangeChanged(false);
            break;
        case Drag::Playhead:
            emit seekRequested(t);
            break;
        case Drag::None:
            return;
    }
    update();
}
