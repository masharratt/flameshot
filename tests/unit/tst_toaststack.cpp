// SPDX-License-Identifier: GPL-3.0-or-later
// Tests for the pure toast stacking geometry.

#include "widgets/toaststack.h"

#include <QTest>

class TestToastStack : public QObject
{
    Q_OBJECT
private slots:
    void single()
    {
        const QRect avail(0, 0, 1000, 800);
        const QSize size(300, 100);
        const QList<QRect> r = toastStackGeometry(avail, size, 1, 16, 8);
        QCOMPARE(r.size(), 1);
        QCOMPARE(r[0].size(), size);
        QCOMPARE(r[0].right(), avail.right() - 16 + 1 - 1);
        QCOMPARE(r[0].bottom(), avail.bottom() - 16 + 1 - 1);
        QVERIFY(avail.contains(r[0]));
    }

    void zero()
    {
        QVERIFY(toastStackGeometry(QRect(0, 0, 100, 100), QSize(10, 10), 0, 4, 4)
                  .isEmpty());
    }

    void threeStackUpward()
    {
        const QRect avail(0, 0, 1000, 800);
        const QSize size(300, 100);
        const int margin = 16, spacing = 8;
        const QList<QRect> r = toastStackGeometry(avail, size, 3, margin, spacing);
        QCOMPARE(r.size(), 3);
        // index 0 is the newest, at the bottom
        QVERIFY(r[0].top() > r[1].top());
        QVERIFY(r[1].top() > r[2].top());
        for (int i = 0; i < 3; ++i) {
            QCOMPARE(r[i].size(), size);
            QVERIFY(avail.contains(r[i]));
            QVERIFY(avail.right() - r[i].right() >= margin);
            QVERIFY(avail.bottom() - r[0].bottom() >= margin);
            for (int j = i + 1; j < 3; ++j) {
                QVERIFY(!r[i].intersects(r[j]));
            }
        }
        QCOMPARE(r[0].top() - r[1].bottom() - 1, spacing);
        QCOMPARE(r[1].top() - r[2].bottom() - 1, spacing);
    }

    void offsetScreen()
    {
        const QRect avail(-1440, 0, 1440, 900);
        const QSize size(300, 100);
        const QList<QRect> r = toastStackGeometry(avail, size, 3, 16, 8);
        QCOMPARE(r.size(), 3);
        for (const QRect& t : r) {
            QVERIFY(avail.contains(t));
        }
        QVERIFY(avail.right() - r[0].right() >= 16);
    }
};

QTEST_GUILESS_MAIN(TestToastStack)
#include "tst_toaststack.moc"
