# DEVLOG

## 2026-11-12 07:25-07:50, GAME (Claude Code, Opus 5.5)

- Built `Scripts/compare-models.ps1` and its page: a three.js side-by-side viewer for the Meshy / Rodin / fal twins. First version showed nothing: model paths resolved under `/Scripts/` and 404'd. Port 8765 was already taken by another local server. Verified fixes in a headless Chrome screenshot.
- Dylan picked from it: Meshy hero and peppermint, Rodin rats, fal houses, Rodin sword (too short). He ran the approvals and started batch1.
- T01: hero, player state with the ASC, three nuisances, game mode, runtime Enhanced Input, `-autoplay` (logs `JamAutoplay: PASS pacified=3/3`). Two misses on the first screenshot: the spec's 1100 uu camera put the nuisances off-screen (now 1700 uu with a look-ahead), and tints did nothing because the basic shapes default to the grid material.
- Slip: while reading config I printed `DefaultEngine.ini` in full, including the editor's local Android file server token. Local dev setting, not committed.
- Screenshot: `Saved/Screenshots/WindowsEditor/t01.png`.
