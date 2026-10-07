// SPDX-License-Identifier: GPL-3.0-or-later

#include "gifplan.h"

#include <QtGlobal>
#include <cmath>

namespace {

int evenAtLeastTwo(double v)
{
    return qMax(2, 2 * qRound(v / 2.0));
}

} // namespace

GifPlan planGif(double durationSec, QSize sourcePx, int fps, int maxWidth)
{
    GifPlan plan;
    fps = qBound(1, fps, 50);
    plan.frameDelay = 1.0 / fps;

    const int srcW = qMax(1, sourcePx.width());
    const int srcH = qMax(1, sourcePx.height());
    // Round the width down so it never exceeds maxWidth
    const int width = qMax(2, 2 * (qMin(srcW, qMax(2, maxWidth)) / 2));
    plan.size = QSize(width, evenAtLeastTwo(double(width) * srcH / srcW));

    // Frames at i/fps for every i whose time is still inside the video
    int count = 1;
    if (durationSec > 0.0) {
        count = qMax(1, static_cast<int>(std::ceil(durationSec * fps - 1e-9)));
    }
    plan.frameTimes.reserve(count);
    for (int i = 0; i < count; ++i) {
        plan.frameTimes.append(static_cast<double>(i) / fps);
    }
    return plan;
}
