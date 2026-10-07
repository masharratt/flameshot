// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include "core/actionlist.h"

#include <QList>
#include <QPixmap>
#include <QRect>
#include <QString>

// What a finished capture hands to the runner.
struct CaptureResult
{
    QPixmap pixmap;
    QRect selection;  // exportCapture units, used for pins
    QRect globalRect; // global logical points
};

// Side effects of the actions. The real implementation lives in Flameshot;
// tests inject a fake.
class ActionSink
{
public:
    virtual ~ActionSink() = default;
    // Returns the saved file path, or an empty string on failure.
    virtual QString save(const QPixmap& pixmap) = 0;
    virtual void copyImage(const QPixmap& pixmap) = 0;
    virtual void copyText(const QString& text) = 0;
    virtual void openEditor(const QString& path,
                            const QPixmap& pixmap,
                            const QRect& globalRect) = 0;
    virtual void pin(const QPixmap& pixmap, const QRect& selection) = 0;
    virtual void showToast(const QString& path,
                           const QPixmap& pixmap,
                           const QRect& selection,
                           const QRect& globalRect) = 0;
    virtual QPixmap applyEffects(const QPixmap& pixmap) = 0;
};

struct RunReport
{
    QList<CaptureAction> performed;
    // Not run: Save itself when it failed, and CopyPath, OpenEditor and
    // ShowToast whenever no saved path is available.
    QList<CaptureAction> skipped;
};

class ActionRunner
{
public:
    explicit ActionRunner(ActionSink& sink);
    // Runs the actions in normalisedOrder. Actions after ApplyEffects receive
    // the processed pixmap.
    RunReport run(const QList<CaptureAction>& actions,
                  const CaptureResult& result);

private:
    ActionSink& m_sink;
};
