// SPDX-License-Identifier: GPL-3.0-or-later

#include "recordingcontrol.h"

#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>

RecordingControl::RecordingControl(QWidget* parent)
  : QWidget(parent,
            Qt::Tool | Qt::FramelessWindowHint | Qt::WindowStaysOnTopHint |
              Qt::WindowDoesNotAcceptFocus)
{
    setAttribute(Qt::WA_ShowWithoutActivating);
    setAttribute(Qt::WA_QuitOnClose, false);
    setFocusPolicy(Qt::NoFocus);
    setObjectName(QStringLiteral("RecordingControl"));
    setStyleSheet(QStringLiteral(
      "#RecordingControl { background-color: #2b2b2b; border: 1px solid #555;"
      " border-radius: 8px; }"
      "QLabel { color: #eee; }"
      "QPushButton { color: #eee; background: #3c3c3c; border: none;"
      " border-radius: 4px; padding: 3px 8px; }"
      "QPushButton:hover { background: #505050; }"
      "QPushButton:disabled { color: #888; }"));

    auto* layout = new QHBoxLayout(this);
    layout->setContentsMargins(10, 6, 10, 6);
    layout->setSpacing(8);

    m_dot = new QLabel(QStringLiteral("●"), this);
    m_dot->setStyleSheet(QStringLiteral("color: #e53935;"));
    layout->addWidget(m_dot);

    m_time = new QLabel(formatElapsed(0), this);
    layout->addWidget(m_time);

    m_stop = new QPushButton(tr("Stop"), this);
    m_stop->setFocusPolicy(Qt::NoFocus);
    m_stop->setEnabled(false);
    connect(m_stop, &QPushButton::clicked, this, &RecordingControl::stopRequested);
    layout->addWidget(m_stop);

    m_cancel = new QPushButton(tr("Cancel"), this);
    m_cancel->setFocusPolicy(Qt::NoFocus);
    m_cancel->setEnabled(false);
    connect(m_cancel,
            &QPushButton::clicked,
            this,
            &RecordingControl::cancelRequested);
    layout->addWidget(m_cancel);

    m_tick.setInterval(250);
    connect(&m_tick, &QTimer::timeout, this, &RecordingControl::updateTime);

    setFixedSize(sizeHint());
}

QString RecordingControl::formatElapsed(int seconds)
{
    seconds = qMax(0, seconds);
    return QStringLiteral("%1:%2")
      .arg(seconds / 60, 2, 10, QLatin1Char('0'))
      .arg(seconds % 60, 2, 10, QLatin1Char('0'));
}

void RecordingControl::startClock()
{
    m_elapsed.start();
    m_stop->setEnabled(true);
    m_cancel->setEnabled(true);
    m_tick.start();
    updateTime();
}

void RecordingControl::showBusy(const QString& text)
{
    m_tick.stop();
    m_stop->hide();
    m_cancel->hide();
    m_dot->setStyleSheet(QStringLiteral("color: #aaa;"));
    m_time->setText(text);
    // Fixed size was computed with the buttons, so shrink to the new content
    setFixedSize(sizeHint());
}

void RecordingControl::updateTime()
{
    m_time->setText(
      formatElapsed(static_cast<int>(m_elapsed.elapsed() / 1000)));
}
