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
    // Re-editable project saved with this capture (base image plus
    // annotations); empty when there is none
    QString project;

    bool exists() const;
};

// Append-only JSON Lines index of saved captures. Pure store: QtCore and
// QtGui only. It never deletes image files. Project files are its own: those
// inside projectDir() are deleted together with their entries.
class CaptureHistory
{
public:
    explicit CaptureHistory(QString indexFile);

    static QString defaultIndexPath();
    // Where the projects of this history live, next to the index
    QString projectDir() const;
    // The project of the newest save of imagePath, when that file exists
    QString projectFor(const QString& imagePath) const;

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
    void deleteProjects(const QList<HistoryEntry>& dropped) const;

    QString m_indexFile;
    mutable int m_skipped = 0;
};
