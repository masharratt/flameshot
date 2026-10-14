// SPDX-License-Identifier: GPL-3.0-or-later

#include "thumbnailgesture.h"

#include <QImage>
#include <QMimeData>
#include <QUrl>

ThumbnailAction thumbnailReleaseAction(Qt::MouseButton button)
{
    switch (button) {
        case Qt::LeftButton:
            return ThumbnailAction::Open;
        case Qt::RightButton:
            return ThumbnailAction::Close;
        default:
            return ThumbnailAction::None;
    }
}

bool thumbnailShouldStartDrag(Qt::MouseButtons buttons,
                              const QPoint& pressPos,
                              const QPoint& currentPos,
                              int dragDistance)
{
    return (buttons & Qt::LeftButton) &&
           (currentPos - pressPos).manhattanLength() >= dragDistance;
}

QMimeData* thumbnailMimeData(const QString& path, const QImage& image)
{
    auto* data = new QMimeData();
    data->setUrls({ QUrl::fromLocalFile(path) });
    data->setImageData(image);
    return data;
}
