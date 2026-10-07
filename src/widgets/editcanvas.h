// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <QPoint>
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

// Result of growing (or, with a negative step, shrinking) the canvas by
// stepLogical points on every edge. The selection is the exported image area:
// its top-left stays put and it grows by twice the step, so the outer margin
// is unchanged and content shifts inward by the step.
struct CanvasExpansion
{
    QSize newCanvasPx;          // canvas size in device pixels
    QPoint imageOffsetPx;       // where existing content moves, device pixels
    QPoint objectShiftLogical;  // shift for annotations, logical points
    QRect newSelectionLogical;
    QRect newSelectionDevice;
};

// canvasPx and selectionDevice are device pixels, selectionLogical is points.
// dpr below 1 is treated as 1. A negative step is the exact inverse of the
// same positive step.
CanvasExpansion expandCanvas(QSize canvasPx,
                             QRect selectionLogical,
                             QRect selectionDevice,
                             int stepLogical,
                             qreal dpr);

// Whitespace added per edge by one click of the Expand canvas button
constexpr int kExpandStepLogical = 40;

// Margin around the image in the windowed editor, in logical points
constexpr int kEditMargin = 8;
