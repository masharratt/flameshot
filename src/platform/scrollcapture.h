// SPDX-License-Identifier: GPL-3.0-or-later
// Platform side of scrolling capture: repeated grabs of one screen area and
// synthetic scroll-wheel events. Implemented with ScreenCaptureKit and
// CGEvent on macOS; elsewhere every call reports "unsupported".

#pragma once

#include <QImage>
#include <QList>
#include <QPoint>
#include <QRect>
#include <QString>
#include <QtGlobal>

#include <functional>
#include <memory>

// Grabs one area of one display again and again, leaving the listed windows
// (the floating control bar) and the mouse pointer out. Callbacks run on the
// Qt main thread.
class AreaGrabber
{
public:
    virtual ~AreaGrabber() = default;

    // Looks up the display and windows once. ready gets an empty error on
    // success. globalLogicalRect is in global logical points.
    virtual void prepare(const QRect& globalLogicalRect,
                         const QList<quint32>& excludeWindowIds,
                         std::function<void(QString error)> ready) = 0;
    // One grab, after prepare succeeded. done gets the area in device pixels
    // (devicePixelRatio set), or a null image when the grab failed.
    virtual void grab(std::function<void(QImage image)> done) = 0;
};

std::unique_ptr<AreaGrabber> createAreaGrabber();

// Whether Flameshot may post input events (macOS Accessibility permission).
// With prompt, macOS shows its own dialog that leads to System Settings.
bool scrollEventsAllowed(bool prompt);

// Posts one scroll-wheel event that scrolls the content under globalLogical
// down by about `points`. Moves the pointer there first when it is outside
// keepInside. False when the event could not be made or posted.
bool postScrollDown(const QPoint& globalLogical,
                    const QRect& keepInside,
                    int points);
