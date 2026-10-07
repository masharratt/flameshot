// SPDX-License-Identifier: GPL-3.0-or-later

#include "editcanvas.h"

#include <QtMath>
#include <algorithm>

EditCanvas planEditCanvas(QSize imagePx,
                          qreal dpr,
                          int margin,
                          QRect availableLogical,
                          int chromeHeight)
{
    if (dpr < 1.0) {
        dpr = 1.0;
    }
    const QSize imageLogical(qCeil(imagePx.width() / dpr),
                             qCeil(imagePx.height() / dpr));
    EditCanvas plan;
    plan.imageRectLogical = QRect(QPoint(margin, margin), imageLogical);
    plan.canvasLogical = imageLogical + QSize(2 * margin, 2 * margin);
    plan.windowSize =
      QSize(std::min(plan.canvasLogical.width(),
                     availableLogical.width() * 9 / 10),
            std::min(plan.canvasLogical.height() + chromeHeight,
                     availableLogical.height() * 9 / 10));
    return plan;
}

int toolbarRows(int count, int buttonSize, int spacing, int width)
{
    if (count <= 0) {
        return 0;
    }
    const int cell = std::max(1, buttonSize + spacing);
    const int perRow = std::max(1, (width + spacing) / cell);
    return (count + perRow - 1) / perRow;
}

CanvasExpansion expandCanvas(QSize canvasPx,
                             QRect selectionLogical,
                             QRect selectionDevice,
                             int stepLogical,
                             qreal dpr)
{
    if (dpr < 1.0) {
        dpr = 1.0;
    }
    const int stepDevice = qRound(stepLogical * dpr);
    CanvasExpansion e;
    e.newCanvasPx = canvasPx + QSize(2 * stepDevice, 2 * stepDevice);
    e.imageOffsetPx = QPoint(stepDevice, stepDevice);
    e.objectShiftLogical = QPoint(stepLogical, stepLogical);
    e.newSelectionLogical =
      QRect(selectionLogical.topLeft(),
            selectionLogical.size() + QSize(2 * stepLogical, 2 * stepLogical));
    e.newSelectionDevice =
      QRect(selectionDevice.topLeft(),
            selectionDevice.size() + QSize(2 * stepDevice, 2 * stepDevice));
    return e;
}
