// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <QString>

#include <functional>

struct GifEncodeParams
{
    QString inputMp4;
    QString outputGif;
    int fps = 15;
    int maxWidth = 960;
};

// Converts a recorded MP4 to a looping GIF on a worker thread. done runs on
// the Qt main thread with ok = false and a message on failure.
void encodeGifAsync(const GifEncodeParams& params,
                    std::function<void(bool ok, QString error)> done);
