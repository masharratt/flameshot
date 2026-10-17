// SPDX-License-Identifier: GPL-3.0-or-later
// Text and QR recognition with Apple Vision. Compiled with ARC.

#define QT_NO_KEYWORDS

#include "platform/macos/textrecognition.h"

#import <CoreGraphics/CoreGraphics.h>
#import <Foundation/Foundation.h>
#import <Vision/Vision.h>

#include <QStringList>
#include <algorithm>
#include <cmath>
#include <vector>

#if !__has_feature(objc_arc)
#error "Compile this file with -fobjc-arc"
#endif

namespace {

struct TextLine
{
    QString text;
    CGRect box; // Vision units: 0..1, origin bottom-left
};

// Copies the pixels, so the CGImage does not depend on the QImage's lifetime
CGImageRef makeCGImage(const QImage& image)
{
    const QImage rgba = image.convertToFormat(QImage::Format_RGBA8888);
    CGColorSpaceRef space = CGColorSpaceCreateDeviceRGB();
    CGContextRef ctx = CGBitmapContextCreate(NULL,
                                             rgba.width(),
                                             rgba.height(),
                                             8,
                                             rgba.width() * 4,
                                             space,
                                             kCGImageAlphaPremultipliedLast |
                                               kCGBitmapByteOrder32Big);
    CGColorSpaceRelease(space);
    if (ctx == NULL) {
        return NULL;
    }
    uint8_t* dst = static_cast<uint8_t*>(CGBitmapContextGetData(ctx));
    const size_t dstStride = CGBitmapContextGetBytesPerRow(ctx);
    for (int y = 0; y < rgba.height(); ++y) {
        // Premultiply while copying, Vision expects a premultiplied context
        const uint8_t* src = rgba.constScanLine(y);
        uint8_t* out = dst + y * dstStride;
        for (int x = 0; x < rgba.width(); ++x) {
            const int a = src[x * 4 + 3];
            out[x * 4 + 0] = uint8_t(src[x * 4 + 0] * a / 255);
            out[x * 4 + 1] = uint8_t(src[x * 4 + 1] * a / 255);
            out[x * 4 + 2] = uint8_t(src[x * 4 + 2] * a / 255);
            out[x * 4 + 3] = uint8_t(a);
        }
    }
    CGImageRef cg = CGBitmapContextCreateImage(ctx);
    CGContextRelease(ctx);
    return cg;
}

// Top to bottom, then left to right. Pieces whose vertical centers are within
// half a line height of the row's first piece share a row, so a slightly
// uneven baseline does not scramble the order.
QStringList orderLines(std::vector<TextLine> lines)
{
    std::sort(lines.begin(), lines.end(), [](const TextLine& a, const TextLine& b) {
        return CGRectGetMidY(a.box) > CGRectGetMidY(b.box);
    });
    QStringList out;
    size_t i = 0;
    while (i < lines.size()) {
        const CGFloat refMid = CGRectGetMidY(lines[i].box);
        const CGFloat tolerance = lines[i].box.size.height * 0.5;
        std::vector<TextLine> row;
        while (i < lines.size() &&
               std::fabs(CGRectGetMidY(lines[i].box) - refMid) <= tolerance) {
            row.push_back(lines[i]);
            ++i;
        }
        std::sort(row.begin(), row.end(), [](const TextLine& a, const TextLine& b) {
            return CGRectGetMinX(a.box) < CGRectGetMinX(b.box);
        });
        QStringList parts;
        for (const TextLine& piece : row) {
            parts << piece.text;
        }
        out << parts.join(QLatin1Char(' '));
    }
    return out;
}

} // namespace

QString recognizeText(const QImage& image)
{
    if (image.isNull()) {
        return {};
    }
    @autoreleasepool {
        CGImageRef cg = makeCGImage(image);
        if (cg == NULL) {
            return {};
        }

        VNRecognizeTextRequest* textRequest = [[VNRecognizeTextRequest alloc] init];
        textRequest.recognitionLevel = VNRequestTextRecognitionLevelAccurate;
        textRequest.usesLanguageCorrection = YES;
        if (@available(macOS 13.0, *)) {
            textRequest.automaticallyDetectsLanguage = YES;
        }
        VNDetectBarcodesRequest* barcodeRequest =
          [[VNDetectBarcodesRequest alloc] init];

        VNImageRequestHandler* handler =
          [[VNImageRequestHandler alloc] initWithCGImage:cg options:@{}];
        NSError* error = nil;
        const BOOL ok =
          [handler performRequests:@[ textRequest, barcodeRequest ] error:&error];
        CGImageRelease(cg);
        if (!ok) {
            return {};
        }

        std::vector<TextLine> lines;
        for (VNRecognizedTextObservation* obs in textRequest.results) {
            VNRecognizedText* top = [[obs topCandidates:1] firstObject];
            if (top == nil || top.string.length == 0) {
                continue;
            }
            lines.push_back({ QString::fromUtf8(top.string.UTF8String),
                              obs.boundingBox });
        }
        QStringList result = orderLines(std::move(lines));

        QStringList codes;
        for (VNBarcodeObservation* obs in barcodeRequest.results) {
            const bool isQr = [obs.symbology isEqualToString:VNBarcodeSymbologyQR];
            if (isQr && obs.payloadStringValue.length > 0) {
                const QString payload =
                  QString::fromUtf8(obs.payloadStringValue.UTF8String);
                if (!codes.contains(payload)) {
                    codes << payload;
                }
            }
        }
        result << codes;
        return result.join(QLatin1Char('\n'));
    }
}
