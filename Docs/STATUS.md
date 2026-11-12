# STATUS (GAME, 2026-11-12 12:30, day 1)

Read this, then `Docs/Design/BUILD_SPEC.md` (governs), then `CLAUDE.md`. Workers: `Docs/Design/WORKERS.md`.

## Next task

**Schedule: well ahead.** Day 1's backlog was done by 10:00 on day 1, and T10 to T14 (day 2) are done too. **Day 2 tasks left, in order:** T15 scene beats and tuning (all three scenes already play), T16 title, pause, Premiere, credits.
**Dylan: please play the film once** (editor `-game`, or `.\Scripts\screenshot.ps1 -At 999`). Dylan found the hero stuck behind pacified rats (fixed in T07: nuisances no longer block the hero). Hold E or A takes a note at CUT!, and Q or B refuses it.
Workers: RULES says days 1 and 2 are done (`rules` merged at `d9d9b51`). ASSETS: generation finished (batch1 and batch2, Fenella regenerated with wings and a camcorder); meshes need reducing and a look in the game camera (T20). BACKEND: B1 and B2 built and tested, deploy pending; asks GAME or RULES to confirm the CUT! prompt ids and suggested notes in `backend/deploy/suggested_notes.json`.

## Decisions (Dylan, Nov 12)

- **Music (T18):** every track comes in two takes, and both are fine; play one of the two at random each time a track starts. The boss music is "ridiculously over-the-top", as intended. Files are in `music/` (untracked; T18 imports them).
- Batch 2 models: Dylan is having another session clean them up and pull them (12:20).

### From the browser model viewer

Test-stage verdicts. Dylan ran the approvals and routes himself and started batch1.
- Hero (Meshy): approved, "pretty damn good". Peppermint: Meshy (Rodin's reads as a regular candy).
- Rat / NPCs: Rodin looks best but is 500k tris; GAME reduces on import unless ASSETS got Rodin to send fewer faces.
- House / terrain: fal looks good, Rodin is terrible. fal adds side walls and a ground platform; strip the platform on import if it shows from the game camera.
- Sword (Rodin): good but too short. Proposed `redo SM_Sword` with "blade four times the grip length, sword nearly as tall as a man". Not yet confirmed by Dylan.
- Batch1 meshes (Dylan, 08:10, browser viewer): all okay **except `SM_Fenella`: "TERRIBLE. That's no fairy, it doesn't even have wings."** Her prompt said "no wings" (ASSET LIST's choice). Redo with wings: replace "no wings" in her `jobs.json` subject with something like "two small translucent dragonfly wings on her back", then `gen_batch.py redo SM_Fenella`. Dylan also: "the camera ON her eye is pretty bad. She should be holding a video camera instead of with a 35mm pasted on her eye." So also replace "toy black-and-white film camera raised beside one eye, other eye and expression visible" with something like "holding a chunky handheld video camcorder in both hands at chest height, both eyes and face clearly visible". ASSETS runs it once Dylan confirms the wording.
- 2D: `T_Skill_Lecture` "fantastic", `T_Card_Clapper` fine.
- Camera (T01 tuning): arm **1700 uu** (not 1100), 50 degree pitch, FOV 55, looks 300 uu ahead of the hero. At 1100 the hero filled a third of the screen and the nuisances were off the top. The hero is now about a fifth of the screen height.

## What exists

- **T10 done.** Widget HUD `UJamHudWidget` (`Source/MegaJam/*/UI/`), tree built in C++ (no editor MCP for a WBP; BUILD_SPEC 5C allows it; all Transient, so a WBP can be reparented later). Draws a plain `FJamHudView` that `AJamHUD` builds from the game state each frame. Layers: slate label, take, blinking REC / STBY and a 24 fps timecode (`AJamGameState::FilmSeconds`, counts while rolling); objective with a meter bar (countdown or crops, pulses red when in danger); hint; bubble layer (round for Fenella, sharp red-edged for lectures; nothing emits bubbles until T19); interact prompt over its target; Fenella's caption at the bottom; centre card with a colour per `EJamCardKind` and a 0.18 s pop. `-uiscreen=hud|slate|action|cut|retake|wrap|meter|bubbles|caption` shows a sample over the running game. Tests `Jam.UI.Hud.*`. Screenshots `Saved/Screenshots/WindowsEditor/t10-*.png`.
- **T14 done.** At each wrap the game mode rolls `JamWrap::Roll` (`UI/JamWrapView.h`, headless, tests `Jam.UI.Wrap.*`): gross = scene base (1200 / 1800 / 2600) x the notes' multiplier x 0.8 per refused note; three loot drops; `JamLedger::Compute` (floor 40). Fenella keeps the best (stamped RESERVED FOR SEQUEL on the first wrap, then PRODUCER'S PICK / EVIDENCE); the hero auto-equips the best of the rest. `FJamProgression` and `Sprinkles` live on `AJamPlayerState`: XP from pacify / treat / collect, levels 8 and 9 at the wraps and 10 at Scene 3's CUT!. The Lecture reads radius, power **and cooldown** from the progression (`UJamGA_LectureCone` overrides `ResolveParams`, `ResolveDamage`, `ApplyCooldown`); move speed scales the hero's tuned 450. The wrap panel replaces the card: ledger (gross, lines with notes, HERO'S CUT, footer), three loot cards with rarity colours, EQUIPPED, her stamp, a character sheet with before -> after, LEVEL UP!. It closes after 14 s or on interact (E / A); the last wrap stays (Premiere comes in T16). Live run: radius 800 -> 848 -> 856 over the film; the change is only 6%, so "visibly larger cone" really arrives with the T19 cone VFX. Skill points pile up unspent until T14a. Screenshot `t14-wrap.png` (1920x1080).
- **T13 done.** A failed take shows RETAKE! with its reason (out of time / the crops are gone / director's call, each ending "Back to one!"), a red film-burn flash over the world (`AJamGameState::TakeFailedAt`, 0.8 s), Fenella's failed-take line, and on the retake the hero is teleported back to the set's mark. Stage actors already reset per take. `-forcefail[=seconds]` fails the run's first take after 3 s (default): screenshot `t13-take2.png` shows the Take 2 slate; full autoplay with it passes. `-uiscreen=retake` sample shows the countdown case.
- **T12 done.** At CUT! a note panel replaces the card: "FENELLA HAS A NOTE", her pitch, and two tiles, TAKE THE NOTE (hold E / A, fills as you hold, shows the box office multiplier) and REFUSE (press Q / B). Glyphs follow the last device. The slate panel counts notes taken and refused over the run (`AJamGameState::RunNotesTaken/Refused`). While she sulks, the caption reads FENELLA (SULKING) and she turns her back on the hero. The hold fill also shows under the interact prompt (`AJamHero::GetInteractHoldFraction`). Autoplay passes with `-autonote=take` (directors_cut) and `=refuse` (theatrical). Screenshots `t12-cut.png`, `t12-sulk.png`. Mouse clicks on the tiles are not wired (keyboard and gamepad only); typed notes are T12b.
- **T11 done.** `AJamFaerie`: cosmetic, basic shapes until `SM_Fenella` (T20): violet cone dress, head, flapping dragonfly wings, camcorder with a blinking tally light; drifts at the hero's shoulder, bobs, films the nearest loose nuisance within 1500 uu, flaps faster while talking. The game mode owns the `FJamNagScheduler` (seed 2026): story lines (scene start, Level7Intro once, failed / forced retake, Take3..6, note taken / refused, wrap) queue; ambient lines (pacified, treated, lecture cast, idle, strayed via `Poll` each second) only fill silence; silenced during CUT!. The caption is a line id plus text on the game state. Cards Fenella now speaks (ACTION!, "Perfect! Again.", wrap, note answered) lost their subtitles. Autoplay PASS line now ends `lines=13`. Screenshots `t11-fenella.png`, `t11-action.png`.
- **T07 done.** Flags: `-scene=<n>` (start at scene n, hero on its set), `-autonote=take|refuse`, `-timescale=<x>` (0.25 to 8). All three scenes play in sequence. After a wrap the HUD hint says to walk up the street, and stepping onto the next set starts its scene.
  - Kinds follow BUILD_SPEC 10: rat, goblin, erratic bug, and the almond (an `AJamPickup`, collected by walking over it).
  - Interact stages treat the previous stage's pacified targets; a retake resets them to pacified. A note's boss (Resolve x3.2) spawns fresh at 1.8x scale and does not flee.
  - Scene 3's crop meter: each loose bug eats one crop per 15 s.
  - CUT! shows the `JamLines` pitch line. Hold E or A takes the note, Q or B refuses it.
  - Autoplay runs:
    - `-autoplay -timescale=4` logs `JamAutoplay: PASS scenes=3 first=1 takes=4 notes=2/0 ending=directors_cut`.
    - Adding `-autonote=refuse` logs `notes=0/2 ending=theatrical`.
  - Nuisances overlap pawns instead of blocking them. The Lecture still finds them, because it queries by object type.

- **T08 done.** MCP spike answer: **no.** The 5.8 `ModelContextProtocol` plugin (HTTP server, port 8000, off unless autostarted) with `EditorToolset` offers selection, camera, capture and PIE, but no actor spawning or map saving, and it is not connected to these sessions. So the backlot is a C++ table: `Game/JamBacklot.h` (`JamBacklot::Sets()`, `Streets()`, `HeroMark(i)`) and `AJamBacklot`, which the game mode spawns on the template map. Up the screen (+X): the field (1800 x 1800, Scene 1), a 4500 uu street, Fort Toothache (1400 x 1400), another street, the orchard (2000 x 2000). Low plywood walls with street-wide openings, a banner per set facing the camera. Scene 1 plays on the field set; rats steer inside its walls. After the wrap, `-autoplay` walks the street and logs `JamAutoplay: walked to set Fort in 14.0 s` (BUILD_SPEC wants about 15 s). Tests `Jam.Game.Backlot.Layout` and `.Room`. Screenshots `t08-field.png`, `t08-street.png`, `t08-fort.png`. Still on `Template_Default`; a real map with lighting comes with T20.
- **RULES work merged** (`d9d9b51`): `JamLedger`, `JamLines` + `FJamNagScheduler`, `FJamProgression`, `JamNoteClamp`, Scenes 2 and 3 as data. See `Docs/Workers/rules.md` for how GAME uses each. 42 tests under `Jam.`, all pass.

- **T03 done (by GAME).** `Source/MegaJam/*/Film/`: `FJamStageDef`, `FJamNoteDef`, `FJamSceneDef`, `FJamShoot` (Slate, Rolling, StageCleared, NotePrompt, FailedTake, Wrap; inputs `Action`, `ReportProgress`, `ReportMeter`, `ReportFail`, `AnswerNote`, `Retake`, `Tick`; events drained as data). `JamScenes::Scene1()`. Tests `Jam.Film.Shoot.*` (happy path, C.10 A to C, note refused, timer and meter, forced retake once) and `Jam.Film.Scenes.Finishable`.
- **T01, T05, T06 done.** `Source/MegaJam/*/Game/`:
  - `AJamGameState`: holds the `FJamShoot` and the centre card; only the game mode writes it.
  - `AJamGameMode` (global default in `DefaultEngine.ini`): plays Scene 1 on a placeholder field set (an 1800 x 1800 box, drawn in orange). Feeds the shoot (pacified during Pacify stages, treated during Interact stages), drains its events, respawns rats per Pacify stage, shows cards (slate, ACTION!, "Perfect! Again.", RETAKE!, THAT'S A WRAP!) and paces itself (slate 2 s, cleared 1.2 s, failed 2.5 s). Rats do not flee on take 1 (`FJamShoot::TargetsFlee`). `-autoplay` bot follows the stage verb; logs `JamAutoplay: PASS scenes=1 takes=2 flees=N time=~26s`, or `FAIL timeout` after 90 s.
  - `AJamPlayerState`: owns the hero's ASC, grants the Lecture, `Team.Hero` tag.
  - `AJamHero : ACharacter`: avatar for the PlayerState ASC; fixed camera; moves relative to the screen. Faces the cursor (mouse), the right stick, or else its walking direction; `SetAimOverride` for the bot. Runtime Enhanced Input: WASD/arrows/left stick move, right stick aims, LMB/Space/RT lecture, hold E/A 0.6 s to interact. `RequestLecture()` and `RequestInteract()` are the only entry points. Debug cone on cast.
  - `AJamNuisance : APawn`: Wander, Flee (hero inside 500 uu), Flustered (0.6 s after a hit that did not pacify), Pacified (sits), Treated. Steers without a navmesh inside `SetBounds` and slides along edges. Colour per state; a debug state label floats above each.
  - `AJamHUD` (canvas, plain text): scene/take label, blinking REC while rolling, objective with progress (and countdown), centre card, "Hold [E] Feed celery" prompt only during the feeding stage. **The prompt has not been seen in a screenshot.**
  - Placeholder bodies are `/Engine/BasicShapes` with `BasicShapeMaterial` (its `Color` parameter tints; the shapes' own default material does not tint).
- GAS layer (pre-jam port, `Docs/Declared.md`): Resolve, damage, Lecture cone 800 uu / 35 deg / 60 Resolve / 1.5 s cooldown. Two casts pacify a 100-Resolve nuisance.
- **T02 done.** `package.ps1` then `smoke-packaged.ps1 -Extra "-autoplay"`: all checks pass, including the new `JamAutoplay: PASS` and "no missing constructor assets" checks. The packaged screenshot matches the editor run (`Builds\Win64-Development\MegaJam\Saved\Screenshots\Windows\t02-packaged.png`). Packaging takes about 4 minutes. No Python plugin errors in the packaged log.
- `-abilitytest` rig still passes alongside the new game mode.
- `Scripts\compare-models.ps1`: browser side-by-side of the generated GLBs (shared orbit camera, clay/wireframe), served on 127.0.0.1:8917. Port 8765 is taken by OneMore.
- Screenshots: `Saved\Screenshots\WindowsEditor\t06-again.png` ("Perfect! Again." card), `t06-wrap.png` (wrap card), `t05-flee2.png` (a rat fleeing).

## What does not exist yet

Game-side use of the ledger, lines, progression and note clamp; the note UI, faerie, widget UI, a map asset, imported assets, scene-specific beats (budget-out, sulking camera). Nothing replicates (solo only, BUILD_SPEC 5).

## Known issues

- The nuisances' debug state labels (`DrawDebugString`) draw over the HUD card. They go when the art does (T20).
- **Tuning:** Scene 3 is far too quick for the bot (19 s against BUILD_SPEC's 3 minutes; bugs fall to one lecture), and its crop meter never fails. Scene 2's 60 s countdown never fails either. Tune with Dylan's play in T15.

- **Shared engine:** builds from other worktrees on the same engine take UBT's global mutex; a GAME build or package started at the same time fails at once with `ConflictingInstance` (UAT reports it as `Error_SDKNotFound`, exit code 10). Retry. Once an editor `-game` run also hung at startup waiting for the Zen cache server while a worker ran `UnrealEditor-Cmd`; retrying worked.

- The packaged folder is 921 MB with almost no content: `MegaJam.pdb` 356 MB, exe 320 MB, paks 129 MB. The submission zip must stay under 1 GB. Before day 4, make `zip-submission.ps1` leave out `*.pdb` (debug symbols, not needed to run), or package with `-nodebuginfo`.

- The hero's visor block (shows facing) is hard to see from behind; fine until the Meshy hero is imported.
- Editor `-game` runs log Python errors from the engine's NiagaraToolsets and ToolsetRegistry plugins (`unreal.AgentSkill`, `PythonTestRunner` missing). Editor-only, not in the packaged log.
- `Config/DefaultEngine.ini` carries the editor's Android file server token block uncommitted. Never commit it; stage config lines from HEAD instead.

## Where other code lives (read source only; never open key/config folders)

Yendor (GAS source), TowerOfRock2 (5.8 Mass, MIDI), CafeteriaBattler (UI patterns; has an `AccessKeys` folder: do not open), LyraSample (session code), OneMore (backend). Paths under `D:\Source\Unreal\`. Anything copied gets a `Docs/Declared.md` line the same day.
