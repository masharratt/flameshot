// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <QColor>
#include <QJsonArray>
#include <QJsonValue>
#include <QPoint>
#include <QPointF>
#include <QRect>

// JSON forms of the values tools save in a project. Points are [x, y], rects
// [x, y, w, h], colours "#AARRGGBB". Each reader returns false and leaves
// `out` alone when the value is missing or malformed.
namespace toolstate {

QJsonArray fromPoint(const QPoint& p);
QJsonArray fromPointF(const QPointF& p);
QJsonArray fromRect(const QRect& r);
QJsonValue fromColor(const QColor& c);

bool toPoint(const QJsonValue& v, QPoint& out);
bool toPointF(const QJsonValue& v, QPointF& out);
bool toRect(const QJsonValue& v, QRect& out);
bool toColor(const QJsonValue& v, QColor& out);
// A whole number in [min, max]
bool toInt(const QJsonValue& v, int min, int max, int& out);

} // namespace toolstate
