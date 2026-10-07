// SPDX-License-Identifier: GPL-3.0-or-later
// Tests for the after-capture action runner using a recording fake sink.

#include "core/actionrunner.h"

#include <QPixmap>
#include <QTest>

using A = CaptureAction;

namespace {

class FakeSink : public ActionSink
{
public:
    QStringList calls;
    QString savePath = "/tmp/shot.png";
    QSize lastSaveSize, lastCopySize, lastPinSize, lastEditorSize, lastToastSize;
    QString lastText, lastEditorPath, lastToastPath;
    QRect lastEditorGlobal, lastToastGlobal, lastPinSel;

    QString save(const QPixmap& p) override
    {
        calls << "save";
        lastSaveSize = p.size();
        return savePath;
    }
    void copyImage(const QPixmap& p) override
    {
        calls << "copyImage";
        lastCopySize = p.size();
    }
    void copyText(const QString& t) override
    {
        calls << "copyText";
        lastText = t;
    }
    void openEditor(const QString& path,
                    const QPixmap& p,
                    const QRect& globalRect) override
    {
        calls << "openEditor";
        lastEditorPath = path;
        lastEditorSize = p.size();
        lastEditorGlobal = globalRect;
    }
    void pin(const QPixmap& p, const QRect& selection) override
    {
        calls << "pin";
        lastPinSize = p.size();
        lastPinSel = selection;
    }
    void showToast(const QString& path,
                   const QPixmap& p,
                   const QRect&,
                   const QRect& globalRect) override
    {
        calls << "showToast";
        lastToastPath = path;
        lastToastSize = p.size();
        lastToastGlobal = globalRect;
    }
    QPixmap applyEffects(const QPixmap& p) override
    {
        calls << "applyEffects";
        return QPixmap(p.width() + 10, p.height() + 10);
    }
};

CaptureResult makeResult()
{
    CaptureResult r;
    r.pixmap = QPixmap(20, 10);
    r.selection = QRect(1, 2, 3, 4);
    r.globalRect = QRect(100, 200, 20, 10);
    return r;
}

} // namespace

class TestActionRunner : public QObject
{
    Q_OBJECT
private slots:
    void runsInNormalisedOrder()
    {
        FakeSink sink;
        ActionRunner runner(sink);
        const RunReport rep = runner.run(
          { A::ShowToast, A::CopyPath, A::CopyImage, A::Save }, makeResult());
        QCOMPARE(sink.calls,
                 (QStringList{ "save", "copyImage", "copyText", "showToast" }));
        QCOMPARE(rep.performed,
                 (QList<A>{ A::Save, A::CopyImage, A::CopyPath, A::ShowToast }));
        QVERIFY(rep.skipped.isEmpty());
        QCOMPARE(sink.lastText, QString("/tmp/shot.png"));
        QCOMPARE(sink.lastToastPath, QString("/tmp/shot.png"));
        QCOMPARE(sink.lastToastGlobal, QRect(100, 200, 20, 10));
    }

    void saveFailureSkipsPathDependentActions()
    {
        FakeSink sink;
        sink.savePath.clear();
        ActionRunner runner(sink);
        const RunReport rep = runner.run(
          { A::Save, A::CopyImage, A::CopyPath, A::OpenEditor, A::ShowToast },
          makeResult());
        QCOMPARE(sink.calls, (QStringList{ "save", "copyImage" }));
        QCOMPARE(rep.performed, (QList<A>{ A::CopyImage }));
        QCOMPARE(rep.skipped,
                 (QList<A>{ A::Save, A::CopyPath, A::OpenEditor, A::ShowToast }));
    }

    void pathActionsWithoutSaveAreSkipped()
    {
        FakeSink sink;
        ActionRunner runner(sink);
        const RunReport rep =
          runner.run({ A::CopyImage, A::OpenEditor }, makeResult());
        QCOMPARE(rep.performed, (QList<A>{ A::CopyImage }));
        QCOMPARE(rep.skipped, (QList<A>{ A::OpenEditor }));
    }

    void effectsOutputReachesLaterActions()
    {
        FakeSink sink;
        ActionRunner runner(sink);
        runner.run({ A::Save,
                     A::CopyImage,
                     A::Pin,
                     A::OpenEditor,
                     A::ShowToast,
                     A::ApplyEffects },
                   makeResult());
        QCOMPARE(sink.calls.first(), QString("applyEffects"));
        const QSize effected(30, 20);
        QCOMPARE(sink.lastSaveSize, effected);
        QCOMPARE(sink.lastCopySize, effected);
        QCOMPARE(sink.lastPinSize, effected);
        QCOMPARE(sink.lastEditorSize, effected);
        QCOMPARE(sink.lastToastSize, effected);
        QCOMPARE(sink.lastPinSel, QRect(1, 2, 3, 4));
    }

    void withoutEffectsOriginalPixmapIsUsed()
    {
        FakeSink sink;
        ActionRunner runner(sink);
        runner.run({ A::Save }, makeResult());
        QCOMPARE(sink.lastSaveSize, QSize(20, 10));
    }

    void emptyListDoesNothing()
    {
        FakeSink sink;
        ActionRunner runner(sink);
        const RunReport rep = runner.run({}, makeResult());
        QVERIFY(sink.calls.isEmpty());
        QVERIFY(rep.performed.isEmpty());
        QVERIFY(rep.skipped.isEmpty());
    }
};

QTEST_MAIN(TestActionRunner)
#include "tst_actionrunner.moc"
