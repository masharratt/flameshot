// SPDX-License-Identifier: GPL-3.0-or-later
// Tests for the options a recording starts with.

#include "core/recordingoptions.h"

#include <QTest>

class TestRecordingOptions : public QObject
{
    Q_OBJECT
private slots:
    void optionsCarryShowClicksOn()
    {
        QVERIFY(recordingOptionsFor(true).showClicks);
    }

    void optionsCarryShowClicksOff()
    {
        QVERIFY(!recordingOptionsFor(false).showClicks);
    }

    void defaultOptionsShowClicks()
    {
        QVERIFY(RecordingOptions().showClicks);
    }

    void clicksDrawnOnlyWhenWantedAndOsCan()
    {
        QVERIFY(clicksWillBeDrawn(recordingOptionsFor(true), true));
        // Older macOS: no highlights, but recording still works
        QVERIFY(!clicksWillBeDrawn(recordingOptionsFor(true), false));
        QVERIFY(!clicksWillBeDrawn(recordingOptionsFor(false), true));
        QVERIFY(!clicksWillBeDrawn(recordingOptionsFor(false), false));
    }
};

QTEST_GUILESS_MAIN(TestRecordingOptions)
#include "tst_recordingoptions.moc"
