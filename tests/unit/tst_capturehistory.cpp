// SPDX-License-Identifier: GPL-3.0-or-later

#include "utils/capturehistory.h"

#include <QDateTime>
#include <QFile>
#include <QTemporaryDir>
#include <QTimeZone>
#include <QtTest>

class TestCaptureHistory : public QObject
{
    Q_OBJECT

private:
    static HistoryEntry entry(const QString& path, qint64 msSinceEpoch)
    {
        HistoryEntry e;
        e.timestamp = QDateTime::fromMSecsSinceEpoch(msSinceEpoch, QTimeZone::UTC);
        e.path = path;
        e.size = QSize(640, 480);
        e.kind = QStringLiteral("image");
        return e;
    }

private slots:
    void appendAndReloadRoundTrip()
    {
        QTemporaryDir dir;
        QVERIFY(dir.isValid());
        const QString index = dir.filePath("history.jsonl");
        {
            CaptureHistory h(index);
            QVERIFY(h.append(entry("/tmp/a.png", 1700000000123)));
        }
        CaptureHistory h(index);
        const auto list = h.entries();
        QCOMPARE(list.size(), 1);
        QCOMPARE(list[0].path, QStringLiteral("/tmp/a.png"));
        QCOMPARE(list[0].size, QSize(640, 480));
        QCOMPARE(list[0].kind, QStringLiteral("image"));
        QCOMPARE(list[0].timestamp.toMSecsSinceEpoch(), qint64(1700000000123));
        QCOMPARE(h.skippedLines(), 0);
    }

    void newestFirstOrdering()
    {
        QTemporaryDir dir;
        const QString index = dir.filePath("history.jsonl");
        CaptureHistory h(index);
        QVERIFY(h.append(entry("/tmp/1.png", 1000)));
        QVERIFY(h.append(entry("/tmp/2.png", 2000)));
        QVERIFY(h.append(entry("/tmp/3.png", 3000)));
        CaptureHistory reloaded(index);
        const auto list = reloaded.entries();
        QCOMPARE(list.size(), 3);
        QCOMPARE(list[0].path, QStringLiteral("/tmp/3.png"));
        QCOMPARE(list[2].path, QStringLiteral("/tmp/1.png"));
    }

    void corruptLineSkippedAndCounted()
    {
        QTemporaryDir dir;
        const QString index = dir.filePath("history.jsonl");
        {
            CaptureHistory h(index);
            QVERIFY(h.append(entry("/tmp/a.png", 1000)));
        }
        QFile f(index);
        QVERIFY(f.open(QIODevice::Append));
        f.write("{not json\n");
        f.write("[1,2,3]\n");
        f.close();
        {
            CaptureHistory h(index);
            QVERIFY(h.append(entry("/tmp/b.png", 2000)));
        }
        CaptureHistory h(index);
        QCOMPARE(h.entries().size(), 2);
        QCOMPARE(h.skippedLines(), 2);
    }

    void blankLinesIgnored()
    {
        QTemporaryDir dir;
        const QString index = dir.filePath("history.jsonl");
        {
            CaptureHistory h(index);
            QVERIFY(h.append(entry("/tmp/a.png", 1000)));
        }
        QFile f(index);
        QVERIFY(f.open(QIODevice::Append));
        f.write("\n   \n\n");
        f.close();
        CaptureHistory h(index);
        QCOMPARE(h.entries().size(), 1);
        QCOMPARE(h.skippedLines(), 0);
    }

    void removeByPath()
    {
        QTemporaryDir dir;
        const QString index = dir.filePath("history.jsonl");
        CaptureHistory h(index);
        QVERIFY(h.append(entry("/tmp/a.png", 1000)));
        QVERIFY(h.append(entry("/tmp/b.png", 2000)));
        QVERIFY(h.remove("/tmp/a.png"));
        QVERIFY(!h.remove("/tmp/missing.png"));
        CaptureHistory reloaded(index);
        QCOMPARE(reloaded.entries().size(), 1);
        QCOMPARE(reloaded.entries()[0].path, QStringLiteral("/tmp/b.png"));
    }

    void trimKeepsNewestAndLeavesFiles()
    {
        QTemporaryDir dir;
        const QString index = dir.filePath("history.jsonl");
        CaptureHistory h(index);
        QStringList files;
        for (int i = 0; i < 5; ++i) {
            const QString p = dir.filePath(QStringLiteral("img%1.png").arg(i));
            QFile img(p);
            QVERIFY(img.open(QIODevice::WriteOnly));
            img.write("x");
            img.close();
            files << p;
            QVERIFY(h.append(entry(p, 1000 + i)));
        }
        h.trimTo(2);
        CaptureHistory reloaded(index);
        const auto list = reloaded.entries();
        QCOMPARE(list.size(), 2);
        QCOMPARE(list[0].path, files[4]);
        QCOMPARE(list[1].path, files[3]);
        for (const auto& p : files) {
            QVERIFY2(QFile::exists(p), qPrintable(p));
        }
    }

    void existsReflectsFile()
    {
        QTemporaryDir dir;
        const QString p = dir.filePath("real.png");
        QFile img(p);
        QVERIFY(img.open(QIODevice::WriteOnly));
        img.close();
        QVERIFY(entry(p, 1).exists());
        QVERIFY(!entry(dir.filePath("nope.png"), 1).exists());
    }

    void unicodePathRoundTrip()
    {
        QTemporaryDir dir;
        const QString index = dir.filePath("history.jsonl");
        const QString p = QString::fromUtf8("/tmp/\xe6\x88\xaa\xe5\x9b\xbe \xc3\xa9\xf0\x9f\x93\xb7.png");
        {
            CaptureHistory h(index);
            QVERIFY(h.append(entry(p, 1000)));
        }
        CaptureHistory h(index);
        QCOMPARE(h.entries().size(), 1);
        QCOMPARE(h.entries()[0].path, p);
    }
};

QTEST_GUILESS_MAIN(TestCaptureHistory)
#include "tst_capturehistory.moc"
