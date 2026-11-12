# STATUS (GAME, 2026-11-12 09:10, day 1)

Read this, then `Docs/Design/BUILD_SPEC.md` (governs), then `CLAUDE.md`. Workers: `Docs/Design/WORKERS.md`.

## Next task

**T07:** `-scene=<n>` (start on scene n, hero on set n's mark), `-autonote=take|refuse`, `-timescale`. Then day 2 (BUILD_SPEC): T10 HUD layers, T11 faerie and captions (RULES' `FJamNagScheduler` is merged), T12 note prompt, T13 failure, T14 wrap card (RULES' ledger and progression are merged), T15 Scenes 2 and 3 (tables merged; sets exist).
**Dylan: please play Scene 1 once** (editor `-game`, or `.\Scripts\screenshot.ps1 -At 999`). The "Hold [E] Feed celery" prompt and the hold-E feel have not been seen in a screenshot or played by a person.
Workers: RULES says days 1 and 2 are done (`rules` merged at `d9d9b51`). ASSETS: generation finished (batch1 and batch2, Fenella regenerated with wings and a camcorder); meshes need reducing and a look in the game camera (T20). BACKEND: B1 and B2 built and tested, deploy pending; asks GAME or RULES to confirm the CUT! prompt ids and suggested notes in `backend/deploy/suggested_notes.json`.

## Decisions (Dylan, Nov 12, from the browser model viewer)

Test-stage verdicts. Dylan ran the approvals and routes himself and started batch1.
- Hero (Meshy): approved, "pretty damn good". Peppermint: Meshy (Rodin's reads as a regular candy).
- Rat / NPCs: Rodin looks best but is 500k tris; GAME reduces on import unless ASSETS got Rodin to send fewer faces.
- House / terrain: fal looks good, Rodin is terrible. fal adds side walls and a ground platform; strip the platform on import if it shows from the game camera.
- Sword (Rodin): good but too short. Proposed `redo SM_Sword` with "blade four times the grip length, sword nearly as tall as a man". Not yet confirmed by Dylan.
- Batch1 meshes (Dylan, 08:10, browser viewer): all okay **except `SM_Fenella`: "TERRIBLE. That's no fairy, it doesn't even have wings."** Her prompt said "no wings" (ASSET LIST's choice). Redo with wings: replace "no wings" in her `jobs.json` subject with something like "two small translucent dragonfly wings on her back", then `gen_batch.py redo SM_Fenella`. Dylan also: "the camera ON her eye is pretty bad. She should be holding a video camera instead of with a 35mm pasted on her eye." So also replace "toy black-and-white film camera raised beside one eye, other eye and expression visible" with something like "holding a chunky handheld video camcorder in both hands at chest height, both eyes and face clearly visible". ASSETS runs it once Dylan confirms the wording.
- 2D: `T_Skill_Lecture` "fantastic", `T_Card_Clapper` fine.
- Camera (T01 tuning): arm **1700 uu** (not 1100), 50 degree pitch, FOV 55, looks 300 uu ahead of the hero. At 1100 the hero filled a third of the screen and the nuisances were off the top. The hero is now about a fifth of the screen height.

## What exists

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

Game-side use of the ledger, lines, progression and note clamp; the note UI, faerie, widget UI, a map asset, imported assets, Scenes 2 and 3 in the game mode (the set capacities RULES asks for: goblins 6, bugs 8, almonds 4). Nothing replicates (solo only, BUILD_SPEC 5).

## Known issues

- **Shared engine:** builds from other worktrees on the same engine take UBT's global mutex; a GAME build or package started at the same time fails at once with `ConflictingInstance` (UAT reports it as `Error_SDKNotFound`, exit code 10). Retry. Once an editor `-game` run also hung at startup waiting for the Zen cache server while a worker ran `UnrealEditor-Cmd`; retrying worked.

- The packaged folder is 921 MB with almost no content: `MegaJam.pdb` 356 MB, exe 320 MB, paks 129 MB. The submission zip must stay under 1 GB. Before day 4, make `zip-submission.ps1` leave out `*.pdb` (debug symbols, not needed to run), or package with `-nodebuginfo`.

- The hero's visor block (shows facing) is hard to see from behind; fine until the Meshy hero is imported.
- Editor `-game` runs log Python errors from the engine's NiagaraToolsets and ToolsetRegistry plugins (`unreal.AgentSkill`, `PythonTestRunner` missing). Editor-only, not in the packaged log.
- `Config/DefaultEngine.ini` carries the editor's Android file server token block uncommitted. Never commit it; stage config lines from HEAD instead.

## Where other code lives (read source only; never open key/config folders)

Yendor (GAS source), TowerOfRock2 (5.8 Mass, MIDI), CafeteriaBattler (UI patterns; has an `AccessKeys` folder: do not open), LyraSample (session code), OneMore (backend). Paths under `D:\Source\Unreal\`. Anything copied gets a `Docs/Declared.md` line the same day.
