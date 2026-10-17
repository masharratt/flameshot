// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <QTimer>
#include <QWidget>

// Small always-on-top countdown shown while the capture timer runs. It never
// takes focus. Clicking it cancels. The caller must close it before the
// screen is grabbed so it never appears in the capture.
class CaptureCountdown : public QWidget
{
    Q_OBJECT

public:
    explicit CaptureCountdown(int seconds, QWidget* parent = nullptr);

    // Shows near the top-center of the screen under the cursor
    void start();

signals:
    void finished();
    void cancelled();

protected:
    void paintEvent(QPaintEvent* event) override;
    void mousePressEvent(QMouseEvent* event) override;

private:
    void tick();

    QTimer m_timer;
    int m_remainingMs;
};
