// SPDX-License-Identifier: GPL-3.0-or-later

#include "curvemath.h"

#include <QLineF>
#include <algorithm>
#include <cmath>

namespace curvemath {

namespace {
constexpr int DistanceSamples = 64;

// Extremum parameter of one coordinate of the quadratic, or -1 if none.
double extremumT(double a, double c, double b)
{
    const double denom = a - 2.0 * c + b;
    if (std::abs(denom) < 1e-12) {
        return -1.0;
    }
    const double t = (a - c) / denom;
    return (t > 0.0 && t < 1.0) ? t : -1.0;
}
}

QPointF controlFromHandle(const QPointF& a,
                          const QPointF& b,
                          const QPointF& handle)
{
    return handle * 2.0 - (a + b) / 2.0;
}

QPointF handleFromControl(const QPointF& a,
                          const QPointF& b,
                          const QPointF& ctrl)
{
    return (ctrl + (a + b) / 2.0) / 2.0;
}

QPointF quadPoint(const QPointF& a,
                  const QPointF& ctrl,
                  const QPointF& b,
                  double t)
{
    const double u = 1.0 - t;
    return a * (u * u) + ctrl * (2.0 * u * t) + b * (t * t);
}

double endTangentAngle(const QPointF& a, const QPointF& ctrl, const QPointF& b)
{
    QPointF d = b - ctrl;
    if (std::abs(d.x()) < 1e-9 && std::abs(d.y()) < 1e-9) {
        d = b - a;
    }
    return std::atan2(d.y(), d.x()) * 180.0 / M_PI;
}

bool isStraight(const QPointF& a,
                const QPointF& b,
                const QPointF& handle,
                double tolerancePx)
{
    return QLineF(handle, (a + b) / 2.0).length() <= tolerancePx;
}

QRectF curveBounds(const QPointF& a, const QPointF& ctrl, const QPointF& b)
{
    QRectF r = QRectF(a, b).normalized();
    const double tx = extremumT(a.x(), ctrl.x(), b.x());
    const double ty = extremumT(a.y(), ctrl.y(), b.y());
    qreal left = r.left(), right = r.right(), top = r.top(), bottom = r.bottom();
    for (double t : { tx, ty }) {
        if (t < 0.0) {
            continue;
        }
        const QPointF p = quadPoint(a, ctrl, b, t);
        left = std::min(left, p.x());
        right = std::max(right, p.x());
        top = std::min(top, p.y());
        bottom = std::max(bottom, p.y());
    }
    return QRectF(QPointF(left, top), QPointF(right, bottom));
}

double distanceToCurve(const QPointF& p,
                       const QPointF& a,
                       const QPointF& ctrl,
                       const QPointF& b)
{
    double best = QLineF(p, a).length();
    for (int i = 1; i <= DistanceSamples; ++i) {
        const double t = static_cast<double>(i) / DistanceSamples;
        best = std::min(best, QLineF(p, quadPoint(a, ctrl, b, t)).length());
    }
    return best;
}

}
