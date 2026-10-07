// SPDX-License-Identifier: GPL-3.0-or-later

#include "toaststack.h"

QList<QRect> toastStackGeometry(const QRect& available,
                                const QSize& toastSize,
                                int count,
                                int margin,
                                int spacing)
{
    QList<QRect> result;
    const int x = available.x() + available.width() - margin - toastSize.width();
    int y = available.y() + available.height() - margin - toastSize.height();
    for (int i = 0; i < count; ++i) {
        result.append(QRect(QPoint(x, y), toastSize));
        y -= toastSize.height() + spacing;
    }
    return result;
}
