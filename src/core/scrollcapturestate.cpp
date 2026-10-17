// SPDX-License-Identifier: GPL-3.0-or-later

#include "scrollcapturestate.h"

bool canTransition(ScrollState from, ScrollState to)
{
    switch (from) {
        case ScrollState::Idle:
            return to == ScrollState::Picking;
        case ScrollState::Picking:
            return to == ScrollState::Scrolling || to == ScrollState::Idle ||
                   to == ScrollState::Failed;
        case ScrollState::Scrolling:
            return to == ScrollState::Done || to == ScrollState::Cancelled ||
                   to == ScrollState::Failed;
        case ScrollState::Done:
        case ScrollState::Cancelled:
        case ScrollState::Failed:
            return to == ScrollState::Idle;
    }
    return false;
}

ScrollHotkeyEffect onScrollHotkey(ScrollState state)
{
    switch (state) {
        case ScrollState::Idle:
            return ScrollHotkeyEffect::StartPicking;
        case ScrollState::Scrolling:
            return ScrollHotkeyEffect::Finish;
        default:
            return ScrollHotkeyEffect::Ignore;
    }
}

ScrollState ScrollCaptureMachine::state() const
{
    return m_state;
}

ScrollMode ScrollCaptureMachine::mode() const
{
    return m_mode;
}

bool ScrollCaptureMachine::transition(ScrollState to, const QString& error)
{
    if (!canTransition(m_state, to)) {
        return false;
    }
    m_state = to;
    if (to == ScrollState::Failed) {
        m_error = error;
    } else if (to == ScrollState::Picking) {
        m_error.clear();
    } else if (to == ScrollState::Scrolling) {
        m_mode = ScrollMode::Manual;
    }
    return true;
}

bool ScrollCaptureMachine::setMode(ScrollMode mode)
{
    if (m_state != ScrollState::Scrolling) {
        return false;
    }
    m_mode = mode;
    return true;
}

QString ScrollCaptureMachine::lastError() const
{
    return m_error;
}
