# STATUS (GAME, 2026-11-13, release plan)

Read this, then **`Docs/Design/RELEASE_PLAN.md` (governs the rest of the jam)**, then `CLAUDE.md`. `BUILD_SPEC.md` still holds for design, architecture and protocol; its backlog and the O2 to O9 scope are replaced by the release plan. The long day-1 STATUS (every T task in detail) is `git show 6d1f5f6:Docs/STATUS.md`.

## Next task

**R0** (checkpoint commits), then **R1** (sound). Tasks, time boxes and fallbacks are in the release plan. Update this line as each R task lands.

| Task | State |
| --- | --- |
| R0 checkpoint | not started |
| R1 sound effects and Fenella's voice | not started |
| R2 theme song | waits on Dylan's file at `music/theme-song.mp3` |
| R3 Candidate A packaged | not started |
| R4 the world, slim | not started |
| R5 Candidate B and the gate | not started |
| R6 cast | not started |
| R7 playtest fixes | not started |
| R8 final zip | not started |

## Dylan's list (details in the release plan)

1. Theme song file into `music/`. Stop any Codex session holding the engine.
2. Deploy the backend (`backend/DEPLOY.md`), then give GAME the URL for `BaseUrl` under `[/Script/MegaJam.JamNoteClient]` in `Config/DefaultGame.ini`. Pass BACKEND the two corrections below first.
3. Play Candidate A after R3 and write the three worst things here.
4. Play Open after R5, decide the gate.
5. Video, itch page, the go-ahead for the upload.

**Three worst things (Dylan fills in):**

## What exists

- **The film, Backlot mode (default):** rating card, title, three scenes with CUT! notes (take, refuse, type your own), retakes, wraps with ledger and loot, levels 7 to 10 and four GAS skills, the budget-out gag, Premiere with two endings (or a generated one from typed notes), credits, back to title. Keyboard, gamepad and mouse. Music: 9 cues, two takes each. Fenella's captions play `/Game/Audio/VO/VO_<LineId>` when the clip exists (none imported yet). No sound effects.
- **Open mode (`-open`): O1 only.** World table (`World/JamWorldTable.cpp`), `Jam.World.*` tests, flat fallback on the template map; on any other map only gameplay actors spawn. Fixed camera, no bot, no quests.
- **Terrain:** `/Game/World/L_Sugarborough` and its review maps; `/Game/World` is always cooked through `PAL_Sugarborough`. Notes: `Docs/Workers/terrain.md`.
- **On disk, not in the game:** voice and SFX under `vocals/output/`, four rigged characters under `ImportSource/Meshy/SK_*_Rig/`, more music in `music/`, Niagara attack systems under `/Game/FX/Attacks` (cut).
- **State last verified (Nov 13, wrist fix):** build passes, 57 `Jam.*` tests pass, Backlot autoplay `PASS scenes=3 ... ending=directors_cut`. The terrain polish after the last package has not been repackaged.

## Flags

`-autoplay -timescale=4` (bot; PASS line in the log), `-autonote=take|refuse|suggest|typed:<words_with_underscores>`, `-scene=<n>`, `-notitle`, `-open` / `-backlot`, `-uiscreen=<name>` (hud, slate, action, cut, retake, wrap, meter, bubbles, caption, noteeditor, rain, snow, fog, rating, title, options, quit, pause, premiere, theatrical, credits, generated), `-noart`, `-nograde`, `-nomusic`, `-nonotes`, `-noteurl=<url>`, `-commentary`, `-budgetout`, `-forcefail[=s]`, `-artreview[=close]`, `-closecam=<uu>`, `-camdist=<uu>`, `-debuglabels`, `-shotat=<s> -shotname=<n>`, `-quitat=<s>`.

Typed notes without the server: `python -I Scripts/note-stub.py` (port 8011), then `-noteurl=http://127.0.0.1:8011`.

## Requests to BACKEND (unchanged, still open)

`backend/deploy/suggested_notes.json`: Scene 2's `set_capacity` is **6**. Scene 3's stage id is **`S3_LastBugs`**, `set_kinds` **`["bug", "almond"]`**, `set_capacity` **8**. The buttons send `motif` as the player's current motif (`""` until a typed note returns one).

## Decisions (Dylan)

- Nov 13: release plan; the quest generator, givers and Niagara hookup are cut. Sound effects are "ridiculously over the top". The new knight (`SK_Knight`) replaces the old look.
- Nov 12: two takes per music cue, pick one at random. Music and Fenella have separate volume sliders. Camera in Backlot: arm 1700 uu, 50 degrees, FOV 55.
- Team **Imaginary Component**, game **The Dread Land of Candy**; zip `ImaginaryComponent_TheDreadLandOfCandy.zip`.

## Known issues and traps

- The game target is a unity build: two .cpp files with the same names in anonymous namespaces fail only there, and a shadowed name is an error. `Scripts\build.ps1 -Game` catches it before a package.
- Editor `-game` sometimes hangs at frame 0 waiting for the Zen cache server, usually after a package or test run. Retry. Never kill Unreal processes blindly.
- A build started while another UBT runs fails with `ConflictingInstance` (UAT says `Error_SDKNotFound`). Retry.
- Smoke test: pass `-timescale=4` with `-autoplay`; with typed notes against a server use `-RunSeconds 110`.
- Import with `UnrealEditor-Cmd -run=ImportAssets` from PowerShell; Git Bash mangles `/Game/...` paths. For skeletal imports do not set `destination_name` (see `Scripts/import_hero_rig.py`).
- Tuning: Scene 3 is quick for the bot and its crop meter never fails; Scene 2's 60 s countdown never fails. Only fix if Dylan's play says so (R7).
- Small leftovers: the title screen should say typed notes go to a server; the bot answers CUT! without its think time; things spawned after the budget-out keep their colour.
- `Config/DefaultEngine.ini` carries the editor's Android token block uncommitted. Never commit it.

## Where other code lives (read source only; never open key or config folders)

Yendor (GAS), TowerOfRock2 (Mass, MIDI), CafeteriaBattler (UI; has an `AccessKeys` folder: do not open), LyraSample, OneMore (backend), under `D:\Source\Unreal\`. Anything copied gets a `Docs/Declared.md` line the same day.
