// SPDX-License-Identifier: GPL-3.0-or-later
// Tests for the pure helpers behind the editor settings bar.

#include "widgets/editorsettings.h"

#include <QTest>

class TestEditorSettings : public QObject
{
    Q_OBJECT
private slots:
    void keepsValidColorsInOrder()
    {
        const QList<QColor> in{ QColor("#ff0000"), QColor("#00ff00") };
        QCOMPARE(swatchColors(in, 16), in);
    }

    void dropsInvalidAndTransparent()
    {
        const QList<QColor> in{ QColor(),
                                QColor(255, 0, 0, 0),
                                QColor("#0000ff") };
        QCOMPARE(swatchColors(in, 16), QList<QColor>{ QColor("#0000ff") });
    }

    void deduplicatesKeepingFirst()
    {
        const QList<QColor> in{ QColor("#ff0000"),
                                QColor("#00ff00"),
                                QColor("#ff0000") };
        QCOMPARE(swatchColors(in, 16),
                 (QList<QColor>{ QColor("#ff0000"), QColor("#00ff00") }));
    }

    void keepsDistinctAlphaVariants()
    {
        const QList<QColor> in{ QColor(255, 0, 0, 255), QColor(255, 0, 0, 128) };
        QCOMPARE(swatchColors(in, 16).size(), 2);
    }

    void capsAtMaxCount()
    {
        QList<QColor> in;
        for (int i = 1; i <= 30; ++i) {
            in.append(QColor(i, 0, 0));
        }
        const QList<QColor> out = swatchColors(in, 16);
        QCOMPARE(out.size(), 16);
        QCOMPARE(out.first(), QColor(1, 0, 0));
        QCOMPARE(out.last(), QColor(16, 0, 0));
    }

    void zeroOrNegativeCapGivesEmpty()
    {
        QCOMPARE(swatchColors({ QColor("#ff0000") }, 0).size(), 0);
        QCOMPARE(swatchColors({ QColor("#ff0000") }, -3).size(), 0);
    }

    void clampThicknessBounds()
    {
        QCOMPARE(clampThickness(0, 1, 50), 1);
        QCOMPARE(clampThickness(-9, 1, 50), 1);
        QCOMPARE(clampThickness(99, 1, 50), 50);
        QCOMPARE(clampThickness(7, 1, 50), 7);
    }
};

QTEST_MAIN(TestEditorSettings)
#include "tst_editorsettings.moc"
