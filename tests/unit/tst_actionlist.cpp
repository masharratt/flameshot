// SPDX-License-Identifier: GPL-3.0-or-later
// Tests for the pure after-capture action list.

#include "core/actionlist.h"

#include <QTest>

using A = CaptureAction;

class TestActionList : public QObject
{
    Q_OBJECT
private slots:
    void namesRoundTrip()
    {
        const QList<A> all = { A::Save,      A::CopyImage,    A::CopyPath,
                               A::OpenEditor, A::Pin,         A::ApplyEffects,
                               A::ShowToast };
        for (A a : all) {
            const auto back = fromName(toName(a));
            QVERIFY2(back.has_value(), qPrintable(toName(a)));
            QCOMPARE(*back, a);
        }
        QCOMPARE(toName(A::CopyImage), QString("copyImage"));
        QCOMPARE(toName(A::ShowToast), QString("showToast"));
        QVERIFY(!fromName("nonsense").has_value());
    }

    void parseKeepsOrder()
    {
        const ParseResult r =
          parseActionList({ "showToast", "save", "copyImage" });
        QCOMPARE(r.actions, (QList<A>{ A::ShowToast, A::Save, A::CopyImage }));
        QVERIFY(r.unknown.isEmpty());
        QVERIFY(r.duplicates.isEmpty());
    }

    void parseReportsUnknownAndDuplicatesByName()
    {
        const ParseResult r =
          parseActionList({ "save", "bogus", "save", "pin", "bogus", "pin" });
        QCOMPARE(r.actions, (QList<A>{ A::Save, A::Pin }));
        QCOMPARE(r.unknown, (QStringList{ "bogus", "bogus" }));
        QCOMPARE(r.duplicates, (QStringList{ "save", "pin" }));
    }

    void serialiseRoundTrips()
    {
        const QList<A> in = { A::Save, A::OpenEditor, A::CopyPath };
        QCOMPARE(serialise(in),
                 (QStringList{ "save", "openEditor", "copyPath" }));
        QCOMPARE(parseActionList(serialise(in)).actions, in);
    }

    void normalisedOrderRules()
    {
        QCOMPARE(normalisedOrder({ A::ShowToast,
                                   A::CopyPath,
                                   A::Pin,
                                   A::Save,
                                   A::ApplyEffects }),
                 (QList<A>{ A::ApplyEffects,
                            A::Save,
                            A::Pin,
                            A::CopyPath,
                            A::ShowToast }));
        // OpenEditor and ShowToast come after everything else
        const QList<A> n =
          normalisedOrder({ A::OpenEditor, A::CopyImage, A::ShowToast });
        QCOMPARE(n.first(), A::CopyImage);
        QCOMPARE(n.last(), A::ShowToast);
        // Save always precedes CopyPath
        const QList<A> m = normalisedOrder({ A::CopyPath, A::Save });
        QCOMPARE(m, (QList<A>{ A::Save, A::CopyPath }));
        QVERIFY(normalisedOrder({}).isEmpty());
    }

    void defaults()
    {
        QCOMPARE(defaultActionsFor("TAKE_SCREENSHOT"),
                 (QList<A>{ A::Save, A::CopyImage, A::ShowToast }));
        QCOMPARE(defaultActionsFor("CAPTURE_AND_EDIT"),
                 (QList<A>{ A::Save, A::OpenEditor }));
        QVERIFY(defaultActionsFor("SOMETHING_ELSE").isEmpty());
    }
};

QTEST_MAIN(TestActionList)
#include "tst_actionlist.moc"
