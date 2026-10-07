# Feature Status (masharratt/flameshot fork)

**Last Updated:** 2026-10-06 (capture history added)

| Token | Meaning |
|-------|---------|
| `prod` | Live, verified, in daily use. |
| `beta` | Feature-complete, under verification. |
| `dev` | In development. |
| `stub` | Scaffold only. |
| `deprecated` | Scheduled for removal. |

Plan: `planning/PLAN_sharex_fork.md` (local, not committed).

| Feature | Status | Description | Dependencies | Known Limitations |
|---------|--------|-------------|--------------|-------------------|
| Upstream capture + editor | prod | Flameshot 14/15 region capture with annotation editor. | Qt 6 | Editor must finish before capture is saved. |
| Unit test harness | beta | QtTest executables registered with CTest under `tests/unit`. | Qt6::Test | Covers strfparse, ruler math, capture-first request, toast stacking, window picking and history store. |
| Capture-first workflow | beta | Hotkey and tray captures save and copy on selection release, then show a toast with Edit, Copy, Pin and Show in Finder. Option `captureFirst`, default on. | Unit test harness | Not yet checked by hand. Edit reopens the editor on a black backdrop around the capture. |
| Hover window detection | beta | Before a selection exists, hovering outlines the window under the cursor; a click selects it, a drag over 4 px draws a region. Option `hoverWindowDetection`, default on. | macOS CGWindowList | Not yet checked by hand. Highlight appears after the first mouse move. macOS only. |
| Capture history | beta | Every save is logged to history.jsonl; a thumbnail window (tray item or Cmd+Shift+Y) offers Open, Edit, Copy, Show in Finder and Remove. Option `captureHistoryMax`, default 500. | Capture-first workflow | Not yet checked by hand. Editing an image larger than the screen saves a numbered copy. |
| Per-hotkey after-capture actions | stub | Each hotkey runs its own list of actions. | Capture-first workflow | Not started. |
| Image effects | stub | Border, rounded corners, drop shadow. | Per-hotkey actions | Not started. |
| Ruler tool | beta | Editor tool that draws a measured line labelled with length and width x height in physical pixels. | Tool system | Not yet checked by hand in the running app. |
| GIF / MP4 recording | stub | Record a screen region to MP4 or GIF. | ScreenCaptureKit, AVFoundation | Not started. |
