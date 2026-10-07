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

    // Regression: Edit from the capture toast mixed device pixels and logical
    // points, so on a Retina screen the editor preselected a region 2x/4x off.
    void editGeometryOnRetinaSecondaryScreen()
    {
        const QRect screen(-1440, 0, 1440, 900);
        const EditGeometry g =
          editGeometry(QRect(-1000, 100, 200, 50), screen, 2.0);
        QCOMPARE(g.localLogical, QRect(440, 100, 200, 50));
        QCOMPARE(g.initialSelectionDevice, QRect(880, 200, 400, 100));
    }

    void editGeometryOnStandardPrimaryScreen()
    {
        const EditGeometry g = editGeometry(
          QRect(10, 20, 30, 40), QRect(0, 0, 1920, 1080), 1.0);
        QCOMPARE(g.localLogical, QRect(10, 20, 30, 40));
        QCOMPARE(g.initialSelectionDevice, QRect(10, 20, 30, 40));
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

    // Regression: editing a history image that had to be resized for the
    // editor overwrote the original with the resized copy.
    void overwriteOnlyWhenEditorKeepsEveryPixel()
    {
        QVERIFY(editKeepsOriginalPixels(QSize(400, 200), QSize(200, 100), 2.0));
        QVERIFY(!editKeepsOriginalPixels(QSize(4000, 2000), QSize(1440, 720), 2.0));
        QVERIFY(!editKeepsOriginalPixels(QSize(401, 200), QSize(201, 100), 2.0));
        QVERIFY(editKeepsOriginalPixels(QSize(300, 150), QSize(300, 150), 1.0));
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
};

QTEST_GUILESS_MAIN(TestCaptureFirstRequest)
#include "tst_capturefirstrequest.moc"
