// SPDX-License-Identifier: GPL-3.0-or-later
// On-screen window enumeration via CoreGraphics (plain C API).

#include "platform/windowlist.h"

#include <CoreFoundation/CoreFoundation.h>
#include <CoreGraphics/CoreGraphics.h>

namespace {

bool readInt(CFDictionaryRef d, CFStringRef key, qint64& out)
{
    auto* n = static_cast<CFNumberRef>(CFDictionaryGetValue(d, key));
    return n && CFNumberGetValue(n, kCFNumberSInt64Type, &out);
}

bool readDouble(CFDictionaryRef d, CFStringRef key, double& out)
{
    auto* n = static_cast<CFNumberRef>(CFDictionaryGetValue(d, key));
    return n && CFNumberGetValue(n, kCFNumberDoubleType, &out);
}

} // namespace

QList<WindowInfo> onScreenWindows()
{
    QList<WindowInfo> out;
    CFArrayRef list = CGWindowListCopyWindowInfo(
      kCGWindowListOptionOnScreenOnly | kCGWindowListExcludeDesktopElements,
      kCGNullWindowID);
    if (!list) {
        return out;
    }
    const CFIndex count = CFArrayGetCount(list);
    for (CFIndex i = 0; i < count; ++i) {
        auto* d = static_cast<CFDictionaryRef>(CFArrayGetValueAtIndex(list, i));
        if (!d) {
            continue;
        }
        auto* boundsDict = static_cast<CFDictionaryRef>(
          CFDictionaryGetValue(d, kCGWindowBounds));
        CGRect r;
        if (!boundsDict ||
            !CGRectMakeWithDictionaryRepresentation(boundsDict, &r)) {
            continue;
        }
        WindowInfo w;
        // CGWindow bounds are global logical points, top-left origin.
        w.bounds = QRect(qRound(r.origin.x),
                         qRound(r.origin.y),
                         qRound(r.size.width),
                         qRound(r.size.height));
        qint64 v = 0;
        w.layer = readInt(d, kCGWindowLayer, v) ? static_cast<int>(v) : 0;
        w.pid = readInt(d, kCGWindowOwnerPID, v) ? v : 0;
        w.id = readInt(d, kCGWindowNumber, v) ? static_cast<quint32>(v) : 0;
        double a = 1.0;
        w.alpha = readDouble(d, kCGWindowAlpha, a) ? a : 1.0;
        auto* owner =
          static_cast<CFStringRef>(CFDictionaryGetValue(d, kCGWindowOwnerName));
        if (owner) {
            const CFIndex len = CFStringGetLength(owner);
            QString s(static_cast<int>(len), QChar());
            CFStringGetCharacters(
              owner, CFRangeMake(0, len), reinterpret_cast<UniChar*>(s.data()));
            w.owner = s;
        }
        out.append(w);
    }
    CFRelease(list);
    return out;
}
