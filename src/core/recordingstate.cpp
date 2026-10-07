// SPDX-License-Identifier: GPL-3.0-or-later

#include "recordingstate.h"

bool canTransition(RecState from, RecState to)
{
    switch (from) {
        case RecState::Idle:
            return to == RecState::Selecting;
        case RecState::Selecting:
            return to == RecState::Starting || to == RecState::Cancelled ||
                   to == RecState::Idle;
        case RecState::Starting:
            return to == RecState::Recording || to == RecState::Cancelled ||
                   to == RecState::Failed;
        case RecState::Recording:
            return to == RecState::Stopping || to == RecState::Cancelled ||
                   to == RecState::Failed;
        case RecState::Stopping:
            return to == RecState::Converting || to == RecState::Done ||
                   to == RecState::Failed;
        case RecState::Converting:
            return to == RecState::Done || to == RecState::Failed;
        case RecState::Done:
        case RecState::Failed:
        case RecState::Cancelled:
            return to == RecState::Idle;
    }
    return false;
}

HotkeyEffect onHotkey(RecState state)
{
    switch (state) {
        case RecState::Idle:
            return HotkeyEffect::StartSelecting;
        case RecState::Recording:
            return HotkeyEffect::Stop;
        default:
            return HotkeyEffect::Ignore;
    }
}

RecState RecordingStateMachine::state() const
{
    return m_state;
}

bool RecordingStateMachine::transition(RecState to, const QString& error)
{
    if (!canTransition(m_state, to)) {
        return false;
    }
    m_state = to;
    if (to == RecState::Failed) {
        m_error = error;
    } else if (to == RecState::Selecting) {
        m_error.clear();
    }
    return true;
}

QString RecordingStateMachine::lastError() const
{
    return m_error;
}
