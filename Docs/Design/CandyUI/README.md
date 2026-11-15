# Candy UI samples — 15 November 2026

Design preparation only, requested by Dylan. No engine, game, build, or automation was launched. No runtime files were changed. Open `index.html` locally to compare the two generated concept boards.

## Selected direction: D — Black Sugar / Frostbound

Dylan selected Black Sugar and authorized implementation. See `IMPLEMENTATION.md` for the source checkpoint and engine verification still required; font files and provenance are in `FONTS.md`. Earlier recommendations of A or C are superseded.

Dylan rejected A/B as too kiddie. The current exploration is **C — Saccharine Reliquary** and **D — Black Sugar / Frostbound**. See `dark-fantasy.html` and `DARK_FANTASY.md`. A/B remain archived comparisons; their cheerful palette, typography and motion proposals are superseded. The source integration seams and preservation requirements below still apply. The earlier recommendation below is historical.

## Earlier directions (rejected)

**A — Frosted Stickerbook (recommended):** marshmallow panels, plum outlines, strawberry and mint buttons, butter-yellow selection, wafer shadows. Friendly, readable, and closest to the existing gingerbread village. Candy decoration belongs around panel edges; keep text surfaces quiet.

**B — Candy Arcade:** raspberry jelly, bright cyan, lemon focus, bold comic tabs and glossy lozenges. More energetic, with restrained starbursts for important events. Use cream text surfaces so saturated colours do not overwhelm subtitles or objectives.

The boards are AI-generated art direction, not engine captures or import-ready UI atlases. Their backgrounds are illustrative; current geometry and characters are not promised to change. Any shortened labels and omitted data are presentation sketches. Preserve every live option and stat when implementing. In particular retain commentary, current quest text, notes taken/refused, recording/timecode, crop/danger meters, character allocation, and both input-device layouts.

## Proposed shared design tokens

| Token | A: Frosted Stickerbook | B: Candy Arcade |
| --- | --- | --- |
| Surface | `#FFF3D6` | `#FFF7E8` |
| Ink / outline | `#40234F` | `#35204F` |
| Primary | `#F65B91` | `#FA4385` |
| Secondary | `#4CD6C0` | `#20CBE5` |
| Focus | `#FFD65A` | `#FFE24C` |

Values are sRGB design tokens; convert correctly to FLinearColor rather than treating hex channel values as linear values. Text stays dark ink on cream. Bright pink/cyan are accents; verify contrast before using them behind text.

At the existing 1920 × 1080 design size: 48 px outer safe margin, 24 px panel padding, 16 px gaps, 4 px outline, 6 px offset shadow, 24–32 px corners. Suggested type: 72 px screen heading, 36 px action label, 30 px body/subtitle, 24 px secondary labels. Select a licensed rounded font with required glyph coverage and package its fallback; do not rely on the workstation having it installed.

## Layout and behaviour

- Menus: one wafer card, clear heading, large rows at least 64 px high. The current pause menu has nine actions; fit all nine or deliberately scroll, including mouse/gamepad focus visibility. The board compresses the volume section for visual exploration; implementing separate sliders is a distinct input/layout change and requires verification.
- Gameplay: compact scene/dignity block at top left; compass and objective in separate vertical bands; pause at top right. Skills and level at bottom left. Keep the centre open. Bottom-centre subtitle card must wrap long Fenella lines without covering skills. Show crops/danger when active.
- Focus: yellow outer ring plus star/arrow marker. Hover gets a raised outline; pressed moves down 3 px. Disabled uses muted fill plus a lock or explanatory text. Cooling skill gets a fill wipe, seconds, and dim icon; ready state restores the full icon and a clear border. Preserve key/controller badges.
- Motion proposal: 120 ms hover lift, 180 ms card settle, one short level-up star pop. No continuous wobble on reading surfaces. Respect any future reduced-motion setting; avoid flashing focus.
- Extend the chosen theme to title/casting, options, controls, pause, quit confirmation, journal, character sheet, note editor, CUT/RETAKE, wrap ledger, premiere and credits. Maintain the film-director jokes and existing authored copy.

## Future implementation handoff

The current renderer builds transient UMG trees in C++, despite the older project preference for WBP layouts. Start with the existing architecture and keep behaviour/data contracts intact.

1. Introduce a shared UI style definition for colours, fonts, brushes, padding and state treatment. Build rounded panels/buttons from Slate/UMG primitives. Commission only small reusable sprinkle, wafer and peppermint accents after a direction is selected; use scalable nine-slice brushes for decorative borders, not full-screen rasterized text.
2. Apply menu styling in `Source/MegaJam/Private/UI/JamScreenWidget.cpp`: `MakeText`, `MakePanel`, `BuildTree`, and selection rendering in `ApplyView`. Preserve `OptionUnder` geometry and the existing input delegate flow, with focus matching actual hit targets.
3. Apply HUD treatment in `Source/MegaJam/Private/UI/JamHudWidget.cpp`, common card colours in `JamHudView.cpp`, and the native canvas compass in `Source/MegaJam/Private/Game/JamHUD.cpp`. Audit both paths so the compass, skill tray and UMG cards use the same visual language. Reconcile current uncommitted work before editing these files.
4. Keep menu actions/copy in `JamScreens.cpp` and gameplay state in the current view models. Skin the note editor and wrap view with the same style. Do not alter quest logic, skill rules, progression or control bindings as part of the reskin.
5. Once engine access is explicitly available: build and relevant UI/input tests, inspect each screen with keyboard, mouse and gamepad at 1080p and 720p, check long quests/quotes, nine-row pause, character allocation, cooling/disabled states, journal scrolling and focus after closing menus. Inspect objective/compass/subtitle overlap in gameplay. Package/smoke only in the coordinated engine slot.

Existing screenshots inspected: `Saved/Screenshots/WindowsEditor/batch2-pause.png` and `batch2-xp.png`. They are historical reference, not evidence of the current dirty source build. No runtime validation is claimed for these samples.
