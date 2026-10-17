// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include "core/capturerequest.h"
#include "core/recordingstate.h"

#include <QObject>
#include <QPointer>
#include <QRect>
#include <QSize>
#include <QString>

#include <memory>

class RecordingControl;
class ScreenRecorder;

// Runs one screen recording from the area pick to the saved file. The legal
// order of steps is enforced by RecordingStateMachine.
class RecordingController : public QObject
{
    Q_OBJECT

public:
    explicit RecordingController(QObject* parent = nullptr);
    ~RecordingController() override;

    RecState state() const;

    // A record hotkey or menu item was used: starts picking an area when idle,
    // stops when recording, otherwise does nothing.
    void trigger(CaptureRequest::RecordMode mode);
    // The overlay settled on an area (global logical points)
    void areaPicked(const QRect& globalRect);
    // The overlay closed without an area
    void pickAbandoned();

    // Next free file name in the save folder for this extension
    static QString nextOutputPath(const QString& extension);
    // Removes editSource when it is a temp file (not the saved path)
    static void discardEditSource(const QString& editSource,
                                  const QString& savedPath);

signals:
    // Ask for the area picker to open
    void pickRequested(CaptureRequest::RecordMode mode);
    // kind is "mp4" or "gif"; size is in pixels. editSource is the MP4 the
    // "Edit recording" window works from: the saved file itself for an MP4,
    // the kept temp recording for a GIF. Whoever ends up holding a source
    // that differs from path removes it with discardEditSource().
    void recordingSaved(const QString& path,
                        const QString& kind,
                        const QSize& size,
                        const QString& editSource);

private:
    void begin();
    void stop();
    void cancel();
    void onStopped(bool ok);
    void startGifConversion();
    void finish(const QString& path,
                const QString& kind,
                const QString& editSource);
    void fail(const QString& message);
    void cleanup();

    RecordingStateMachine m_machine;
    CaptureRequest::RecordMode m_mode = CaptureRequest::RecordNone;
    QRect m_rect;
    QString m_tempPath;
    QPointer<RecordingControl> m_control;
    std::unique_ptr<ScreenRecorder> m_recorder;
};
