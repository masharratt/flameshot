// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

// What a recording starts with, built from config once per recording.
struct RecordingOptions
{
    // Draw a highlight where the mouse is clicked
    bool showClicks = true;
};

RecordingOptions recordingOptionsFor(bool showClicks);

// True when click highlights will appear: wanted and the OS can draw them
// (ScreenCaptureKit showMouseClicks, macOS 15+).
bool clicksWillBeDrawn(const RecordingOptions& options, bool osSupportsClicks);
