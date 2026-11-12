# STATUS (GAME, 2026-11-12 morning, day 1)

Read this, then `Docs/Design/BUILD_SPEC.md` (governs), then `CLAUDE.md`. Workers: `Docs/Design/WORKERS.md`.

## Next task

**T08:** MCP spike (30 min) then the Sugarborough blockout (BUILD_SPEC section 7). T06 (Scene 1 playable) waits on RULES' T03 (`FJamShoot`) being merged from the `rules` branch (worktree `D:\Source\Unreal\MegaJam2026-rules`); check `git log main..rules` first and take T06 if it is there.
ASSETS is running batch1 (Dylan started it about 07:50); meshes are landing in `ImportSource/`. Import approved ones for T20.

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

- **T01, T05 done.** `Source/MegaJam/*/Game/`:
  - `AJamGameMode` (global default in `DefaultEngine.ini`): hero as default pawn, builds a placeholder test set (an 1800 x 1800 box, drawn in orange) with three rats. `HandleInteract(Hero, Target)` validates and treats. `-autoplay` bot: chases the nearest untreated rat, lectures from 450 uu (inside the flee radius on purpose), walks in and treats. Logs `JamAutoplay: PASS pacified=3/3 treated=3/3 flees=N time=~7s`, or `FAIL timeout` after 60 s.
  - `AJamPlayerState`: owns the hero's ASC, grants the Lecture, `Team.Hero` tag.
  - `AJamHero : ACharacter`: avatar for the PlayerState ASC; fixed camera; moves relative to the screen. Faces the cursor (mouse), the right stick, or else its walking direction; `SetAimOverride` for the bot. Runtime Enhanced Input: WASD/arrows/left stick move, right stick aims, LMB/Space/RT lecture, hold E/A 0.6 s to interact. `RequestLecture()` and `RequestInteract()` are the only entry points. Debug cone on cast.
  - `AJamNuisance : APawn`: Wander, Flee (hero inside 500 uu), Flustered (0.6 s after a hit that did not pacify), Pacified (sits), Treated. Steers without a navmesh inside `SetBounds` and slides along edges. Colour per state; a debug state label floats above each.
  - `AJamHUD`: canvas prompt "Hold [E] Treat" (or "(A)") over the treatable rat. **Not yet seen in a screenshot**: the bot treats too fast for the timed shots.
  - Placeholder bodies are `/Engine/BasicShapes` with `BasicShapeMaterial` (its `Color` parameter tints; the shapes' own default material does not tint).
- GAS layer (pre-jam port, `Docs/Declared.md`): Resolve, damage, Lecture cone 800 uu / 35 deg / 60 Resolve / 1.5 s cooldown. Two casts pacify a 100-Resolve nuisance.
- **T02 done.** `package.ps1` then `smoke-packaged.ps1 -Extra "-autoplay"`: all checks pass, including the new `JamAutoplay: PASS` and "no missing constructor assets" checks. The packaged screenshot matches the editor run (`Builds\Win64-Development\MegaJam\Saved\Screenshots\Windows\t02-packaged.png`). Packaging takes about 4 minutes. No Python plugin errors in the packaged log.
- `-abilitytest` rig still passes alongside the new game mode.
- Tests: `Jam.Skeleton.ModuleLoaded`, `Jam.Gas.AoeShapes`, `Jam.Gas.ResolveDamage`, `Jam.Gas.LectureCone`, all pass.
- `Scripts\compare-models.ps1`: browser side-by-side of the generated GLBs (shared orbit camera, clay/wireframe), served on 127.0.0.1:8917. Port 8765 is taken by OneMore.
- Screenshots: `Saved\Screenshots\WindowsEditor\t05-flee2.png` (a rat fleeing), `t05-flee.png` (Flustered), `t05-a.png` (Treated).
- T05 was not re-packaged; the last packaged smoke test is T02's.

## What does not exist yet

Stage/take machine, ledger, lines, faerie, UI, map, imported assets. Nothing replicates (solo only, BUILD_SPEC 5).

## Known issues

- The packaged folder is 921 MB with almost no content: `MegaJam.pdb` 356 MB, exe 320 MB, paks 129 MB. The submission zip must stay under 1 GB. Before day 4, make `zip-submission.ps1` leave out `*.pdb` (debug symbols, not needed to run), or package with `-nodebuginfo`.

- The hero's visor block (shows facing) is hard to see from behind; fine until the Meshy hero is imported.
- Editor `-game` runs log Python errors from the engine's NiagaraToolsets and ToolsetRegistry plugins (`unreal.AgentSkill`, `PythonTestRunner` missing). Editor-only, not in the packaged log.
- `Config/DefaultEngine.ini` carries the editor's Android file server token block uncommitted. Never commit it; stage config lines from HEAD instead.

## Where other code lives (read source only; never open key/config folders)

Yendor (GAS source), TowerOfRock2 (5.8 Mass, MIDI), CafeteriaBattler (UI patterns; has an `AccessKeys` folder: do not open), LyraSample (session code), OneMore (backend). Paths under `D:\Source\Unreal\`. Anything copied gets a `Docs/Declared.md` line the same day.
