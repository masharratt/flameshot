// SPDX-License-Identifier: GPL-3.0-or-later
// Tests for the GIF frame sampling and size planner.

#include "core/gifplan.h"

#include <QTest>

class TestGifPlan : public QObject
{
    Q_OBJECT
private slots:
    void smallSourceKeepsItsSize()
    {
        const GifPlan p = planGif(2.0, QSize(800, 400), 10, 960);
        QCOMPARE(p.size, QSize(800, 400));
    }

    void wideSourceScalesDownKeepingAspect()
    {
        const GifPlan p = planGif(1.0, QSize(1920, 1080), 15, 960);
        QCOMPARE(p.size, QSize(960, 540));
    }

    void dimensionsAreAlwaysEven()
    {
        // 1001x501 at maxWidth 501: width rounds down to 500, never above max
        const GifPlan p = planGif(1.0, QSize(1001, 501), 15, 501);
        QCOMPARE(p.size.width() % 2, 0);
        QCOMPARE(p.size.height() % 2, 0);
        QCOMPARE(p.size.width(), 500);

        const GifPlan odd = planGif(1.0, QSize(301, 201), 15, 960);
        QCOMPARE(odd.size.width() % 2, 0);
        QCOMPARE(odd.size.height() % 2, 0);
    }

    void extremeAspectNeverCollapsesToZero()
    {
        const GifPlan p = planGif(1.0, QSize(4000, 3), 15, 400);
        QCOMPARE(p.size.width(), 400);
        QVERIFY(p.size.height() >= 2);
        QCOMPARE(p.size.height() % 2, 0);
    }

    void frameTimesStartAtZeroAndStayBelowDuration()
    {
        const GifPlan p = planGif(2.0, QSize(640, 480), 10, 960);
        QCOMPARE(p.frameTimes.size(), 20);
        QCOMPARE(p.frameTimes.first(), 0.0);
        QVERIFY(p.frameTimes.last() < 2.0);
        QVERIFY(qFuzzyCompare(p.frameTimes[1], 0.1));
        for (int i = 1; i < p.frameTimes.size(); ++i) {
            QVERIFY(p.frameTimes[i] > p.frameTimes[i - 1]);
        }
    }

    void frameDelayIsOneOverFps()
    {
        QVERIFY(qFuzzyCompare(planGif(1.0, QSize(100, 100), 10, 960).frameDelay,
                              0.1));
        QVERIFY(qFuzzyCompare(planGif(1.0, QSize(100, 100), 20, 960).frameDelay,
                              0.05));
    }

    void atLeastOneFrameForTinyOrZeroDuration()
    {
        const GifPlan tiny = planGif(0.01, QSize(100, 100), 15, 960);
        QCOMPARE(tiny.frameTimes.size(), 1);
        QCOMPARE(tiny.frameTimes.first(), 0.0);
        const GifPlan zero = planGif(0.0, QSize(100, 100), 15, 960);
        QCOMPARE(zero.frameTimes.size(), 1);
        const GifPlan negative = planGif(-3.0, QSize(100, 100), 15, 960);
        QCOMPARE(negative.frameTimes.size(), 1);
    }

    void fpsIsClampedToOneThroughFifty()
    {
        const GifPlan low = planGif(3.0, QSize(100, 100), 0, 960);
        QVERIFY(qFuzzyCompare(low.frameDelay, 1.0));
        QCOMPARE(low.frameTimes.size(), 3);

        const GifPlan high = planGif(1.0, QSize(100, 100), 500, 960);
        QVERIFY(qFuzzyCompare(high.frameDelay, 1.0 / 50.0));
        QCOMPARE(high.frameTimes.size(), 50);
    }

    void fractionalDurationRoundsFrameCountUp()
    {
        // 0.25 s at 10 fps: frames at 0, 0.1, 0.2 (all below 0.25)
        const GifPlan p = planGif(0.25, QSize(100, 100), 10, 960);
        QCOMPARE(p.frameTimes.size(), 3);
        QVERIFY(p.frameTimes.last() < 0.25);
    }
};

QTEST_GUILESS_MAIN(TestGifPlan)
#include "tst_gifplan.moc"
