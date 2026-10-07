// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <QRect>
#include <QSize>

// Layout of the windowed editor. Every rect and size here is in logical
// points. The canvas is the image placed at `margin` on every side. The tool
// buttons live in a fixed strip above the canvas, not in the margin.
struct EditCanvas
{
    QSize canvasLogical;
    QRect imageRectLogical;
    QSize windowSize;
};

// imagePx is in device pixels and dpr is the image's own device pixel ratio
// (values below 1 are treated as 1). Image size in points is imagePx / dpr,
// rounded up so an odd pixel count never loses its last pixel. The window is
// the canvas plus chromeHeight (the fixed toolbar strip) clamped to 90% of
// availableLogical; the canvas is never clamped.
EditCanvas planEditCanvas(QSize imagePx,
                          qreal dpr,
                          int margin,
                          QRect availableLogical,
                          int chromeHeight = 0);

// Number of rows needed to lay out `count` square buttons of `buttonSize`
// separated by `spacing` in a strip `width` wide. At least one button is
// placed per row, so a tiny width never yields zero columns. 0 for no buttons.
int toolbarRows(int count, int buttonSize, int spacing, int width);

// Margin around the image in the windowed editor, in logical points
constexpr int kEditMargin = 8;
