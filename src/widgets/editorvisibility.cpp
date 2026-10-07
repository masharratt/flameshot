// SPDX-License-Identifier: GPL-3.0-or-later

#include "editorvisibility.h"

bool shouldHideEditors(Qt::ApplicationState state, bool modalOpen)
{
    return state == Qt::ApplicationInactive && !modalOpen;
}

bool editorsWantDockIcon(int openEditors, bool appActive, bool isDockApp)
{
    if (openEditors <= 0) {
        return false;
    }
    return isDockApp || !appActive;
}

bool shouldReshowEditors(Qt::ApplicationState state, bool haveHidden)
{
    return state == Qt::ApplicationActive && haveHidden;
}
