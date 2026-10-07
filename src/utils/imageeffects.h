// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <QColor>
#include <QImage>
#include <QPoint>

// Settings for the after-capture effects. Sizes are logical pixels; the
// functions multiply them by the image devicePixelRatio.
struct EffectSettings
{
    int borderPx = 0;
    QColor borderColor = Qt::black;
    int cornerRadius = 0;
    bool shadow = false;
    int shadowBlur = 16;
    QPoint shadowOffset{ 0, 6 };
    QColor shadowColor = QColor(0, 0, 0, 110);
};

namespace imageeffects {

// Canvas grows by 2 * px (logical) on each axis
QImage addBorder(const QImage& src, int px, const QColor& color);

// Corner pixels become fully transparent
QImage roundCorners(const QImage& src, int radius);

// Canvas grows to fit the blur and offset; the original is drawn on top
QImage dropShadow(const QImage& src,
                  int blur,
                  const QPoint& offset,
                  const QColor& color);

// Order: border, round corners, shadow. Identity when everything is off.
QImage applyEffects(const QImage& src, const EffectSettings& settings);

// True when the result can contain transparent pixels
bool needsAlpha(const EffectSettings& settings);

} // namespace imageeffects
