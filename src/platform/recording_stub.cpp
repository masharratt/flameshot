// SPDX-License-Identifier: GPL-3.0-or-later
// Platforms without native recording: everything reports "unsupported".

#include "gifencoder.h"
#include "screenrecorder.h"
#include "videoedit.h"
#include "videopreview.h"
#include "videothumbnail.h"

#include <QLabel>

namespace {

class UnsupportedRecorder : public ScreenRecorder
{
public:
    void start(const QRect&,
               const QString&,
               const QList<quint32>&,
               std::function<void(QString error)> onFailed,
               std::function<void()>) override
    {
        onFailed(QStringLiteral("Screen recording is not supported here."));
    }
    void stop(std::function<void(bool ok, QString path)> done) override
    {
        done(false, QString());
    }
    void cancel() override {}
};

class UnsupportedPreview : public VideoPreview
{
public:
    explicit UnsupportedPreview(QWidget* parent)
      : m_label(new QLabel(QStringLiteral("Preview is not available here."),
                           parent))
    {}
    QWidget* widget() override { return m_label; }
    void play() override {}
    void pause() override {}
    bool isPlaying() const override { return false; }
    void seek(double) override {}
    double position() const override { return 0.0; }

private:
    QLabel* m_label;
};

} // namespace

bool screenRecordingSupported()
{
    return false;
}

quint32 nativeWindowId(QWidget*)
{
    return 0;
}

std::unique_ptr<ScreenRecorder> createScreenRecorder()
{
    return std::make_unique<UnsupportedRecorder>();
}

void encodeGifAsync(const GifEncodeParams&,
                    std::function<void(bool ok, QString error)> done)
{
    done(false, QStringLiteral("GIF encoding is not supported here."));
}

QImage videoFirstFrame(const QString&, const QSize&)
{
    return QImage();
}

QImage videoFrameAt(const QString&, double, const QSize&)
{
    return QImage();
}

VideoInfo probeVideo(const QString&)
{
    return {};
}

bool exportVideoBlocking(const VideoExportParams&, QString* error)
{
    *error = QStringLiteral("Video export is not supported here.");
    return false;
}

void exportVideoAsync(const VideoExportParams&,
                      std::function<void(bool ok, QString error)> done)
{
    done(false, QStringLiteral("Video export is not supported here."));
}

std::unique_ptr<VideoPreview> createVideoPreview(const QString&, QWidget* parent)
{
    return std::make_unique<UnsupportedPreview>(parent);
}
