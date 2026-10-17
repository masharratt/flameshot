// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <QString>

// Lifecycle of one scrolling capture
enum class ScrollState
{
    Idle,
    Picking,
    Scrolling,
    Done,
    Cancelled,
    Failed,
};

// How the frames are produced while Scrolling
enum class ScrollMode
{
    Manual, // the user scrolls; frames are grabbed as they change
    Auto,   // synthetic scroll-wheel events drive the content
};

// True when the capture may move from one state to the other
bool canTransition(ScrollState from, ScrollState to);

enum class ScrollHotkeyEffect
{
    StartPicking,
    Finish,
    Ignore,
};

// What pressing the scrolling-capture hotkey does in the given state
ScrollHotkeyEffect onScrollHotkey(ScrollState state);

class ScrollCaptureMachine
{
public:
    ScrollState state() const;
    ScrollMode mode() const;
    // Moves to `to` when legal and returns true. An illegal move changes
    // nothing. Entering Scrolling starts in Manual mode; `error` is kept as
    // lastError() when moving to Failed.
    bool transition(ScrollState to, const QString& error = QString());
    // Mode changes are only legal while Scrolling
    bool setMode(ScrollMode mode);
    QString lastError() const;

private:
    ScrollState m_state = ScrollState::Idle;
    ScrollMode m_mode = ScrollMode::Manual;
    QString m_error;
};
