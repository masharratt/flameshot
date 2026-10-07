// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <QColor>
#include <QList>
#include <QWidget>

class QHBoxLayout;
class QSlider;
class QSpinBox;
class QToolButton;

// Second row of the editor window's top strip: colour swatches from the
// user's palette, a "+" button for any other colour, and a size slider with a
// number box. It only emits user intent; the owner applies it and calls
// setColor / setSize to reflect changes made anywhere else.
class EditorSettingsBar : public QWidget
{
    Q_OBJECT
public:
    explicit EditorSettingsBar(QWidget* parent = nullptr);

    // Fixed height of the bar in logical points
    static int barHeight();

    // Reflect state; never emits the user-intent signals
    void setColor(const QColor& color);
    void setSize(int size);

signals:
    void colorPicked(const QColor& color);
    void sizePicked(int size);

private:
    void rebuildSwatches();
    void pickCustomColor();
    void refreshSelection();

    QHBoxLayout* m_swatchLayout{ nullptr };
    QList<QToolButton*> m_swatches;
    QList<QColor> m_colors;
    QToolButton* m_plus{ nullptr };
    QSlider* m_slider{ nullptr };
    QSpinBox* m_spin{ nullptr };
    QColor m_current;
};
