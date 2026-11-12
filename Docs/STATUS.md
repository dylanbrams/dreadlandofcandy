# STATUS (GAME, 2026-11-12 morning, day 1)

Read this, then `Docs/Design/BUILD_SPEC.md` (governs), then `CLAUDE.md`. Workers: `Docs/Design/WORKERS.md`.

## Next task

**T05:** nuisance states and steering, interact (E) with a prompt (BUILD_SPEC section 7). T03 (`FJamShoot`) belongs to RULES in `D:\Source\Unreal\MegaJam2026-rules`; the `rules` branch had no commits past `main` at 08:05. T06 waits on T03 being merged.
ASSETS is running batch1 (Dylan started it about 07:50); meshes are landing in `ImportSource/`. Import approved ones for T20.

## Decisions (Dylan, Nov 12, from the browser model viewer)

Test-stage verdicts. Dylan ran the approvals and routes himself and started batch1.
- Hero (Meshy): approved, "pretty damn good". Peppermint: Meshy (Rodin's reads as a regular candy).
- Rat / NPCs: Rodin looks best but is 500k tris; GAME reduces on import unless ASSETS got Rodin to send fewer faces.
- House / terrain: fal looks good, Rodin is terrible. fal adds side walls and a ground platform; strip the platform on import if it shows from the game camera.
- Sword (Rodin): good but too short. Proposed `redo SM_Sword` with "blade four times the grip length, sword nearly as tall as a man". Not yet confirmed by Dylan.
- 2D: `T_Skill_Lecture` "fantastic", `T_Card_Clapper` fine.
- Camera (T01 tuning): arm **1700 uu** (not 1100), 50 degree pitch, FOV 55, looks 300 uu ahead of the hero. At 1100 the hero filled a third of the screen and the nuisances were off the top. The hero is now about a fifth of the screen height.

## What exists

- **T01 done.** `Source/MegaJam/*/Game/`:
  - `AJamGameMode` (global default in `DefaultEngine.ini`): hero as default pawn, spawns three static test nuisances in front of the hero, counts pacifications. `-autoplay` casts the Lecture twice and logs `JamAutoplay: PASS pacified=3/3 casts=2`.
  - `AJamPlayerState`: owns the hero's ASC, grants the Lecture, `Team.Hero` tag.
  - `AJamHero : ACharacter`: avatar for the PlayerState ASC; fixed camera; moves relative to the screen and faces where it walks. Enhanced Input actions and mapping context created in C++ at runtime: WASD/arrows/left stick move, LMB/Space/gamepad A lecture. `RequestLecture()` is the one entry point (input and autoplay). Debug cone on cast.
  - `AJamNuisance : APawn`: ASC + Resolve, pink basic-shape cylinder, turns grey when pacified.
  - Placeholder bodies are `/Engine/BasicShapes` with `BasicShapeMaterial` (its `Color` parameter tints; the shapes' own default material does not tint).
- GAS layer (pre-jam port, `Docs/Declared.md`): Resolve, damage, Lecture cone 800 uu / 35 deg / 60 Resolve / 1.5 s cooldown. Two casts pacify a 100-Resolve nuisance.
- **T02 done.** `package.ps1` then `smoke-packaged.ps1 -Extra "-autoplay"`: all checks pass, including the new `JamAutoplay: PASS` and "no missing constructor assets" checks. The packaged screenshot matches the editor run (`Builds\Win64-Development\MegaJam\Saved\Screenshots\Windows\t02-packaged.png`). Packaging takes about 4 minutes. No Python plugin errors in the packaged log.
- `-abilitytest` rig still passes alongside the new game mode.
- Tests: `Jam.Skeleton.ModuleLoaded`, `Jam.Gas.AoeShapes`, `Jam.Gas.ResolveDamage`, `Jam.Gas.LectureCone`, all pass.
- `Scripts\compare-models.ps1`: browser side-by-side of the generated GLBs (shared orbit camera, clay/wireframe), served on 127.0.0.1:8917. Port 8765 is taken by OneMore.
- Screenshot: `Saved\Screenshots\WindowsEditor\t01.png`.

## What does not exist yet

Stage/take machine, ledger, lines, faerie, UI, map, imported assets. Nothing replicates (solo only, BUILD_SPEC 5).

## Known issues

- The packaged folder is 921 MB with almost no content: `MegaJam.pdb` 356 MB, exe 320 MB, paks 129 MB. The submission zip must stay under 1 GB. Before day 4, make `zip-submission.ps1` leave out `*.pdb` (debug symbols, not needed to run), or package with `-nodebuginfo`.

- The hero's visor block (shows facing) is hard to see from behind; fine until the Meshy hero is imported.
- Editor `-game` runs log Python errors from the engine's NiagaraToolsets and ToolsetRegistry plugins (`unreal.AgentSkill`, `PythonTestRunner` missing). Editor-only, not in the packaged log.
- `Config/DefaultEngine.ini` carries the editor's Android file server token block uncommitted. Never commit it; stage config lines from HEAD instead.

## Where other code lives (read source only; never open key/config folders)

Yendor (GAS source), TowerOfRock2 (5.8 Mass, MIDI), CafeteriaBattler (UI patterns; has an `AccessKeys` folder: do not open), LyraSample (session code), OneMore (backend). Paths under `D:\Source\Unreal\`. Anything copied gets a `Docs/Declared.md` line the same day.
