// SPDX-License-Identifier: GPL-3.0-or-later
// Tests for the scrolling-capture state machine and hotkey mapping.

#include "core/scrollcapturestate.h"

#include <QTest>

Q_DECLARE_METATYPE(ScrollState)

class TestScrollCaptureState : public QObject
{
    Q_OBJECT
private:
    static QList<ScrollState> allStates()
    {
        return { ScrollState::Idle,      ScrollState::Picking,
                 ScrollState::Scrolling, ScrollState::Done,
                 ScrollState::Cancelled, ScrollState::Failed };
    }

    static bool isLegal(ScrollState a, ScrollState b)
    {
        using S = ScrollState;
        const QList<QPair<S, S>> legal = {
            { S::Idle, S::Picking },        { S::Picking, S::Scrolling },
            { S::Picking, S::Idle },        { S::Picking, S::Failed },
            { S::Scrolling, S::Done },      { S::Scrolling, S::Cancelled },
            { S::Scrolling, S::Failed },    { S::Done, S::Idle },
            { S::Cancelled, S::Idle },      { S::Failed, S::Idle },
        };
        return legal.contains({ a, b });
    }

private slots:
    void transitionTableIsExact()
    {
        for (ScrollState a : allStates()) {
            for (ScrollState b : allStates()) {
                QVERIFY2(canTransition(a, b) == isLegal(a, b),
                         qPrintable(QStringLiteral("%1 -> %2")
                                      .arg(static_cast<int>(a))
                                      .arg(static_cast<int>(b))));
            }
        }
    }

    void illegalMoveChangesNothing()
    {
        ScrollCaptureMachine m;
        QVERIFY(!m.transition(ScrollState::Done));
        QCOMPARE(m.state(), ScrollState::Idle);
    }

    void fullManualRun()
    {
        ScrollCaptureMachine m;
        QVERIFY(m.transition(ScrollState::Picking));
        QVERIFY(m.transition(ScrollState::Scrolling));
        QCOMPARE(m.mode(), ScrollMode::Manual);
        QVERIFY(m.transition(ScrollState::Done));
        QVERIFY(m.transition(ScrollState::Idle));
    }

    void modeOnlyChangesWhileScrolling()
    {
        ScrollCaptureMachine m;
        QVERIFY(!m.setMode(ScrollMode::Auto));
        m.transition(ScrollState::Picking);
        QVERIFY(!m.setMode(ScrollMode::Auto));
        m.transition(ScrollState::Scrolling);
        QVERIFY(m.setMode(ScrollMode::Auto));
        QCOMPARE(m.mode(), ScrollMode::Auto);
        QVERIFY(m.setMode(ScrollMode::Manual));
        QVERIFY(m.setMode(ScrollMode::Auto));
        m.transition(ScrollState::Cancelled);
        m.transition(ScrollState::Idle);
        // A new capture always starts manual
        m.transition(ScrollState::Picking);
        m.transition(ScrollState::Scrolling);
        QCOMPARE(m.mode(), ScrollMode::Manual);
    }

    void failureKeepsTheMessageUntilNextPick()
    {
        ScrollCaptureMachine m;
        m.transition(ScrollState::Picking);
        m.transition(ScrollState::Scrolling);
        QVERIFY(m.transition(ScrollState::Failed, QStringLiteral("boom")));
        QCOMPARE(m.lastError(), QStringLiteral("boom"));
        m.transition(ScrollState::Idle);
        QCOMPARE(m.lastError(), QStringLiteral("boom"));
        m.transition(ScrollState::Picking);
        QCOMPARE(m.lastError(), QString());
    }

    void hotkeyStartsOrFinishes()
    {
        QCOMPARE(onScrollHotkey(ScrollState::Idle),
                 ScrollHotkeyEffect::StartPicking);
        QCOMPARE(onScrollHotkey(ScrollState::Scrolling),
                 ScrollHotkeyEffect::Finish);
        for (ScrollState s : { ScrollState::Picking,
                               ScrollState::Done,
                               ScrollState::Cancelled,
                               ScrollState::Failed }) {
            QCOMPARE(onScrollHotkey(s), ScrollHotkeyEffect::Ignore);
        }
    }
};

QTEST_MAIN(TestScrollCaptureState)
#include "tst_scrollcapturestate.moc"
