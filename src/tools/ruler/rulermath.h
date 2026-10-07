// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <QPointF>
#include <QString>

// Pure measurement math for the ruler tool. No widgets involved.
namespace rulermath {

struct Measurement
{
    double length = 0.0;       // physical pixels
    double dx = 0.0;           // signed, physical pixels (second - first)
    double dy = 0.0;           // signed, physical pixels, screen y grows down
    double angleDegrees = 0.0; // y-up convention, range (-180, 180]
};

// Points are in logical coordinates; dpr converts them to physical pixels.
// A non-positive dpr is treated as 1.0.
Measurement measure(const QPointF& first, const QPointF& second, double dpr);

// Example: "212 px (200 × 70)". Width and height are magnitudes.
QString labelText(const Measurement& m);

}
