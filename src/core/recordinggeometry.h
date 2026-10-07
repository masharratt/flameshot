// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <QPoint>
#include <QRect>
#include <QSize>

// Rect in global logical points translated to the coordinates of the display
// that contains it (origin at that display's top-left).
QRect displayLocalRect(const QRect& globalLogical,
                       const QRect& displayBoundsGlobal);

// Pixel size of a logical size at the given backing scale, each side rounded
// to an even number (video encoders need even sizes) and at least 2.
QSize outputPixels(const QSize& logical, double scale);

// Top-left for the floating recording control: centred below the recorded
// area, else above it, else inside its bottom edge. Always clamped to
// screenAvail.
QPoint controlPosition(const QRect& recorded,
                       const QSize& control,
                       const QRect& screenAvail,
                       int gap);
