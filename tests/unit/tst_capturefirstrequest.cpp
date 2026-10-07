// SPDX-License-Identifier: GPL-3.0-or-later
// Tests for the pure capture-first request builder.

#include "core/capturefirstrequest.h"

#include <QTest>

class TestCaptureFirstRequest : public QObject
{
    Q_OBJECT
private slots:
    void tasksAreExact()
    {
        // The workflow's actions replace the SAVE and COPY tasks
        const CaptureRequest r =
          buildCaptureFirstRequest("TAKE_SCREENSHOT", "/tmp/shots", "/pics");
        QCOMPARE(static_cast<int>(r.tasks()),
                 static_cast<int>(CaptureRequest::ACCEPT_ON_SELECT));
        QCOMPARE(r.captureMode(), CaptureRequest::GRAPHICAL_MODE);
    }

    void carriesWorkflowName()
    {
        QCOMPARE(
          buildCaptureFirstRequest("CAPTURE_AND_EDIT", "/a", "/b").workflow(),
          QString("CAPTURE_AND_EDIT"));
        QVERIFY(CaptureRequest(CaptureRequest::GRAPHICAL_MODE)
                  .workflow()
                  .isEmpty());
    }

    void usesSavePathWhenSet()
    {
        const CaptureRequest r =
          buildCaptureFirstRequest("TAKE_SCREENSHOT", "/tmp/shots", "/pics");
        QCOMPARE(r.path(), QString("/tmp/shots"));
    }

    void fallsBackWhenSavePathEmpty()
    {
        const CaptureRequest r =
          buildCaptureFirstRequest("TAKE_SCREENSHOT", QString(), "/pics");
        QCOMPARE(r.path(), QString("/pics"));
    }

    void marksRequestAsCaptureFirst()
    {
        QVERIFY(buildCaptureFirstRequest("TAKE_SCREENSHOT", "/a", "/b")
                  .captureFirst());
        QVERIFY(!CaptureRequest(CaptureRequest::GRAPHICAL_MODE).captureFirst());
    }

    void nativeFullscreenSkippedOnlyForCaptureFirst()
    {
        const CaptureRequest first =
          buildCaptureFirstRequest("TAKE_SCREENSHOT", "/a", "/b");
        const CaptureRequest plain(CaptureRequest::GRAPHICAL_MODE);
        QVERIFY(!shouldUseNativeFullscreen(first, true));
        QVERIFY(shouldUseNativeFullscreen(plain, true));
        QVERIFY(!shouldUseNativeFullscreen(plain, false));
        QVERIFY(!shouldUseNativeFullscreen(first, false));
    }

    void hasNoInitialSelection()
    {
        const CaptureRequest r =
          buildCaptureFirstRequest("TAKE_SCREENSHOT", "/a", "/b");
        QVERIFY(r.initialSelection().isNull());
    }

    void lastRegionTruthTable_data()
    {
        QTest::addColumn<bool>("acceptOnSelect");
        QTest::addColumn<bool>("settingEnabled");
        QTest::addColumn<bool>("hasSelection");
        QTest::addColumn<bool>("expected");

        QTest::newRow("plain, off, none") << false << false << false << false;
        QTest::newRow("plain, on, none") << false << true << false << true;
        QTest::newRow("plain, on, selection") << false << true << true << false;
        QTest::newRow("plain, off, selection") << false << false << true << false;
        QTest::newRow("accept, on, none") << true << true << false << false;
        QTest::newRow("accept, off, none") << true << false << false << false;
        QTest::newRow("accept, on, selection") << true << true << true << false;
        QTest::newRow("accept, off, selection") << true << false << true << false;
    }

    void lastRegionTruthTable()
    {
        QFETCH(bool, acceptOnSelect);
        QFETCH(bool, settingEnabled);
        QFETCH(bool, hasSelection);
        QFETCH(bool, expected);

        CaptureRequest r(CaptureRequest::GRAPHICAL_MODE);
        if (acceptOnSelect) {
            r.addTask(CaptureRequest::ACCEPT_ON_SELECT);
        }
        if (hasSelection) {
            r.setInitialSelection(QRect(1, 2, 30, 40));
        }
        QCOMPARE(shouldApplyLastRegion(r, settingEnabled), expected);
    }

    void lastRegionNeverForNonGraphical()
    {
        CaptureRequest r(CaptureRequest::FULLSCREEN_MODE);
        QVERIFY(!shouldApplyLastRegion(r, true));
    }

    void globalSelectionAddsOverlayOrigin()
    {
        QCOMPARE(globalSelectionRect(QRect(5, 6, 70, 80), QPoint(-1440, 0)),
                 QRect(-1435, 6, 70, 80));
    }

    void captureRequestCarriesGlobalRect()
    {
        CaptureRequest r(CaptureRequest::GRAPHICAL_MODE);
        QVERIFY(r.capturedGlobalRect().isNull());
        r.setCapturedGlobalRect(QRect(1, 2, 3, 4));
        QCOMPARE(r.capturedGlobalRect(), QRect(1, 2, 3, 4));
    }

    void recordHotkeysMapToRecordModes()
    {
        QCOMPARE(recordModeForHotkey("RECORD_MP4"), CaptureRequest::RecordMp4);
        QCOMPARE(recordModeForHotkey("RECORD_GIF"), CaptureRequest::RecordGif);
        QCOMPARE(recordModeForHotkey("TAKE_SCREENSHOT"),
                 CaptureRequest::RecordNone);
        QCOMPARE(recordModeForHotkey(QString()), CaptureRequest::RecordNone);
    }

    void recordRequestPicksAreaOnly()
    {
        const CaptureRequest r = buildRecordRequest(CaptureRequest::RecordGif);
        QCOMPARE(static_cast<int>(r.tasks()),
                 static_cast<int>(CaptureRequest::ACCEPT_ON_SELECT));
        QCOMPARE(r.recordMode(), CaptureRequest::RecordGif);
        QCOMPARE(r.captureMode(), CaptureRequest::GRAPHICAL_MODE);
        QVERIFY(r.captureFirst());
        // No image is saved, so nothing may be run for the area pick
        QVERIFY(r.workflow().isEmpty());
        QVERIFY(r.path().isEmpty());
    }

    void ordinaryRequestsAreNotRecordRequests()
    {
        QCOMPARE(CaptureRequest(CaptureRequest::GRAPHICAL_MODE).recordMode(),
                 CaptureRequest::RecordNone);
        QCOMPARE(
          buildCaptureFirstRequest("TAKE_SCREENSHOT", "/a", "/b").recordMode(),
          CaptureRequest::RecordNone);
    }

    // Regression: pressing Save in the editor opened from Edit called
    // addSaveTask() with no path, which wiped the original file path, so a
    // second numbered copy was saved instead of overwriting the original.
    void editorSaveKeepsOverwriteTarget()
    {
        CaptureRequest r(CaptureRequest::GRAPHICAL_MODE);
        r.addSaveTask("/pics/shot.png");
        r.setOverwriteExisting(true);
        r.addSaveTask();
        QCOMPARE(r.path(), QString("/pics/shot.png"));
        QVERIFY(r.tasks() & CaptureRequest::SAVE);
    }

    void plainSaveTaskStillClearsPath()
    {
        CaptureRequest r(CaptureRequest::GRAPHICAL_MODE);
        r.addSaveTask("/pics/shot.png");
        r.addSaveTask();
        QCOMPARE(r.path(), QString());
    }
};

QTEST_GUILESS_MAIN(TestCaptureFirstRequest)
#include "tst_capturefirstrequest.moc"
