// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <QList>
#include <QRect>
#include <QString>
#include <QtGlobal>

#include <functional>
#include <memory>

// True when this OS can record the screen (macOS 15 or newer). When false the
// recording hotkeys and menu items are not offered.
bool screenRecordingSupported();

// Native window id (CGWindowID on macOS) of a top-level widget's window, or 0
// when it has none yet. Used to leave the recording control out of the video.
class QWidget;
quint32 nativeWindowId(QWidget* widget);

// Records an area of one display to an MP4 file. All callbacks run on the Qt
// main thread.
class ScreenRecorder
{
public:
    virtual ~ScreenRecorder() = default;

    // globalLogicalRect is in global logical points. onStarted fires once
    // frames are being written, onFailed with a message if starting fails or
    // the recording breaks later. The file at tempPath is created by the
    // recorder.
    virtual void start(const QRect& globalLogicalRect,
                       const QString& tempPath,
                       const QList<quint32>& excludeWindowIds,
                       std::function<void(QString error)> onFailed,
                       std::function<void()> onStarted = {}) = 0;
    // Finishes the file. done gets ok = false when it could not be finalised.
    virtual void stop(std::function<void(bool ok, QString path)> done) = 0;
    // Stops and deletes the file.
    virtual void cancel() = 0;
};

std::unique_ptr<ScreenRecorder> createScreenRecorder();
