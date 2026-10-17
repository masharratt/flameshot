// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <QImage>
#include <QString>

// Text found in the image by Apple Vision (accurate level, language
// correction, automatic language detection where available). Lines are
// ordered top to bottom, then left to right, joined with "\n". A QR code or
// other barcode payload is appended after the text, or returned alone when no
// text is found. Empty when nothing is recognised. Runs synchronously.
QString recognizeText(const QImage& image);
