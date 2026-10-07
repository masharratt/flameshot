// SPDX-License-Identifier: GPL-3.0-or-later

#include "capturefirstrequest.h"

CaptureRequest buildCaptureFirstRequest(const QString& savePath,
                                        const QString& picturesFallback)
{
    CaptureRequest req(CaptureRequest::GRAPHICAL_MODE);
    req.addTask(CaptureRequest::ACCEPT_ON_SELECT);
    req.addTask(CaptureRequest::COPY);
    req.setCaptureFirst(true);
    req.addSaveTask(savePath.isEmpty() ? picturesFallback : savePath);
    return req;
}

bool shouldApplyLastRegion(const CaptureRequest& req,
                           bool saveLastRegionEnabled)
{
    return saveLastRegionEnabled &&
           req.captureMode() == CaptureRequest::GRAPHICAL_MODE &&
           req.initialSelection().isNull() &&
           !(req.tasks() & CaptureRequest::ACCEPT_ON_SELECT);
}

bool shouldUseNativeFullscreen(const CaptureRequest& req,
                               bool nativeFullscreenEnabled)
{
    return nativeFullscreenEnabled && !req.captureFirst();
}

QRect globalSelectionRect(const QRect& localLogical, const QPoint& overlayTopLeft)
{
    return localLogical.translated(overlayTopLeft);
}

EditGeometry editGeometry(const QRect& globalLogical,
                          const QRect& screenGeometry,
                          qreal devicePixelRatio)
{
    const QRect local = globalLogical.translated(-screenGeometry.topLeft());
    const QRect device(qRound(local.x() * devicePixelRatio),
                       qRound(local.y() * devicePixelRatio),
                       qRound(local.width() * devicePixelRatio),
                       qRound(local.height() * devicePixelRatio));
    return { local, device };
}

bool editKeepsOriginalPixels(const QSize& imagePixels,
                             const QSize& logicalSize,
                             qreal devicePixelRatio)
{
    return qRound(logicalSize.width() * devicePixelRatio) ==
             imagePixels.width() &&
           qRound(logicalSize.height() * devicePixelRatio) ==
             imagePixels.height();
}
