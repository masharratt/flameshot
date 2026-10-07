// SPDX-License-Identifier: GPL-3.0-or-later

#include "recordinggeometry.h"

#include <QtGlobal>

QRect displayLocalRect(const QRect& globalLogical,
                       const QRect& displayBoundsGlobal)
{
    return globalLogical.translated(-displayBoundsGlobal.topLeft());
}

QSize outputPixels(const QSize& logical, double scale)
{
    const auto even = [scale](int logicalSide) {
        return qMax(2, 2 * qRound(logicalSide * scale / 2.0));
    };
    return QSize(even(logical.width()), even(logical.height()));
}

QPoint controlPosition(const QRect& recorded,
                       const QSize& control,
                       const QRect& screenAvail,
                       int gap)
{
    const int below = recorded.bottom() + 1 + gap;
    const int above = recorded.top() - gap - control.height();
    int y;
    if (below + control.height() <= screenAvail.bottom() + 1) {
        y = below;
    } else if (above >= screenAvail.top()) {
        y = above;
    } else {
        y = recorded.bottom() + 1 - gap - control.height();
    }
    // QRect::center() rounds down a pixel for even widths; centre exactly
    int x = recorded.x() + (recorded.width() - control.width()) / 2;

    // Clamp; when the control is larger than the screen the top-left wins
    x = qMin(x, screenAvail.right() + 1 - control.width());
    x = qMax(x, screenAvail.left());
    y = qMin(y, screenAvail.bottom() + 1 - control.height());
    y = qMax(y, screenAvail.top());
    return QPoint(x, y);
}
