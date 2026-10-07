// SPDX-License-Identifier: GPL-3.0-or-later
// Tests for the pure global hotkey helpers.

#include "core/hotkeyutils.h"

#include <QTest>

class TestHotkeys : public QObject
{
    Q_OBJECT
private slots:
    void changedShortcutsDetectsAddedRemovedChanged()
    {
        const QMap<QString, QString> before = {
            { "TAKE_SCREENSHOT", "Ctrl+Shift+X" },
            { "CAPTURE_AND_EDIT", "" },
            { "SCREENSHOT_HISTORY", "Ctrl+Shift+Y" },
            { "GONE", "Ctrl+G" },
        };
        const QMap<QString, QString> after = {
            { "TAKE_SCREENSHOT", "Ctrl+Shift+X" },
            { "CAPTURE_AND_EDIT", "Ctrl+Shift+E" },
            { "SCREENSHOT_HISTORY", "Ctrl+Shift+H" },
            { "NEW", "Ctrl+N" },
        };
        QCOMPARE(changedShortcuts(before, after),
                 (QStringList{
                   "CAPTURE_AND_EDIT", "GONE", "NEW", "SCREENSHOT_HISTORY" }));
    }

    void changedShortcutsUnchangedIsEmpty()
    {
        const QMap<QString, QString> m = { { "A", "Ctrl+A" }, { "B", "" } };
        QVERIFY(changedShortcuts(m, m).isEmpty());
        QVERIFY(changedShortcuts({}, {}).isEmpty());
    }

    void changedShortcutsTreatsEquivalentSequencesAsEqual()
    {
        QVERIFY(changedShortcuts({ { "A", "ctrl+shift+x" } },
                                 { { "A", "Ctrl+Shift+X" } })
                  .isEmpty());
    }

    void optionOnlyShortcuts()
    {
        QVERIFY(isOptionOnlyShortcut(QKeySequence("Alt+X")));
        QVERIFY(isOptionOnlyShortcut(QKeySequence("Alt+Shift+X")));
        QVERIFY(!isOptionOnlyShortcut(QKeySequence("Ctrl+Shift+X")));
        QVERIFY(!isOptionOnlyShortcut(QKeySequence("Meta+Alt+X")));
        QVERIFY(!isOptionOnlyShortcut(QKeySequence("Ctrl+Alt+X")));
        QVERIFY(!isOptionOnlyShortcut(QKeySequence()));
        QVERIFY(!isOptionOnlyShortcut(QKeySequence("X")));
    }
};

QTEST_MAIN(TestHotkeys)
#include "tst_hotkeys.moc"
