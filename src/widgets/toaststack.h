// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <QList>
#include <QRect>
#include <QSize>

// Positions for `count` toasts stacked upward from the bottom-right corner of
// `available`. Index 0 is the newest toast and sits at the bottom.
QList<QRect> toastStackGeometry(const QRect& available,
                                const QSize& toastSize,
                                int count,
                                int margin,
                                int spacing);
