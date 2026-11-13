# STATUS (GAME, 2026-11-13, release plan)

Read this, then **`Docs/Design/RELEASE_PLAN.md` (governs the rest of the jam)**, then `CLAUDE.md`. `BUILD_SPEC.md` still holds for design, architecture and protocol; its backlog and the O2 to O9 scope are replaced by the release plan. The long day-1 STATUS (every T task in detail) is `git show 6d1f5f6:Docs/STATUS.md`.

## Next task

**R7** with Dylan's four fixes ("Three worst things" below); research each before changing it. The gate is still Dylan's call (release plan, "The gate"). If the gate passes: `GameDefaultMap=/Game/World/L_Sugarborough`, no flag means Open, Backlot behind `-backlot`; repackage, smoke, zip. Tasks, time boxes and fallbacks are in the release plan. Update this line as each R task lands.

## Hand-off to ChatGPT (itch push, with Dylan's go-ahead)

- Zip: `Builds/ImaginaryComponent_TheDreadLandOfCandy.zip`, 689.3 MB (debug symbols left out), packaged Nov 13 18:20 from commit after `9695d9d`.
- Still Backlot by default (the gate is not decided). Open: run `MegaJam.exe /Game/World/L_Sugarborough -open` (the map must be the first argument).
- New since the last package: terrain polish, sound effects and Fenella's voice (R1), theme song in the credits (R2), the world mode (R4), enemies hit back (R4b), the cast (R6).
- Packaged PASS lines: Backlot `PASS scenes=3 first=1 takes=4 notes=2/0 ending=directors_cut ... hits=3 dignityfails=0 time=117.9`; Open (`-class=archer`) `PASS mode=open main=3 side=0` and `PASS scenes=3 ... hits=3 dignityfails=0 time=154.8`. No missing assets or warnings in either log.

| Task | State |
| --- | --- |
| R0 checkpoint | done: scoped commits, tag `pre-release` |
| R1 sound effects and Fenella's voice | done: 47 SFX, 88 VO clips, `JamSfx` table, Effects slider |
| R2 theme song | done in editor: credits play `M_theme_song_take1` (falls back to `credits`); `M_village_take1/2` imported for R4. Packaged check is in R5 |
| R3 | dropped: no arena build goes out; Backlot is smoked inside R5 |
| R4 the world, slim | done: follow camera, scenes at the film sets, compass, biome music, bot PASS on `L_Sugarborough`. Ambient camps dropped (the plan's first fallback) |
| R4b enemies hit back (Dignity meter) | done: goblins, bugs and the boss charge, knockback and stagger, Dignity bar under the slate, "No dignity left" fails the take; both PASS lines hold |
| R5 Candidate B and the gate | packaged, both modes smoked PASS, zipped (689 MB). Gate waits on Dylan's play |
| R6 cast | done: four rigs, `-class=`, a Casting line on the title, battle cries; mage Backlot PASS, barbarian Open PASS |
| R7 playtest fixes | not started |
| R8 final zip | not started |

## Dylan's list (details in the release plan)

1. (done) Theme song file. Stop any Codex session holding the engine.
2. (done, Nov 13) Backend live at `https://api.imaginarycomponent.com`, set as `BaseUrl` in `Config/DefaultGame.ini` (quoted: unquoted, the ini cuts it at `//`). `live_check.py` 0 problems; in game, `-autonote=typed:make_it_rain_and_sad` got model notes (200, 2.3 to 3.5 s) and a generated premiere. Python on this PC rejects the chain (expired ISRG X2 cross-sign) unless run with `SSL_CERT_FILE=<UE>/Engine/Content/Certificates/ThirdParty/cacert.pem`; Unreal's bundle has Root X2, so the game is fine. Redeploy pending: the backend repo (`backend/`, its own git) has commits `e1ce3ae` and `8ae1aad` (suggested notes match the game; the prompt says capacity is a ceiling, keep about 3 unless the note asks for more; `PROMPT_VERSION` n2). Redeploy per `backend/DEPLOY.md`, then re-run `scripts/prewarm.py`.
3. (dropped: the first build out is the world build)
4. Play Open after R5, decide the gate.
5. Video; the itch page and uploads go through ChatGPT (release plan, "Who runs it"), with Dylan's go-ahead per push.

**Three worst things (Dylan fills in):**

Dylan's R5 play (Nov 13), the R7 list. Research so far is noted; nothing is fixed yet.

1. **The hero should move faster.** Found: `JamHeroTuning::WalkSpeed = 450` (`Game/JamHero.cpp:50`), scaled by the `MoveSpeed` stat (`JamHero.cpp:747`). Not yet decided: by how much, and whether nuisance chase speeds and the bot's timings need to follow.
2. **The "LMB" hint is wrong, and clicking did not work.** Dylan's mouse buttons are swapped in Windows; he ended up using Space. Found: the engine respects the Windows swap (`WindowsApplication.cpp`, raw input reads `SM_SWAPBUTTON`), so the primary button already reaches the game as `LeftMouseButton`. The labels (`JamHUD.cpp:262` and `:292`, `JamHudView.cpp:35`) should say "Mouse 1" / "Mouse 2" (or "Click" / "Right-click"), not LMB and RMB. **Not yet researched:** why clicking did not lecture (which mode, first click eaten by viewport capture, the input mode the HUD sets in `JamHUD.cpp:401-412`). Reproduce before fixing.
3. **The skills need images.** None exist: `FJamHudSkill` has a name and a key, no icon. Needs four icons (Lecture, Form A Line, Sigh, Last Judgment) from ASSETS or generated, plus an image slot in the HUD skill bar. Not researched yet.
4. **Before each round in the arena, the direction should go big in the centre of the screen for a second, then shrink to where it sits now.** Not researched yet: find the objective or direction line in `JamHudWidget` and where a stage starts.

## What exists

- **The film, Backlot mode (default):** rating card, title, three scenes with CUT! notes (take, refuse, type your own), retakes, wraps with ledger and loot, levels 7 to 10 and four GAS skills, the budget-out gag, Premiere with two endings (or a generated one from typed notes), credits, back to title. Keyboard, gamepad and mouse. Music: 9 cues, two takes each. Fenella's captions play `/Game/Audio/VO/VO_<LineId>` (88 English clips imported). Sound effects (R1): `Game/JamSfx` holds one row per event (clip, volume, pitch jitter, repeat guard), played 2D at the Effects volume from the game mode (slate, ACTION!, CUT!, retake, stage clear, ledger and coins at the wrap, budget-out, each skill, pacify, bubbles), the nuisances (flustered, treated) and the HUD (menu clicks, level up, the backlot ambience while shooting). Re-import: `UnrealEditor-Cmd <uproject> -run=pythonscript -script=Scripts/import_audio.py`. `Quest_Accept` has a row but no caller until R4's film sets.
- **They hit back (R4b):** the rule is `Film/JamDignity` (plain C++, `Jam.Film.Dignity.*`): `JamAttack::For(kind, boss)` gives charge range, speed, damage, cooldown, knockback, stagger; rats and almonds never attack. A loose goblin or bug inside its charge range asks `AJamGameMode::RequestCharge` (shoot rolling, at most 2 charging, hero not in his hit grace), winds up 0.35 s (quivers, red), then runs at him; a skill hit flusters it out of the charge, except the boss (`bUnstoppable`). On contact `HandleNuisanceHit`: `FJamDignity::Hit` (goblin 20, bug 10, boss 30 of 100; 1.4 s grace), `AJamHero::TakeHit` (launch away, stagger blocks movement and skills, the rig tips back), `Hero_Hit` SFX (clip `Shield_Bash`). Zero fails the take ("No dignity left. Back to one!"); every stage start and retake refills it. HUD: DIGNITY bar in the slate panel while an attacker is loose (`-uiscreen=dignity`). PASS lines now add `hits= dignityfails=`. Tuning seen: the bot takes 2 to 4 boss hits in Scene 2 and none from bugs (they out-run it); `-scene=2` at level 7 (Lecture only) fails a take on Dignity.
- **The cast (R6):** `Film/JamCast` (plain C++, `Jam.Film.Cast.*`): four `FJamClassDef` rows (knight, mage, archer, barbarian: name, rig, blurb, cries). The title's `Casting` line (left / right, or A / D) recasts the hero on the spot and he shouts; the choice is saved as `UJamSettings::CastId`, and `-class=<id>` wins until the title line is used. `AJamHero::ApplyCast` loads `/Game/Art/Cast/<Rig>/SK_<Rig>_Rig` (falls back to the old `SK_Hero_Rig`), fits it to the capsule, puts the sword in `hand_r`. Only the knight has SwordRaise, Sigh, Scrub; the other three play Idle and Walk with the sword in hand. Every skill cast plays the cast member's next English battle cry (`/Game/Audio/VO/Cast/VO_Cry_<K01..>`) at the Fenella volume, at most one per 3.5 s. Re-import: `UnrealEditor-Cmd <uproject> -run=pythonscript -script=Scripts/import_cast.py` (reapplies the knight's wrist fix). The new knight is the default and replaces the old look. Lineup: `Saved/Screenshots/WindowsEditor/r6_lineup.png`. Blurbs are in `Jokes.md`, "Cards and menus".
- **Open mode (`-open`, R4):** the title, then the hero starts in the village and walks to each film set in turn (`JamWorld::FilmSet(i)`); stepping onto a set starts its scene, which plays exactly as in Backlot on that floor (`AJamGameMode::SceneFloor/SceneMark/SceneSign` pick backlot or world). A compass strip at the top (`AJamHUD::DrawCompass`) shows the next set's name, distance and the hint. Follow camera (`AJamHero::EnableFollowCamera`): the mouse or right stick turns it, WASD moves relative to it, the cursor hides in play, skills turn the hero to the camera's yaw. Walking: 900 uu at 14 degrees down; a take: 1500 uu at 32 degrees, looking along the path into the set (the candy trees have no camera collision, so the camera must stay over the path). Fenella flies 3.2 m out to the camera's right with a warm spotlight on the hero (250 lumens, Dylan: still too bright at 500, not yet compiled at that value). Nuisances follow the ground. Biome music while walking (the village has its own `village` cue). `JamLook::Apply(World, true)`: Open is brighter (sky x2.6, exposure +0.8, softer contrast, more saturation, bloom 1.4 above threshold 1). No ambient camps. Flags: `-camyaw= -campitch= -camdist=` pin the camera. Play it: `UnrealEditor.exe MegaJam.uproject /Game/World/L_Sugarborough -game -open`.
- **Terrain:** `/Game/World/L_Sugarborough` and its review maps; `/Game/World` is always cooked through `PAL_Sugarborough`. Notes: `Docs/Workers/terrain.md`.
- **On disk, not in the game:** cast voices (villagers etc.) and non-English battle cries under `vocals/output/`, more music in `music/`, Niagara attack systems under `/Game/FX/Attacks` (cut).
- **State last verified (Nov 13, R6):** editor and game targets build, 66 `Jam.*` tests pass. Backlot autoplay: knight (default) `PASS scenes=3 ... hits=4 dignityfails=0 time=125.3`, `-class=mage` `PASS scenes=3 ... time=123.6`; Open autoplay on `L_Sugarborough` with `-class=barbarian` `PASS mode=open main=3 side=0` (time=154.9). No missing cries. Not yet heard or played by a human. Not repackaged since the terrain polish (R5 does it). Visual tuning (light, grade, spotlight) is Dylan's with ChatGPT on Nov 14; GAME only makes it playable.

## Flags

`-autoplay -timescale=4` (bot; PASS line in the log), `-autonote=take|refuse|suggest|typed:<words_with_underscores>`, `-scene=<n>`, `-class=knight|mage|archer|barbarian`, `-notitle`, `-open` / `-backlot`, `-uiscreen=<name>` (hud, slate, action, cut, retake, wrap, meter, bubbles, caption, noteeditor, rain, snow, fog, rating, title, options, quit, pause, premiere, theatrical, credits, generated), `-noart`, `-nograde`, `-nomusic`, `-nonotes`, `-noteurl=<url>`, `-commentary`, `-budgetout`, `-forcefail[=s]`, `-artreview[=close]`, `-closecam=<uu>`, `-camdist=<uu>`, `-debuglabels`, `-shotat=<s> -shotname=<n>`, `-quitat=<s>`.

Typed notes without the server: `python -I Scripts/note-stub.py` (port 8011), then `-noteurl=http://127.0.0.1:8011`.

## Requests to BACKEND (done in `backend/` on Nov 13, needs the redeploy above)

`backend/deploy/suggested_notes.json`: Scene 2's `set_capacity` is **6**. Scene 3's stage id is **`S3_LastBugs`**, `set_kinds` **`["bug", "almond"]`**, `set_capacity` **8**. The buttons send `motif` as the player's current motif (`""` until a typed note returns one).

## Decisions (Dylan)

- Nov 13: the title pairing everywhere is "THE DREAD LAND OF CANDY" with an edition stamp (`JamScreens::EditionStamp`: DIRECTOR'S CUT or THEATRICAL RELEASE) at 45 degrees, a little below and right of the title: title screen, Premiere and credits. A generated premiere keeps its own title with the same stamp.
- Nov 13: itch deployment is ChatGPT's; GAME stops at a zip on disk.
- Nov 13: title screen subtitle: "A weekend project made by a director who should know better / and machines that can't say no." A typed note may ask for up to the set's capacity (6 goblins, 8 bugs), but that is not the default.
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
- `Docs/Declared.md` has no line yet for the ElevenLabs Fenella lines and sound effects imported in R1 (the battle cries have one, R6). Reconcile in R8.
- `Config/DefaultEngine.ini` carries the editor's Android token block uncommitted. Never commit it.

## Where other code lives (read source only; never open key or config folders)

Yendor (GAS), TowerOfRock2 (Mass, MIDI), CafeteriaBattler (UI; has an `AccessKeys` folder: do not open), LyraSample, OneMore (backend), under `D:\Source\Unreal\`. Anything copied gets a `Docs/Declared.md` line the same day.
