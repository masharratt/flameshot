// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <QColor>
#include <QList>

// Maximum swatches shown in the editor settings bar
constexpr int kMaxSwatches = 16;

// The colours to show as swatches: invalid and fully transparent entries are
// dropped, duplicates (same RGBA) keep their first position, and the result
// holds at most maxCount colours. A maxCount of zero or less gives an empty
// list.
QList<QColor> swatchColors(const QList<QColor>& userColors, int maxCount);

// value limited to [min, max]
int clampThickness(int value, int min, int max);
