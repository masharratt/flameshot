// SPDX-License-Identifier: GPL-3.0-or-later
// Video preview for the "Edit recording" window: an AVPlayerLayer in a
// native view, hosted in Qt as a foreign window. Compiled with ARC.

#define QT_NO_KEYWORDS

#include "platform/videopreview.h"

#include <QWidget>
#include <QWindow>

#import <AVFoundation/AVFoundation.h>
#import <AppKit/AppKit.h>
#import <CoreMedia/CoreMedia.h>
#import <QuartzCore/QuartzCore.h>

#include <cmath>

// This file relies on automatic reference counting. Without it, objects held
// across asynchronous callbacks are freed early and the app crashes.
#if !__has_feature(objc_arc)
#error "Compile this file with -fobjc-arc"
#endif

namespace {

class MacVideoPreview : public VideoPreview
{
public:
    MacVideoPreview(const QString& path, QWidget* parent)
    {
        NSURL* url = [NSURL
          fileURLWithPath:[NSString
                            stringWithUTF8String:path.toUtf8().constData()]];
        m_player = [AVPlayer playerWithURL:url];
        m_player.actionAtItemEnd = AVPlayerActionAtItemEndPause;
        AVPlayerLayer* layer = [AVPlayerLayer playerLayerWithPlayer:m_player];
        layer.videoGravity = AVLayerVideoGravityResizeAspect;
        layer.backgroundColor = NSColor.blackColor.CGColor;
        // Layer-hosting view: the player layer follows the view's size
        m_view = [[NSView alloc] initWithFrame:NSMakeRect(0, 0, 480, 270)];
        m_view.layer = layer;
        m_view.wantsLayer = YES;

        QWindow* window = QWindow::fromWinId(
          reinterpret_cast<WId>((__bridge void*)m_view));
        m_widget = QWidget::createWindowContainer(window, parent);
    }

    ~MacVideoPreview() override { [m_player pause]; }

    QWidget* widget() override { return m_widget; }
    void play() override { [m_player play]; }
    void pause() override { [m_player pause]; }
    bool isPlaying() const override { return m_player.rate != 0.0f; }

    void seek(double seconds) override
    {
        [m_player seekToTime:CMTimeMakeWithSeconds(seconds, 600)
             toleranceBefore:kCMTimeZero
              toleranceAfter:kCMTimeZero];
    }

    double position() const override
    {
        const double t = CMTimeGetSeconds(m_player.currentTime);
        return std::isfinite(t) ? t : 0.0;
    }

private:
    AVPlayer* m_player = nil;
    NSView* m_view = nil;
    QWidget* m_widget = nullptr;
};

} // namespace

std::unique_ptr<VideoPreview> createVideoPreview(const QString& path,
                                                 QWidget* parent)
{
    return std::make_unique<MacVideoPreview>(path, parent);
}
