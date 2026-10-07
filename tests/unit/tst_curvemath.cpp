// SPDX-License-Identifier: GPL-3.0-or-later
// Tests for the pure quadratic curve math behind bendable arrows and lines.

#include "tools/curvemath.h"

#include <QPointF>
#include <QRectF>
#include <QTest>
#include <cmath>

using namespace curvemath;

namespace {
bool near(const QPointF& p, const QPointF& q, double eps = 1e-9)
{
    return std::abs(p.x() - q.x()) < eps && std::abs(p.y() - q.y()) < eps;
}
}

class TestCurveMath : public QObject
{
    Q_OBJECT
private slots:
    void handleControlRoundTrip()
    {
        const QPointF a(10, 20), b(210, 80), h(120, -40);
        const QPointF c = controlFromHandle(a, b, h);
        QVERIFY(near(handleFromControl(a, b, c), h));
    }

    void controlFormula()
    {
        // control = 2 * handle - (a + b) / 2
        const QPointF a(0, 0), b(100, 0), h(50, -30);
        QVERIFY(near(controlFromHandle(a, b, h), QPointF(50, -60)));
    }

    void curvePassesThroughHandle()
    {
        const QPointF a(5, 5), b(305, 105), h(100, 200);
        const QPointF c = controlFromHandle(a, b, h);
        QVERIFY(near(quadPoint(a, c, b, 0.5), h));
        QVERIFY(near(quadPoint(a, c, b, 0.0), a));
        QVERIFY(near(quadPoint(a, c, b, 1.0), b));
    }

    void straightTangentEqualsChordAngle()
    {
        const QPointF a(10, 10), b(110, 60);
        const QPointF mid = (a + b) / 2;
        const double expected =
          std::atan2(b.y() - a.y(), b.x() - a.x()) * 180.0 / M_PI;
        const double got = endTangentAngle(a, controlFromHandle(a, b, mid), b);
        QVERIFY(std::abs(got - expected) < 1e-9);
    }

    void bentTangentDiffersFromChord()
    {
        const QPointF a(0, 0), b(100, 0);
        // Arch up (negative y): the curve arrives at b heading down-right.
        const QPointF c = controlFromHandle(a, b, QPointF(50, -40));
        const double angle = endTangentAngle(a, c, b);
        QVERIFY(angle > 30.0 && angle < 90.0);
    }

    void tangentDegenerateControlFallsBack()
    {
        const QPointF a(0, 0), b(100, 0);
        QVERIFY(std::abs(endTangentAngle(a, b, b)) < 1e-9);
    }

    void boundsOfSymmetricArch()
    {
        const QPointF a(0, 100), b(100, 100);
        const QPointF c(50, 0); // peak of the curve is at y = 50
        const QRectF r = curveBounds(a, c, b);
        QVERIFY(std::abs(r.left() - 0.0) < 1e-9);
        QVERIFY(std::abs(r.right() - 100.0) < 1e-9);
        QVERIFY(std::abs(r.top() - 50.0) < 1e-9);
        QVERIFY(std::abs(r.bottom() - 100.0) < 1e-9);
        // The control polygon would reach y = 0; the curve must not.
        QVERIFY(r.top() > 1.0);
    }

    void boundsOfStraightLine()
    {
        const QPointF a(10, 10), b(60, 30);
        const QRectF r = curveBounds(a, controlFromHandle(a, b, (a + b) / 2), b);
        QVERIFY(std::abs(r.left() - 10.0) < 1e-9);
        QVERIFY(std::abs(r.right() - 60.0) < 1e-9);
        QVERIFY(std::abs(r.top() - 10.0) < 1e-9);
        QVERIFY(std::abs(r.bottom() - 30.0) < 1e-9);
    }

    void snapTolerance()
    {
        const QPointF a(0, 0), b(100, 0);
        QVERIFY(isStraight(a, b, QPointF(50, 0), 3.0));
        QVERIFY(isStraight(a, b, QPointF(52, 2), 3.0));
        QVERIFY(!isStraight(a, b, QPointF(50, 4), 3.0));
        QVERIFY(!isStraight(a, b, QPointF(54, 0), 3.0));
    }

    void distanceZeroOnCurvePositiveOff()
    {
        const QPointF a(0, 0), b(100, 0);
        const QPointF h(50, -40);
        const QPointF c = controlFromHandle(a, b, h);
        QVERIFY(distanceToCurve(h, a, c, b) < 0.1);
        QVERIFY(distanceToCurve(a, a, c, b) < 0.1);
        QVERIFY(distanceToCurve(QPointF(50, 0), a, c, b) > 10.0);
        QVERIFY(distanceToCurve(QPointF(50, -60), a, c, b) > 10.0);
    }
};

QTEST_APPLESS_MAIN(TestCurveMath)
#include "tst_curvemath.moc"
