// SPDX-License-Identifier: GPL-3.0-or-later
// Tests for the pure image effects (border, rounded corners, drop shadow).

#include "utils/imageeffects.h"

#include <QImage>
#include <QTest>

namespace {

QImage solid(int w, int h, const QColor& color, qreal dpr = 1.0)
{
    QImage img(w, h, QImage::Format_ARGB32_Premultiplied);
    img.fill(color);
    img.setDevicePixelRatio(dpr);
    return img;
}

} // namespace

class TestImageEffects : public QObject
{
    Q_OBJECT
private slots:
    void identityWhenOff()
    {
        const QImage src = solid(40, 30, QColor(10, 200, 30));
        const QImage out = imageeffects::applyEffects(src, EffectSettings{});
        QCOMPARE(out.size(), src.size());
        QCOMPARE(out, src);
    }

    void borderGrowsCanvas()
    {
        const QImage src = solid(40, 30, Qt::white);
        const QImage out = imageeffects::addBorder(src, 5, QColor(255, 0, 0));
        QCOMPARE(out.size(), QSize(50, 40));
        QCOMPARE(out.pixelColor(0, 0), QColor(255, 0, 0));
        QCOMPARE(out.pixelColor(49, 39), QColor(255, 0, 0));
        QCOMPARE(out.pixelColor(25, 20), QColor(Qt::white));
    }

    void roundedCorners()
    {
        const QImage src = solid(60, 60, QColor(0, 0, 255));
        const QImage out = imageeffects::roundCorners(src, 12);
        QCOMPARE(out.size(), src.size());
        QCOMPARE(out.format(), QImage::Format_ARGB32_Premultiplied);
        QCOMPARE(out.pixelColor(0, 0).alpha(), 0);
        QCOMPARE(out.pixelColor(59, 59).alpha(), 0);
        QCOMPARE(out.pixelColor(30, 30), QColor(0, 0, 255));
    }

    void shadowGrowsCanvas()
    {
        const QImage src = solid(40, 30, Qt::white);
        const QPoint offset(0, 6);
        const int blur = 8;
        const QImage out = imageeffects::dropShadow(
          src, blur, offset, QColor(0, 0, 0, 200));
        QVERIFY(out.width() >= src.width() + 2 * blur);
        QVERIFY(out.height() >= src.height() + 2 * blur + offset.y());
        // Find the original: the centre of the opaque image stays opaque white
        // and a pixel just below the image carries partial shadow alpha.
        const int ox = (out.width() - src.width()) / 2;
        // Image is placed so that shadow room is symmetric minus the offset.
        int top = -1;
        for (int y = 0; y < out.height(); ++y) {
            if (out.pixelColor(out.width() / 2, y) == QColor(Qt::white)) {
                top = y;
                break;
            }
        }
        QVERIFY(top >= 0);
        QVERIFY(ox >= 0);
        const int below = top + src.height() + 2;
        QVERIFY(below < out.height());
        const int alpha = out.pixelColor(out.width() / 2, below).alpha();
        QVERIFY2(alpha > 0 && alpha < 255, qPrintable(QString::number(alpha)));
    }

    void combinedOrder()
    {
        const QImage src = solid(50, 50, Qt::white);
        EffectSettings s;
        s.borderPx = 4;
        s.borderColor = QColor(255, 0, 0);
        s.cornerRadius = 10;
        s.shadow = true;
        s.shadowBlur = 6;
        s.shadowOffset = QPoint(0, 4);
        const QImage out = imageeffects::applyEffects(src, s);
        // Border first, then shadow around the bordered image.
        QVERIFY(out.width() >= 58 + 12);
        QVERIFY(out.height() >= 58 + 12 + 4);
        // Centre is the original image; border colour sits between.
        QCOMPARE(out.pixelColor(out.width() / 2, out.height() / 2),
                 QColor(Qt::white));
        // Corners of the canvas are not opaque (shadow/rounding).
        QVERIFY(out.pixelColor(0, 0).alpha() < 255);
        // Walking down the centre column we meet red border before white.
        bool sawRed = false;
        for (int y = 0; y < out.height() / 2; ++y) {
            if (out.pixelColor(out.width() / 2, y) == QColor(255, 0, 0)) {
                sawRed = true;
            }
        }
        QVERIFY(sawRed);
    }

    void dprScalesBorder()
    {
        const QImage src = solid(40, 30, Qt::white, 2.0);
        const QImage out = imageeffects::addBorder(src, 3, QColor(255, 0, 0));
        QCOMPARE(out.size(), QSize(40 + 12, 30 + 12));
        QCOMPARE(out.devicePixelRatio(), 2.0);
        QCOMPARE(out.pixelColor(5, 5), QColor(255, 0, 0));
        QCOMPARE(out.pixelColor(6, 6), QColor(Qt::white));
    }

    void dprPreservedByAllEffects()
    {
        const QImage src = solid(40, 30, Qt::white, 2.0);
        EffectSettings s;
        s.cornerRadius = 4;
        s.shadow = true;
        QCOMPARE(imageeffects::roundCorners(src, 4).devicePixelRatio(), 2.0);
        QCOMPARE(imageeffects::applyEffects(src, s).devicePixelRatio(), 2.0);
    }

    void needsAlphaTable()
    {
        EffectSettings s;
        QVERIFY(!imageeffects::needsAlpha(s));
        s.borderPx = 5;
        QVERIFY(!imageeffects::needsAlpha(s));
        s.cornerRadius = 3;
        QVERIFY(imageeffects::needsAlpha(s));
        s.cornerRadius = 0;
        s.shadow = true;
        QVERIFY(imageeffects::needsAlpha(s));
    }

    void paddingGrowsCanvasByTwicePad()
    {
        const QImage src = solid(40, 30, Qt::white);
        EffectSettings s;
        s.paddingPx = 20;
        s.backgroundMode = BackgroundMode::Solid;
        const QImage out = imageeffects::applyEffects(src, s);
        QCOMPARE(out.size(), QSize(80, 70));
        QCOMPARE(out.pixelColor(40, 35), QColor(Qt::white));
    }

    void paddingAddsToShadowMargins()
    {
        const QImage src = solid(40, 30, Qt::white);
        EffectSettings s;
        s.shadow = true;
        const QImage shadowOnly = imageeffects::applyEffects(src, s);
        s.paddingPx = 10;
        s.backgroundMode = BackgroundMode::Solid;
        const QImage out = imageeffects::applyEffects(src, s);
        QCOMPARE(out.size(), shadowOnly.size() + QSize(20, 20));
    }

    void solidBackgroundFillsCorner()
    {
        const QImage src = solid(40, 30, Qt::white);
        EffectSettings s;
        s.paddingPx = 16;
        s.cornerRadius = 8;
        s.backgroundMode = BackgroundMode::Solid;
        s.backgroundColor1 = QColor(139, 92, 246);
        const QImage out = imageeffects::applyEffects(src, s);
        QCOMPARE(out.pixelColor(0, 0), QColor(139, 92, 246));
        QCOMPARE(out.pixelColor(out.width() - 1, out.height() - 1),
                 QColor(139, 92, 246));
        QCOMPARE(out.pixelColor(0, 0).alpha(), 255);
    }

    void gradientRunsDiagonally()
    {
        const QImage src = solid(10, 10, Qt::white);
        EffectSettings s;
        s.paddingPx = 100;
        s.backgroundMode = BackgroundMode::Gradient;
        s.backgroundColor1 = QColor(255, 0, 0);
        s.backgroundColor2 = QColor(0, 0, 255);
        const QImage out = imageeffects::applyEffects(src, s);
        const QColor tl = out.pixelColor(0, 0);
        const QColor br = out.pixelColor(out.width() - 1, out.height() - 1);
        QVERIFY2(tl.red() > 240 && tl.blue() < 15, qPrintable(tl.name()));
        QVERIFY2(br.blue() > 240 && br.red() < 15, qPrintable(br.name()));
    }

    void backgroundNoneKeepsPaddingTransparent()
    {
        const QImage src = solid(40, 30, Qt::white);
        EffectSettings s;
        s.paddingPx = 10;
        const QImage out = imageeffects::applyEffects(src, s);
        QCOMPARE(out.size(), QSize(60, 50));
        QCOMPARE(out.pixelColor(0, 0).alpha(), 0);
        QVERIFY(imageeffects::needsAlpha(s));
    }

    void paddingScalesWithDpr()
    {
        const QImage src = solid(40, 30, Qt::white, 2.0);
        EffectSettings s;
        s.paddingPx = 5;
        s.backgroundMode = BackgroundMode::Solid;
        const QImage out = imageeffects::applyEffects(src, s);
        QCOMPARE(out.size(), QSize(60, 50));
        QCOMPARE(out.devicePixelRatio(), 2.0);
    }

    void needsAlphaFalseWithBackground()
    {
        EffectSettings s;
        s.cornerRadius = 8;
        s.shadow = true;
        QVERIFY(imageeffects::needsAlpha(s));
        s.backgroundMode = BackgroundMode::Gradient;
        QVERIFY(!imageeffects::needsAlpha(s));
    }
};

QTEST_GUILESS_MAIN(TestImageEffects)
#include "tst_imageeffects.moc"
