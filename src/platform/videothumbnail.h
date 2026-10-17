// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <QImage>
#include <QSize>
#include <QString>

// First frame of a video file, scaled to fit maxPixels. Null when the file
// cannot be read or this platform has no decoder hook.
QImage videoFirstFrame(const QString& path, const QSize& maxPixels);

// Frame at `seconds` into the video, scaled to fit maxPixels (an empty size
// keeps the full size). Null when the file or time cannot be read.
QImage videoFrameAt(const QString& path, double seconds, const QSize& maxPixels);
