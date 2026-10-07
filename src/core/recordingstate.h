// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <QString>

// Lifecycle of one screen recording
enum class RecState
{
    Idle,
    Selecting,
    Starting,
    Recording,
    Stopping,
    Converting,
    Done,
    Failed,
    Cancelled,
};

// True when the recording may move from one state to the other
bool canTransition(RecState from, RecState to);

enum class HotkeyEffect
{
    StartSelecting,
    Stop,
    Ignore,
};

// What pressing a record hotkey does in the given state
HotkeyEffect onHotkey(RecState state);

class RecordingStateMachine
{
public:
    RecState state() const;
    // Moves to `to` when legal and returns true. An illegal move changes
    // nothing. `error` is kept as lastError() when moving to Failed.
    bool transition(RecState to, const QString& error = QString());
    QString lastError() const;

private:
    RecState m_state = RecState::Idle;
    QString m_error;
};
