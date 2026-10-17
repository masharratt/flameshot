// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <QString>

#include <memory>

class QWidget;

// Plays a video file inside a Qt window (AVPlayer on macOS). Times are in
// seconds from the start of the video.
class VideoPreview
{
public:
    virtual ~VideoPreview() = default;

    // The widget showing the video, owned by the parent given at creation
    virtual QWidget* widget() = 0;
    virtual void play() = 0;
    virtual void pause() = 0;
    virtual bool isPlaying() const = 0;
    // Exact seek, so a trim handle shows the frame it points at
    virtual void seek(double seconds) = 0;
    virtual double position() const = 0;
};

// Never null. Where there is no native player the widget only says so.
std::unique_ptr<VideoPreview> createVideoPreview(const QString& path,
                                                 QWidget* parent);
