# STATUS (handoff, written 2026-11-11, the night before the jam)

Read this first, then `Docs/Design/BUILD_SPEC.md`, then `CLAUDE.md`. **BUILD_SPEC.md governs** (added Nov 11 by a design review): it narrows the game to a solo eight-minute film with three scenes and an ending, replaces the build order and "First steps tomorrow" below with a task backlog (T01 onward), and lists questions for Dylan in its section 11. Authored text and jokes: `Docs/Design/Jokes.md`. The three briefs are reference only; do not read them unless a task names a section.
Everything under `Docs/Design/` except Appendix C was drafted with ChatGPT before the jam and is a proposal, not evidence that anything works. Appendix C was written in a Claude session the same night.

## Decisions (Dylan, Nov 12, from `Scripts/compare-models.ps1`, the browser side-by-side viewer)

Test-stage verdicts. ASSETS runs these; GAME does not:
`gen_batch.py approve character npc terrain_piece particle_mesh equipment 2d_knight 2d_fenella`, `route npc rodin`, `route terrain_piece fal`.

- Hero (Meshy): approved, "pretty damn good". Peppermint: Meshy (Rodin reads as a regular candy).
- Rat / NPCs: Rodin looks best but is 500k tris. Ask Rodin for fewer faces if its API allows; otherwise GAME reduces on import.
- House / terrain: fal looks good, Rodin is terrible. fal adds side walls and a ground platform; GAME strips the platform on import if it shows. 17 jobs at about $0.53 each.
- Sword (Rodin): good but too short. Proposed: `redo SM_Sword` with "blade four times the grip length, sword nearly as tall as a man" (0.5 credits). Not yet confirmed by Dylan.
- 2D: `T_Skill_Lecture` "fantastic", `T_Card_Clapper` fine.

## The game in one paragraph

**The Dread Land of Candy: Director's Cut** (jam theme: "Director's Cut"). A grim, serious dark-fantasy hero does absurd nonviolent chores in a cheerful candy village. A bossy low-budget **Production Faerie** (working name Fenella Flicker, original character, no franchise lines) floats behind the hero "shooting a movie": she nags, yells, and rewrites quests mid-scene, and she takes most of the villagers' pay, leaving the hero a cut. The player can edit scenes with CUT! at stage transitions.

## What exists in the repo

- Project skeleton, scripts, agent hooks (`-shotat`, `-quitat`): pre-jam, see `Docs/Declared.md`.
- **GAS ability layer** (`Source/MegaJam/*/Gas/`, committed in `063af55`): Resolve attribute set (zero = once-only `State.Pacified`, not death), damage effect + magnitude calc, Lecture cooldown effect, AoE shape library (cone/ring/line/...), committing area ability, `UJamGA_LectureCone` (800 uu, 35 deg half angle, 60 Resolve, 1.5 s cooldown). Ported from Spirits of Yendor, adapted to 5.8. Declared in `Docs/Declared.md`.
- **Test scene** (`Source/MegaJam/*/Testing/JamAbilityTestRig.*`): run `.\Scripts\screenshot.ps1 -Extra "-abilitytest"`. Hero casts the Lecture three times against 9 nuisances; log line `JamAbilityTest: PASS` means 5 pacified, cooldown refused cast 2, 4 untouched.
- **Tests**: `.\Scripts\run-tests.ps1` runs `Jam.Skeleton.ModuleLoaded`, `Jam.Gas.AoeShapes`, `Jam.Gas.ResolveDamage`, `Jam.Gas.LectureCone`. All passed at the time of writing.
- Not committed at the time of writing: the Appendix C / theme edits to `Docs/Design/*` and this file. `Config/DefaultEngine.ini` is modified by the editor itself (it writes an Android file server block with a security token): do not commit that diff.

## What does not exist yet

- No stage/take state machine, quest generator, reputation, ledger, loot, inventory, save, UI screens, map, NPCs, faerie, voice, VFX, input/camera, packaged build. The Gas layer has no GameplayCues, no cost effect, no replication; the ASC sits on a test pawn, not a PlayerState.

## Decisions made (and why)

- **Slice**: one class (the knight), one NPC, one location, one quest family with a CUT! point, then widen. The briefs' four classes / six NPCs / three POIs are cut candidates, not goals.
- **Local-first**: a local `SaveGame` and an offline quest generator. The OneMore FastAPI/Postgres backend is **not** needed for the jam; at most it can later serve optional AI Director notes with authored fallbacks and a timeout (Appendix C.9). Never required to play, never in the packaged client with secrets.
- **GAS**: Yendor's port is the source (Cafeteria Battler has no GAS; TowerOfRock2's GAS lives in its `Plugins/RaveZombieGame` and `TowerDefenseCore` and is thinner: no GameplayEffects, cues or player ASC). Yendor is UE 5.5 and was not run recently; the 5.8 port builds and passes tests.
- **Multiplayer**: not a day-4 feature. Gate: if solo is packaged and smoke-tested by end of day 2, time-box LAN host/join to half of day 3, otherwise drop it. Lyra's `CommonUser`/`CommonGame` session code is the candidate source (unverified for 5.8; a throwaway two-instance spike first). Keep authority discipline now: gameplay state changes through one path, player state on PlayerState/PlayerController not singletons, VFX triggered from events.
- **Voice**: Dylan generates faerie lines in ACE Studio after the jam opens (not before Nov 12) and declares them in `Docs/Declared.md`. No lines or names from other franchises.
- **Build order** (Appendix C.8): stage/take state machine as headless C++ with tests -> ledger math -> Scene/Take label + ACTION!/CUT!/RETAKE!/Wrap overlays -> faerie actor + nag scheduler -> voice and polish. Day 4 is polish, packaging, submission, not features.
- **Reward split numbers** (40% hero's cut, x1.25/x1.5 gross for CUT! variants): placeholders to tune so no option dominates.

## Where other code lives (read source only; never open key/config folders)

- `D:\Source\Unreal\Yendor\SpiritsOfYendor` (GAS source we ported; also inventory component, draft pool, AoE patterns).
- `D:\Source\Unreal\TowerOfRock2\TowerOfRock5point8` (5.8, thin GAS, Mass crowd, MIDI).
- `D:\Source\Unreal\CafeteriaBattler` (UI screen patterns, source of the CLAUDE.md traps; has an `AccessKeys` folder: do not open).
- `D:\Source\Unreal\LyraSample` (candidate multiplayer session code).
- `D:\Source\Unreal\OneMore` (REST backend client/server).
Anything copied into this project gets a `Docs/Declared.md` line the same day.

## Gotchas found this session (also in CLAUDE.md)

- 5.8 ensures on `SetIsReplicated` in a component constructor; use `SetIsReplicatedByDefault`.
- `IMPLEMENT_SIMPLE_AUTOMATION_TEST` flags must be written inline as an expression (a `constexpr` of `EAutomationTestFlags::Type` does not compile); a macro works.
- `FGameplayAbilitySpecHandle` lives in `GameplayAbilitySpecHandle.h`, not under `Abilities/`.
- The editor writes an Android file server token into `Config/DefaultEngine.ini` when it runs; do not commit it.

## First steps tomorrow

Superseded by `Docs/Design/BUILD_SPEC.md` section 7. Start at T01 (hero on screen) and T02 (package it), then T03 onward. Read `Docs/Declared.md` first; the jam rules say the build starts Nov 12.

Where the "Decisions made" above disagree with BUILD_SPEC (multiplayer gate, reward split numbers, build order), BUILD_SPEC wins until Dylan says otherwise.
