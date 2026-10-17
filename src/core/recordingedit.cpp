// SPDX-License-Identifier: GPL-3.0-or-later

#include "recordingedit.h"

#include <QtGlobal>
#include <algorithm>
#include <cmath>

namespace {

// Slack for floating point comparisons of times, in seconds
constexpr double kEpsilon = 1e-6;
// A range within this of both ends counts as the whole video
constexpr double kFullSlack = 1e-3;

} // namespace

TrimRange fullTrim(double duration)
{
    return { 0.0, std::max(0.0, duration) };
}

TrimRange clampTrim(TrimRange range, double duration)
{
    if (!(duration > 0.0)) {
        return {};
    }
    if (duration <= kMinTrimSeconds) {
        return fullTrim(duration);
    }
    double start = std::clamp(std::min(range.start, range.end), 0.0, duration);
    double end = std::clamp(std::max(range.start, range.end), 0.0, duration);
    if (end - start < kMinTrimSeconds) {
        end = start + kMinTrimSeconds;
        if (end > duration) {
            end = duration;
            start = duration - kMinTrimSeconds;
        }
    }
    return { start, end };
}

bool isValidTrim(TrimRange range, double duration)
{
    if (!(duration > 0.0) || range.start < -kEpsilon ||
        range.end > duration + kEpsilon) {
        return false;
    }
    if (duration <= kMinTrimSeconds) {
        return isFullRange(range, duration);
    }
    return range.end - range.start >= kMinTrimSeconds - kEpsilon;
}

bool isFullRange(TrimRange range, double duration)
{
    return range.start <= kFullSlack && range.end >= duration - kFullSlack;
}

TrimRange moveTrimStart(TrimRange range, double start, double duration)
{
    const double latest = std::max(0.0, range.end - kMinTrimSeconds);
    range.start = std::clamp(start, 0.0, latest);
    return range;
}

TrimRange moveTrimEnd(TrimRange range, double end, double duration)
{
    const double earliest = std::min(duration, range.start + kMinTrimSeconds);
    range.end = std::clamp(end, earliest, std::max(earliest, duration));
    return range;
}

double timeAtX(int x, int width, double duration)
{
    if (width <= 0 || !(duration > 0.0)) {
        return 0.0;
    }
    return double(std::clamp(x, 0, width)) / width * duration;
}

int xAtTime(double seconds, int width, double duration)
{
    if (width <= 0 || !(duration > 0.0)) {
        return 0;
    }
    return std::clamp(qRound(seconds / duration * width), 0, width);
}

bool canEditTransition(EditState from, EditState to)
{
    switch (from) {
        case EditState::Editing:
            return to == EditState::Exporting || to == EditState::Closed;
        case EditState::Exporting:
            return to == EditState::Converting || to == EditState::Saved ||
                   to == EditState::Failed;
        case EditState::Converting:
            return to == EditState::Saved || to == EditState::Failed;
        case EditState::Failed:
            return to == EditState::Editing || to == EditState::Closed;
        case EditState::Saved:
            return to == EditState::Closed;
        case EditState::Closed:
            return false;
    }
    return false;
}

EditState EditStateMachine::state() const
{
    return m_state;
}

bool EditStateMachine::transition(EditState to, const QString& error)
{
    if (!canEditTransition(m_state, to)) {
        return false;
    }
    m_state = to;
    if (to == EditState::Failed) {
        m_error = error;
    } else if (to == EditState::Editing) {
        m_error.clear();
    }
    return true;
}

QString EditStateMachine::lastError() const
{
    return m_error;
}

bool EditStateMachine::isInteractive() const
{
    return m_state == EditState::Editing;
}
