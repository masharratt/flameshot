// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <QList>
#include <QSize>

struct GifPlan
{
    QSize size;               // output size in pixels, both even
    QList<double> frameTimes; // seconds into the video, first is 0
    double frameDelay = 0.0;  // seconds each frame is shown
};

// Plans the MP4 to GIF conversion: the output is at most maxWidth wide with
// the source aspect ratio, and frames are sampled every 1/fps seconds.
// fps is clamped to 1..50.
GifPlan planGif(double durationSec, QSize sourcePx, int fps, int maxWidth);
