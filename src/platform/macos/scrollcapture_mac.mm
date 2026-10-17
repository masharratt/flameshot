// SPDX-License-Identifier: GPL-3.0-or-later
// Scrolling capture on macOS: ScreenCaptureKit still grabs of one area
// (macOS 14+, offered from 15 like recording), Accessibility check and
// CGEvent scroll-wheel posting. Compiled with ARC.

#define QT_NO_KEYWORDS

#include "platform/scrollcapture.h"
#include "platform/macos/cgimage.h"
#include "platform/macos/sckdisplay.h"

#include <QCoreApplication>
#include <QMetaObject>

#import <ApplicationServices/ApplicationServices.h>
#import <CoreGraphics/CoreGraphics.h>
#import <ScreenCaptureKit/ScreenCaptureKit.h>

#if !__has_feature(objc_arc)
#error "Compile this file with -fobjc-arc"
#endif

namespace {

// Qt owns the main thread; SCK calls back on its own queues
void postToMain(std::function<void()> fn)
{
    QMetaObject::invokeMethod(
      QCoreApplication::instance(), std::move(fn), Qt::QueuedConnection);
}

QString fromNSString(NSString* s)
{
    return s == nil ? QString() : QString::fromUtf8(s.UTF8String);
}

class MacAreaGrabber : public AreaGrabber
{
public:
    void prepare(const QRect& rect,
                 const QList<quint32>& excludeWindowIds,
                 std::function<void(QString error)> ready) override
    {
        if (@available(macos 15.0, *)) {
            NSMutableArray<NSNumber*>* ids = [NSMutableArray array];
            for (quint32 id : excludeWindowIds) {
                [ids addObject:@(id)];
            }
            const CGRect area =
              CGRectMake(rect.x(), rect.y(), rect.width(), rect.height());
            // The block keeps this state alive even if the grabber goes
            auto state = m_state;
            [SCShareableContent getShareableContentWithCompletionHandler:^(
                                  SCShareableContent* content, NSError* error) {
                if (content == nil) {
                    const QString message =
                      fromNSString(error.localizedDescription);
                    postToMain([ready, message]() {
                        ready(message.isEmpty()
                                ? QStringLiteral(
                                    "Screen Recording permission is needed.")
                                : message);
                    });
                    return;
                }
                const SCKDisplayArea where = sckDisplayArea(content, area);
                if (where.display == nil) {
                    postToMain([ready]() {
                        ready(QStringLiteral(
                          "The selected area is not on a capturable display."));
                    });
                    return;
                }
                SCContentFilter* filter = [[SCContentFilter alloc]
                     initWithDisplay:where.display
                    excludingWindows:sckWindowsWithIds(content, ids)];
                double scale = where.scale;
                if (scale <= 0) {
                    scale =
                      filter.pointPixelScale > 0 ? filter.pointPixelScale : 1.0;
                }
                SCStreamConfiguration* config =
                  [[SCStreamConfiguration alloc] init];
                config.sourceRect = CGRectMake(where.local.x(),
                                               where.local.y(),
                                               where.local.width(),
                                               where.local.height());
                config.width = static_cast<size_t>(
                  qMax(1, qRound(where.local.width() * scale)));
                config.height = static_cast<size_t>(
                  qMax(1, qRound(where.local.height() * scale)));
                // A pointer resting over the content would break matching
                config.showsCursor = NO;
                config.pixelFormat = kCVPixelFormatType_32BGRA;
                postToMain([state, filter, config, scale, ready]() {
                    state->filter = filter;
                    state->config = config;
                    state->scale = scale;
                    ready(QString());
                });
            }];
        } else {
            ready(QStringLiteral("Scrolling capture needs macOS 15 or newer."));
        }
    }

    void grab(std::function<void(QImage image)> done) override
    {
        if (@available(macos 15.0, *)) {
            SCContentFilter* filter = m_state->filter;
            SCStreamConfiguration* config = m_state->config;
            if (filter == nil || config == nil) {
                done(QImage());
                return;
            }
            const double scale = m_state->scale;
            [SCScreenshotManager
              captureImageWithFilter:filter
                       configuration:config
                   completionHandler:^(CGImageRef image, NSError*) {
                       // Converted here, off the main thread
                       QImage img = imageFromCGImage(image);
                       if (!img.isNull()) {
                           img.setDevicePixelRatio(scale);
                       }
                       postToMain([done, img]() { done(img); });
                   }];
        } else {
            done(QImage());
        }
    }

private:
    struct State
    {
        id filter = nil;
        id config = nil;
        double scale = 1.0;
    };
    std::shared_ptr<State> m_state = std::make_shared<State>();
};

} // namespace

std::unique_ptr<AreaGrabber> createAreaGrabber()
{
    return std::make_unique<MacAreaGrabber>();
}

bool scrollEventsAllowed(bool prompt)
{
    if (!prompt) {
        return AXIsProcessTrusted();
    }
    NSDictionary* options =
      @{ (__bridge NSString*)kAXTrustedCheckOptionPrompt : @YES };
    return AXIsProcessTrustedWithOptions((__bridge CFDictionaryRef)options);
}

bool postScrollDown(const QPoint& globalLogical,
                    const QRect& keepInside,
                    int points)
{
    const CGPoint target = CGPointMake(globalLogical.x(), globalLogical.y());
    // Wheel events go to the window under the pointer
    CGEventRef probe = CGEventCreate(NULL);
    if (probe != NULL) {
        const CGPoint now = CGEventGetLocation(probe);
        CFRelease(probe);
        if (!keepInside.contains(qRound(now.x), qRound(now.y))) {
            CGWarpMouseCursorPosition(target);
        }
    }
    // Negative wheel values scroll towards the end of the content
    CGEventRef event = CGEventCreateScrollWheelEvent(
      NULL, kCGScrollEventUnitPixel, 1, static_cast<int32_t>(-points));
    if (event == NULL) {
        return false;
    }
    CGEventSetLocation(event, target);
    CGEventPost(kCGHIDEventTap, event);
    CFRelease(event);
    return true;
}
