// SPDX-License-Identifier: GPL-3.0-or-later
// Tests for the screen recording state machine and hotkey mapping.

#include "core/recordingstate.h"

#include <QTest>

Q_DECLARE_METATYPE(RecState)

class TestRecordingState : public QObject
{
    Q_OBJECT
private:
    static QList<RecState> allStates()
    {
        return { RecState::Idle,      RecState::Selecting, RecState::Starting,
                 RecState::Recording, RecState::Stopping,  RecState::Converting,
                 RecState::Done,      RecState::Failed,    RecState::Cancelled };
    }

    static QString stateName(RecState s)
    {
        static const QStringList names = {
            "idle",     "selecting",  "starting", "recording", "stopping",
            "converting", "done",     "failed",   "cancelled"
        };
        return names.value(static_cast<int>(s));
    }

private slots:
    void legalTransitionsAreAccepted_data()
    {
        QTest::addColumn<RecState>("from");
        QTest::addColumn<RecState>("to");
        const auto row = [](const char* name, RecState a, RecState b) {
            QTest::newRow(name) << a << b;
        };
        row("idle>selecting", RecState::Idle, RecState::Selecting);
        row("selecting>starting", RecState::Selecting, RecState::Starting);
        row("selecting>cancelled", RecState::Selecting, RecState::Cancelled);
        row("selecting>idle", RecState::Selecting, RecState::Idle);
        row("starting>recording", RecState::Starting, RecState::Recording);
        row("starting>failed", RecState::Starting, RecState::Failed);
        // Regression: Cancel during the start delay left the control window
        // open and the temp file behind because this transition was refused.
        row("starting>cancelled", RecState::Starting, RecState::Cancelled);
        row("recording>stopping", RecState::Recording, RecState::Stopping);
        row("recording>cancelled", RecState::Recording, RecState::Cancelled);
        row("recording>failed", RecState::Recording, RecState::Failed);
        row("stopping>converting", RecState::Stopping, RecState::Converting);
        row("stopping>done", RecState::Stopping, RecState::Done);
        row("stopping>failed", RecState::Stopping, RecState::Failed);
        row("converting>done", RecState::Converting, RecState::Done);
        row("converting>failed", RecState::Converting, RecState::Failed);
        row("done>idle", RecState::Done, RecState::Idle);
        row("failed>idle", RecState::Failed, RecState::Idle);
        row("cancelled>idle", RecState::Cancelled, RecState::Idle);
    }

    void legalTransitionsAreAccepted()
    {
        QFETCH(RecState, from);
        QFETCH(RecState, to);
        QVERIFY(canTransition(from, to));
    }

    // Compares the whole legal set by name, so a failure lists exactly which
    // transitions are missing or unexpected instead of a bare count.
    void exactlyTheListedTransitionsAreLegal()
    {
        QStringList actual;
        for (RecState a : allStates()) {
            for (RecState b : allStates()) {
                if (canTransition(a, b)) {
                    actual << stateName(a) + ">" + stateName(b);
                }
            }
        }
        const QStringList expected = {
            "idle>selecting",      "selecting>starting", "selecting>cancelled",
            "selecting>idle",      "starting>recording", "starting>failed",
            "starting>cancelled",  "recording>stopping", "recording>cancelled",
            "recording>failed",    "stopping>converting", "stopping>done",
            "stopping>failed",     "converting>done",    "converting>failed",
            "done>idle",           "failed>idle",        "cancelled>idle"
        };
        QStringList missing, unexpected;
        for (const QString& t : expected) {
            if (!actual.contains(t)) {
                missing << t;
            }
        }
        for (const QString& t : actual) {
            if (!expected.contains(t)) {
                unexpected << t;
            }
        }
        QVERIFY2(missing.isEmpty() && unexpected.isEmpty(),
                 qPrintable(QStringLiteral("missing=[%1] unexpected=[%2]")
                              .arg(missing.join(", "), unexpected.join(", "))));
    }

    void illegalTransitionsAreRejected()
    {
        QVERIFY(!canTransition(RecState::Idle, RecState::Recording));
        QVERIFY(!canTransition(RecState::Idle, RecState::Idle));
        QVERIFY(!canTransition(RecState::Selecting, RecState::Recording));
        QVERIFY(!canTransition(RecState::Starting, RecState::Stopping));
        QVERIFY(!canTransition(RecState::Recording, RecState::Done));
        QVERIFY(!canTransition(RecState::Recording, RecState::Recording));
        QVERIFY(!canTransition(RecState::Stopping, RecState::Cancelled));
        QVERIFY(!canTransition(RecState::Converting, RecState::Cancelled));
        QVERIFY(!canTransition(RecState::Done, RecState::Recording));
        QVERIFY(!canTransition(RecState::Failed, RecState::Selecting));
        QVERIFY(!canTransition(RecState::Cancelled, RecState::Done));
    }

    void machineStartsIdleAndWalksHappyPath()
    {
        RecordingStateMachine m;
        QCOMPARE(m.state(), RecState::Idle);
        QVERIFY(m.transition(RecState::Selecting));
        QVERIFY(m.transition(RecState::Starting));
        QVERIFY(m.transition(RecState::Recording));
        QVERIFY(m.transition(RecState::Stopping));
        QVERIFY(m.transition(RecState::Converting));
        QVERIFY(m.transition(RecState::Done));
        QVERIFY(m.transition(RecState::Idle));
        QCOMPARE(m.state(), RecState::Idle);
        QVERIFY(m.lastError().isEmpty());
    }

    void machineKeepsStateOnIllegalTransition()
    {
        RecordingStateMachine m;
        QVERIFY(!m.transition(RecState::Recording));
        QCOMPARE(m.state(), RecState::Idle);
        QVERIFY(m.transition(RecState::Selecting));
        QVERIFY(!m.transition(RecState::Done));
        QCOMPARE(m.state(), RecState::Selecting);
    }

    void failedTransitionRecordsError()
    {
        RecordingStateMachine m;
        QVERIFY(m.transition(RecState::Selecting));
        QVERIFY(m.transition(RecState::Starting));
        QVERIFY(m.transition(RecState::Failed, QStringLiteral("no permission")));
        QCOMPARE(m.lastError(), QStringLiteral("no permission"));
        // A new run clears the previous error
        QVERIFY(m.transition(RecState::Idle));
        QVERIFY(m.transition(RecState::Selecting));
        QVERIFY(m.lastError().isEmpty());
    }

    void hotkeyMapping()
    {
        QCOMPARE(onHotkey(RecState::Idle), HotkeyEffect::StartSelecting);
        QCOMPARE(onHotkey(RecState::Recording), HotkeyEffect::Stop);
        for (RecState s : { RecState::Selecting, RecState::Starting,
                            RecState::Stopping, RecState::Converting,
                            RecState::Done, RecState::Failed,
                            RecState::Cancelled }) {
            QCOMPARE(onHotkey(s), HotkeyEffect::Ignore);
        }
    }
};

QTEST_GUILESS_MAIN(TestRecordingState)
#include "tst_recordingstate.moc"
