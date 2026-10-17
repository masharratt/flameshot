// SPDX-License-Identifier: GPL-3.0-or-later
// Trim and Beautify export for screen recordings (AVFoundation on macOS).

#pragma once

#include "core/recordingedit.h"
#include "utils/imageeffects.h"

#include <QImage>
#include <QSize>
#include <QString>

#include <functional>

struct VideoInfo
{
    bool ok = false;
    double duration = 0.0; // seconds
    QSize pixels;          // size of the video track
};

// Duration and size of a video file. ok = false when it cannot be read.
VideoInfo probeVideo(const QString& path);

struct VideoExportParams
{
    QString input;
    QString output; // MP4, replaced when it exists
    TrimRange range;
    // Composite every frame onto the Beautify canvas. The output size is
    // videoCanvasSize(beautifyLayout(source, effects, dpr).canvas); with no
    // background the area around the shot is black, as a video has no
    // transparency.
    bool beautify = false;
    EffectSettings effects;
    qreal dpr = 1.0;
};

// Writes the trimmed (and optionally beautified) MP4. Without beautify the
// frames are copied, not re-encoded. Blocks; call from a worker thread or
// a test.
bool exportVideoBlocking(const VideoExportParams& params, QString* error);

// exportVideoBlocking on a worker thread; done runs on the Qt main thread
void exportVideoAsync(const VideoExportParams& params,
                      std::function<void(bool ok, QString error)> done);
