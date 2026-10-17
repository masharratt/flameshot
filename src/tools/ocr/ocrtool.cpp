// SPDX-License-Identifier: GPL-3.0-or-later

#include "ocrtool.h"
#include "core/flameshotdaemon.h"
#include "platform/macos/textrecognition.h"
#include "utils/abstractlogger.h"

OcrTool::OcrTool(QObject* parent)
  : AbstractActionTool(parent)
{}

bool OcrTool::closeOnButtonPressed() const
{
    return true;
}

QIcon OcrTool::icon(const QColor& background, bool inEditor) const
{
    Q_UNUSED(inEditor)
    return QIcon(iconPath(background) + "ocr.svg");
}

QString OcrTool::name() const
{
    return tr("Copy Text");
}

CaptureTool::Type OcrTool::type() const
{
    return CaptureTool::TYPE_OCR;
}

QString OcrTool::description() const
{
    return tr("Copy the text in the selection");
}

CaptureTool* OcrTool::copy(QObject* parent)
{
    return new OcrTool(parent);
}

void OcrTool::pressed(CaptureContext& context)
{
    // Recognise on the untouched capture, so annotations do not get in the way
    const QPixmap source = context.origScreenshot.isNull()
                             ? context.screenshot
                             : context.origScreenshot;
    const QPixmap area =
      context.selection.isNull() ? source : source.copy(context.selection);
    // Vision on one selection is fast, so this runs on the GUI thread
    const QString text = recognizeText(area.toImage());
    if (text.isEmpty()) {
        AbstractLogger::info() << tr("No text found");
    } else {
        FlameshotDaemon::copyToClipboard(
          text, tr("Copied %1 characters").arg(text.length()));
    }
    emit requestAction(REQ_CLOSE_GUI);
}
