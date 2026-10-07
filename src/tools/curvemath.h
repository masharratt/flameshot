// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <QPointF>
#include <QRectF>

// Pure math for the bendable arrow and line: a quadratic Bezier from a to b
// with control point ctrl. The user drags a handle that the curve passes
// through at t = 0.5, so the handle maps to a control point and back.
namespace curvemath {

// Control point of the quadratic that passes through handle at t = 0.5.
QPointF controlFromHandle(const QPointF& a,
                          const QPointF& b,
                          const QPointF& handle);

// Inverse of controlFromHandle: the curve point at t = 0.5.
QPointF handleFromControl(const QPointF& a,
                          const QPointF& b,
                          const QPointF& ctrl);

// Point on the curve, t in [0, 1].
QPointF quadPoint(const QPointF& a,
                  const QPointF& ctrl,
                  const QPointF& b,
                  double t);

// Direction of travel at b in degrees, atan2(dy, dx) in screen coordinates
// (y grows down). Equals the straight a->b angle when ctrl is on the chord
// midpoint. Falls back to the chord direction if ctrl coincides with b.
double endTangentAngle(const QPointF& a, const QPointF& ctrl, const QPointF& b);

// True when handle is within tolerancePx of the straight midpoint of a-b.
bool isStraight(const QPointF& a,
                const QPointF& b,
                const QPointF& handle,
                double tolerancePx);

// Tight bounds of the curve itself, not of its control polygon.
QRectF curveBounds(const QPointF& a, const QPointF& ctrl, const QPointF& b);

// Approximate distance from p to the curve (sampled).
double distanceToCurve(const QPointF& p,
                       const QPointF& a,
                       const QPointF& ctrl,
                       const QPointF& b);

}
