// SPDX-License-Identifier: GPL-3.0-or-later

#include "actionlist.h"

#include <QSet>
#include <algorithm>

namespace {

struct NameEntry
{
    CaptureAction action;
    const char* name;
};

const NameEntry kNames[] = {
    { CaptureAction::Save, "save" },
    { CaptureAction::CopyImage, "copyImage" },
    { CaptureAction::CopyPath, "copyPath" },
    { CaptureAction::OpenEditor, "openEditor" },
    { CaptureAction::Pin, "pin" },
    { CaptureAction::ApplyEffects, "applyEffects" },
    { CaptureAction::ShowToast, "showToast" },
};

int rank(CaptureAction action)
{
    switch (action) {
        case CaptureAction::ApplyEffects:
            return 0;
        case CaptureAction::Save:
            return 1;
        case CaptureAction::CopyImage:
        case CaptureAction::Pin:
            return 2;
        case CaptureAction::CopyPath:
            return 3;
        case CaptureAction::OpenEditor:
        case CaptureAction::ShowToast:
            return 4;
    }
    return 4;
}

} // namespace

QString toName(CaptureAction action)
{
    for (const NameEntry& e : kNames) {
        if (e.action == action) {
            return QString::fromLatin1(e.name);
        }
    }
    return {};
}

std::optional<CaptureAction> fromName(const QString& name)
{
    for (const NameEntry& e : kNames) {
        if (name == QLatin1String(e.name)) {
            return e.action;
        }
    }
    return std::nullopt;
}

ParseResult parseActionList(const QStringList& names)
{
    ParseResult result;
    QSet<CaptureAction> seen;
    for (const QString& name : names) {
        const auto action = fromName(name);
        if (!action) {
            result.unknown << name;
        } else if (seen.contains(*action)) {
            result.duplicates << name;
        } else {
            seen.insert(*action);
            result.actions << *action;
        }
    }
    return result;
}

QStringList serialise(const QList<CaptureAction>& actions)
{
    QStringList names;
    for (CaptureAction a : actions) {
        names << toName(a);
    }
    return names;
}

QList<CaptureAction> normalisedOrder(QList<CaptureAction> actions)
{
    std::stable_sort(actions.begin(),
                     actions.end(),
                     [](CaptureAction a, CaptureAction b) {
                         return rank(a) < rank(b);
                     });
    return actions;
}

QList<CaptureAction> defaultActionsFor(const QString& hotkeyName)
{
    if (hotkeyName == QLatin1String("TAKE_SCREENSHOT")) {
        return { CaptureAction::Save,
                 CaptureAction::CopyImage,
                 CaptureAction::ShowToast };
    }
    if (hotkeyName == QLatin1String("CAPTURE_AND_EDIT")) {
        return { CaptureAction::Save, CaptureAction::OpenEditor };
    }
    return {};
}
