// SPDX-License-Identifier: GPL-3.0-or-later

#include "capturehistory.h"

#include <QSet>
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QJsonDocument>
#include <QJsonObject>
#include <QSaveFile>
#include <QStandardPaths>

#include <algorithm>

namespace {

QByteArray encode(const HistoryEntry& e)
{
    QJsonObject o;
    o.insert(QStringLiteral("ts"),
             e.timestamp.toUTC().toString(Qt::ISODateWithMs));
    o.insert(QStringLiteral("path"), e.path);
    o.insert(QStringLiteral("w"), e.size.width());
    o.insert(QStringLiteral("h"), e.size.height());
    o.insert(QStringLiteral("kind"), e.kind);
    if (!e.project.isEmpty()) {
        o.insert(QStringLiteral("proj"), e.project);
    }
    return QJsonDocument(o).toJson(QJsonDocument::Compact) + '\n';
}

bool decode(const QByteArray& line, HistoryEntry& out)
{
    QJsonParseError err;
    const QJsonDocument doc = QJsonDocument::fromJson(line, &err);
    if (err.error != QJsonParseError::NoError || !doc.isObject()) {
        return false;
    }
    const QJsonObject o = doc.object();
    const QDateTime ts = QDateTime::fromString(
      o.value(QStringLiteral("ts")).toString(), Qt::ISODateWithMs);
    const QString path = o.value(QStringLiteral("path")).toString();
    if (!ts.isValid() || path.isEmpty()) {
        return false;
    }
    out.timestamp = ts;
    out.path = path;
    out.size = QSize(o.value(QStringLiteral("w")).toInt(),
                     o.value(QStringLiteral("h")).toInt());
    out.kind = o.value(QStringLiteral("kind")).toString(QStringLiteral("image"));
    out.project = o.value(QStringLiteral("proj")).toString();
    return true;
}

} // namespace

bool HistoryEntry::exists() const
{
    return QFileInfo::exists(path);
}

CaptureHistory::CaptureHistory(QString indexFile)
  : m_indexFile(std::move(indexFile))
{}

QString CaptureHistory::defaultIndexPath()
{
    return QStandardPaths::writableLocation(QStandardPaths::AppDataLocation) +
           QStringLiteral("/history.jsonl");
}

QString CaptureHistory::projectDir() const
{
    return QFileInfo(m_indexFile).absolutePath() + QStringLiteral("/projects");
}

QString CaptureHistory::projectFor(const QString& imagePath) const
{
    for (const HistoryEntry& e : entries()) {
        if (e.path == imagePath) {
            return !e.project.isEmpty() && QFileInfo::exists(e.project)
                     ? e.project
                     : QString();
        }
    }
    return {};
}

// Only files directly inside projectDir are ours to delete; a path anywhere
// else in the index is left alone
void CaptureHistory::deleteProjects(const QList<HistoryEntry>& dropped) const
{
    const QString dir = QDir::cleanPath(projectDir());
    for (const HistoryEntry& e : dropped) {
        if (e.project.isEmpty()) {
            continue;
        }
        const QFileInfo info(e.project);
        if (QDir::cleanPath(info.absolutePath()) == dir) {
            QFile::remove(info.absoluteFilePath());
        }
    }
}

bool CaptureHistory::append(const HistoryEntry& entry)
{
    QDir().mkpath(QFileInfo(m_indexFile).absolutePath());
    QFile f(m_indexFile);
    if (!f.open(QIODevice::Append)) {
        return false;
    }
    const QByteArray line = encode(entry);
    return f.write(line) == line.size();
}

QList<HistoryEntry> CaptureHistory::load() const
{
    m_skipped = 0;
    QList<HistoryEntry> result;
    QFile f(m_indexFile);
    if (!f.open(QIODevice::ReadOnly)) {
        return result;
    }
    while (!f.atEnd()) {
        const QByteArray line = f.readLine().trimmed();
        if (line.isEmpty()) {
            continue;
        }
        HistoryEntry e;
        if (decode(line, e)) {
            result.append(e);
        } else {
            ++m_skipped;
        }
    }
    return result;
}

bool CaptureHistory::rewrite(const QList<HistoryEntry>& oldestFirst) const
{
    QDir().mkpath(QFileInfo(m_indexFile).absolutePath());
    QSaveFile out(m_indexFile);
    if (!out.open(QIODevice::WriteOnly)) {
        return false;
    }
    for (const HistoryEntry& e : oldestFirst) {
        const QByteArray line = encode(e);
        if (out.write(line) != line.size()) {
            out.cancelWriting();
            return false;
        }
    }
    return out.commit();
}

QList<HistoryEntry> CaptureHistory::entries() const
{
    QList<HistoryEntry> list = load();
    std::reverse(list.begin(), list.end());
    // A file saved again (for example after Edit) is listed once, at the
    // position of its newest save.
    QSet<QString> seen;
    QList<HistoryEntry> unique;
    for (const HistoryEntry& e : list) {
        if (!seen.contains(e.path)) {
            seen.insert(e.path);
            unique.append(e);
        }
    }
    return unique;
}

bool CaptureHistory::remove(const QString& path)
{
    QList<HistoryEntry> list = load();
    QList<HistoryEntry> dropped;
    for (const HistoryEntry& e : list) {
        if (e.path == path) {
            dropped.append(e);
        }
    }
    if (dropped.isEmpty()) {
        return false;
    }
    list.removeIf([&](const HistoryEntry& e) { return e.path == path; });
    if (!rewrite(list)) {
        return false;
    }
    deleteProjects(dropped);
    return true;
}

void CaptureHistory::trimTo(int max)
{
    QList<HistoryEntry> list = load();
    if (max < 0 || list.size() <= max) {
        return;
    }
    const QList<HistoryEntry> dropped = list.mid(0, list.size() - max);
    list = list.mid(list.size() - max);
    if (rewrite(list)) {
        deleteProjects(dropped);
    }
}

int CaptureHistory::skippedLines() const
{
    load();
    return m_skipped;
}
