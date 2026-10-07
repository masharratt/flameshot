// SPDX-License-Identifier: GPL-3.0-or-later

#include "platform/windowspace.h"

#include <QWidget>

#import <AppKit/AppKit.h>

void showOnActiveSpace(QWidget* window)
{
    if (!window) {
        return;
    }
    // winId() creates the native window if it does not exist yet
    auto* view = reinterpret_cast<NSView*>(window->winId());
    NSWindow* nsWindow = view.window;
    if (nsWindow) {
        nsWindow.collectionBehavior |=
          NSWindowCollectionBehaviorMoveToActiveSpace |
          NSWindowCollectionBehaviorFullScreenAuxiliary;
    }
    if (@available(macOS 14.0, *)) {
        [NSApp activate];
    } else {
        [NSApp activateIgnoringOtherApps:YES];
    }
}
