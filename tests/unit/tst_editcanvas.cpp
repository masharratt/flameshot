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

    // Canvas 416x316 logical, image selection at the 8 point margin.
    void expandGrowsCanvasAndSelectionDpr1()
    {
        const CanvasExpansion e = expandCanvas(
          QSize(416, 316), QRect(8, 8, 400, 300), QRect(8, 8, 400, 300), 40, 1.0);
        QCOMPARE(e.newCanvasPx, QSize(496, 396));
        QCOMPARE(e.imageOffsetPx, QPoint(40, 40));
        QCOMPARE(e.objectShiftLogical, QPoint(40, 40));
        QCOMPARE(e.newSelectionLogical, QRect(8, 8, 480, 380));
        QCOMPARE(e.newSelectionDevice, QRect(8, 8, 480, 380));
    }

    void expandScalesByDpr2()
    {
        const CanvasExpansion e = expandCanvas(
          QSize(832, 632), QRect(8, 8, 400, 300), QRect(16, 16, 800, 600), 40, 2.0);
        QCOMPARE(e.newCanvasPx, QSize(992, 792));
        QCOMPARE(e.imageOffsetPx, QPoint(80, 80));
        QCOMPARE(e.objectShiftLogical, QPoint(40, 40));
        QCOMPARE(e.newSelectionLogical, QRect(8, 8, 480, 380));
        QCOMPARE(e.newSelectionDevice, QRect(16, 16, 960, 760));
    }

    void expandRepeatedAccumulates()
    {
        const CanvasExpansion a = expandCanvas(
          QSize(416, 316), QRect(8, 8, 400, 300), QRect(8, 8, 400, 300), 40, 1.0);
        const CanvasExpansion b = expandCanvas(a.newCanvasPx,
                                               a.newSelectionLogical,
                                               a.newSelectionDevice,
                                               40,
                                               1.0);
        QCOMPARE(b.newCanvasPx, QSize(576, 476));
        QCOMPARE(b.newSelectionLogical, QRect(8, 8, 560, 460));
        QCOMPARE(b.newSelectionDevice, QRect(8, 8, 560, 460));
        QCOMPARE(b.imageOffsetPx, QPoint(40, 40));
    }

    void expandNegativeStepIsInverse()
    {
        for (qreal dpr : { 1.0, 2.0 }) {
            const QSize canvas(QSize(416, 316) * dpr);
            const QRect selL(8, 8, 400, 300);
            const QRect selD(QPoint(qRound(8 * dpr), qRound(8 * dpr)),
                             QSize(qRound(400 * dpr), qRound(300 * dpr)));
            const CanvasExpansion up =
              expandCanvas(canvas, selL, selD, 40, dpr);
            const CanvasExpansion down = expandCanvas(
              up.newCanvasPx, up.newSelectionLogical, up.newSelectionDevice, -40, dpr);
            QCOMPARE(down.newCanvasPx, canvas);
            QCOMPARE(down.newSelectionLogical, selL);
            QCOMPARE(down.newSelectionDevice, selD);
            QCOMPARE(down.objectShiftLogical, -up.objectShiftLogical);
            QCOMPARE(down.imageOffsetPx, -up.imageOffsetPx);
        }
    }

    void expandRepeatedThenInverseTwiceRoundTrips()
    {
        const QSize canvas(832, 632);
        const QRect selL(8, 8, 400, 300);
        const QRect selD(16, 16, 800, 600);
        CanvasExpansion s1 = expandCanvas(canvas, selL, selD, 40, 2.0);
        CanvasExpansion s2 = expandCanvas(
          s1.newCanvasPx, s1.newSelectionLogical, s1.newSelectionDevice, 40, 2.0);
        CanvasExpansion d1 = expandCanvas(
          s2.newCanvasPx, s2.newSelectionLogical, s2.newSelectionDevice, -40, 2.0);
        CanvasExpansion d2 = expandCanvas(
          d1.newCanvasPx, d1.newSelectionLogical, d1.newSelectionDevice, -40, 2.0);
        QCOMPARE(d2.newCanvasPx, canvas);
        QCOMPARE(d2.newSelectionLogical, selL);
        QCOMPARE(d2.newSelectionDevice, selD);
    }

    // The original image (selection top-left plus the step) never moves
    // relative to the objects: both shift by the same logical step.
    void expandOffsetsMatchAtFractionalDpr()
    {
        const CanvasExpansion e = expandCanvas(
          QSize(300, 200), QRect(8, 8, 100, 50), QRect(12, 12, 150, 75), 40, 1.5);
        QCOMPARE(e.imageOffsetPx, QPoint(60, 60));
        QCOMPARE(e.newCanvasPx, QSize(420, 320));
        QCOMPARE(e.newSelectionDevice, QRect(12, 12, 270, 195));
    }

    void expandZeroStepIsNoOp()
    {
        const CanvasExpansion e = expandCanvas(
          QSize(416, 316), QRect(8, 8, 400, 300), QRect(8, 8, 400, 300), 0, 1.0);
        QCOMPARE(e.newCanvasPx, QSize(416, 316));
        QCOMPARE(e.imageOffsetPx, QPoint(0, 0));
        QCOMPARE(e.newSelectionLogical, QRect(8, 8, 400, 300));
    }

    void expandInvalidDprFallsBackToOne()
    {
        const CanvasExpansion e = expandCanvas(
          QSize(416, 316), QRect(8, 8, 400, 300), QRect(8, 8, 400, 300), 40, 0.0);
        QCOMPARE(e.imageOffsetPx, QPoint(40, 40));
    }
};

QTEST_MAIN(TestEditCanvas)
#include "tst_editcanvas.moc"
