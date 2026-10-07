// SPDX-License-Identifier: GPL-3.0-or-later
// Tests for recording geometry helpers.

#include "core/recordinggeometry.h"

#include <QTest>

class TestRecordingGeometry : public QObject
{
    Q_OBJECT
private slots:
    void displayLocalOnPrimaryIsUnchanged()
    {
        QCOMPARE(displayLocalRect(QRect(100, 200, 300, 150),
                                  QRect(0, 0, 1512, 982)),
                 QRect(100, 200, 300, 150));
    }

    void displayLocalOnRightHandSecondary()
    {
        QCOMPARE(displayLocalRect(QRect(1600, 120, 400, 300),
                                  QRect(1512, 0, 1920, 1080)),
                 QRect(88, 120, 400, 300));
    }

    void displayLocalOnLeftAndAboveSecondary()
    {
        // Secondary display placed left of and above the primary
        QCOMPARE(displayLocalRect(QRect(-1800, -900, 200, 100),
                                  QRect(-1920, -1080, 1920, 1080)),
                 QRect(120, 180, 200, 100));
    }

    void outputPixelsAtScaleOne()
    {
        QCOMPARE(outputPixels(QSize(640, 480), 1.0), QSize(640, 480));
    }

    void outputPixelsRetinaScaleTwo()
    {
        QCOMPARE(outputPixels(QSize(640, 480), 2.0), QSize(1280, 960));
        QCOMPARE(outputPixels(QSize(101, 51), 2.0), QSize(202, 102));
    }

    void outputPixelsRoundsOddToEven()
    {
        QCOMPARE(outputPixels(QSize(101, 99), 1.0), QSize(102, 100));
        // 101 * 1.5 = 151.5, rounded to the nearest even number
        QCOMPARE(outputPixels(QSize(101, 101), 1.5), QSize(152, 152));
    }

    void outputPixelsNeverBelowTwo()
    {
        QCOMPARE(outputPixels(QSize(0, 1), 1.0), QSize(2, 2));
    }

    void controlGoesBelowWhenThereIsRoom()
    {
        const QRect recorded(400, 300, 400, 200); // bottom edge y = 499
        const QSize control(180, 36);
        const QRect screen(0, 0, 1440, 900);
        const QPoint p = controlPosition(recorded, control, screen, 8);
        QCOMPARE(p.y(), 508); // 500 + gap
        // Centred under the area
        QCOMPARE(p.x(), 600 - 90);
    }

    void controlGoesAboveNearBottomEdge()
    {
        const QRect recorded(400, 700, 400, 180); // bottom edge y = 879
        const QSize control(180, 36);
        const QRect screen(0, 0, 1440, 900);
        const QPoint p = controlPosition(recorded, control, screen, 8);
        QCOMPARE(p.y(), 700 - 8 - 36);
    }

    void controlGoesInsideBottomWhenAreaFillsTheScreen()
    {
        const QRect recorded(0, 0, 1440, 900);
        const QSize control(180, 36);
        const QRect screen(0, 0, 1440, 900);
        const QPoint p = controlPosition(recorded, control, screen, 8);
        QVERIFY(screen.contains(QRect(p, control)));
        QVERIFY(recorded.contains(QRect(p, control)));
        QCOMPARE(p.y() + control.height(), 900 - 8);
    }

    void controlIsClampedHorizontally()
    {
        const QRect screen(100, 50, 1000, 800);
        const QSize control(180, 36);
        const QPoint left =
          controlPosition(QRect(100, 100, 40, 40), control, screen, 8);
        QCOMPARE(left.x(), 100);
        const QPoint right =
          controlPosition(QRect(1060, 100, 40, 40), control, screen, 8);
        QCOMPARE(right.x(), 1100 - 180);
    }

    void controlStaysOnSecondaryScreen()
    {
        const QRect screen(1512, 0, 1920, 1080);
        const QRect recorded(1600, 900, 300, 170); // bottom edge y = 1069
        const QSize control(180, 36);
        const QPoint p = controlPosition(recorded, control, screen, 8);
        QVERIFY(screen.contains(QRect(p, control)));
        QCOMPARE(p.y(), 900 - 8 - 36);
    }

    void controlNearTopEdgeStillGoesBelow()
    {
        const QRect recorded(300, 0, 400, 200);
        const QPoint p =
          controlPosition(recorded, QSize(180, 36), QRect(0, 0, 1440, 900), 8);
        QCOMPARE(p.y(), 208);
    }
};

QTEST_GUILESS_MAIN(TestRecordingGeometry)
#include "tst_recordinggeometry.moc"
