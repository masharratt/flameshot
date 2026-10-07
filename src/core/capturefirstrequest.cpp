// SPDX-License-Identifier: GPL-3.0-or-later

#include "capturefirstrequest.h"

CaptureRequest buildCaptureFirstRequest(const QString& workflow,
                                        const QString& savePath,
                                        const QString& picturesFallback)
{
    CaptureRequest req(CaptureRequest::GRAPHICAL_MODE);
    req.addTask(CaptureRequest::ACCEPT_ON_SELECT);
    req.setCaptureFirst(true);
    req.setWorkflow(workflow);
    // The path is where the Save action writes; the SAVE task itself is not
    // used because the workflow replaces it.
    req.addSaveTask(savePath.isEmpty() ? picturesFallback : savePath);
    req.removeTask(CaptureRequest::SAVE);
    return req;
}

CaptureRequest::RecordMode recordModeForHotkey(const QString& hotkey)
{
    if (hotkey == QLatin1String("RECORD_MP4")) {
        return CaptureRequest::RecordMp4;
    }
    if (hotkey == QLatin1String("RECORD_GIF")) {
        return CaptureRequest::RecordGif;
    }
    return CaptureRequest::RecordNone;
}

CaptureRequest buildRecordRequest(CaptureRequest::RecordMode mode)
{
    CaptureRequest req(CaptureRequest::GRAPHICAL_MODE);
    req.addTask(CaptureRequest::ACCEPT_ON_SELECT);
    // Plain overlay window, as for every capture-first request
    req.setCaptureFirst(true);
    req.setRecordMode(mode);
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
