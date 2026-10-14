// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <QPoint>
#include <QString>
#include <Qt>

class QImage;
class QMimeData;

// What a mouse release on the after-edit thumbnail does
enum class ThumbnailAction
{
    None,
    Open,
    Close,
};

// Left click opens the image again, right click closes the thumbnail.
// A release is only a click when no drag started; the caller tracks that.
ThumbnailAction thumbnailReleaseAction(Qt::MouseButton button);

// True once a left-button press has moved far enough to become a drag
bool thumbnailShouldStartDrag(Qt::MouseButtons buttons,
                              const QPoint& pressPos,
                              const QPoint& currentPos,
                              int dragDistance);

// Drop payload: the saved file plus the image, so apps that take files and
// apps that only take images both accept it. The caller owns the result.
QMimeData* thumbnailMimeData(const QString& path, const QImage& image);
