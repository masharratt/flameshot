// SPDX-License-Identifier: GPL-3.0-or-later

#include "platform/macos/cgimage.h"

QImage imageFromCGImage(CGImageRef cg)
{
    if (cg == NULL) {
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
        CGContextDrawImage(ctx, CGRectMake(0, 0, img.width(), img.height()), cg);
        CGContextRelease(ctx);
    }
    return img;
}
