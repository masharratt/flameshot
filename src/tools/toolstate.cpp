// SPDX-License-Identifier: GPL-3.0-or-later

#include "toolstate.h"

#include <cmath>

namespace toolstate {

namespace {
// Coordinates beyond this are damage, not a real canvas
constexpr double kMaxCoord = 1e7;

bool toCoord(const QJsonValue& v, double& out)
{
    if (!v.isDouble()) {
        return false;
    }
    const double d = v.toDouble();
    if (!std::isfinite(d) || std::abs(d) > kMaxCoord) {
        return false;
    }
    out = d;
    return true;
}

bool toWhole(const QJsonValue& v, int& out)
{
    double d = 0;
    if (!toCoord(v, d) || d != std::floor(d)) {
        return false;
    }
    out = static_cast<int>(d);
    return true;
}
} // namespace

QJsonArray fromPoint(const QPoint& p)
{
    return { p.x(), p.y() };
}

QJsonArray fromPointF(const QPointF& p)
{
    return { p.x(), p.y() };
}

QJsonArray fromRect(const QRect& r)
{
    return { r.x(), r.y(), r.width(), r.height() };
}

QJsonValue fromColor(const QColor& c)
{
    return c.name(QColor::HexArgb);
}

bool toPoint(const QJsonValue& v, QPoint& out)
{
    const QJsonArray a = v.toArray();
    int x = 0;
    int y = 0;
    if (!v.isArray() || a.size() != 2 || !toWhole(a[0], x) ||
        !toWhole(a[1], y)) {
        return false;
    }
    out = QPoint(x, y);
    return true;
}

bool toPointF(const QJsonValue& v, QPointF& out)
{
    const QJsonArray a = v.toArray();
    double x = 0;
    double y = 0;
    if (!v.isArray() || a.size() != 2 || !toCoord(a[0], x) ||
        !toCoord(a[1], y)) {
        return false;
    }
    out = QPointF(x, y);
    return true;
}

bool toRect(const QJsonValue& v, QRect& out)
{
    const QJsonArray a = v.toArray();
    int x = 0;
    int y = 0;
    int w = 0;
    int h = 0;
    if (!v.isArray() || a.size() != 4 || !toWhole(a[0], x) ||
        !toWhole(a[1], y) || !toWhole(a[2], w) || !toWhole(a[3], h)) {
        return false;
    }
    out = QRect(x, y, w, h);
    return true;
}

bool toColor(const QJsonValue& v, QColor& out)
{
    if (!v.isString()) {
        return false;
    }
    const QColor c = QColor::fromString(v.toString());
    if (!c.isValid()) {
        return false;
    }
    out = c;
    return true;
}

bool toInt(const QJsonValue& v, int min, int max, int& out)
{
    int i = 0;
    if (!toWhole(v, i) || i < min || i > max) {
        return false;
    }
    out = i;
    return true;
}

} // namespace toolstate
