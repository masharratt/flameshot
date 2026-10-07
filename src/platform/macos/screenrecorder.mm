// SPDX-License-Identifier: GPL-3.0-or-later
// Region screen recording with ScreenCaptureKit (macOS 15+). Compiled with
// ARC. SCRecordingOutput writes the MP4 itself, so no sample buffer handling
// is needed here.

#define QT_NO_KEYWORDS

#include "platform/screenrecorder.h"
#include "core/recordinggeometry.h"

#include <QCoreApplication>
#include <QMetaObject>
#include <QWidget>

#import <AppKit/AppKit.h>
#import <AVFoundation/AVFoundation.h>
#import <CoreMedia/CoreMedia.h>
#import <CoreVideo/CoreVideo.h>
#import <ScreenCaptureKit/ScreenCaptureKit.h>

// This file relies on automatic reference counting. Without it, objects held
// across asynchronous callbacks are freed early and the app crashes.
#if !__has_feature(objc_arc)
#error "Compile this file with -fobjc-arc"
#endif


namespace {

NSString* toNSString(const QString& s)
{
    return [NSString stringWithUTF8String:s.toUtf8().constData()];
}

QString fromNSString(NSString* s)
{
    return s == nil ? QString() : QString::fromUtf8(s.UTF8String);
}

// Qt owns the main thread; SCK calls back on its own queues
void postToMain(std::function<void()> fn)
{
    QMetaObject::invokeMethod(
      QCoreApplication::instance(), std::move(fn), Qt::QueuedConnection);
}

} // namespace

API_AVAILABLE(macos(15.0))
@interface FSRecorderImpl : NSObject <SCStreamDelegate, SCRecordingOutputDelegate>
- (void)startWithRect:(CGRect)globalLogical
             tempPath:(NSString*)tempPath
           excludeIds:(NSArray<NSNumber*>*)excludeIds
            onStarted:(void (^)(void))onStarted
             onFailed:(void (^)(NSString* error))onFailed;
- (void)stopWithDone:(void (^)(BOOL ok, NSString* path))done;
- (void)cancel;
@end

API_AVAILABLE(macos(15.0))
@implementation FSRecorderImpl {
    SCStream* _stream;
    SCRecordingOutput* _output;
    NSString* _tempPath;
    void (^_onFailed)(NSString*);
    void (^_stopDone)(BOOL, NSString*);
    BOOL _cancelled;
    BOOL _stopping;
    BOOL _stopReported;
    BOOL _failureReported;
}

- (void)reportFailure:(NSString*)message
{
    @synchronized(self) {
        if (_failureReported || _cancelled || _stopReported) {
            return;
        }
        _failureReported = YES;
    }
    if (_stopDone != nil) {
        [self reportStopped:NO];
        return;
    }
    if (_onFailed != nil) {
        _onFailed(message);
    }
}

- (void)reportStopped:(BOOL)ok
{
    void (^done)(BOOL, NSString*) = nil;
    @synchronized(self) {
        if (_stopReported) {
            return;
        }
        _stopReported = YES;
        done = _stopDone;
    }
    if (done != nil) {
        done(ok, _tempPath);
    }
    // Break the delegate cycle (stream and output hold self)
    _stream = nil;
    _output = nil;
}

- (void)startWithRect:(CGRect)globalLogical
             tempPath:(NSString*)tempPath
           excludeIds:(NSArray<NSNumber*>*)excludeIds
            onStarted:(void (^)(void))onStarted
             onFailed:(void (^)(NSString* error))onFailed
{
    _tempPath = tempPath;
    _onFailed = onFailed;
    [SCShareableContent
      getShareableContentWithCompletionHandler:^(SCShareableContent* content,
                                                 NSError* error) {
          if (_cancelled) {
              return;
          }
          if (content == nil) {
              onFailed(error.localizedDescription ?: @"Screen Recording "
                                                      "permission is needed.");
              return;
          }

          // The display holding most of the area
          SCDisplay* display = nil;
          CGFloat bestArea = 0;
          for (SCDisplay* d in content.displays) {
              CGRect inter = CGRectIntersection(d.frame, globalLogical);
              CGFloat area = CGRectIsNull(inter)
                               ? 0
                               : inter.size.width * inter.size.height;
              if (area > bestArea) {
                  bestArea = area;
                  display = d;
              }
          }
          if (display == nil) {
              onFailed(@"The selected area is not on a recordable display.");
              return;
          }

          const CGRect area = CGRectIntersection(display.frame, globalLogical);
          const QRect local = displayLocalRect(
            QRect(qRound(area.origin.x),
                  qRound(area.origin.y),
                  qRound(area.size.width),
                  qRound(area.size.height)),
            QRect(qRound(display.frame.origin.x),
                  qRound(display.frame.origin.y),
                  qRound(display.frame.size.width),
                  qRound(display.frame.size.height)));

          double scale = 0;
          for (NSScreen* screen in NSScreen.screens) {
              NSNumber* number = screen.deviceDescription[@"NSScreenNumber"];
              if (number.unsignedIntValue == display.displayID) {
                  scale = screen.backingScaleFactor;
                  break;
              }
          }

          NSMutableArray<SCWindow*>* excluded = [NSMutableArray array];
          for (SCWindow* w in content.windows) {
              if ([excludeIds containsObject:@(w.windowID)]) {
                  [excluded addObject:w];
              }
          }
          SCContentFilter* filter =
            [[SCContentFilter alloc] initWithDisplay:display
                                    excludingWindows:excluded];
          if (scale <= 0) {
              scale = filter.pointPixelScale > 0 ? filter.pointPixelScale : 1.0;
          }
          const QSize pixels = outputPixels(local.size(), scale);

          SCStreamConfiguration* config = [[SCStreamConfiguration alloc] init];
          config.sourceRect = CGRectMake(
            local.x(), local.y(), local.width(), local.height());
          config.width = static_cast<size_t>(pixels.width());
          config.height = static_cast<size_t>(pixels.height());
          config.minimumFrameInterval = CMTimeMake(1, 30);
          config.showsCursor = YES;
          config.pixelFormat = kCVPixelFormatType_32BGRA;

          SCRecordingOutputConfiguration* outConfig =
            [[SCRecordingOutputConfiguration alloc] init];
          outConfig.outputURL = [NSURL fileURLWithPath:_tempPath];
          outConfig.outputFileType = AVFileTypeMPEG4;
          outConfig.videoCodecType = AVVideoCodecTypeH264;

          _stream = [[SCStream alloc] initWithFilter:filter
                                       configuration:config
                                            delegate:self];
          _output = [[SCRecordingOutput alloc] initWithConfiguration:outConfig
                                                            delegate:self];
          NSError* addError = nil;
          if (![_stream addRecordingOutput:_output error:&addError]) {
              onFailed(addError.localizedDescription ?: @"Could not set up "
                                                         "the recording.");
              return;
          }
          [_stream startCaptureWithCompletionHandler:^(NSError* startError) {
              if (startError != nil) {
                  onFailed(startError.localizedDescription);
              } else if (_cancelled) {
                  [self finishCancel];
              } else {
                  onStarted();
              }
          }];
      }];
}

- (void)stopWithDone:(void (^)(BOOL ok, NSString* path))done
{
    @synchronized(self) {
        if (_stopping || _stream == nil) {
            if (done != nil) {
                done(NO, _tempPath);
            }
            return;
        }
        _stopping = YES;
        _stopDone = done;
    }
    [_stream stopCaptureWithCompletionHandler:^(NSError* error) {
        if (error != nil) {
            [self reportFailure:error.localizedDescription];
        }
        // Success is reported by recordingOutputDidFinishRecording:
    }];
    // The file is final only after the delegate fires; do not wait forever
    dispatch_after(
      dispatch_time(DISPATCH_TIME_NOW, 10 * NSEC_PER_SEC),
      dispatch_get_global_queue(QOS_CLASS_UTILITY, 0),
      ^{
          [self reportFailure:@"The recording did not finish in time."];
      });
}

- (void)cancel
{
    _cancelled = YES;
    if (_stream != nil) {
        [self finishCancel];
    }
}

- (void)finishCancel
{
    SCStream* stream = _stream;
    NSString* path = _tempPath;
    _stream = nil;
    [stream stopCaptureWithCompletionHandler:^(NSError*) {
        // Give the writer a moment to release the file before removing it
        dispatch_after(
          dispatch_time(DISPATCH_TIME_NOW, 1 * NSEC_PER_SEC),
          dispatch_get_global_queue(QOS_CLASS_UTILITY, 0),
          ^{
              [[NSFileManager defaultManager] removeItemAtPath:path error:nil];
          });
    }];
}

#pragma mark SCRecordingOutputDelegate

- (void)recordingOutputDidFinishRecording:(SCRecordingOutput*)recordingOutput
{
    [self reportStopped:YES];
}

- (void)recordingOutput:(SCRecordingOutput*)recordingOutput
       didFailWithError:(NSError*)error
{
    [self reportFailure:error.localizedDescription];
}

#pragma mark SCStreamDelegate

- (void)stream:(SCStream*)stream didStopWithError:(NSError*)error
{
    [self reportFailure:error.localizedDescription];
}

@end

namespace {

class MacScreenRecorder : public ScreenRecorder
{
public:
    void start(const QRect& rect,
               const QString& tempPath,
               const QList<quint32>& excludeWindowIds,
               std::function<void(QString error)> onFailed,
               std::function<void()> onStarted) override
    {
        if (@available(macos 15.0, *)) {
            NSMutableArray<NSNumber*>* ids = [NSMutableArray array];
            for (quint32 id : excludeWindowIds) {
                [ids addObject:@(id)];
            }
            FSRecorderImpl* impl = [[FSRecorderImpl alloc] init];
            m_impl = impl;
            [impl startWithRect:CGRectMake(rect.x(),
                                           rect.y(),
                                           rect.width(),
                                           rect.height())
                       tempPath:toNSString(tempPath)
                     excludeIds:ids
                      onStarted:^{
                          postToMain([onStarted]() {
                              if (onStarted) {
                                  onStarted();
                              }
                          });
                      }
                       onFailed:^(NSString* error) {
                           const QString message = fromNSString(error);
                           postToMain([onFailed, message]() {
                               onFailed(message);
                           });
                       }];
        } else {
            onFailed(QStringLiteral("Screen recording needs macOS 15 or newer."));
        }
    }

    void stop(std::function<void(bool ok, QString path)> done) override
    {
        if (@available(macos 15.0, *)) {
            FSRecorderImpl* impl = m_impl;
            if (impl == nil) {
                done(false, QString());
                return;
            }
            // Keep the recorder alive until its callback has run
            [impl stopWithDone:^(BOOL ok, NSString* path) {
                const QString p = fromNSString(path);
                postToMain([done, ok, p, impl]() { done(ok, p); });
            }];
        } else {
            done(false, QString());
        }
    }

    void cancel() override
    {
        if (@available(macos 15.0, *)) {
            FSRecorderImpl* impl = m_impl;
            if (impl != nil) {
                [impl cancel];
            }
        }
    }

private:
    id m_impl = nil;
};

} // namespace

bool screenRecordingSupported()
{
    if (@available(macos 15.0, *)) {
        return true;
    }
    return false;
}

quint32 nativeWindowId(QWidget* widget)
{
    if (widget == nullptr) {
        return 0;
    }
    NSView* view = (__bridge NSView*)reinterpret_cast<void*>(widget->winId());
    return view.window == nil ? 0 : static_cast<quint32>(view.window.windowNumber);
}

std::unique_ptr<ScreenRecorder> createScreenRecorder()
{
    return std::make_unique<MacScreenRecorder>();
}
