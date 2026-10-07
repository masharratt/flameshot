// SPDX-License-Identifier: GPL-3.0-or-later

#include "hotkeyutils.h"

#include <QSet>

QStringList changedShortcuts(const QMap<QString, QString>& before,
                             const QMap<QString, QString>& after)
{
    QSet<QString> names;
    for (auto it = before.begin(); it != before.end(); ++it) {
        names.insert(it.key());
    }
    for (auto it = after.begin(); it != after.end(); ++it) {
        names.insert(it.key());
    }

    QStringList changed;
    for (const QString& name : names) {
        if (!before.contains(name) || !after.contains(name) ||
            QKeySequence(before.value(name)) !=
              QKeySequence(after.value(name))) {
            changed << name;
        }
    }
    changed.sort();
    return changed;
}

bool isOptionOnlyShortcut(const QKeySequence& sequence)
{
    if (sequence.isEmpty()) {
        return false;
    }
    const Qt::KeyboardModifiers mods =
      QKeyCombination(sequence[0]).keyboardModifiers();
    if (!(mods & Qt::AltModifier)) {
        return false;
    }
    const Qt::KeyboardModifiers others =
      mods & ~(Qt::AltModifier | Qt::ShiftModifier);
    return others == Qt::NoModifier;
}
