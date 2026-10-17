// SPDX-License-Identifier: GPL-3.0-or-later
// Trim range rules and the lifecycle of the "Edit recording" window.

#include "core/recordingedit.h"

#include <QTest>

namespace {
constexpr double kMin = kMinTrimSeconds;
}

class TestRecordingEdit : public QObject
{
    Q_OBJECT
private slots:
    void fullRangeByDefault()
    {
        const TrimRange r = fullTrim(4.0);
        QCOMPARE(r.start, 0.0);
        QCOMPARE(r.end, 4.0);
        QVERIFY(isFullRange(r, 4.0));
        QVERIFY(isValidTrim(r, 4.0));
    }

    void clampToDuration()
    {
        const TrimRange r = clampTrim({ -1.0, 9.0 }, 4.0);
        QCOMPARE(r.start, 0.0);
        QCOMPARE(r.end, 4.0);
    }

    void swappedEndsAreOrdered()
    {
        const TrimRange r = clampTrim({ 3.0, 1.0 }, 4.0);
        QCOMPARE(r.start, 1.0);
        QCOMPARE(r.end, 3.0);
    }

    void tooShortGrowsToMinimum()
    {
        const TrimRange r = clampTrim({ 2.0, 2.05 }, 4.0);
        QCOMPARE(r.start, 2.0);
        QCOMPARE(r.end, 2.0 + kMin);
        // At the very end the start moves back instead
        const TrimRange e = clampTrim({ 3.99, 4.0 }, 4.0);
        QCOMPARE(e.end, 4.0);
        QCOMPARE(e.start, 4.0 - kMin);
    }

    void clipShorterThanMinimumKeepsEverything()
    {
        const TrimRange r = clampTrim({ 0.05, 0.1 }, 0.2);
        QCOMPARE(r.start, 0.0);
        QCOMPARE(r.end, 0.2);
    }

    void unknownDurationGivesEmptyRange()
    {
        const TrimRange r = clampTrim({ 1.0, 2.0 }, 0.0);
        QCOMPARE(r.start, 0.0);
        QCOMPARE(r.end, 0.0);
        QVERIFY(!isValidTrim(r, 0.0));
    }

    void validity()
    {
        QVERIFY(isValidTrim({ 0.5, 1.5 }, 2.0));
        QVERIFY(!isValidTrim({ 1.5, 0.5 }, 2.0));
        QVERIFY(!isValidTrim({ 0.5, 0.55 }, 2.0));
        QVERIFY(!isValidTrim({ -0.1, 1.0 }, 2.0));
        QVERIFY(!isValidTrim({ 0.5, 2.5 }, 2.0));
        QVERIFY(!isFullRange({ 0.5, 2.0 }, 2.0));
    }

    void dragStartStopsBeforeEnd()
    {
        const TrimRange r{ 1.0, 3.0 };
        QCOMPARE(moveTrimStart(r, 2.0, 4.0).start, 2.0);
        QCOMPARE(moveTrimStart(r, 3.5, 4.0).start, 3.0 - kMin);
        QCOMPARE(moveTrimStart(r, -2.0, 4.0).start, 0.0);
        QCOMPARE(moveTrimStart(r, 3.5, 4.0).end, 3.0);
    }

    void dragEndStopsAfterStart()
    {
        const TrimRange r{ 1.0, 3.0 };
        QCOMPARE(moveTrimEnd(r, 2.0, 4.0).end, 2.0);
        QCOMPARE(moveTrimEnd(r, 0.5, 4.0).end, 1.0 + kMin);
        QCOMPARE(moveTrimEnd(r, 9.0, 4.0).end, 4.0);
        QCOMPARE(moveTrimEnd(r, 0.5, 4.0).start, 1.0);
    }

    void timelineMapping()
    {
        QCOMPARE(timeAtX(0, 200, 4.0), 0.0);
        QCOMPARE(timeAtX(100, 200, 4.0), 2.0);
        QCOMPARE(timeAtX(250, 200, 4.0), 4.0);
        QCOMPARE(timeAtX(-5, 200, 4.0), 0.0);
        QCOMPARE(xAtTime(2.0, 200, 4.0), 100);
        QCOMPARE(xAtTime(1.0, 200, 0.0), 0);
    }

    void exactlyTheListedTransitionsAreLegal()
    {
        using S = EditState;
        const QList<S> all = { S::Editing, S::Exporting, S::Converting,
                               S::Saved,   S::Failed,    S::Closed };
        const QList<std::pair<S, S>> legal = {
            { S::Editing, S::Exporting },  { S::Editing, S::Closed },
            { S::Exporting, S::Converting }, { S::Exporting, S::Saved },
            { S::Exporting, S::Failed },   { S::Converting, S::Saved },
            { S::Converting, S::Failed },  { S::Failed, S::Editing },
            { S::Failed, S::Closed },      { S::Saved, S::Closed },
        };
        QStringList wrong;
        for (S from : all) {
            for (S to : all) {
                const bool expected = legal.contains({ from, to });
                if (canEditTransition(from, to) != expected) {
                    wrong << QStringLiteral("%1->%2=%3")
                               .arg(int(from))
                               .arg(int(to))
                               .arg(!expected);
                }
            }
        }
        QVERIFY2(wrong.isEmpty(), qPrintable(wrong.join(", ")));
    }

    void machineWalksGifPath()
    {
        EditStateMachine m;
        QCOMPARE(m.state(), EditState::Editing);
        QVERIFY(m.isInteractive());
        QVERIFY(m.transition(EditState::Exporting));
        QVERIFY(!m.isInteractive());
        QVERIFY(!m.transition(EditState::Closed));
        QCOMPARE(m.state(), EditState::Exporting);
        QVERIFY(m.transition(EditState::Converting));
        QVERIFY(m.transition(EditState::Saved));
        QVERIFY(m.transition(EditState::Closed));
    }

    void failureKeepsErrorAndAllowsRetry()
    {
        EditStateMachine m;
        m.transition(EditState::Exporting);
        QVERIFY(m.transition(EditState::Failed, QStringLiteral("disk full")));
        QCOMPARE(m.lastError(), QStringLiteral("disk full"));
        QVERIFY(m.transition(EditState::Editing));
        QVERIFY(m.isInteractive());
        QVERIFY(m.lastError().isEmpty());
    }
};

QTEST_MAIN(TestRecordingEdit)
#include "tst_recordingedit.moc"
