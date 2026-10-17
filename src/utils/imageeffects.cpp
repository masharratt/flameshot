// SPDX-License-Identifier: GPL-3.0-or-later

#include "imageeffects.h"

#include <QLinearGradient>
#include <QPainter>
#include <QPainterPath>
#include <QVector>
#include <algorithm>
#include <cmath>

namespace {

constexpr int kBlurPasses = 3;

int scaled(int logical, qreal dpr)
{
    return static_cast<int>(std::lround(logical * dpr));
}

// All painting happens in physical pixels, so work on a DPR 1 view of the image
QImage physical(const QImage& src)
{
    QImage plain = src;
    plain.setDevicePixelRatio(1.0);
    return plain;
}

// One box blur pass along rows or columns, treating pixels outside as zero
void boxBlur(QVector<float>& data, int w, int h, int radius, bool horizontal)
{
    if (radius <= 0) {
        return;
    }
    const int lines = horizontal ? h : w;
    const int length = horizontal ? w : h;
    const int stride = horizontal ? 1 : w;
    const int lineStep = horizontal ? w : 1;
    const float norm = 1.0f / float(2 * radius + 1);
    QVector<float> out(length);
    for (int line = 0; line < lines; ++line) {
        float* base = data.data() + line * lineStep;
        float sum = 0;
        for (int i = 0; i <= std::min(radius, length - 1); ++i) {
            sum += base[i * stride];
        }
        for (int i = 0; i < length; ++i) {
            out[i] = sum * norm;
            const int add = i + radius + 1;
            const int sub = i - radius;
            if (add < length) {
                sum += base[add * stride];
            }
            if (sub >= 0) {
                sum -= base[sub * stride];
            }
        }
        for (int i = 0; i < length; ++i) {
            base[i * stride] = out[i];
        }
    }
}

constexpr int kBalanceTolerance = 8;
constexpr double kMaxTrimFraction = 0.4;

bool near(QRgb a, QRgb b)
{
    return std::abs(qRed(a) - qRed(b)) <= kBalanceTolerance &&
           std::abs(qGreen(a) - qGreen(b)) <= kBalanceTolerance &&
           std::abs(qBlue(a) - qBlue(b)) <= kBalanceTolerance &&
           std::abs(qAlpha(a) - qAlpha(b)) <= kBalanceTolerance;
}

// Ratio width:height for the aspect presets, {0, 0} when off or unknown
QSize aspectRatio(int aspect)
{
    switch (aspect) {
        case 1:
            return { 1, 1 };
        case 2:
            return { 16, 9 };
        case 3:
            return { 9, 16 };
        case 4:
            return { 4, 5 };
        case 5:
            return { 4, 3 };
        default:
            return {};
    }
}

// Number of leading uniform lines (rows or columns) from one edge, limited
// to maxCount. A line is uniform when every pixel is near the edge's first
// pixel.
int uniformMargin(const QImage& img, bool rows, bool fromStart, int maxCount)
{
    const int lines = rows ? img.height() : img.width();
    const int length = rows ? img.width() : img.height();
    auto pixelAt = [&](int line, int i) {
        return rows ? img.pixel(i, line) : img.pixel(line, i);
    };
    const int first = fromStart ? 0 : lines - 1;
    const QRgb ref = pixelAt(first, 0);
    int count = 0;
    while (count < maxCount) {
        const int line = fromStart ? count : lines - 1 - count;
        for (int i = 0; i < length; ++i) {
            if (!near(pixelAt(line, i), ref)) {
                return count;
            }
        }
        ++count;
    }
    return count;
}

} // namespace

namespace imageeffects {

QImage addBorder(const QImage& src, int px, const QColor& color)
{
    const qreal dpr = src.devicePixelRatio();
    const int b = scaled(px, dpr);
    if (src.isNull() || b <= 0) {
        return src;
    }
    QImage out(src.width() + 2 * b,
               src.height() + 2 * b,
               src.hasAlphaChannel() ? QImage::Format_ARGB32_Premultiplied
                                     : QImage::Format_RGB32);
    out.fill(color);
    {
        QPainter p(&out);
        p.setCompositionMode(QPainter::CompositionMode_Source);
        p.drawImage(b, b, physical(src));
    }
    out.setDevicePixelRatio(dpr);
    return out;
}

QImage roundCorners(const QImage& src, int radius)
{
    const qreal dpr = src.devicePixelRatio();
    const qreal r = radius * dpr;
    if (src.isNull() || r <= 0) {
        return src;
    }
    QImage out(src.size(), QImage::Format_ARGB32_Premultiplied);
    out.fill(Qt::transparent);
    {
        QPainter p(&out);
        p.setRenderHint(QPainter::Antialiasing);
        QPainterPath path;
        path.addRoundedRect(QRectF(QPointF(0, 0), QSizeF(src.size())), r, r);
        p.setClipPath(path);
        p.drawImage(0, 0, physical(src));
    }
    out.setDevicePixelRatio(dpr);
    return out;
}

QImage dropShadow(const QImage& src,
                  int blur,
                  const QPoint& offset,
                  const QColor& color)
{
    if (src.isNull()) {
        return src;
    }
    const qreal dpr = src.devicePixelRatio();
    const int radius =
      std::max(0, (scaled(blur, dpr) + kBlurPasses - 1) / kBlurPasses);
    const int pad = radius * kBlurPasses;
    const int dx = scaled(offset.x(), dpr);
    const int dy = scaled(offset.y(), dpr);
    const int w = src.width() + 2 * pad + std::abs(dx);
    const int h = src.height() + 2 * pad + std::abs(dy);
    // Position of the original inside the canvas
    const int ix = pad + (dx < 0 ? -dx : 0);
    const int iy = pad + (dy < 0 ? -dy : 0);
    const QImage plain = physical(src);

    // Alpha mask of the original at the shadow position
    QImage mask(w, h, QImage::Format_ARGB32_Premultiplied);
    mask.fill(Qt::transparent);
    {
        QPainter p(&mask);
        p.setCompositionMode(QPainter::CompositionMode_Source);
        p.drawImage(ix + dx, iy + dy, plain);
    }
    QVector<float> alpha(w * h);
    for (int y = 0; y < h; ++y) {
        const QRgb* line = reinterpret_cast<const QRgb*>(mask.constScanLine(y));
        for (int x = 0; x < w; ++x) {
            alpha[y * w + x] = qAlpha(line[x]) / 255.0f;
        }
    }
    for (int i = 0; i < kBlurPasses; ++i) {
        boxBlur(alpha, w, h, radius, true);
        boxBlur(alpha, w, h, radius, false);
    }

    QImage out(w, h, QImage::Format_ARGB32_Premultiplied);
    const float colorAlpha = color.alphaF();
    for (int y = 0; y < h; ++y) {
        QRgb* line = reinterpret_cast<QRgb*>(out.scanLine(y));
        for (int x = 0; x < w; ++x) {
            const float a = std::clamp(alpha[y * w + x] * colorAlpha, 0.f, 1.f);
            line[x] = qPremultiply(qRgba(color.red(),
                                         color.green(),
                                         color.blue(),
                                         int(a * 255.f + 0.5f)));
        }
    }
    {
        QPainter p(&out);
        p.drawImage(ix, iy, plain);
    }
    out.setDevicePixelRatio(dpr);
    return out;
}

QImage addBackground(const QImage& src,
                     int padding,
                     BackgroundMode mode,
                     const QColor& color1,
                     const QColor& color2,
                     int aspect)
{
    const qreal dpr = src.devicePixelRatio();
    const int pad = std::max(0, scaled(padding, dpr));
    const QSize ratio = aspectRatio(aspect);
    if (src.isNull() ||
        (pad == 0 && mode == BackgroundMode::None && ratio.isEmpty())) {
        return src;
    }
    int w = src.width() + 2 * pad;
    int h = src.height() + 2 * pad;
    if (!ratio.isEmpty()) {
        // Grow the shorter axis, rounding up so the shot always fits
        if (qint64(w) * ratio.height() < qint64(h) * ratio.width()) {
            w = int((qint64(h) * ratio.width() + ratio.height() - 1) /
                    ratio.height());
        } else {
            h = int((qint64(w) * ratio.height() + ratio.width() - 1) /
                    ratio.width());
        }
    }
    const bool opaque = mode != BackgroundMode::None;
    QImage out(w,
               h,
               opaque ? QImage::Format_RGB32
                      : QImage::Format_ARGB32_Premultiplied);
    if (mode == BackgroundMode::Gradient) {
        QPainter p(&out);
        QLinearGradient gradient(0, 0, out.width(), out.height());
        gradient.setColorAt(0, color1);
        gradient.setColorAt(1, color2);
        p.fillRect(out.rect(), gradient);
    } else {
        out.fill(mode == BackgroundMode::Solid ? color1 : QColor(Qt::transparent));
    }
    {
        QPainter p(&out);
        p.drawImage((w - src.width()) / 2, (h - src.height()) / 2, physical(src));
    }
    out.setDevicePixelRatio(dpr);
    return out;
}

QImage autoBalance(const QImage& src)
{
    if (src.isNull()) {
        return src;
    }
    const QImage plain = physical(src).convertToFormat(
      QImage::Format_ARGB32);
    // Trim amount for one axis: the wider margin shrinks to the narrower one
    auto trims = [&](bool rows) {
        const int size = rows ? plain.height() : plain.width();
        const int cap = std::max(0, int(size * kMaxTrimFraction));
        const int a = uniformMargin(plain, rows, true, cap);
        const int b = uniformMargin(plain, rows, false, cap);
        const int keep = std::min(a, b);
        return std::pair<int, int>{ a - keep, b - keep };
    };
    const auto [top, bottom] = trims(true);
    const auto [left, right] = trims(false);
    if (top + bottom + left + right == 0) {
        return src;
    }
    QImage out = src.copy(
      left, top, src.width() - left - right, src.height() - top - bottom);
    out.setDevicePixelRatio(src.devicePixelRatio());
    return out;
}

QImage applyEffects(const QImage& src, const EffectSettings& settings)
{
    QImage out = settings.autoBalance ? autoBalance(src) : src;
    if (settings.borderPx > 0) {
        out = addBorder(out, settings.borderPx, settings.borderColor);
    }
    if (settings.cornerRadius > 0) {
        out = roundCorners(out, settings.cornerRadius);
    }
    if (settings.shadow) {
        out = dropShadow(
          out, settings.shadowBlur, settings.shadowOffset, settings.shadowColor);
    }
    return addBackground(out,
                         settings.paddingPx,
                         settings.backgroundMode,
                         settings.backgroundColor1,
                         settings.backgroundColor2,
                         settings.aspect);
}

bool needsAlpha(const EffectSettings& settings)
{
    if (settings.backgroundMode != BackgroundMode::None) {
        return false;
    }
    return settings.cornerRadius > 0 || settings.shadow ||
           settings.paddingPx > 0 || settings.aspect > 0;
}

} // namespace imageeffects
