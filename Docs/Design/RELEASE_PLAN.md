# RELEASE PLAN: on itch in 6 hours of AI time and 3 to 4 hours of Dylan's

Written 2026-11-13 by GAME, at Dylan's request. **This file governs the rest of the jam.** It overrules `BUILD_SPEC.md` section 7 (backlog), the O2 to O9 scope in section 3e, and the day tables in `WORKERS.md`. The game design, architecture and session protocol in `BUILD_SPEC.md` still hold.

Deadline: **Nov 15, 5:59 PM PST**. Target upload: **Nov 15, 2:00 PM PST**.

**What "release" means here (Dylan, Nov 13):** a build on the game's own itch page for testers and feedback, not the jam submission. The six AI hours end at that tester build (after R5, or R6 if it fits). The jam submission follows on Nov 15 from the same page, after feedback. **The first build testers see is the full terrain world, with sound, voice and the cast** (first impressions count; no arena build is pushed). So: put a "what to tell me" line on the itch page (where you got lost, what was not funny, frame rate and your GPU), and spend R7 on what testers say, not only on Dylan's own play. Each later push replaces the build through `upload-itch.ps1`, with Dylan's go-ahead every time. **ChatGPT, not GAME, does every itch deployment** (see "Who runs it").

## Where things stand (checked on disk, Nov 13)

| Piece | State |
| --- | --- |
| The film (Backlot: three scenes, notes, wrap, Premiere, credits) | Works, packaged, 57 `Jam.*` tests, autoplay PASS. Has music, **no sound effects, no voice** |
| Open world framework | **O1 only.** `-open` builds a world from the table; fixed camera, no bot, no quests. O2 to O9 not started |
| Terrain `/Game/World/L_Sugarborough` | Delivered: 85,091 baked instances, landmarks, 19 locations, cooks and launches packaged with `-open`. Latest polish passes are **uncommitted and not repackaged**. Performance not measured in play |
| Fenella's voice | 100 English clips in `vocals/output/fenella/`, already named `VO_<LineId>`. Not imported |
| Sound effects | 47 in `vocals/output/sfx/`. Not imported; the game has no SFX code |
| Cast voices, battle cries | `vocals/output/cast/` (8 roles), `vocals/output/attacks/<lang>/` (8 languages, 24 English). Not imported |
| Characters | `SK_Knight` (new look, all 8 clips), `SK_Mage`, `SK_Archer`, `SK_Barbarian` (Idle, Walk only) in `ImportSource/Meshy/SK_*_Rig/`. Not imported; the game still uses `SK_Hero_Rig` |
| Theme song (Big Rock Candy Mountain) | Lyrics in `Docs/BigRockCandyMountainAdjustedLyrics.txt`. **No audio file found in the repo** |
| Music | 9 cues in the game; `village`, `title-theme` and 9 more in `music/` not imported |
| Niagara attack effects (FX) | 33 systems, review maps only, not wired to gameplay |
| Backend (typed notes) | Built and tested, **not deployed** |

## What ships

The film, with sound and voice, playable by one of four characters, set in the candy world if the gate passes.

- **The game we ship is the new world** (Dylan, Nov 13): the three scenes played at the film sets of `L_Sugarborough`, with a follow camera and a compass to the next set. The world is something to walk through between scenes, not a quest system.
- **Fallback only if R4 fails its time box or the packaged world will not run:** the Backlot arenas with the same sound and voice. Candidate A (R3) exists so there is always something to submit, not because it is the plan.
- **Stretch:** pick the knight, mage, archer or barbarian on the title screen.

**Cut, do not start:** the quest generator, villager quest givers and dialogue, typed quest requests (O3, O5, O8), the TUNING worker, wiring the Niagara attack effects, non-English voice (stays on disk for after the jam), new terrain or asset generation, T24 street gags.

## Tasks, in order

Each task is one commit and ends with: build, `Jam.*` tests, Backlot autoplay PASS, a screenshot or log looked at. Stop at the time box and take the fallback; do not borrow time from a later task.

| ID | Time | Task | Done when | If it overruns |
| --- | --- | --- | --- | --- |
| R0 | 0:15 | Checkpoint what is on disk: scoped commits of the terrain, vocals scripts and docs. Tag `pre-release`. Never the `DefaultEngine.ini` token block | `git status` shows only ignored and source-audio files | Commit docs only |
| R1 | 1:00 | **Sound.** Import the 47 SFX to `/Game/Audio/SFX` and the English Fenella clips to `/Game/Audio/VO`. One table (`JamSfx`: event to cue, volume) played from the cosmetic events that already exist (list below). An Effects volume in `UJamSettings` | Autoplay log has no missing-clip warnings for imported lines; Dylan hears a lecture, a CUT!, a wrap | Ship the 12 loudest events only |
| R2 | 0:15 | **Theme song in the credits only** (and Dylan's trailer); it is a different style from the score and stays off the title and out of play. Import `village` for the Open village. `Docs/Declared.md` line for the song | Credits play it in a packaged run | Credits keep the `credits` cue |
| R3 | 0:00 | Dropped as a separate step (Dylan: no arena build goes out). Backlot is packaged and smoked inside R5, which keeps it as the local fallback | | |
| R4 | 2:00 | **The world, slim.** Follow camera (BUILD_SPEC 3e "Camera and controls"). In Open mode the three scenes run at the table's `FilmSet` locations in order; a HUD compass marker and distance point to the next set; the hero starts in the village; ground found by tracing down; a few ambient nuisances at camps near the hero (XP only). Music by biome. The bot walks to each set and plays it | `JamAutoplay: PASS mode=open main=3` on `L_Sugarborough` in editor `-game`; a screenshot behind the hero with Fenella in frame | Drop ambient camps, then biome music. If the bot cannot pass by 2:00, Open stays behind `-open` and R5 is skipped |
| R4b | 1:00 | **They hit back.** Goblins, bugs and the boss target charge the hero when he is near and they are not fleeing; rats never attack. A hit knocks him back, staggers him briefly and drains a Dignity meter on the HUD; at zero the take fails through the existing failed-take path (RETAKE!, retake resets the meter). The rule (charge range, damage, stagger, meter) is plain C++ with `Jam.*` tests | Tests pass; Backlot autoplay PASS and `JamAutoplay: PASS mode=open main=3` still pass with attackers on | Knockback only, no meter |
| R5 | 0:45 | **Candidate B and the gate:** package, smoke both modes, zip. Dylan plays ten minutes of each | Both PASS lines in the packaged log | See the gate below |
| R6 | 1:00 | **Cast.** Import `SK_Knight` (run `correct_sword_raise.py` on its raise) as the hero. `FJamClassDef` rows for the four; `-class=` and a "Casting" line on the title screen. All four use the knight's skills; the other three play Idle and Walk and their English battle cry on a cast | `-class=mage` passes Backlot autoplay; lineup screenshot | New knight only; the other three stay out |
| R7 | 0:30 | The three worst things from Dylan's play, nothing else | Dylan agrees | Skip |
| R8 | 0:15 | Final package, smoke, `zip-submission.ps1 -TeamName ImaginaryComponent -GameName TheDreadLandOfCandy`, credits and tools list, `Declared.md` reconciled | Zip on disk, under 1 GB | None |

Total 7:00 (R4b added by Dylan, Nov 13). Order for the first push: R0, R1, R2, R4, R4b, R6, then R5 (package, smoke, Dylan plays, push). R7 and R8 come after tester feedback. If time runs short, R6 shrinks to the new knight alone before anything else is cut.

**The gate (after R5):** the world is the default. `GameDefaultMap` becomes `/Game/World/L_Sugarborough`, no flag means Open, and Backlot stays behind `-backlot`. It is reversed only if packaged Open fails its smoke test or does not hold a playable frame rate on Dylan's machine while walking and fighting; then Candidate A's flow ships and Open stays behind `-open`.

**R1 event list** (cue names are the files in `vocals/output/sfx/`): lecture cast `Sword_Raise` then `Hero_Inhale`, release `Lecture_Impact`; `FormLine_Contact`, `DisappointedSigh`, `LastJudgment_Contact`; `Flustered`, `Pacify`, `Interact_Complete`; `Bubble_Pop` per bubble; slate and ACTION! `Clapper`; `Cut_Horn`; `Retake_Rewind`; `Stage_Complete`; wrap `Ledger_Deduction` per line then `Coin_Trickle`; `LevelUp`; `UI_Click`, `UI_Back`, `UI_Unavailable`; `Budget_Warning` and `Budget_Empty` at the budget-out; `Backlot_Ambience_Loop`; `Quest_Accept` on entering a film set in Open mode. Footsteps and the Fenella fly loop only if time is left in the box.

## Dylan's time (about 3.5 hours)

| When | Minutes | What |
| --- | --- | --- |
| Now | 5 | Put the theme song at `music/theme-song.mp3` (or `.wav`). Stop any Codex session that still has the engine; terrain is frozen |
| Before R3 ends | 45 | **Deploy the backend** (`backend/DEPLOY.md`, off stream), give GAME the URL. This is the Ambition score. Not done by the end of Nov 14: the text box ships hidden and the three suggestion buttons stay |
| After R5 | 25 | Play the world build. Say whether it runs well enough to ship. Approve the look |
| After R6 | 10 | Approve the knight and the cast |
| Nov 15 morning | 40 | Record the 30 to 60 second video (sword fakeout, CUT!, ledger, budget-out, a walk through the world), upload it |
| Nov 15 | 30 | With ChatGPT: itch page, tools used, team details; say "go" for its `upload-itch.ps1` run; download the zip, unzip, run it |
| Nov 15 | 15 | Publish the dev log link (Glass Box) |
| Spare | 20 | |

## Who runs it

GAME (Claude) takes R0 onward while quota lasts. Every task is written so Codex can pick it up from `STATUS.md` with this prompt: "You are GAME. Read `CLAUDE.md`, `Docs/STATUS.md` and `Docs/Design/RELEASE_PLAN.md`. Take the next R task, finish it to its 'done when' or its fallback inside the time box, commit, update STATUS and DEVLOG." One session in the checkout at a time; whoever holds it holds the engine.

**Itch deployment is ChatGPT's (Dylan, Nov 13), to save Claude's quota for Nov 14.** That covers the itch page text, the "what to tell me" line, and every `upload-itch.ps1` (butler) push, tester builds and the jam submission alike, each with Dylan's go-ahead. GAME's part stops at a zip on disk: R5 and R8 end with `zip-submission.ps1` and a hand-off line in `STATUS.md` (zip path, size, what changed since the last push, the PASS lines). GAME does not run `upload-itch.ps1`.
