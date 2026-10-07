// SPDX-License-Identifier: GPL-3.0-or-later
// Pure logic for hover-to-window detection. No platform or widget code.

#pragma once

#include "platform/windowlist.h"

#include <QPoint>
#include <QRect>
#include <optional>

// Keep ordinary app windows: layer 0, visible, at least minSize in both
// dimensions (logical points) and not owned by ownPid. Order is preserved.
QList<WindowInfo> filterCandidates(const QList<WindowInfo>& windows,
                                   qint64 ownPid,
                                   int minSize = 40);

// First window containing globalPos in a front-to-back list.
std::optional<WindowInfo> windowAt(const QList<WindowInfo>& frontToBack,
                                   QPoint globalPos);

// Convert global logical bounds to overlay-local logical coordinates and
// clip to overlayRect. overlayTopLeft is the overlay's global top-left.
QRect toOverlayRect(const QRect& globalBounds,
                    const QPoint& overlayTopLeft,
                    const QRect& overlayRect);

enum class PressKind
{
    Click,
    Drag
};

// Manhattan distance above threshold (logical px) is a drag.
PressKind classifyPress(QPoint press, QPoint current, int threshold = 4);
