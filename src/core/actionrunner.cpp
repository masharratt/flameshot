// SPDX-License-Identifier: GPL-3.0-or-later

#include "actionrunner.h"

ActionRunner::ActionRunner(ActionSink& sink)
  : m_sink(sink)
{}

RunReport ActionRunner::run(const QList<CaptureAction>& actions,
                            const CaptureResult& result)
{
    RunReport report;
    QPixmap pixmap = result.pixmap;
    QString savedPath;

    for (CaptureAction action : normalisedOrder(actions)) {
        switch (action) {
            case CaptureAction::ApplyEffects:
                pixmap = m_sink.applyEffects(pixmap);
                break;
            case CaptureAction::Save:
                savedPath = m_sink.save(pixmap);
                if (savedPath.isEmpty()) {
                    report.skipped << action;
                    continue;
                }
                break;
            case CaptureAction::CopyImage:
                m_sink.copyImage(pixmap);
                break;
            case CaptureAction::Pin:
                m_sink.pin(pixmap, result.selection);
                break;
            case CaptureAction::CopyPath:
            case CaptureAction::OpenEditor:
            case CaptureAction::ShowToast:
                if (savedPath.isEmpty()) {
                    report.skipped << action;
                    continue;
                }
                if (action == CaptureAction::CopyPath) {
                    m_sink.copyText(savedPath);
                } else if (action == CaptureAction::OpenEditor) {
                    m_sink.openEditor(savedPath, pixmap, result.globalRect);
                } else {
                    m_sink.showToast(savedPath,
                                     pixmap,
                                     result.selection,
                                     result.globalRect);
                }
                break;
        }
        report.performed << action;
    }
    return report;
}
