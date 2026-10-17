// SPDX-License-Identifier: GPL-3.0-or-later

#include "recordingoptions.h"

RecordingOptions recordingOptionsFor(bool showClicks)
{
    RecordingOptions options;
    options.showClicks = showClicks;
    return options;
}

bool clicksWillBeDrawn(const RecordingOptions& options, bool osSupportsClicks)
{
    return options.showClicks && osSupportsClicks;
}
