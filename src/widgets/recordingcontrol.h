// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <QElapsedTimer>
#include <QString>
#include <QTimer>
#include <QWidget>

class QLabel;
class QPushButton;

// Small always-on-top control shown beside the recorded area: red dot, elapsed
// time, Stop and Cancel. It never takes keyboard focus.
class RecordingControl : public QWidget
{
    Q_OBJECT

public:
    explicit RecordingControl(QWidget* parent = nullptr);

    // "mm:ss" for a number of whole seconds
    static QString formatElapsed(int seconds);

    // Starts the clock and enables the buttons
    void startClock();
    // Freezes the clock and shows a status instead of the buttons
    void showBusy(const QString& text);

signals:
    void stopRequested();
    void cancelRequested();

private:
    void updateTime();

    QLabel* m_dot;
    QLabel* m_time;
    QPushButton* m_stop;
    QPushButton* m_cancel;
    QTimer m_tick;
    QElapsedTimer m_elapsed;
};
