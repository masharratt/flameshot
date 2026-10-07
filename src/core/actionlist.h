// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <QList>
#include <QString>
#include <QStringList>
#include <optional>

// Steps a hotkey can run after a capture. The list is configured per hotkey.
enum class CaptureAction
{
    Save,
    CopyImage,
    CopyPath,
    OpenEditor,
    Pin,
    ApplyEffects,
    ShowToast,
};

// Config name of an action: "save", "copyImage", "copyPath", "openEditor",
// "pin", "applyEffects", "showToast".
QString toName(CaptureAction action);
std::optional<CaptureAction> fromName(const QString& name);

struct ParseResult
{
    QList<CaptureAction> actions; // known, first occurrence only, in order
    QStringList unknown;          // names that are not actions
    QStringList duplicates;       // repeated names of known actions
};
// Keeps the given order. Unknown and repeated names are dropped and reported.
ParseResult parseActionList(const QStringList& names);

QStringList serialise(const QList<CaptureAction>& actions);

// Order actions run in, whatever order the user listed them:
//   1. ApplyEffects (so later steps see the processed image)
//   2. Save (first, so its outcome is known before anything else happens)
//   3. CopyImage, Pin (kept in the given relative order)
//   4. CopyPath (needs the path Save produced)
//   5. OpenEditor, ShowToast (kept in the given relative order, always last)
QList<CaptureAction> normalisedOrder(QList<CaptureAction> actions);

// Built-in action list for a hotkey name. Empty for an unknown hotkey.
QList<CaptureAction> defaultActionsFor(const QString& hotkeyName);
