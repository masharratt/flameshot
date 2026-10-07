// SPDX-License-Identifier: GPL-3.0-or-later
// Tests for the rule that picks the ShareX-style area picker.

#include "core/capturerequest.h"
#include "widgets/capture/pickerstyle.h"

#include <QTest>

class TestPickerStyle : public QObject
{
    Q_OBJECT
private slots:
    void plainGuiKeepsUpstreamLook()
    {
        CaptureRequest r(CaptureRequest::GRAPHICAL_MODE);
        QVERIFY(!useShareXPicker(r));
    }

    void captureFirstUsesPicker()
    {
        CaptureRequest r(CaptureRequest::GRAPHICAL_MODE);
        r.setCaptureFirst(true);
        QVERIFY(useShareXPicker(r));
    }

    void workflowUsesPicker()
    {
        CaptureRequest r(CaptureRequest::GRAPHICAL_MODE);
        r.setWorkflow(QStringLiteral("TAKE_SCREENSHOT"));
        QVERIFY(useShareXPicker(r));
    }

    void emptyWorkflowDoesNotCount()
    {
        CaptureRequest r(CaptureRequest::GRAPHICAL_MODE);
        r.setWorkflow(QString());
        QVERIFY(!useShareXPicker(r));
    }

    void recordingPickUsesPicker()
    {
        CaptureRequest r(CaptureRequest::GRAPHICAL_MODE);
        r.setCaptureFirst(true);
        r.setRecordMode(CaptureRequest::RecordGif);
        QVERIFY(useShareXPicker(r));
    }

    void editImageRequestNeverUsesPicker()
    {
        QPixmap image(10, 10);
        CaptureRequest r(CaptureRequest::GRAPHICAL_MODE);
        r.setEditImage(image);
        QVERIFY(!useShareXPicker(r));
        // Even if some other flag leaks in, an editor is never a picker
        r.setCaptureFirst(true);
        r.setWorkflow(QStringLiteral("TAKE_SCREENSHOT"));
        QVERIFY(!useShareXPicker(r));
    }
};

QTEST_MAIN(TestPickerStyle)
#include "tst_pickerstyle.moc"
