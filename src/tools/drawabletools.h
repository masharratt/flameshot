// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include "tools/capturetool.h"

// Creates the tool for a type that draws an object on the capture (pencil,
// arrow, text, ...). nullptr for action tools and unknown types. The single
// type-to-class map for drawing tools: ToolFactory and the project loader
// both use it.
CaptureTool* createDrawableTool(CaptureTool::Type type,
                                QObject* parent = nullptr);
