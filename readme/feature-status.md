# Feature Status (masharratt/flameshot fork)

**Last Updated:** 2026-10-06 (ruler tool added)

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
| Unit test harness | beta | QtTest executables registered with CTest under `tests/unit`. | Qt6::Test | Covers strfparse and ruler math so far. |
| Capture-first workflow | stub | Save immediately on selection, edit optionally afterwards. | Unit test harness | Not started. |
| Hover window detection | stub | Hover highlights a window; drag draws a region. | macOS CGWindowList | Not started. |
| Capture history | stub | Browsable list of past captures. | Capture-first workflow | Not started. |
| Per-hotkey after-capture actions | stub | Each hotkey runs its own list of actions. | Capture-first workflow | Not started. |
| Image effects | stub | Border, rounded corners, drop shadow. | Per-hotkey actions | Not started. |
| Ruler tool | beta | Editor tool that draws a measured line labelled with length and width x height in physical pixels. | Tool system | Not yet checked by hand in the running app. |
| GIF / MP4 recording | stub | Record a screen region to MP4 or GIF. | ScreenCaptureKit, AVFoundation | Not started. |
