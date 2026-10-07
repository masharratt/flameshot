// SPDX-License-Identifier: GPL-3.0-or-later

#include "pickerstyle.h"
#include "core/capturerequest.h"

#include <QPainter>
#include <QPen>

bool useShareXPicker(const CaptureRequest& req)
{
    if (!req.editImage().isNull()) {
        return false;
    }
    return req.captureFirst() || !req.workflow().isEmpty();
}

QColor pickerLineColor()
{
    return Qt::white;
}

QColor pickerShadowColor()
{
    return QColor(0, 0, 0, 160);
}

QColor pickerLabelBackground()
{
    return QColor(0, 0, 0, 170);
}

void drawPickerOutline(QPainter* painter, const QRect& rect)
{
    // Dotted line: white dashes over a dark solid line, so it reads on any
    // background. Inset by a pixel so it stays visible at the screen edges.
    painter->save();
    painter->setBrush(Qt::NoBrush);
    const QRect line = rect.adjusted(1, 1, -2, -2);
    painter->setPen(QPen(pickerShadowColor(), 2));
    painter->drawRect(line);
    QPen dashes(pickerLineColor(), 2);
    dashes.setDashPattern({ 3, 3 });
    painter->setPen(dashes);
    painter->drawRect(line);
    painter->restore();
}
