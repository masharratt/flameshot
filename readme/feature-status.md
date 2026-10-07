# Feature Status (masharratt/flameshot fork)

**Last Updated:** 2026-10-07 (local certificate signing so Screen Recording permission survives rebuilds)

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
| Unit test harness | beta | QtTest executables registered with CTest under `tests/unit`. | Qt6::Test | Covers strfparse, ruler math, capture-first request, toast stacking, window picking, history store, action lists, hotkey helpers and image effects. |
| Capture-first workflow | beta | Hotkey and tray captures save and copy on selection release, then show a toast with Edit, Copy, Pin and Show in Finder. Edit opens a normal window with a fixed toolbar on the active Space. Option `captureFirst`, default on. | Unit test harness | Edit window has no Dock icon. Esc closes it only when the canvas has focus. |
| Hover window detection | beta | Picker shows a dotted outline around the window under the cursor, or the whole screen when no window is there; a click captures it, a drag over 4 px draws a region. No purple help box in picker mode. Option `hoverWindowDetection`, default on. | macOS CGWindowList | macOS only. |
| Capture history | beta | Every save is logged to history.jsonl; a thumbnail window (tray item or Cmd+Shift+Y) offers Open, Edit, Copy, Show in Finder and Remove. Option `captureHistoryMax`, default 500. | Capture-first workflow | Not yet checked by hand. Editing an image larger than the screen saves a numbered copy. |
| Per-hotkey after-capture actions | beta | TAKE_SCREENSHOT and CAPTURE_AND_EDIT each run a configurable action list (save, copy image, copy path, open editor, pin, effects, toast) set on the Workflows tab. Hotkeys re-register live when changed. | Capture-first workflow | Not yet checked by hand. |
| Image effects | beta | Border, rounded corners and drop shadow applied by the Apply Effects action, configured with a live preview on the Workflows tab. | Per-hotkey actions | Not yet checked by hand. Effects needing transparency save as PNG only on the workflow save path. |
| Ruler tool | beta | Editor tool that draws a measured line labelled with length and width x height in physical pixels. | Tool system | Not yet checked by hand in the running app. |
| GIF / MP4 recording | dev | RECORD_MP4 and RECORD_GIF hotkeys (and tray items) pick an area, record it with ScreenCaptureKit, and save MP4 or GIF with a history entry and toast. Options `gifFps`, `gifMaxWidth` on the Workflows tab. | macOS 15+, ScreenCaptureKit, AVFoundation, ImageIO | GIF verified by hand once; MP4 not yet. macOS 15+ only, hidden elsewhere. No audio. |
| Expand canvas | beta | Editor toolbar button that adds 40 pt of white space on every side so you can draw outside the image; drawings move with the image and Cmd+Z undoes it. | Windowed editor | Window grows but does not shrink on undo. Shortcut E. |
| Editor settings bar | beta | Second toolbar row in the windowed editor with colour swatches from the user palette, a custom colour button and a size slider, kept in sync with all other ways of changing them. | Windowed editor | Every size change is saved as the new default. |
| Bendable arrows and lines | beta | Arrows and lines show a midpoint handle; dragging it bends them into a smooth curve with the arrowhead following the tangent. Undoable. | Tool system | None known. |
| Editor select tool | beta | Pointer button (V) in the editor toolbar that turns drawing off so existing objects can be selected, moved, bent or re-edited. | Windowed editor | None known. |
| Editor hide and Cmd+Tab | beta | Edit windows hide when another app is activated and return via Cmd+Tab, the Dock or the menu bar; a Dock icon appears only while editors are open and the app is in the background. Clicking the toast picture opens Edit. | Windowed editor | Opening the menu bar menu hides editors until the app is reactivated. |
| Local certificate signing | beta | `CODE_SIGN_IDENTITY` plus `CODE_SIGN_HARDENED_RUNTIME=OFF` signs with a self-signed local certificate, giving every rebuild the same identity so macOS keeps the Screen Recording grant. | codesign, local Keychain certificate | Hardened runtime must stay off with a self-signed certificate or bundled Qt frameworks fail to load. |
