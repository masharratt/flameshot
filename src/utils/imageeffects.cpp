// SPDX-License-Identifier: GPL-3.0-or-later

#include "imageeffects.h"

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

QImage applyEffects(const QImage& src, const EffectSettings& settings)
{
    QImage out = src;
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
    return out;
}

bool needsAlpha(const EffectSettings& settings)
{
    return settings.cornerRadius > 0 || settings.shadow;
}

} // namespace imageeffects
