// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include "core/recordingedit.h"

#include <QWidget>

// Timeline for the "Edit recording" window: the kept part between two
// draggable handles, and a playhead. Clicking elsewhere asks for a seek.
class TrimTimeline : public QWidget
{
    Q_OBJECT

public:
    explicit TrimTimeline(QWidget* parent = nullptr);

    void setDuration(double seconds);
    void setRange(TrimRange range);
    TrimRange range() const;
    void setPosition(double seconds);

    QSize sizeHint() const override;

signals:
    // A handle moved; `start` tells which one
    void rangeChanged(bool start);
    void seekRequested(double seconds);

protected:
    void paintEvent(QPaintEvent* event) override;
    void mousePressEvent(QMouseEvent* event) override;
    void mouseMoveEvent(QMouseEvent* event) override;
    void mouseReleaseEvent(QMouseEvent* event) override;

private:
    enum class Drag
    {
        None,
        Start,
        End,
        Playhead,
    };

    QRect track() const;
    int xFor(double seconds) const;
    double timeFor(int x) const;
    void dragTo(int x);

    double m_duration = 0.0;
    double m_position = 0.0;
    TrimRange m_range;
    Drag m_drag = Drag::None;
};
