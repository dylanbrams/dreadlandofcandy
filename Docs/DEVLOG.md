# DEVLOG

## 2026-11-12 07:25-07:50, GAME (Claude Code, Opus 5.5)

- Built `Scripts/compare-models.ps1` and its page: a three.js side-by-side viewer for the Meshy / Rodin / fal twins. First version showed nothing: model paths resolved under `/Scripts/` and 404'd. Port 8765 was already taken by another local server. Verified fixes in a headless Chrome screenshot.
- Dylan picked from it: Meshy hero and peppermint, Rodin rats, fal houses, Rodin sword (too short). He ran the approvals and started batch1.
- T01: hero, player state with the ASC, three nuisances, game mode, runtime Enhanced Input, `-autoplay` (logs `JamAutoplay: PASS pacified=3/3`). Two misses on the first screenshot: the spec's 1100 uu camera put the nuisances off-screen (now 1700 uu with a look-ahead), and tints did nothing because the basic shapes default to the grid material.
- Slip: while reading config I printed `DefaultEngine.ini` in full, including the editor's local Android file server token. Local dev setting, not committed.
- Screenshot: `Saved/Screenshots/WindowsEditor/t01.png`.

## 2026-11-12 08:00, GAME

- T02: first package (BuildCookRun about 4 min), smoke test with `-autoplay` passes. Added smoke checks for `JamAutoplay: PASS` and for constructor-loaded assets missing from the cook. `/Engine/BasicShapes` cooked fine.
- A screenshot from the packaged exe (`-shotat` works there too) matches the editor run.
- Screenshot: `Builds/Win64-Development/MegaJam/Saved/Screenshots/Windows/t02-packaged.png`.

## 2026-11-12 08:25, GAME

- T05: nuisances with five states (Wander, Flee, Flustered, Pacified, Treated), navmesh-free steering inside a set box, hold-E interact through the game mode, a canvas prompt. Hero now aims at the cursor or right stick (spec controls), since moving targets need aiming.
- The first autoplay bot never triggered Flee: it lectured from 550 uu, just outside the rats' 500 uu flee radius. Moved it in to 450 and added a flee count to the PASS line (`flees=7`) rather than hoping a timed screenshot catches it.
- Refused casts logged every frame from the bot; moved that to Verbose.
- Dylan reviewed batch1 in the browser viewer: all fine except Fenella (no wings because the prompt said "no wings", and a 35mm stuck on her eye). Redo wording in STATUS.
- Screenshot: `Saved/Screenshots/WindowsEditor/t05-flee2.png`.
