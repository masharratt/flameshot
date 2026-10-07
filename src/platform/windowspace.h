// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

class QWidget;

// Makes a top-level window open on the Space the user is in, including over
// another app's full-screen Space, and brings Flameshot forward. Call it
// right before show(). No-op outside macOS.
void showOnActiveSpace(QWidget* window);
