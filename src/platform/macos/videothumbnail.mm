// SPDX-License-Identifier: GPL-3.0-or-later
// First frame of a video for the history grid. Compiled with ARC.

#define QT_NO_KEYWORDS

#include "platform/videothumbnail.h"

#import <AVFoundation/AVFoundation.h>
#import <CoreMedia/CoreMedia.h>
#import <CoreGraphics/CoreGraphics.h>
#import <Foundation/Foundation.h>

// This file relies on automatic reference counting. Without it, objects held
// across asynchronous callbacks are freed early and the app crashes.
#if !__has_feature(objc_arc)
#error "Compile this file with -fobjc-arc"
#endif


QImage videoFirstFrame(const QString& path, const QSize& maxPixels)
{
    @autoreleasepool {
        NSURL* url = [NSURL
          fileURLWithPath:[NSString stringWithUTF8String:path.toUtf8().constData()]];
        AVURLAsset* asset = [AVURLAsset URLAssetWithURL:url options:nil];
        AVAssetImageGenerator* generator =
          [AVAssetImageGenerator assetImageGeneratorWithAsset:asset];
        generator.appliesPreferredTrackTransform = YES;
        generator.maximumSize = CGSizeMake(maxPixels.width(), maxPixels.height());

        __block CGImageRef cg = NULL;
        dispatch_semaphore_t done = dispatch_semaphore_create(0);
        [generator generateCGImageAsynchronouslyForTime:kCMTimeZero
                                      completionHandler:^(CGImageRef image,
                                                          CMTime,
                                                          NSError*) {
                                          if (image != NULL) {
                                              cg = CGImageRetain(image);
                                          }
                                          dispatch_semaphore_signal(done);
                                      }];
        if (dispatch_semaphore_wait(
              done, dispatch_time(DISPATCH_TIME_NOW, 5 * NSEC_PER_SEC)) != 0 ||
            cg == NULL) {
            return QImage();
        }

        QImage img(static_cast<int>(CGImageGetWidth(cg)),
                   static_cast<int>(CGImageGetHeight(cg)),
                   QImage::Format_ARGB32_Premultiplied);
        img.fill(Qt::transparent);
        CGColorSpaceRef space = CGColorSpaceCreateDeviceRGB();
        CGContextRef ctx = CGBitmapContextCreate(
          img.bits(),
          img.width(),
          img.height(),
          8,
          img.bytesPerLine(),
          space,
          kCGImageAlphaPremultipliedFirst | kCGBitmapByteOrder32Host);
        CGColorSpaceRelease(space);
        if (ctx != NULL) {
            CGContextDrawImage(
              ctx, CGRectMake(0, 0, img.width(), img.height()), cg);
            CGContextRelease(ctx);
        }
        CGImageRelease(cg);
        return img;
    }
}
