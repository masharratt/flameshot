// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <QColor>
#include <QRect>

class CaptureRequest;
class QPainter;

// True for requests that only pick an area: capture-first hotkeys and tray
// captures, workflow hotkeys and recording area picks. Plain `flameshot gui`
// and the windowed editor never use it.
bool useShareXPicker(const CaptureRequest& req);

// Colors of the ShareX-style picker
QColor pickerLineColor();
QColor pickerShadowColor();
QColor pickerLabelBackground();

// Draws the picker outline for a rect in logical points: a 1 px white line on
// the rect's own edge and a 1 px translucent black line just outside it.
void drawPickerOutline(QPainter* painter, const QRect& rect);
