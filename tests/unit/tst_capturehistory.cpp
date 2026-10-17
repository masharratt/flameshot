// SPDX-License-Identifier: GPL-3.0-or-later

#include "utils/capturehistory.h"

#include <QDateTime>
#include <QDir>
#include <QFileInfo>
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

    // Regression: editing a capture saves the same file again, which added a
    // second history entry, so the history window showed duplicates.
    void resavedPathShowsOnceAtNewestPosition()
    {
        QTemporaryDir dir;
        const QString index = dir.filePath("history.jsonl");
        CaptureHistory h(index);
        QVERIFY(h.append(entry("/tmp/1.png", 1000)));
        QVERIFY(h.append(entry("/tmp/2.png", 2000)));
        QVERIFY(h.append(entry("/tmp/1.png", 3000)));
        const auto list = CaptureHistory(index).entries();
        QCOMPARE(list.size(), 2);
        QCOMPARE(list[0].path, QStringLiteral("/tmp/1.png"));
        QCOMPARE(list[0].timestamp.toMSecsSinceEpoch(), qint64(3000));
        QCOMPARE(list[1].path, QStringLiteral("/tmp/2.png"));
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

    // Re-editable projects: an entry may name its project file, which the
    // history owns (it lives in projectDir) and deletes with the entry
    static QString touch(const QString& path)
    {
        QDir().mkpath(QFileInfo(path).absolutePath());
        QFile f(path);
        if (f.open(QIODevice::WriteOnly)) {
            f.write("p");
        }
        return path;
    }

    void projectPathRoundTrip()
    {
        QTemporaryDir dir;
        const QString index = dir.filePath("history.jsonl");
        CaptureHistory h(index);
        QCOMPARE(h.projectDir(), dir.filePath("projects"));
        HistoryEntry e = entry("/tmp/a.png", 1000);
        e.project = h.projectDir() + "/a.fsproj";
        QVERIFY(h.append(e));
        QVERIFY(h.append(entry("/tmp/b.png", 2000)));
        CaptureHistory reloaded(index);
        const auto list = reloaded.entries();
        QCOMPARE(list.size(), 2);
        QCOMPARE(list[1].project, e.project);
        QVERIFY(list[0].project.isEmpty());
    }

    void projectForPrefersNewestExistingProject()
    {
        QTemporaryDir dir;
        CaptureHistory h(dir.filePath("history.jsonl"));
        HistoryEntry older = entry("/tmp/a.png", 1000);
        older.project = touch(h.projectDir() + "/old.fsproj");
        HistoryEntry newer = entry("/tmp/a.png", 2000);
        newer.project = touch(h.projectDir() + "/new.fsproj");
        QVERIFY(h.append(older));
        QVERIFY(h.append(newer));
        QCOMPARE(h.projectFor("/tmp/a.png"), newer.project);
        QFile::remove(newer.project);
        // A newer save without a project file (flattened) has none either
        QVERIFY(h.projectFor("/tmp/a.png").isEmpty());
        QVERIFY(h.projectFor("/tmp/other.png").isEmpty());
    }

    void removeDeletesOwnedProjectsOnly()
    {
        QTemporaryDir dir;
        CaptureHistory h(dir.filePath("history.jsonl"));
        HistoryEntry a = entry(touch(dir.filePath("a.png")), 1000);
        a.project = touch(h.projectDir() + "/a.fsproj");
        HistoryEntry a2 = entry(a.path, 1500);
        a2.project = touch(h.projectDir() + "/a2.fsproj");
        // A path outside projectDir is never deleted
        HistoryEntry b = entry(touch(dir.filePath("b.png")), 2000);
        b.project = touch(dir.filePath("elsewhere.fsproj"));
        QVERIFY(h.append(a));
        QVERIFY(h.append(a2));
        QVERIFY(h.append(b));
        QVERIFY(h.remove(a.path));
        QVERIFY(h.remove(b.path));
        QVERIFY(!QFile::exists(a.project));
        QVERIFY(!QFile::exists(a2.project));
        QVERIFY(QFile::exists(b.project));
        QVERIFY(QFile::exists(a.path));
        QVERIFY(QFile::exists(b.path));
    }

    void trimDeletesProjectsOfDroppedEntries()
    {
        QTemporaryDir dir;
        CaptureHistory h(dir.filePath("history.jsonl"));
        QStringList projects;
        for (int i = 0; i < 4; ++i) {
            HistoryEntry e =
              entry(dir.filePath(QStringLiteral("img%1.png").arg(i)), 1000 + i);
            e.project =
              touch(h.projectDir() + QStringLiteral("/p%1.fsproj").arg(i));
            projects << e.project;
            QVERIFY(h.append(e));
        }
        h.trimTo(2);
        QVERIFY(!QFile::exists(projects[0]));
        QVERIFY(!QFile::exists(projects[1]));
        QVERIFY(QFile::exists(projects[2]));
        QVERIFY(QFile::exists(projects[3]));
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
