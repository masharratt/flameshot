// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include "core/capturerequest.h"

#include <QPoint>
#include <QRect>
#include <QString>

// Request used by the hotkeys and tray in capture-first mode: pick a region,
// then run the actions configured for `workflow` (a hotkey name such as
// "TAKE_SCREENSHOT") with no further clicks. The request carries only
// ACCEPT_ON_SELECT; saving and copying are done by the workflow's actions.
CaptureRequest buildCaptureFirstRequest(const QString& workflow,
                                        const QString& savePath,
                                        const QString& picturesFallback);

// Native macOS fullscreen animates and slows rapid captures, so capture-first
// requests always use a plain overlay window.
bool shouldUseNativeFullscreen(const CaptureRequest& req,
                               bool nativeFullscreenEnabled);

// Whether Flameshot::gui() should preset the last used region. A request that
// accepts on select must never get a preset, or it would fire on open.
bool shouldApplyLastRegion(const CaptureRequest& req,
                           bool saveLastRegionEnabled);

// Selection in global logical points, from the selection in overlay-local
// logical points and the overlay window's top-left.
QRect globalSelectionRect(const QRect& localLogical, const QPoint& overlayTopLeft);

// Geometry needed to reopen the editor on a saved capture. localLogical is
// where to draw the capture on a screen-sized backdrop; initialSelectionDevice
// is the CaptureRequest initial selection, which is in device pixels.
struct EditGeometry
{
    QRect localLogical;
    QRect initialSelectionDevice;
};
EditGeometry editGeometry(const QRect& globalLogical,
                          const QRect& screenGeometry,
                          qreal devicePixelRatio);

// True when an image of imagePixels shown in the editor at logicalSize on a
// screen with devicePixelRatio maps one to one onto its pixels, so saving the
// edit may overwrite the original file without resampling it.
bool editKeepsOriginalPixels(const QSize& imagePixels,
                             const QSize& logicalSize,
                             qreal devicePixelRatio);
