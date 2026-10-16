// SPDX-License-Identifier: GPL-3.0-or-later

#include "beautifytool.h"

BeautifyTool::BeautifyTool(QObject* parent)
  : AbstractActionTool(parent)
{}

bool BeautifyTool::closeOnButtonPressed() const
{
    return true;
}

QIcon BeautifyTool::icon(const QColor& background, bool inEditor) const
{
    Q_UNUSED(inEditor)
    return QIcon(iconPath(background) + "beautify.svg");
}

QString BeautifyTool::name() const
{
    return tr("Beautify");
}

CaptureTool::Type BeautifyTool::type() const
{
    return CaptureTool::TYPE_BEAUTIFY;
}

QString BeautifyTool::description() const
{
    return tr("Add background, padding and shadow");
}

CaptureTool* BeautifyTool::copy(QObject* parent)
{
    return new BeautifyTool(parent);
}

void BeautifyTool::pressed(CaptureContext& context)
{
    // Same finish as Accept: the request's own tasks (save, copy, ...) run
    // on the beautified image
    context.request.addTask(CaptureRequest::BEAUTIFY);
    emit requestAction(REQ_CAPTURE_DONE_OK);
    emit requestAction(REQ_CLOSE_GUI);
}
