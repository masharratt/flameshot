// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <QPixmap>
#include <QPointer>
#include <QRect>
#include <QString>
#include <QTimer>
#include <QWidget>

// Small always-on-top preview shown after a capture-first save. It never takes
// keyboard focus, hides itself after a delay and pauses while hovered.
class CaptureToast : public QWidget
{
    Q_OBJECT

public:
    // Creates, stacks and shows a toast. Does nothing when seconds <= 0.
    static void showFor(const QString& path,
                        const QPixmap& capture,
                        const QRect& selection,
                        const QRect& globalRect,
                        int seconds);
    // Toast for a saved screen recording (kind "mp4" or "gif"): first-frame
    // thumbnail with Show in Finder and Copy path only.
    static void showForRecording(const QString& path,
                                 const QString& kind,
                                 int seconds);

protected:
    void enterEvent(QEnterEvent* event) override;
    void leaveEvent(QEvent* event) override;
    void closeEvent(QCloseEvent* event) override;

private:
    CaptureToast(const QString& path,
                 const QPixmap& capture,
                 const QRect& selection,
                 const QRect& globalRect,
                 int seconds,
                 bool recording = false);

    static void restack();

    QString m_path;
    QPixmap m_capture;
    QRect m_selection;  // exportCapture units, used for pins
    QRect m_globalRect; // global logical points
    QTimer m_timer;

    // Newest first, matching toastStackGeometry()
    static QList<QPointer<CaptureToast>> s_toasts;
};
