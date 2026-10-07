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

// Hotkey names that start a screen recording
CaptureRequest::RecordMode recordModeForHotkey(const QString& hotkey);

// Request that only picks the area to record: ACCEPT_ON_SELECT, no workflow
// and no save path, so nothing is exported as an image.
CaptureRequest buildRecordRequest(CaptureRequest::RecordMode mode);

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
