// SPDX-License-Identifier: GPL-3.0-or-later

#include "rulermath.h"

#include <cmath>

namespace rulermath {

namespace {
const double RAD_TO_DEG = 180.0 / 3.14159265358979323846;
}

Measurement measure(const QPointF& first, const QPointF& second, double dpr)
{
    if (!(dpr > 0.0)) {
        dpr = 1.0;
    }
    Measurement m;
    m.dx = (second.x() - first.x()) * dpr;
    m.dy = (second.y() - first.y()) * dpr;
    m.length = std::hypot(m.dx, m.dy);
    if (m.dx == 0.0 && m.dy == 0.0) {
        m.angleDegrees = 0.0;
    } else {
        // Negate dy so that "up on screen" is a positive angle.
        m.angleDegrees = std::atan2(-m.dy, m.dx) * RAD_TO_DEG;
        if (m.angleDegrees == 0.0) {
            m.angleDegrees = 0.0; // normalise -0.0
        }
    }
    return m;
}

QString labelText(const Measurement& m)
{
    return QStringLiteral("%1 px (%2 × %3)")
      .arg(qRound(m.length))
      .arg(qRound(std::abs(m.dx)))
      .arg(qRound(std::abs(m.dy)));
}

}
