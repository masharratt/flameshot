// SPDX-License-Identifier: GPL-3.0-or-later

#include "capturecountdown.h"
#include "core/capturedelay.h"

#include <QCursor>
#include <QFont>
#include <QGuiApplication>
#include <QPainter>
#include <QScreen>

namespace {
const int kSize = 96;
const int kTickMs = 100;
const int kTopMargin = 48;
} // namespace

CaptureCountdown::CaptureCountdown(int seconds, QWidget* parent)
  : QWidget(parent,
            Qt::Tool | Qt::FramelessWindowHint | Qt::WindowStaysOnTopHint |
              Qt::WindowDoesNotAcceptFocus)
  , m_remainingMs(seconds * 1000)
{
    setAttribute(Qt::WA_ShowWithoutActivating);
    setAttribute(Qt::WA_TranslucentBackground);
    setAttribute(Qt::WA_QuitOnClose, false);
    setFocusPolicy(Qt::NoFocus);
    setFixedSize(kSize, kSize);
    setToolTip(tr("Click to cancel"));
    m_timer.setInterval(kTickMs);
    connect(&m_timer, &QTimer::timeout, this, &CaptureCountdown::tick);
}

void CaptureCountdown::start()
{
    QScreen* screen = QGuiApplication::screenAt(QCursor::pos());
    if (screen == nullptr) {
        screen = QGuiApplication::primaryScreen();
    }
    const QRect area = screen->availableGeometry();
    move(area.center().x() - width() / 2, area.top() + kTopMargin);
    show();
    m_timer.start();
}

void CaptureCountdown::tick()
{
    m_remainingMs -= kTickMs;
    if (m_remainingMs <= 0) {
        m_timer.stop();
        emit finished();
        return;
    }
    update();
}

void CaptureCountdown::paintEvent(QPaintEvent*)
{
    QPainter p(this);
    p.setRenderHint(QPainter::Antialiasing);
    p.setPen(QPen(QColor(255, 255, 255, 90), 1));
    p.setBrush(QColor(30, 30, 30, 215));
    p.drawRoundedRect(rect().adjusted(1, 1, -1, -1), 16, 16);
    p.setPen(Qt::white);
    QFont f = font();
    f.setPixelSize(52);
    f.setBold(true);
    p.setFont(f);
    p.drawText(rect(),
               Qt::AlignCenter,
               QString::number(countdownSecondsLeft(m_remainingMs)));
}

void CaptureCountdown::mousePressEvent(QMouseEvent*)
{
    m_timer.stop();
    emit cancelled();
}
