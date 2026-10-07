// SPDX-License-Identifier: GPL-3.0-or-later
// Platforms without native recording: everything reports "unsupported".

#include "gifencoder.h"
#include "screenrecorder.h"
#include "videothumbnail.h"

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
