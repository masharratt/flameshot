// SPDX-License-Identifier: GPL-3.0-or-later
// Tests for the pure image effects (border, rounded corners, drop shadow).

#include "utils/imageeffects.h"

#include <QImage>
#include <QTest>
#include <algorithm>
#include <cmath>

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

    void aspectRatiosAreExactAndKeepShot_data()
    {
        QTest::addColumn<int>("aspect");
        QTest::addColumn<int>("rw");
        QTest::addColumn<int>("rh");
        QTest::newRow("square") << 1 << 1 << 1;
        QTest::newRow("landscape") << 2 << 16 << 9;
        QTest::newRow("portrait") << 3 << 9 << 16;
        QTest::newRow("social") << 4 << 4 << 5;
        QTest::newRow("classic") << 5 << 4 << 3;
    }

    void aspectRatiosAreExactAndKeepShot()
    {
        QFETCH(int, aspect);
        QFETCH(int, rw);
        QFETCH(int, rh);
        const QImage src = solid(200, 120, QColor(10, 200, 30));
        EffectSettings s;
        s.paddingPx = 10;
        s.backgroundMode = BackgroundMode::Solid;
        s.backgroundColor1 = QColor(255, 0, 0);
        s.aspect = aspect;
        const QImage out = imageeffects::applyEffects(src, s);
        QVERIFY2(out.width() >= 220 && out.height() >= 140,
                 qPrintable(QString("%1x%2").arg(out.width()).arg(out.height())));
        QVERIFY2(std::abs(out.width() * rh - out.height() * rw) <=
                   std::max(rw, rh),
                 qPrintable(QString("%1x%2").arg(out.width()).arg(out.height())));
        // Shot is unscaled and centered: its center pixel and size survive
        QCOMPARE(out.pixelColor(out.width() / 2, out.height() / 2),
                 QColor(10, 200, 30));
        QCOMPARE(out.pixelColor(0, 0), QColor(255, 0, 0));
    }

    void aspectOffLeavesSizeAlone()
    {
        const QImage src = solid(200, 120, Qt::white);
        EffectSettings s;
        s.paddingPx = 10;
        s.backgroundMode = BackgroundMode::Solid;
        QCOMPARE(imageeffects::applyEffects(src, s).size(), QSize(220, 140));
    }

    void aspectWithoutBackgroundExpandsTransparent()
    {
        const QImage src = solid(200, 100, Qt::white);
        EffectSettings s;
        s.aspect = 1;
        const QImage out = imageeffects::applyEffects(src, s);
        QCOMPARE(out.size(), QSize(200, 200));
        QCOMPARE(out.pixelColor(0, 0).alpha(), 0);
        QCOMPARE(out.pixelColor(100, 100), QColor(Qt::white));
        QVERIFY(imageeffects::needsAlpha(s));
    }

    void autoBalanceTrimsToEqualMargins()
    {
        // 100x60 canvas: content 60x40 with 30px white margin left, 5px right
        QImage src = solid(100, 60, Qt::white);
        for (int y = 10; y < 50; ++y) {
            for (int x = 30; x < 95; ++x) {
                src.setPixelColor(x, y, QColor(20, 40, 200));
            }
        }
        const QImage out = imageeffects::autoBalance(src);
        // Margins end up equal horizontally (5px each side) and vertically
        QCOMPARE(out.size(), QSize(75, 40 + 2 * 10));
        QCOMPARE(out.pixelColor(0, 30), QColor(Qt::white));
        QCOMPARE(out.pixelColor(5, 30), QColor(20, 40, 200));
        QCOMPARE(out.pixelColor(out.width() - 6, 30), QColor(20, 40, 200));
        QCOMPARE(out.pixelColor(out.width() - 1, 30), QColor(Qt::white));
    }

    void autoBalanceLeavesNoMarginImageUnchanged()
    {
        // Content reaches every edge, so there is no margin to trim
        QImage src = solid(50, 40, QColor(20, 40, 200));
        for (int i = 0; i < 40; ++i) {
            src.setPixelColor(0, i, Qt::white);
            src.setPixelColor(25, i, Qt::red);
        }
        for (int i = 0; i < 50; ++i) {
            src.setPixelColor(i, 0, Qt::yellow);
            src.setPixelColor(i, 39, Qt::green);
        }
        QCOMPARE(imageeffects::autoBalance(src), src);
    }

    void autoBalanceNeverTrimsMoreThanFortyPercentPerSide()
    {
        const QImage src = solid(100, 100, Qt::white); // all margin
        const QImage out = imageeffects::autoBalance(src);
        QVERIFY(out.width() >= 20 && out.height() >= 20);
        QVERIFY(!out.isNull());
    }

    void autoBalanceRunsFirstInApplyEffects()
    {
        QImage src = solid(100, 60, Qt::white);
        for (int y = 10; y < 50; ++y) {
            for (int x = 30; x < 95; ++x) {
                src.setPixelColor(x, y, QColor(20, 40, 200));
            }
        }
        EffectSettings s;
        s.autoBalance = true;
        QCOMPARE(imageeffects::applyEffects(src, s).size(), QSize(75, 60));
    }
};

QTEST_GUILESS_MAIN(TestImageEffects)
#include "tst_imageeffects.moc"
