// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <QKeySequence>
#include <QMap>
#include <QString>
#include <QStringList>

// Names (sorted) of shortcuts that were added, removed or changed between two
// name -> key sequence maps. A shortcut missing on one side counts as changed.
// Sequences compare as key sequences, so "ctrl+x" equals "Ctrl+X".
QStringList changedShortcuts(const QMap<QString, QString>& before,
                             const QMap<QString, QString>& after);

// True when the only modifiers are Option (Alt) or Option+Shift. macOS 15+
// refuses to register global hotkeys like these.
bool isOptionOnlyShortcut(const QKeySequence& sequence);
