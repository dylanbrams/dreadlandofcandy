# Checkpoint 1: The Dread Land of Candy, Nov 14 2026, 10:00

What exists after 3 days of the jam (Nov 12 to Nov 14 morning, plus Nov 11 prep). Facts come from `git log`, `Docs/DEVLOG.md`, `Docs/STATUS.md` and `Docs/Declared.md`. Where something is a guess it says so.

## One paragraph

A top-down film-shoot comedy. A knight, cast in a movie set in a candy land, lectures rats, goblins, bugs and almonds into behaving while a faerie director (Fenella) films him, narrates, and sulks when her notes are refused. Three scenes with takes and retakes, a ledger at each wrap, loot, ranks and three skills, four playable classes, an open PCG candy world, and a typed "director's note" at CUT! that a Claude Haiku backend turns into the next shot. The finale runs out of budget: the world goes grey and the bugs become boxes labelled BUG.

## Timeline

| Day | What happened |
| --- | --- |
| Nov 11 (prep, declared) | UE 5.8 skeleton, scripts, mesh-generator clients, GAS test template, design briefs. Build spec, joke sheet and style lock written by Claude Fable 5.2. Worker split decided. |
| Nov 12 | 07:49 first hero on screen; 11:23 whole film plays under a bot; afternoon: HUD, Fenella, CUT! notes, wrap card, title/pause/Premiere/credits, skills, music, art pass, rigged knight, voice hookup; evening: finale waves, typed notes, generated Premiere, look pass, open-world spec. ~80 commits. ChatGPT starts terrain and Niagara. |
| Nov 13 | Dense PCG candy world; three more classes generated and rigged; ElevenLabs SFX and voices; release plan R0 to R8; 47 SFX + 88 Fenella lines + theme song; open-world mode plays the film; enemies hit back (Dignity meter); typed notes live on the deployed backend; four classes playable; first package (689 MB zip); Dylan's 18-item play list; class gestures, stowed weapons, brushing foam; itch page copy. |
| Nov 14 (so far) | Test-fix-release loop. Batches 0 to 2 fixed (clickable HUD, compass overlap, grey-screen bug, camera controls, controls card, numeric XP). Backend redeployed (n2). 72 tests pass. No package or upload since batch 0, by Dylan's choice. |

## By the numbers

- 142 commits. 113 carry a Claude co-author trailer (Fable 5.2, Opus 5.5, Sonnet 5.5); 29 do not (ChatGPT/Codex or Dylan, to be confirmed). Per day: Nov 11: 17, Nov 12: 80, Nov 13: 34, Nov 14: 11.
- ~17,500 lines of C++ under `Source/`; 72 `Jam.*` automation tests; a bot (`-autoplay`) that plays title to credits in about 2 minutes.
- Audio: 47 SFX, 88 voiced Fenella lines (ElevenLabs), 18 music tracks and a theme song with Dylan's vocals (ACE Studio).
- Content: 4 classes, 3 scenes, 12 skill icons, 30+ generated models.
- Package: Win64 Development, zip 689 to 721 MB (limit 1 GB).

## Who did what (Claude side)

| Model | Role | Evidence |
| --- | --- | --- |
| Fable 5.2 | Specs and plans: build spec, joke sheet, worker split, spec 3d/3e, release plan | Commits Nov 11 22:00 to 22:36, Nov 12 17:34 and 18:52, Nov 13 17:48 to 18:00 |
| Opus 5.5 | GAME worker (only one that opens the editor): all of T01 to T23, O1, R0 to R6, Nov 14 planning | ~85 commits |
| Sonnet 5.5 | Pre-jam setup, RULES worker (ledger, progression, note clamp, scenes), BACKEND (director service, Nov 14 redeploy), this checkpoint | Commits with that trailer |

Gantt: `gantt/gantt.png`, rows in `gantt/work_log.csv`. ChatGPT side: not yet gathered; `gantt/chatgpt_candidates.csv` lists the 14 groups of untrailered commits as a starting point.

## What went wrong (kept for the honest slide)

- Printed `DefaultEngine.ini` on stream including a local token (not committed).
- Three background packages killed for low memory.
- A package "passed" because the exit code was lost in a pipe; the smoke test ran the old exe.
- Unity-build name clashes failed only the game target; an ini `//` cut the backend URL so typed notes silently fell back.
- The generated rolling pin had extra handles; rebuilt in Blender.
- Dylan rejected the shared knight gestures on the other classes; replaced with per-class presentation.

## Open items

Hands-on checks (swapped mouse buttons, camera feel, typed directions, cast visuals, golden rain), game HTTPS to the backend, one package + smoke + zip, itch tester push on Dylan's go, submission by Nov 15 5:59 PM PST.
