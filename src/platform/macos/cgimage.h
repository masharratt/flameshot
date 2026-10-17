// SPDX-License-Identifier: GPL-3.0-or-later
// CoreGraphics image conversion shared by the macOS platform files.

#pragma once

#include <CoreGraphics/CoreGraphics.h>
#include <QImage>

// Copies a CGImage into a premultiplied ARGB32 QImage (null image for NULL)
QImage imageFromCGImage(CGImageRef image);
