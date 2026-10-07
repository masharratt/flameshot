// SPDX-License-Identifier: GPL-3.0-or-later
// Platform window enumeration used by hover-to-window detection.

#pragma once

#include <QList>
#include <QRect>
#include <QString>
#include <QtGlobal>

// Bounds are in global logical points (top-left origin, same space as
// QWidget::mapToGlobal), not device pixels.
struct WindowInfo
{
    QRect bounds;
    int layer = 0;
    double alpha = 1.0;
    qint64 pid = 0;
    QString owner;
    quint32 id = 0;
};

// All on-screen windows ordered front to back. Empty on unsupported
// platforms.
QList<WindowInfo> onScreenWindows();
