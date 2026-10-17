// SPDX-License-Identifier: GPL-3.0-or-later
// Tests for the capture timer logic: which delay applies and the countdown
// lifecycle.

#include "core/capturedelay.h"

#include <QTest>

class TestCaptureDelay : public QObject
{
    Q_OBJECT
private slots:
    void configDelayUsedWhenRequestHasNone()
    {
        QCOMPARE(captureDelayMs(0, 3), 3000u);
    }

    void explicitRequestDelayWinsOverConfig()
    {
        QCOMPARE(captureDelayMs(500, 3), 500u);
    }

    void zeroConfigMeansNoDelay()
    {
        QCOMPARE(captureDelayMs(0, 0), 0u);
    }

    void negativeConfigIsTreatedAsZero()
    {
        QCOMPARE(captureDelayMs(0, -4), 0u);
    }

    void configIsClampedToTenSeconds()
    {
        QCOMPARE(captureDelayMs(0, 99), 10000u);
    }

    void countdownHappyPath()
    {
        CountdownStateMachine m;
        QCOMPARE(m.state(), CountdownState::Idle);
        QVERIFY(m.transition(CountdownState::Counting));
        QVERIFY(m.transition(CountdownState::Capturing));
        QVERIFY(m.transition(CountdownState::Idle));
    }

    void countdownCanBeCancelled()
    {
        CountdownStateMachine m;
        QVERIFY(m.transition(CountdownState::Counting));
        QVERIFY(m.transition(CountdownState::Cancelled));
        QVERIFY(m.transition(CountdownState::Idle));
    }

    void illegalMovesChangeNothing()
    {
        CountdownStateMachine m;
        QVERIFY(!m.transition(CountdownState::Capturing));
        QCOMPARE(m.state(), CountdownState::Idle);
        QVERIFY(m.transition(CountdownState::Counting));
        // A second hotkey press while counting must not restart the timer
        QVERIFY(!m.transition(CountdownState::Counting));
        QCOMPARE(m.state(), CountdownState::Counting);
    }

    void secondsLeftRoundsUp()
    {
        QCOMPARE(countdownSecondsLeft(3000), 3);
        QCOMPARE(countdownSecondsLeft(2001), 3);
        QCOMPARE(countdownSecondsLeft(1), 1);
        QCOMPARE(countdownSecondsLeft(0), 0);
    }
};

QTEST_GUILESS_MAIN(TestCaptureDelay)
#include "tst_capturedelay.moc"
