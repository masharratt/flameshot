// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <QDateTime>
#include <QList>
#include <QSize>
#include <QString>

// One saved capture. kind is one of "image", "gif", "mp4".
struct HistoryEntry
{
    QDateTime timestamp;
    QString path;
    QSize size;
    QString kind;

    bool exists() const;
};

// Append-only JSON Lines index of saved captures. Pure store: QtCore and
// QtGui only. It never deletes image files.
class CaptureHistory
{
public:
    explicit CaptureHistory(QString indexFile);

    static QString defaultIndexPath();

    bool append(const HistoryEntry& entry);
    // Newest first
    QList<HistoryEntry> entries() const;
    bool remove(const QString& path);
    void trimTo(int max);
    // Lines that were not blank but could not be parsed on the last load
    int skippedLines() const;

private:
    QList<HistoryEntry> load() const; // oldest first, as stored
    bool rewrite(const QList<HistoryEntry>& oldestFirst) const;

    QString m_indexFile;
    mutable int m_skipped = 0;
};
