// SPDX-License-Identifier: GPL-3.0-or-later

#include "editorsettings.h"

#include <QSet>
#include <algorithm>

QList<QColor> swatchColors(const QList<QColor>& userColors, int maxCount)
{
    QList<QColor> out;
    QSet<QRgb> seen;
    for (const QColor& c : userColors) {
        if (out.size() >= maxCount) {
            break;
        }
        if (!c.isValid() || c.alpha() == 0 || seen.contains(c.rgba())) {
            continue;
        }
        seen.insert(c.rgba());
        out.append(c);
    }
    return out;
}

int clampThickness(int value, int min, int max)
{
    return std::clamp(value, min, max);
}
