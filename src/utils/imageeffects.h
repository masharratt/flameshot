// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <QColor>
#include <QImage>
#include <QPoint>

enum class BackgroundMode
{
    None = 0,
    Solid = 1,
    Gradient = 2, // diagonal, top-left color1 to bottom-right color2
};

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
    int paddingPx = 0; // margin on each side of the shot (and its shadow)
    BackgroundMode backgroundMode = BackgroundMode::None;
    QColor backgroundColor1 = QColor(0x8B, 0x5C, 0xF6);
    QColor backgroundColor2 = QColor(0x3B, 0x82, 0xF6);
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

// Canvas grows by 2 * padding (logical) on each axis, src is centered. With
// BackgroundMode::None the padding is transparent. A background fills the whole
// canvas, even when padding is 0.
QImage addBackground(const QImage& src,
                     int padding,
                     BackgroundMode mode,
                     const QColor& color1,
                     const QColor& color2);

// Order: border, round corners, shadow, padding + background. Identity when
// everything is off.
QImage applyEffects(const QImage& src, const EffectSettings& settings);

// True when the result can contain transparent pixels (a background fills
// the canvas, so it makes the result opaque)
bool needsAlpha(const EffectSettings& settings);

} // namespace imageeffects
