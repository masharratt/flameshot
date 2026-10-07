// SPDX-License-Identifier: GPL-3.0-or-later

#include "platform/windowpick.h"

QList<WindowInfo> filterCandidates(const QList<WindowInfo>& windows,
                                   qint64 ownPid,
                                   int minSize)
{
    QList<WindowInfo> out;
    for (const WindowInfo& w : windows) {
        if (w.layer != 0 || w.alpha <= 0.0 || w.pid == ownPid ||
            w.bounds.width() < minSize || w.bounds.height() < minSize) {
            continue;
        }
        out.append(w);
    }
    return out;
}

std::optional<WindowInfo> windowAt(const QList<WindowInfo>& frontToBack,
                                   QPoint globalPos)
{
    for (const WindowInfo& w : frontToBack) {
        if (w.bounds.contains(globalPos)) {
            return w;
        }
    }
    return std::nullopt;
}

QRect toOverlayRect(const QRect& globalBounds,
                    const QPoint& overlayTopLeft,
                    const QRect& overlayRect)
{
    return globalBounds.translated(-overlayTopLeft).intersected(overlayRect);
}

PressKind classifyPress(QPoint press, QPoint current, int threshold)
{
    return (current - press).manhattanLength() > threshold ? PressKind::Drag
                                                           : PressKind::Click;
}
