// SPDX-License-Identifier: GPL-3.0-or-later
// Platforms without scrolling capture: everything reports "unsupported".

#include "scrollcapture.h"

namespace {

class UnsupportedAreaGrabber : public AreaGrabber
{
public:
    void prepare(const QRect&,
                 const QList<quint32>&,
                 std::function<void(QString error)> ready) override
    {
        ready(QStringLiteral("Scrolling capture is not supported here."));
    }
    void grab(std::function<void(QImage image)> done) override
    {
        done(QImage());
    }
};

} // namespace

std::unique_ptr<AreaGrabber> createAreaGrabber()
{
    return std::make_unique<UnsupportedAreaGrabber>();
}

bool scrollEventsAllowed(bool)
{
    return false;
}

bool postScrollDown(const QPoint&, const QRect&, int)
{
    return false;
}
