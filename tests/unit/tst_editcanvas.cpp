// SPDX-License-Identifier: GPL-3.0-or-later
// Tests for the windowed editor canvas plan. Units: imagePx is device pixels,
// every rect and size in EditCanvas is logical points.

#include "widgets/editcanvas.h"

#include <QTest>

class TestEditCanvas : public QObject
{
    Q_OBJECT
private slots:
    void smallImageFitsWindow()
    {
        const EditCanvas c =
          planEditCanvas(QSize(400, 300), 1.0, 56, QRect(0, 0, 1920, 1080));
        QCOMPARE(c.imageRectLogical, QRect(56, 56, 400, 300));
        QCOMPARE(c.canvasLogical, QSize(512, 412));
        QCOMPARE(c.windowSize, QSize(512, 412));
    }

    void imageLargerThanScreenClampsWindowNotCanvas()
    {
        const EditCanvas c =
          planEditCanvas(QSize(4000, 3000), 1.0, 56, QRect(0, 0, 1920, 1080));
        QCOMPARE(c.canvasLogical, QSize(4112, 3112));
        QCOMPARE(c.imageRectLogical, QRect(56, 56, 4000, 3000));
        QCOMPARE(c.windowSize, QSize(1728, 972));
    }

    void onlyOneAxisClamped()
    {
        const EditCanvas c =
          planEditCanvas(QSize(3000, 200), 1.0, 56, QRect(0, 0, 1000, 1000));
        QCOMPARE(c.windowSize, QSize(900, 312));
    }

    void retinaHalvesLogicalSize()
    {
        const EditCanvas c =
          planEditCanvas(QSize(800, 600), 2.0, 56, QRect(0, 0, 1440, 900));
        QCOMPARE(c.imageRectLogical, QRect(56, 56, 400, 300));
        QCOMPARE(c.canvasLogical, QSize(512, 412));
        QCOMPARE(c.windowSize, QSize(512, 412));
    }

    // 401 device pixels at 2x is 200.5 points: round up so the whole image
    // stays inside the canvas.
    void oddPixelSizesRoundUp()
    {
        const EditCanvas c =
          planEditCanvas(QSize(401, 201), 2.0, 56, QRect(0, 0, 1440, 900));
        QCOMPARE(c.imageRectLogical, QRect(56, 56, 201, 101));
        QCOMPARE(c.canvasLogical, QSize(313, 213));
    }

    void availableGeometryOffsetDoesNotMatter()
    {
        const EditCanvas c = planEditCanvas(
          QSize(5000, 100), 1.0, 56, QRect(-1440, 25, 1440, 875));
        QCOMPARE(c.windowSize, QSize(1296, 212));
    }

    void invalidDprFallsBackToOne()
    {
        const EditCanvas c =
          planEditCanvas(QSize(100, 50), 0.0, 10, QRect(0, 0, 800, 600));
        QCOMPARE(c.imageRectLogical, QRect(10, 10, 100, 50));
        QCOMPARE(c.canvasLogical, QSize(120, 70));
    }

    // The window grows by the fixed toolbar strip; the canvas does not.
    void chromeHeightAddsToWindowNotCanvas()
    {
        const EditCanvas c = planEditCanvas(
          QSize(400, 300), 1.0, 8, QRect(0, 0, 1920, 1080), 48);
        QCOMPARE(c.imageRectLogical, QRect(8, 8, 400, 300));
        QCOMPARE(c.canvasLogical, QSize(416, 316));
        QCOMPARE(c.windowSize, QSize(416, 364));
    }

    void chromeHeightStillClampedToScreen()
    {
        const EditCanvas c = planEditCanvas(
          QSize(4000, 3000), 1.0, kEditMargin, QRect(0, 0, 1920, 1080), 48);
        QCOMPARE(c.canvasLogical,
                 QSize(4000 + 2 * kEditMargin, 3000 + 2 * kEditMargin));
        QCOMPARE(c.windowSize, QSize(1728, 972));
    }

    void editMarginIsSmall()
    {
        QVERIFY(kEditMargin <= 8);
        QVERIFY(kEditMargin > 0);
    }

    void toolbarRowsEmptyIsZero()
    {
        QCOMPARE(toolbarRows(0, 32, 4, 400), 0);
    }

    void toolbarRowsFitsOneRow()
    {
        // 10 * 32 + 9 * 4 = 356 <= 360
        QCOMPARE(toolbarRows(10, 32, 4, 360), 1);
    }

    void toolbarRowsWrapsWhenNarrow()
    {
        // 9 per row fit in 355, so 10 buttons need 2 rows
        QCOMPARE(toolbarRows(10, 32, 4, 355), 2);
        QCOMPARE(toolbarRows(19, 32, 4, 355), 3);
    }

    void toolbarRowsOneButtonPerRowWhenTiny()
    {
        QCOMPARE(toolbarRows(5, 32, 4, 10), 5);
        QCOMPARE(toolbarRows(5, 32, 4, 0), 5);
    }

    void toolbarRowsExactFit()
    {
        // 2 buttons need exactly 68
        QCOMPARE(toolbarRows(2, 32, 4, 68), 1);
        QCOMPARE(toolbarRows(2, 32, 4, 67), 2);
    }
};

QTEST_MAIN(TestEditCanvas)
#include "tst_editcanvas.moc"
