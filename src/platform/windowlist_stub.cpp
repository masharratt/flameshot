// SPDX-License-Identifier: GPL-3.0-or-later
// Window enumeration is only implemented on macOS; elsewhere hover-to-window
// detection finds nothing and the overlay behaves as upstream.

#include "platform/windowlist.h"

QList<WindowInfo> onScreenWindows()
{
    return {};
}
