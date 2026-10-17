// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

// Longest capture timer the setting allows
constexpr int kMaxCaptureDelaySec = 10;

// Delay before a GUI capture starts, in milliseconds. A delay carried by the
// request (the CLI --delay) wins over the configured timer; otherwise the
// configured seconds apply, clamped to 0..kMaxCaptureDelaySec.
unsigned int captureDelayMs(unsigned int requestDelayMs, int configSeconds);

// Whole seconds to show on the countdown for the time still remaining
int countdownSecondsLeft(int remainingMs);

// Lifecycle of one capture timer countdown
enum class CountdownState
{
    Idle,
    Counting,
    Capturing,
    Cancelled,
};

class CountdownStateMachine
{
public:
    CountdownState state() const;
    // Moves to `to` when legal and returns true. An illegal move changes
    // nothing.
    bool transition(CountdownState to);

private:
    CountdownState m_state = CountdownState::Idle;
};
