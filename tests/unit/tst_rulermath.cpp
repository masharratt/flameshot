// SPDX-License-Identifier: GPL-3.0-or-later
// Tests for the pure ruler measurement math.

#include "tools/ruler/rulermath.h"

#include <QPointF>
#include <QTest>

using namespace rulermath;

class TestRulerMath : public QObject
{
    Q_OBJECT
private slots:
    void horizontal()
    {
        const Measurement m = measure(QPointF(10, 20), QPointF(210, 20), 1.0);
        QCOMPARE(m.dx, 200.0);
        QCOMPARE(m.dy, 0.0);
        QCOMPARE(m.length, 200.0);
        QCOMPARE(m.angleDegrees, 0.0);
    }

    void vertical()
    {
        // Screen y grows downward; angle uses the y-up convention.
        const Measurement m = measure(QPointF(5, 5), QPointF(5, 105), 1.0);
        QCOMPARE(m.dx, 0.0);
        QCOMPARE(m.dy, 100.0);
        QCOMPARE(m.length, 100.0);
        QCOMPARE(m.angleDegrees, -90.0);
    }

    void diagonal()
    {
        const Measurement m = measure(QPointF(0, 0), QPointF(3, 4), 1.0);
        QCOMPARE(m.dx, 3.0);
        QCOMPARE(m.dy, 4.0);
        QCOMPARE(m.length, 5.0);
        QVERIFY(qAbs(m.angleDegrees - (-53.13010235)) < 1e-6);
    }

    void diagonalFortyFive()
    {
        const Measurement m = measure(QPointF(0, 0), QPointF(10, -10), 1.0);
        QVERIFY(qAbs(m.angleDegrees - 45.0) < 1e-9);
    }

    void zeroLength()
    {
        const Measurement m = measure(QPointF(7, 7), QPointF(7, 7), 1.0);
        QCOMPARE(m.length, 0.0);
        QCOMPARE(m.dx, 0.0);
        QCOMPARE(m.dy, 0.0);
        QCOMPARE(m.angleDegrees, 0.0);
        QCOMPARE(labelText(m), QStringLiteral("0 px (0 × 0)"));
    }

    void negativeDirection()
    {
        const Measurement m = measure(QPointF(210, 90), QPointF(10, 20), 1.0);
        QCOMPARE(m.dx, -200.0);
        QCOMPARE(m.dy, -70.0);
        QVERIFY(qAbs(m.length - std::hypot(200.0, 70.0)) < 1e-9);
        QVERIFY(m.angleDegrees > 90.0 && m.angleDegrees <= 180.0);
        // Label shows magnitudes only.
        QCOMPARE(labelText(m), QStringLiteral("212 px (200 × 70)"));
    }

    void dprScalesToPhysicalPixels()
    {
        const Measurement m = measure(QPointF(0, 0), QPointF(100, 50), 2.0);
        QCOMPARE(m.dx, 200.0);
        QCOMPARE(m.dy, 100.0);
        QVERIFY(qAbs(m.length - std::hypot(200.0, 100.0)) < 1e-9);
    }

    void dprDoesNotChangeAngle()
    {
        const Measurement a = measure(QPointF(0, 0), QPointF(30, -40), 1.0);
        const Measurement b = measure(QPointF(0, 0), QPointF(30, -40), 2.0);
        QVERIFY(qAbs(a.angleDegrees - b.angleDegrees) < 1e-9);
    }

    void invalidDprFallsBackToOne()
    {
        const Measurement m = measure(QPointF(0, 0), QPointF(10, 0), 0.0);
        QCOMPARE(m.dx, 10.0);
    }

    void labelFormat()
    {
        const Measurement m = measure(QPointF(10, 20), QPointF(210, 90), 1.0);
        QCOMPARE(labelText(m), QStringLiteral("212 px (200 × 70)"));
    }

    void labelFormatDpr()
    {
        const Measurement m = measure(QPointF(0, 0), QPointF(100, 0), 2.0);
        QCOMPARE(labelText(m), QStringLiteral("200 px (200 × 0)"));
    }
};

QTEST_GUILESS_MAIN(TestRulerMath)
#include "tst_rulermath.moc"
