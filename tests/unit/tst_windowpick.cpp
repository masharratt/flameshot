// SPDX-License-Identifier: GPL-3.0-or-later
// Tests for the pure window picking logic used by hover-to-window detection.

#include "platform/windowpick.h"

#include <QTest>

namespace {
WindowInfo win(QRect bounds,
               quint32 id,
               int layer = 0,
               double alpha = 1.0,
               qint64 pid = 100)
{
    WindowInfo w;
    w.bounds = bounds;
    w.layer = layer;
    w.alpha = alpha;
    w.pid = pid;
    w.owner = QStringLiteral("app");
    w.id = id;
    return w;
}
} // namespace

class TestWindowPick : public QObject
{
    Q_OBJECT
private slots:
    void filterKeepsNormalWindows()
    {
        const auto out = filterCandidates({ win({ 0, 0, 200, 200 }, 1) }, 999);
        QCOMPARE(out.size(), 1);
    }

    void filterDropsNonZeroLayer()
    {
        const auto out =
          filterCandidates({ win({ 0, 0, 200, 200 }, 1, 25) }, 999);
        QVERIFY(out.isEmpty());
    }

    void filterDropsTransparent()
    {
        const auto out = filterCandidates(
          { win({ 0, 0, 200, 200 }, 1, 0, 0.0) }, 999);
        QVERIFY(out.isEmpty());
    }

    void filterKeepsFaintButVisible()
    {
        const auto out = filterCandidates(
          { win({ 0, 0, 200, 200 }, 1, 0, 0.05) }, 999);
        QCOMPARE(out.size(), 1);
    }

    void filterDropsTooSmall()
    {
        QVERIFY(filterCandidates({ win({ 0, 0, 39, 200 }, 1) }, 999).isEmpty());
        QVERIFY(filterCandidates({ win({ 0, 0, 200, 39 }, 1) }, 999).isEmpty());
        QCOMPARE(filterCandidates({ win({ 0, 0, 40, 40 }, 1) }, 999).size(), 1);
    }

    void filterHonoursCustomMinSize()
    {
        QCOMPARE(filterCandidates({ win({ 0, 0, 10, 10 }, 1) }, 999, 5).size(),
                 1);
        QVERIFY(
          filterCandidates({ win({ 0, 0, 10, 10 }, 1) }, 999, 11).isEmpty());
    }

    void filterDropsOwnPid()
    {
        const auto out = filterCandidates(
          { win({ 0, 0, 200, 200 }, 1, 0, 1.0, 555),
            win({ 0, 0, 200, 200 }, 2, 0, 1.0, 100) },
          555);
        QCOMPARE(out.size(), 1);
        QCOMPARE(out.first().id, quint32(2));
    }

    void filterPreservesOrder()
    {
        const auto out = filterCandidates({ win({ 0, 0, 100, 100 }, 3),
                                            win({ 0, 0, 100, 100 }, 1),
                                            win({ 0, 0, 100, 100 }, 2) },
                                          999);
        QCOMPARE(out.size(), 3);
        QCOMPARE(out[0].id, quint32(3));
        QCOMPARE(out[1].id, quint32(1));
        QCOMPARE(out[2].id, quint32(2));
    }

    void windowAtPicksFrontmostOfOverlap()
    {
        const QList<WindowInfo> list = { win({ 0, 0, 100, 100 }, 1),
                                         win({ 50, 50, 100, 100 }, 2) };
        const auto hit = windowAt(list, QPoint(60, 60));
        QVERIFY(hit.has_value());
        QCOMPARE(hit->id, quint32(1));
        const auto behind = windowAt(list, QPoint(120, 120));
        QVERIFY(behind.has_value());
        QCOMPARE(behind->id, quint32(2));
    }

    void windowAtEdgesRightBottomExclusive()
    {
        const QList<WindowInfo> list = { win({ 10, 10, 100, 100 }, 1) };
        QVERIFY(windowAt(list, QPoint(10, 10)).has_value());
        QVERIFY(windowAt(list, QPoint(109, 109)).has_value());
        QVERIFY(!windowAt(list, QPoint(110, 50)).has_value());
        QVERIFY(!windowAt(list, QPoint(50, 110)).has_value());
        QVERIFY(!windowAt(list, QPoint(9, 50)).has_value());
    }

    void windowAtEmptyList()
    {
        QVERIFY(!windowAt({}, QPoint(1, 1)).has_value());
    }

    void windowAtCursorOffAllWindows()
    {
        const QList<WindowInfo> list = { win({ 0, 0, 100, 100 }, 1),
                                         win({ 200, 0, 100, 100 }, 2) };
        QVERIFY(!windowAt(list, QPoint(150, 50)).has_value());
    }

    void overlayRectOriginAtZero()
    {
        QCOMPARE(toOverlayRect(
                   { 100, 200, 300, 400 }, QPoint(0, 0), { 0, 0, 1440, 900 }),
                 QRect(100, 200, 300, 400));
    }

    void overlayRectLeftMonitor()
    {
        // Overlay on a monitor left of the primary, top-left at (-1440, 0).
        QCOMPARE(toOverlayRect({ -1340, 100, 300, 400 },
                               QPoint(-1440, 0),
                               { 0, 0, 1440, 900 }),
                 QRect(100, 100, 300, 400));
    }

    void overlayRectMonitorAbove()
    {
        // Overlay on a monitor above the primary, top-left at (0, -900).
        QCOMPARE(toOverlayRect({ 50, -800, 300, 400 },
                               QPoint(0, -900),
                               { 0, 0, 1440, 900 }),
                 QRect(50, 100, 300, 400));
    }

    void overlayRectClipsToOverlay()
    {
        // Window hangs off the left and bottom edges of the overlay.
        QCOMPARE(toOverlayRect({ -100, 800, 400, 400 },
                               QPoint(0, 0),
                               { 0, 0, 1440, 900 }),
                 QRect(0, 800, 300, 100));
    }

    void overlayRectFullyOutsideIsEmpty()
    {
        QVERIFY(toOverlayRect({ 2000, 0, 300, 300 },
                              QPoint(0, 0),
                              { 0, 0, 1440, 900 })
                  .isEmpty());
    }

    void classifyPressThreshold()
    {
        QCOMPARE(classifyPress({ 10, 10 }, { 10, 10 }), PressKind::Click);
        QCOMPARE(classifyPress({ 10, 10 }, { 14, 10 }), PressKind::Click);
        QCOMPARE(classifyPress({ 10, 10 }, { 15, 10 }), PressKind::Drag);
        QCOMPARE(classifyPress({ 10, 10 }, { 10, 5 }), PressKind::Drag);
    }

    void classifyPressDiagonalUsesManhattan()
    {
        // 3 + 3 = 6 > 4 even though each axis is within the threshold.
        QCOMPARE(classifyPress({ 0, 0 }, { 3, 3 }), PressKind::Drag);
        QCOMPARE(classifyPress({ 0, 0 }, { 2, 2 }), PressKind::Click);
        QCOMPARE(classifyPress({ 0, 0 }, { -2, -2 }), PressKind::Click);
        QCOMPARE(classifyPress({ 0, 0 }, { -3, 2 }), PressKind::Drag);
    }

    void classifyPressCustomThreshold()
    {
        QCOMPARE(classifyPress({ 0, 0 }, { 8, 0 }, 8), PressKind::Click);
        QCOMPARE(classifyPress({ 0, 0 }, { 9, 0 }, 8), PressKind::Drag);
    }

    // With no window under the cursor (menu bar, desktop, screen corner) the
    // target is the whole overlay, so a click takes a full-screen capture.
    void hoverTargetFallsBackToWholeScreen()
    {
        const QList<WindowInfo> list = { win(QRect(100, 100, 400, 300), 1) };
        const QRect overlay(0, 0, 1512, 982);
        QCOMPARE(hoverTarget(list, QPoint(2, 2), QPoint(0, 0), overlay), overlay);
        QCOMPARE(hoverTarget({}, QPoint(2, 2), QPoint(0, 0), overlay), overlay);
    }

    void hoverTargetUsesWindowUnderCursor()
    {
        const QList<WindowInfo> list = { win(QRect(100, 100, 400, 300), 1) };
        QCOMPARE(hoverTarget(list,
                             QPoint(150, 150),
                             QPoint(0, 0),
                             QRect(0, 0, 1512, 982)),
                 QRect(100, 100, 400, 300));
    }
};

QTEST_MAIN(TestWindowPick)
#include "tst_windowpick.moc"
