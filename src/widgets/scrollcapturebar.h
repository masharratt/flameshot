// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <QImage>
#include <QString>
#include <QWidget>

class QLabel;
class QPushButton;

// Always-on-top bar shown beside the area of a scrolling capture: a small
// preview of the stitched image, its height, a status line and the Auto,
// Done and Cancel buttons. It never takes keyboard focus, and the grabs
// leave it out.
class ScrollCaptureBar : public QWidget
{
    Q_OBJECT

public:
    explicit ScrollCaptureBar(QWidget* parent = nullptr);

    void setStatus(const QString& text);
    // heightPx is the stitched height in pixels
    void setStitched(int heightPx, const QImage& preview);
    // Auto button reads "Stop auto" while automatic scrolling runs
    void setAutoRunning(bool running);
    // Buttons ready once grabbing works
    void setReady(bool ready);

signals:
    void doneRequested();
    void cancelRequested();
    void autoRequested();

private:
    QLabel* m_preview;
    QLabel* m_status;
    QLabel* m_height;
    QPushButton* m_auto;
    QPushButton* m_done;
    QPushButton* m_cancel;
};
