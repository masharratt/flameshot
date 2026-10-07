// SPDX-License-Identifier: GPL-3.0-or-later
// Characterisation tests for the filename time-pattern parser.

#include "utils/strfparse.h"

#include <QDate>
#include <QRegularExpression>
#include <QTest>

class TestStrfparse : public QObject
{
    Q_OBJECT
private slots:
    void emptyPatternGivesEmptyString()
    {
        QCOMPARE(QString::fromStdString(strfparse::format_time_string("")),
                 QString());
    }

    void plainTextIsUnchanged()
    {
        QCOMPARE(
          QString::fromStdString(strfparse::format_time_string("screenshot")),
          QStringLiteral("screenshot"));
    }

    void yearSpecifierIsReplaced()
    {
        const QString out =
          QString::fromStdString(strfparse::format_time_string("shot-%Y"));
        QCOMPARE(out,
                 QStringLiteral("shot-%1").arg(QDate::currentDate().year()));
    }

    void dateTimePatternHasExpectedShape()
    {
        const QString out = QString::fromStdString(
          strfparse::format_time_string("%Y-%m-%d_%H-%M-%S"));
        static const QRegularExpression shape(
          QStringLiteral("^\\d{4}-\\d{2}-\\d{2}_\\d{2}-\\d{2}-\\d{2}$"));
        QVERIFY2(shape.match(out).hasMatch(), qPrintable(out));
    }

    void unknownSpecifierIsLeftAlone()
    {
        QCOMPARE(QString::fromStdString(strfparse::format_time_string("a%Qb")),
                 QStringLiteral("a%Qb"));
    }

    void replaceAllReplacesEveryOccurrence()
    {
        QCOMPARE(
          QString::fromStdString(strfparse::replace_all("a-b-c", "-", "--")),
          QStringLiteral("a--b--c"));
    }
};

QTEST_GUILESS_MAIN(TestStrfparse)
#include "tst_strfparse.moc"
