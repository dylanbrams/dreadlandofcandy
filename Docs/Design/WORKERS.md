# Workers

How the backlog in `BUILD_SPEC.md` section 7 is split across sessions that can run at the same time. Each worker has its own checkout or its own repo, owns a set of folders, and takes its tasks in the order listed. Task details and "done when" stay in the spec.

## Why these boundaries

- **Builds queue, they do not collide.** `Scripts\build.ps1` passes `-WaitMutex`, so two checkouts can build; the second waits. The scripts find the project from the `.uproject` beside them, so they work unchanged in a git worktree.
- **One Unreal editor.** Only the Game worker opens the editor or uses the editor MCP. Nobody else imports assets or touches `Content/`.
- **Folder ownership prevents merge conflicts.** A worker edits only what it owns. If it needs a change elsewhere, it writes the request in its notes file and the owner makes it.
- **Dylan steers two or three at once, not six.** The day table at the bottom says which are live.

## The workers

| Worker | Model | Where it runs | Owns |
| --- | --- | --- | --- |
| **GAME** | Opus | Main checkout, branch `main`. The only one with the editor open | `Source/MegaJam/*/Game/`, `*/UI/`, `*/Gas/`, `*/Testing/`, `Content/`, `Config/`, `Scripts/`, `Docs/STATUS.md`, `Docs/DEVLOG.md` |
| **RULES** | Sonnet | Worktree `D:\Source\Unreal\MegaJam2026-rules`, branch `rules` | `Source/MegaJam/*/Film/`, `Source/MegaJam/Private/Tests/Film*.cpp`, `Docs/Workers/rules.md` |
| **FX** | Codex | Worktree `D:\Source\Unreal\MegaJam2026-fx`, branch `fx`. Builds and runs tests; never opens the editor | `Source/MegaJam/*/Combat/`, `Source/MegaJam/Private/Tests/Combat*.cpp`, `Docs/Workers/fx.md` |
| **BACKEND** | Opus for B1, Sonnet after | Its own repo, beside OneMore's server; never this repo's source | The note service and its database; `Docs/Workers/backend.md` in this repo (notes only) |
| **ASSETS** | Sonnet, with Dylan approving | Main repo files outside `Source/` and `Content/`; no builds, no editor | `Assets/Incoming/` (generated meshes, textures, manifests), `Docs/styles.json`, `Docs/Workers/assets.md` |
| **AUDIO** | Dylan, ACE Studio | Outside the repo, delivering files | `Assets/Incoming/Audio/`, `Docs/Workers/audio.md` |
| **DESK** | Sonnet, short sessions | Main repo, docs only | `Docs/Design/Jokes.md`, itch page text, submission checklist, credits text, `Docs/Declared.md` reconciliation |
| **REVIEW** | Fable, once at the end of each day | Read-only | Nothing. Reports to Dylan |

### GAME: everything that needs the engine

The critical path. If only one worker is running, it is this one.

- Day 1: T01, T02, T05, T06 (needs RULES T03 merged), T07, T08
- Day 2: T10, T11, T12, T13, T14, T05a, T14a, T15, T16, T12b (needs RULES T12a and BACKEND B1), T16a (needs B2), import pass for ASSETS (T17)
- Day 3: T19, T20, T23, T21, T24, T25, T22 hookup if ahead
- Day 4: fixes, package, clean-machine test, video capture

Merges `rules` into `main` (fast-forward or a plain merge) before any task that depends on it, then builds and runs tests. Imports generated assets at set points (end of T08, start of T20, and the approval pass below), not continuously. Adds every runtime-loaded folder to `DirectoriesToAlwaysCook`.

#### Watching asset state

ASSETS (usually Codex, following `Docs/GenerationRunbook.md`) works in this same checkout and never builds, so its results show up on disk at once. Codex can't join Claude's session-to-session messages, so files are the channel:

| File | Changes when | Means |
| --- | --- | --- |
| `ImportSource/_gen/last_run.txt` | a 3D run (`gen_batch.py run`) finishes | meshes are done or failed; the summary says which |
| `ImportSource/_gen/state.json` | any job starts, finishes, is approved, rerouted or ingested | the full picture; read it with `python -I Scripts/gen_batch.py status` (or `--json`) |
| `Docs/Workers/assets.md` | ASSETS ends a stage and stops for Dylan | **the stage is complete**; its notes say what looks wrong |

- **At the start of every task:** run `python -I Scripts/gen_batch.py status` and read `Docs/Workers/assets.md`. Both are cheap. Fold anything new into `STATUS.md`.
- **During a long task (Claude Code):** start a background watch, so a finished stage wakes you instead of being found hours later:

  ```bash
  w() { stat -c %Y ImportSource/_gen/last_run.txt Docs/Workers/assets.md 2>/dev/null | tr '\n' ' '; }
  s0=$(w); until [ "$(w)" != "$s0" ]; do sleep 60; done
  echo "asset state changed"; cat ImportSource/_gen/last_run.txt 2>/dev/null; git log -1 --format=%s -- Docs/Workers/assets.md
  ```

  When it fires, **don't drop the current task**. Note it, finish to the next green checkpoint, then act. Restart the watch afterwards.
- **When the `test` stage completes, do the approval pass.** Dylan approves meshes in the game camera, not from generator thumbnails. Import the test meshes (including the `*_Rodin` and `*_Fal` twins) into a throwaway review map. Line each subject's versions up side by side at the game camera (about 1100 uu, 50 degrees). Show nuisances at scale 1 and 3, tinted `pink` and `grey`. Then take one screenshot per subject with `screenshot.ps1`. Put the screenshot paths in `STATUS.md`. If Dylan is away, send him a push notification saying the review shots are ready. Approving is still his call: GAME never runs `gen_batch.py approve` or `route`.
- **Importing:** take only jobs that `status` shows `done` in an approved group. Meshes are at `ImportSource/<Meshy|Hyper3D|Fal|Tripo>/<id>.glb` (`state.json` records which tool made each). The rigged hero is `ImportSource/Meshy/SK_Hero_Rig/SK_Hero_Rig.fbx`, with its animations as `A_SK_Hero_Rig_<Anim>.fbx` (UE5 mannequin skeleton, so mannequin animations retarget). 2D is at `Assets/Incoming/2D/<id>/<id>.png`, keyed and trimmed. For a twin that won, import it under the plain name (`SM_Rat_Fal` becomes `SM_Rat`). Generated props arrive heavier than the budgets in `ASSETS.md` (fal's floor is 40k faces), so reduce them on import.
- **Asking ASSETS for something** (a redo, a missing asset): add it under a "Requests to ASSETS" heading in `STATUS.md` and tell Dylan. If another Claude session is running ASSETS instead of Codex, message it directly.

### RULES: headless C++ and tests, safe to leave unattended

No world, no engine objects, no assets, so it never needs the editor. Builds and runs `Jam.Film.*` in its own worktree.

- Day 1: T03 (stage machine), T04 (ledger), then T09 (lines table from `Jokes.md` and the nag scheduler)
- Day 2: T04a (progression and rank tables), T12a (note clamp and keyword fallback, against `NoteForm.md`), the three scenes as data tables with the finishability test
- Day 3: T22's commentary and sulking line pools, any rule fixes GAME asks for

Commits to `rules` at the end of each task. Does not edit `MegaJam.Build.cs`, the `.uproject` or anything GAME owns; asks in `Docs/Workers/rules.md`.

Set-up once, by Dylan or GAME: `git worktree add ..\MegaJam2026-rules -b rules`. First build there compiles the whole module.

### FX: the combat and effects system (expansion work)

Builds the class-agnostic combat system in `BUILD_SPEC.md` section 3d. Expansion work: it ranks below every slice task and is cut first. Unlike ASSETS, this worker builds (`Scripts\build.ps1`) and runs `Jam.Combat.*` in its own worktree, so it never shares a checkout with GAME's uncommitted work.

In order:

1. Attack definition table, with the knight's four skills as the first rows, and a test that every row is well formed.
2. Cast events (Start, Release, Contact, HitResolved, End, Cancel) as plain data, and the presentation coordinator that consumes them.
3. Primitives in C++, one at a time: lecture cone and comic bubbles, then projectile rain, orbiting props, foam.
4. New archetypes as headless hit logic with tests: pulsed area, projectile, sustained cone.
5. Rows for the other classes, reached with `-class=<name>`.

Does not edit `Game/`, `UI/`, `Gas/`, `Content/`, `Config/`, `MegaJam.Build.cs` or the `.uproject`. The hookups in those (calling the coordinator from `AJamHero::RequestSkill`, pointing the abilities at the table, a new module dependency, a Niagara or other editor-made asset) are requests to GAME in `Docs/Workers/fx.md`; GAME makes them when it merges `fx`, and checks the autoplay PASS line and the T19 screenshots are unchanged. Text for bubbles and cries comes from `Jokes.md` through `JamLines` (RULES), not from the two effects documents.

Commits to `fx` at the end of each task, once the build and `Jam.*` tests pass. Set-up once: `git worktree add ..\MegaJam2026-fx -b fx`.

### BACKEND: the typed-note service

Runs entirely outside Unreal. Contract is `Docs/Design/NoteForm.md`; if the contract has to change, change that file first and tell RULES.

- Day 1: B1 (`POST /v1/director/note`: second Gunicorn service, second Postgres database, cache table, normalisation, daily budget, rate limits, Haiku call with the form as structured output, twenty canned notes as tests including hostile ones). Pre-warm the suggested notes.
- Day 2: B2 (`POST /v1/director/premiere`), deploy, give GAME the URL, watch the first real requests
- Day 3 and 4: tune the prompt from real notes, check the budget counter, leave it alone

Loads the `claude-api` skill before writing the model call. On stream: never print environment variables, keys, connection strings or config files; never commit them.

### ASSETS: generate, clean, hand over

Follows `Docs/GenerationRunbook.md`: `Scripts/gen_batch.py` runs the jobs in `Assets/Generation/jobs.json` (3D in parallel through Meshy, Rodin, fal or Tripo; 2D through Codex's own image generation). The Blender clean-up in `Docs/BlenderCleanup.md` comes after. Produces files, manifests and `ImportSource/_gen/state.json`; does not import them. Usually run by Codex on Dylan's ChatGPT plan. It works in the main checkout because it never builds and touches only its own paths, so it commits with `git commit -- <its paths>` and never a bare `git commit`.

- Day 1: the asset list (Dylan's next session), one test asset per type, Dylan approves each, adjust `styles.json` once
- Day 2: hero (with the Meshy rig time-box), rat, goblin, bug, three villagers, Fenella, house fronts, celery, toothbrush, almonds, clapperboard, one Rodin prop
- Day 3: set dressing, skill icons, replacements for anything that failed (Fab or Synty packs, chosen by Dylan)

Every generated or bought asset gets a `Docs/Declared.md` line the same session (`gen_batch.py declare`). Hand-over: meshes in `ImportSource/<tool>/<id>.glb`, 2D in `Assets/Incoming/2D/<id>/`, the state in `ImportSource/_gen/state.json`, and notes on what looks wrong in `Docs/Workers/assets.md`. GAME watches these (see "Watching asset state" under GAME).

### AUDIO: music, sounds, voice

- Day 2: village loop, the sword sting, first eight sounds
- Day 3: credits song, remaining sounds, Fenella clips if a speech tool is chosen (most-heard triggers first, named `VO_<LineId>`)

### DESK: words and paperwork

Short sessions when Dylan wants something written. Jokes changes (before RULES turns them into the lines table, T09; after that, changes go through RULES), the itch page, the tools-used list, the credits text, the video shot list, checking `Docs/Declared.md` against what is actually in the build.

### REVIEW: end of each day

Reads the day's commits, `STATUS.md`, the worker notes and the latest screenshots. Answers three questions for Dylan: what is behind, what to cut now from the cut order, what the first task tomorrow is. Changes nothing itself.

## Dependencies between workers

| Needs | From | Before |
| --- | --- | --- |
| Stage machine (T03) | RULES | GAME T06 |
| Lines and scheduler (T09) | RULES | GAME T11 |
| Ledger (T04) | RULES | GAME T14 |
| Progression (T04a) | RULES | GAME T14a |
| Note clamp (T12a) | RULES | GAME T12b |
| Note endpoint live (B1) | BACKEND | GAME T12b |
| Premiere endpoint (B2) | BACKEND | GAME T16a |
| `NoteForm.md` agreed | this file's author; frozen at jam start | RULES T12a, BACKEND B1 |
| One approved asset per type | ASSETS | GAME T20 |
| Music and first sounds | AUDIO | GAME T16 (volume slider), T19 (sting) |

GAME never waits idle: every task above has engine work that can go first against a stub.

## Which workers are live

| | Day 1 (Nov 12) | Day 2 (Nov 13) | Day 3 (Nov 14) | Day 4 (Nov 15) |
| --- | --- | --- | --- | --- |
| GAME | all day | all day | all day | all day |
| RULES | all day, unattended | all day, unattended | morning only | off |
| FX | off | when the slice is ahead | when the slice is ahead; freeze at end of day | off |
| BACKEND | B1 | B2 and deploy | prompt tuning, short | off |
| ASSETS | list and test assets | main batch | replacements | off |
| AUDIO | off | music and sounds | song, voice | levels only |
| DESK | off | off | itch page, credits | submission |
| REVIEW | end of day | end of day | end of day, freeze call | off |

When Dylan is away from the keyboard: RULES and BACKEND keep going; GAME stops at its next screenshot checkpoint rather than guessing at how something looks.

## Notes files

RULES, FX, BACKEND, ASSETS and AUDIO each keep `Docs/Workers/<name>.md`: what is done, what is in progress, requests for other workers. GAME reads them at the start of each task and folds what matters into `Docs/STATUS.md` and `Docs/DEVLOG.md`, which only GAME writes. That keeps the two shared files free of merge conflicts.

## Start prompts

**GAME:** "You are the GAME worker. Read `CLAUDE.md`, `Docs/STATUS.md`, `Docs/Design/BUILD_SPEC.md` and `Docs/Design/WORKERS.md`. Read `Docs/Workers/*.md` for hand-overs and run `python -I Scripts/gen_batch.py status`. Merge `rules` if it is ahead. Start the asset-state watch (WORKERS.md, GAME, 'Watching asset state'). Take the next GAME task, finish it to its 'done when', commit, update STATUS and DEVLOG."

**RULES:** "You are the RULES worker in the worktree at `D:\Source\Unreal\MegaJam2026-rules` on branch `rules`. Read `CLAUDE.md`, `Docs/Design/BUILD_SPEC.md` sections 3, 3a, 3c and 5, and `Docs/Design/WORKERS.md`. Edit only what RULES owns. Take the next RULES task, write the tests first, build and run `Jam.Film.*`, commit to `rules`, update `Docs/Workers/rules.md`, then take the next one."

**FX:** "You are the FX worker in the worktree at `D:\Source\Unreal\MegaJam2026-fx` on branch `fx`. Read `CLAUDE.md`, `Docs/Design/BUILD_SPEC.md` sections 3, 3c, 3d and 5, and `Docs/Design/WORKERS.md`. Then read `Docs/Design/The_Dread_Land_of_Candy_Attacks_and_Effects_Pipeline.md` (roster and contract) and skim the `..._VFX_Design.txt` bible (reference only); where they disagree with the spec, the spec wins. Read the existing code you build on: `Gas/JamGameplayAbility.h`, `Gas/JamAoePattern.h`, `Film/JamProgression.h`, `Game/JamSkillFx.h` and `AJamHero::RequestSkill`. Edit only what FX owns. Take the next FX task, write the tests first, build and run `Jam.*`, commit to `fx`, write requests for GAME and what you finished in `Docs/Workers/fx.md`, then take the next one. Never open the editor, never print or open config or key files (this is streamed)."

**BACKEND:** "You are the BACKEND worker. Read `Docs/Design/NoteForm.md` and `BUILD_SPEC.md` section 3a in the MegaJam2026 repo, and the route and client pattern in OneMore's word server (source only; never open config, env or key files; this is streamed). Build the next B task as a separate Gunicorn service with its own database. Load the `claude-api` skill before the model call. Record progress in `Docs/Workers/backend.md`."

**ASSET LIST (one Opus session, before ASSETS starts generating):** "Read `Docs/Design/BUILD_SPEC.md` (sections 3, 3a, 3c, 6), `Docs/Design/Jokes.md` (set pieces and signs), `Docs/GenerationRouting.md` and `Docs/styles.json`. Write `Docs/Design/ASSETS.md`: every asset the three scenes need, grouped as hero, nuisances, NPCs, set pieces and terrain per scene, props, Niagara effects, materials, UI art, audio. For each: name, which scene or system needs it, tool (Meshy, Rodin, engine, 2D), rigged or static, rough triangle budget, priority (must, should, stretch), and the cheapest acceptable fallback. Reuse aggressively: one mesh with tint and scale beats two meshes. Mark at least one prop for Rodin. Terrain and Niagara are built in the engine by the GAME worker, not generated: for those, write a spec of what to build, not a prompt to run. Remember what the systems imply: typed notes need rain, sprinkle-snow and fog, and tint and scale that work on every nuisance; progression needs four skill effects (cone, line, ring, ultimate) and four icons; the finale needs the grey-checker material swap. End with a generation order for the day 1 test assets and the day 2 batch. Do not generate anything."

**ASSETS:** "You are the ASSETS worker. Follow `Docs/GenerationRunbook.md`, stage `test`." (Then `batch1`, `batch2` as Dylan approves.) Do not build, do not open the editor, do not touch `Source/` or `Content/`.
