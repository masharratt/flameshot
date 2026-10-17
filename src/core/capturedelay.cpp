// SPDX-License-Identifier: GPL-3.0-or-later

#include "capturedelay.h"

#include <algorithm>

unsigned int captureDelayMs(unsigned int requestDelayMs, int configSeconds)
{
    if (requestDelayMs > 0) {
        return requestDelayMs;
    }
    return static_cast<unsigned int>(
             std::clamp(configSeconds, 0, kMaxCaptureDelaySec)) *
           1000u;
}

int countdownSecondsLeft(int remainingMs)
{
    return remainingMs <= 0 ? 0 : (remainingMs + 999) / 1000;
}

namespace {

bool canTransition(CountdownState from, CountdownState to)
{
    switch (from) {
        case CountdownState::Idle:
            return to == CountdownState::Counting;
        case CountdownState::Counting:
            return to == CountdownState::Capturing ||
                   to == CountdownState::Cancelled;
        case CountdownState::Capturing:
        case CountdownState::Cancelled:
            return to == CountdownState::Idle;
    }
    return false;
}

} // namespace

CountdownState CountdownStateMachine::state() const
{
    return m_state;
}

bool CountdownStateMachine::transition(CountdownState to)
{
    if (!canTransition(m_state, to)) {
        return false;
    }
    m_state = to;
    return true;
}
