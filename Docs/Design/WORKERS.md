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

Merges `rules` into `main` (fast-forward or a plain merge) before any task that depends on it, then builds and runs tests. Imports from `Assets/Incoming/` at set points (end of T08, start of T20), not continuously. Adds every runtime-loaded folder to `DirectoriesToAlwaysCook`.

### RULES: headless C++ and tests, safe to leave unattended

No world, no engine objects, no assets, so it never needs the editor. Builds and runs `Jam.Film.*` in its own worktree.

- Day 1: T03 (stage machine), T04 (ledger), then T09 (lines table from `Jokes.md` and the nag scheduler)
- Day 2: T04a (progression and rank tables), T12a (note clamp and keyword fallback, against `NoteForm.md`), the three scenes as data tables with the finishability test
- Day 3: T22's commentary and sulking line pools, any rule fixes GAME asks for

Commits to `rules` at the end of each task. Does not edit `MegaJam.Build.cs`, the `.uproject` or anything GAME owns; asks in `Docs/Workers/rules.md`.

Set-up once, by Dylan or GAME: `git worktree add ..\MegaJam2026-rules -b rules`. First build there compiles the whole module.

### BACKEND: the typed-note service

Runs entirely outside Unreal. Contract is `Docs/Design/NoteForm.md`; if the contract has to change, change that file first and tell RULES.

- Day 1: B1 (`POST /v1/director/note`: second Gunicorn service, second Postgres database, cache table, normalisation, daily budget, rate limits, Haiku call with the form as structured output, twenty canned notes as tests including hostile ones). Pre-warm the suggested notes.
- Day 2: B2 (`POST /v1/director/premiere`), deploy, give GAME the URL, watch the first real requests
- Day 3 and 4: tune the prompt from real notes, check the budget counter, leave it alone

Loads the `claude-api` skill before writing the model call. On stream: never print environment variables, keys, connection strings or config files; never commit them.

### ASSETS: generate, clean, hand over

Runs `Scripts/meshy.py`, `Scripts/rodin.py`, the Blender clean-up in `Docs/BlenderCleanup.md`. Produces files and a manifest; does not import them.

- Day 1: the asset list (Dylan's next session), one test asset per type, Dylan approves each, adjust `styles.json` once
- Day 2: hero (with the Meshy rig time-box), rat, goblin, bug, three villagers, Fenella, house fronts, celery, toothbrush, almonds, clapperboard, one Rodin prop
- Day 3: set dressing, skill icons, replacements for anything that failed (Fab or Synty packs, chosen by Dylan)

Every generated or bought asset gets a `Docs/Declared.md` line the same session. Hand-over is a folder per asset under `Assets/Incoming/<type>/<name>/` with the mesh, textures and a one-line note on scale and pivot.

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
| BACKEND | B1 | B2 and deploy | prompt tuning, short | off |
| ASSETS | list and test assets | main batch | replacements | off |
| AUDIO | off | music and sounds | song, voice | levels only |
| DESK | off | off | itch page, credits | submission |
| REVIEW | end of day | end of day | end of day, freeze call | off |

When Dylan is away from the keyboard: RULES and BACKEND keep going; GAME stops at its next screenshot checkpoint rather than guessing at how something looks.

## Notes files

RULES, BACKEND, ASSETS and AUDIO each keep `Docs/Workers/<name>.md`: what is done, what is in progress, requests for other workers. GAME reads them at the start of each task and folds what matters into `Docs/STATUS.md` and `Docs/DEVLOG.md`, which only GAME writes. That keeps the two shared files free of merge conflicts.

## Start prompts

**GAME:** "You are the GAME worker. Read `CLAUDE.md`, `Docs/STATUS.md`, `Docs/Design/BUILD_SPEC.md` and `Docs/Design/WORKERS.md`. Read `Docs/Workers/*.md` for hand-overs. Merge `rules` if it is ahead. Take the next GAME task, finish it to its 'done when', commit, update STATUS and DEVLOG."

**RULES:** "You are the RULES worker in the worktree at `D:\Source\Unreal\MegaJam2026-rules` on branch `rules`. Read `CLAUDE.md`, `Docs/Design/BUILD_SPEC.md` sections 3, 3a, 3c and 5, and `Docs/Design/WORKERS.md`. Edit only what RULES owns. Take the next RULES task, write the tests first, build and run `Jam.Film.*`, commit to `rules`, update `Docs/Workers/rules.md`, then take the next one."

**BACKEND:** "You are the BACKEND worker. Read `Docs/Design/NoteForm.md` and `BUILD_SPEC.md` section 3a in the MegaJam2026 repo, and the route and client pattern in OneMore's word server (source only; never open config, env or key files; this is streamed). Build the next B task as a separate Gunicorn service with its own database. Load the `claude-api` skill before the model call. Record progress in `Docs/Workers/backend.md`."

**ASSET LIST (one Opus session, before ASSETS starts generating):** "Read `Docs/Design/BUILD_SPEC.md` (sections 3, 3a, 3c, 6), `Docs/Design/Jokes.md` (set pieces and signs), `Docs/GenerationRouting.md` and `Docs/styles.json`. Write `Docs/Design/ASSETS.md`: every asset the three scenes need, grouped as hero, nuisances, NPCs, set pieces and terrain per scene, props, Niagara effects, materials, UI art, audio. For each: name, which scene or system needs it, tool (Meshy, Rodin, engine, 2D), rigged or static, rough triangle budget, priority (must, should, stretch), and the cheapest acceptable fallback. Reuse aggressively: one mesh with tint and scale beats two meshes. Mark at least one prop for Rodin. Terrain and Niagara are built in the engine by the GAME worker, not generated: for those, write a spec of what to build, not a prompt to run. Remember what the systems imply: typed notes need rain, sprinkle-snow and fog, and tint and scale that work on every nuisance; progression needs four skill effects (cone, line, ring, ultimate) and four icons; the finale needs the grey-checker material swap. End with a generation order for the day 1 test assets and the day 2 batch. Do not generate anything."

**ASSETS:** "You are the ASSETS worker. Work from `Docs/Design/ASSETS.md`. Read `Docs/GenerationRouting.md`, `Docs/styles.json`, `BUILD_SPEC.md` section 6 and `WORKERS.md`. Do not build, do not open the editor, do not touch `Source/` or `Content/`. Generate the next asset on the list, clean it, put it under `Assets/Incoming/`, add its `Docs/Declared.md` line, update `Docs/Workers/assets.md`."
