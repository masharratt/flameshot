// SPDX-License-Identifier: GPL-3.0-or-later

#include "rulertool.h"
#include "rulermath.h"

#include <QFont>
#include <QFontMetricsF>
#include <QPainter>
#include <cmath>

namespace {
const int TICK_BASE_HALF_LENGTH = 5;
const int LABEL_PADDING = 4;
const int LABEL_GAP = 2;
}

RulerTool::RulerTool(QObject* parent)
  : AbstractTwoPointTool(parent)
{
    m_supportsOrthogonalAdj = true;
    m_supportsDiagonalAdj = true;
}

QIcon RulerTool::icon(const QColor& background, bool inEditor) const
{
    Q_UNUSED(inEditor)
    return QIcon(iconPath(background) + "ruler.svg");
}

QString RulerTool::name() const
{
    return tr("Ruler");
}

CaptureTool::Type RulerTool::type() const
{
    return CaptureTool::TYPE_RULER;
}

QString RulerTool::description() const
{
    return tr("Set the Ruler as the paint tool");
}

CaptureTool* RulerTool::copy(QObject* parent)
{
    auto* tool = new RulerTool(parent);
    copyParams(this, tool);
    return tool;
}

int RulerTool::tickHalfLength() const
{
    return TICK_BASE_HALF_LENGTH + size();
}

// Label box placed beside the midpoint, on the upper side of the line, so it
// never covers the line itself. Uses logical coordinates and the default font
// so that boundingRect() and process() agree.
QRectF RulerTool::labelRect(const QString& text) const
{
    const QPointF a = points().first;
    const QPointF b = points().second;
    const QFontMetricsF fm{ QFont() };
    const double w = fm.horizontalAdvance(text) + 2 * LABEL_PADDING;
    const double h = fm.height() + 2 * LABEL_PADDING;

    const double len = std::hypot(b.x() - a.x(), b.y() - a.y());
    double nx = 0.0;
    double ny = -1.0;
    if (len > 0.0) {
        nx = -(b.y() - a.y()) / len;
        ny = (b.x() - a.x()) / len;
        if (ny > 0.0 || (ny == 0.0 && nx < 0.0)) {
            nx = -nx;
            ny = -ny;
        }
    }
    const double dist = tickHalfLength() + LABEL_GAP +
                        std::abs(nx) * w / 2.0 + std::abs(ny) * h / 2.0;
    const QPointF mid = (a + b) / 2.0;
    const QPointF center(mid.x() + nx * dist, mid.y() + ny * dist);
    return QRectF(center.x() - w / 2.0, center.y() - h / 2.0, w, h);
}

QRect RulerTool::boundingRect() const
{
    if (!isValid()) {
        return {};
    }
    const int tick = tickHalfLength();
    QRectF lineArea = QRectF(points().first, points().second).normalized();
    lineArea.adjust(-tick, -tick, tick, tick);
    // The preview during drawing has no pixmap yet, so use DPR 1 for sizing;
    // the label width differs by at most a few digits.
    const rulermath::Measurement m =
      rulermath::measure(points().first, points().second, 1.0);
    const QRectF label = labelRect(rulermath::labelText(m));
    return lineArea.united(label).toAlignedRect().adjusted(-2, -2, 2, 2);
}

void RulerTool::process(QPainter& painter, const QPixmap& pixmap)
{
    const QPointF a = points().first;
    const QPointF b = points().second;
    const rulermath::Measurement m =
      rulermath::measure(a, b, pixmap.devicePixelRatio());

    painter.save();
    painter.setRenderHint(QPainter::Antialiasing);
    painter.setPen(QPen(color(), size()));
    painter.drawLine(a, b);

    // Perpendicular end ticks.
    const double len = std::hypot(b.x() - a.x(), b.y() - a.y());
    if (len > 0.0) {
        const double nx = -(b.y() - a.y()) / len;
        const double ny = (b.x() - a.x()) / len;
        const double t = tickHalfLength();
        for (const QPointF& p : { a, b }) {
            painter.drawLine(QPointF(p.x() - nx * t, p.y() - ny * t),
                             QPointF(p.x() + nx * t, p.y() + ny * t));
        }
    }

    // Label with a readable background box.
    const QString text = rulermath::labelText(m);
    const QRectF box = labelRect(text);
    painter.setFont(QFont());
    painter.setPen(Qt::NoPen);
    painter.setBrush(QColor(0, 0, 0, 200));
    painter.drawRoundedRect(box, 3, 3);
    painter.setPen(Qt::white);
    painter.drawText(box, Qt::AlignCenter, text);
    painter.restore();
}

void RulerTool::pressed(CaptureContext& context)
{
    Q_UNUSED(context)
}
