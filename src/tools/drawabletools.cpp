// SPDX-License-Identifier: GPL-3.0-or-later

#include "drawabletools.h"
#include "tools/arrow/arrowtool.h"
#include "tools/circle/circletool.h"
#include "tools/circlecount/circlecounttool.h"
#include "tools/invert/inverttool.h"
#include "tools/line/linetool.h"
#include "tools/marker/markertool.h"
#include "tools/pencil/penciltool.h"
#include "tools/pixelate/pixelatetool.h"
#include "tools/rectangle/rectangletool.h"
#include "tools/ruler/rulertool.h"
#include "tools/selection/selectiontool.h"
#include "tools/text/texttool.h"

CaptureTool* createDrawableTool(CaptureTool::Type type, QObject* parent)
{
    switch (type) {
        case CaptureTool::TYPE_PENCIL:
            return new PencilTool(parent);
        case CaptureTool::TYPE_DRAWER:
            return new LineTool(parent);
        case CaptureTool::TYPE_ARROW:
            return new ArrowTool(parent);
        case CaptureTool::TYPE_SELECTION:
            return new SelectionTool(parent);
        case CaptureTool::TYPE_RECTANGLE:
            return new RectangleTool(parent);
        case CaptureTool::TYPE_CIRCLE:
            return new CircleTool(parent);
        case CaptureTool::TYPE_MARKER:
            return new MarkerTool(parent);
        case CaptureTool::TYPE_PIXELATE:
            return new PixelateTool(parent);
        case CaptureTool::TYPE_TEXT:
            return new TextTool(parent);
        case CaptureTool::TYPE_CIRCLECOUNT:
            return new CircleCountTool(parent);
        case CaptureTool::TYPE_INVERT:
            return new InvertTool(parent);
        case CaptureTool::TYPE_RULER:
            return new RulerTool(parent);
        default:
            return nullptr;
    }
}
