// SPDX-License-Identifier: GPL-3.0-or-later
// Pure logic for the "Edit recording" window: the trim range and the
// window's lifecycle. No platform or widget code.

#pragma once

#include <QString>

// Shortest clip the trim handles allow, in seconds
inline constexpr double kMinTrimSeconds = 0.25;

// Part of a video to keep, in seconds from its start
struct TrimRange
{
    double start = 0.0;
    double end = 0.0;
};

// The whole video
TrimRange fullTrim(double duration);

// Puts the ends in order, clamps them to 0..duration and grows the range to
// kMinTrimSeconds (moving the end first, the start when the end is at the
// end of the video). A video shorter than the minimum keeps all of itself.
// duration <= 0 gives an empty range.
TrimRange clampTrim(TrimRange range, double duration);

// True when 0 <= start, end <= duration and the range is at least
// kMinTrimSeconds long (or the whole video when that is shorter)
bool isValidTrim(TrimRange range, double duration);

// True when the range keeps the whole video (within a millisecond)
bool isFullRange(TrimRange range, double duration);

// Dragging one handle: it stays inside the video and at least
// kMinTrimSeconds from the other handle, which does not move
TrimRange moveTrimStart(TrimRange range, double start, double duration);
TrimRange moveTrimEnd(TrimRange range, double end, double duration);

// Timeline mapping between a pixel column (0..width) and a time
double timeAtX(int x, int width, double duration);
int xAtTime(double seconds, int width, double duration);

// Lifecycle of one "Edit recording" window
enum class EditState
{
    Editing,    // preview and trim handles are live
    Exporting,  // trimmed (and beautified) MP4 is being written
    Converting, // GIF only: the exported MP4 is being turned into a GIF
    Saved,      // new file written; history entry and toast follow
    Failed,     // export or conversion failed; the user may retry
    Closed,     // window gone; an owned temp source has been removed
};

bool canEditTransition(EditState from, EditState to);

class EditStateMachine
{
public:
    EditState state() const;
    // Moves to `to` when legal and returns true. `error` is kept as
    // lastError() when moving to Failed and cleared on a retry.
    bool transition(EditState to, const QString& error = QString());
    QString lastError() const;
    // True while the user may change the trim, play or export
    bool isInteractive() const;

private:
    EditState m_state = EditState::Editing;
    QString m_error;
};
