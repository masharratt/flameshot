// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <Qt>

// True when editor windows should be hidden: the app went inactive (the user
// switched to another app) and none of our modal dialogs is open.
bool shouldHideEditors(Qt::ApplicationState state, bool modalOpen);

// Whether open editor windows should give the app a Dock icon (and so a
// Cmd+Tab entry). Only switch while the app is inactive: switching while an
// editor is shown in another app's full-screen Space makes macOS jump to the
// desktop. Once switched, keep it while editors stay open.
bool editorsWantDockIcon(int openEditors, bool appActive, bool isDockApp);

// Whether hidden editors should come back: the user returned to the app
// (Cmd+Tab, Dock or menu bar) while some were hidden.
bool shouldReshowEditors(Qt::ApplicationState state, bool haveHidden);
