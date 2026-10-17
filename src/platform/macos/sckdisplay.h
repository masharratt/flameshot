// SPDX-License-Identifier: GPL-3.0-or-later
// ScreenCaptureKit display lookup shared by recording and scrolling capture.
// Objective-C++ only.

#pragma once

#include <QRect>

#import <ScreenCaptureKit/ScreenCaptureKit.h>

// Part of a global logical area that one display can capture
struct SCKDisplayArea
{
    SCDisplay* display = nil;
    // Area in the display's own points (origin at its top-left)
    QRect local;
    // Backing scale of that display (pixels per point)
    double scale = 1.0;
};

// The display holding most of the area; display is nil when none overlaps
API_AVAILABLE(macos(12.3))
SCKDisplayArea sckDisplayArea(SCShareableContent* content, CGRect globalLogical);

// The shareable windows whose ids are listed
API_AVAILABLE(macos(12.3))
NSArray<SCWindow*>* sckWindowsWithIds(SCShareableContent* content,
                                      NSArray<NSNumber*>* ids);
