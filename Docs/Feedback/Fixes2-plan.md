# Fixes2 plan: split and HOLD rules (Nov 14)

Source: `Docs/Feedback/Fixes2.txt` (Dylan's QA round on the P1 package).
Two threads work at once in the main checkout: **CLAUDE (GAME)** and **CODEX**.
Feature freeze 11 PM PST tonight; submission Nov 15, 5:59 PM PST.

## UPDATE (Nov 14, later): CODEX runs all eight

Claude is out of quota for about seven hours. **CODEX takes every row below**, the
CLAUDE ones included, and owns every file listed in both ownership lists. Order: 8, 7,
2+3, 1 (code side), 4 (write the Proposals contract in `QuestBranches.md` first, then
client, then backend), 5, 6 (Meshy job, stop for Dylan's approval). HOLD still applies.
Editor imports (voice, shield) stay post-GO. Claude returns at the end to review,
integrate and package, so keep `Docs/Workers/codex-fixes2.md` current: per fix, the
commit hash, files touched, what's unfinished, open questions, and the "verify at GO" list.

## HOLD: no testing until Dylan says GO

Dylan is doing a QA pass on this machine. Until he writes **GO** in the thread
(or adds a line `GO <time>` at the bottom of this file), both threads:

- **May:** read code, edit the files they own (below), write docs and notes,
  compile with `Scripts\build.ps1` (editor target only; builds queue on a
  mutex, so never two at once by hand), and commit each fix after it compiles,
  with `[untested]` at the start of the commit subject.
- **May not:** run `Scripts\run-tests.ps1`, `screenshot.ps1`, `package.ps1`,
  `smoke-packaged.ps1`, `verify_village_quests.ps1`, the game, `-autoplay`, or
  the editor; run backend `pytest` or any deploy; launch test/smoke subagents
  (test-runner, smoke-checker, Luna). No `upload-itch.ps1`, ever, without Dylan.
- **Must:** keep a running list of "to verify at GO" for each fix (what to run,
  what PASS looks like) in their progress file. Nothing counts as done until it
  has been verified after GO.

Compile errors are fine to fix during HOLD. Behaviour is unverified until GO;
say so in every note and commit.

## The split

| # | Fix | Owner | Why |
| --- | --- | --- | --- |
| 1 | Fenella's reality-show intro + import the new voice lines | CLAUDE | Captions/story queue live in `JamGameMode.cpp`; audio import needs the editor (post-GO) |
| 2 | Active quest gets a distinct marker | CODEX | Self-contained in `JamQuestMarker` |
| 3 | Candy-cane markers (red/white; active = red/gold and moving) | CODEX | Same actor as #2 |
| 4 | Player proposes a solution; AI reply tells the game what to spawn and do | CLAUDE (contract + client), CODEX (backend endpoint) | Cross-cutting design; backend is a separate Python repo |
| 5 | Relationship levels per villager; quests grow more complex | CLAUDE | Quest data and flow (`JamVillageQuests`, `JamVillageFlow`) |
| 6 | Knight shield, generated with Meshy to match his look | CODEX (generate), CLAUDE (import + socket, post-GO) | ASSETS runbook; import needs the editor |
| 7 | Skill-bar cooldown graphics don't show | CODEX | HUD + GAS cooldown read; needs a root cause, not a guess |
| 8 | Fenella's subtitles draw over the pause menu | CODEX | HUD layering; same files as #7 |

Order inside each thread: Codex does 8, 7, 2+3, then 4-backend, then 6.
Claude does 1 (text/hooks), 4 (contract first), 5, then post-GO imports.

## File ownership (no thread edits the other's files)

**CLAUDE:** `Game/JamGameMode.*`, `Game/JamVillageFlow.*`, `Game/JamVillager.*`,
`World/JamVillageQuests.*`, `Game/JamNoteClient.*`, `Film/JamLines.*`,
`UI/JamScreens.*` (quest choice / proposal screen), `Docs/Design/QuestBranches.md`,
`Docs/Design/Jokes.md`, `Tests/VillageQuestTests.cpp`, `Content/` (post-GO only),
`Config/`, `Docs/STATUS.md`, `Docs/DEVLOG.md`, `Docs/Feedback/Fixes2-progress.md`.

**CODEX:** `Game/JamQuestMarker.*`, `Game/JamHUD.*`, `UI/JamHudWidget.*`,
`UI/JamHudView.*`, `Gas/JamGameplayEffects.*`, `Gas/JamGameplayAbility.*`
(cooldown only), `Tests/UiHud*Tests.cpp`, `Tests/GasTests.cpp`, `backend/`
(its own repo), `Assets/Incoming/`, `ImportSource/` (shield), `Assets/Generation/jobs.json`,
`Docs/Declared.md` (shield line), `Docs/Workers/codex-fixes2.md`.

Need a change in the other thread's file? Write it under "Requests" in your
progress file and tell Dylan; don't make it yourself. Commit with
`git commit -- <your paths>`, never a bare `git commit` or `git add -A`.
Never commit the `DefaultEngine.ini` Android SecurityToken diff, `music/`,
`vocals/output/*` or `Docs/Devlog/PersonalNotes.txt`.

## Fix notes

**1. Fenella intro + voice.** The five-caption reality-show intro is already in
code (after P1, so Dylan hasn't seen it). Check it reads as Fenella explaining the
show, *Heroes of Dread: The Land of Candy*, before the first job. New audio:
`vocals/output/fenella/` (MP3/WAV pairs), manifest
`vocals/output/fenella_pickups_2026-11-14/manifest.json`, notes
`vocals/Fenella_NewLines_2026-11-14.md`. During HOLD: map line IDs to the sound
assets in code and pick which of the 20 candidates get hooks. After GO: import the
WAVs to `/Game/Audio/VO/Fenella/` (must be cooked: check `DirectoriesToAlwaysCook`),
then listen in game. Captions and voice must stay in sync; talking to her still skips.

**2+3. Candy-cane markers.** Build in C++ from engine basic shapes and
dynamic material instances (no new editor assets; anything loaded by path must be
cooked). Shaft of alternating red/white bands with a hook on top, pointing down at the
target. Active quest: red/gold bands, larger, spinning and bobbing, readable from far
away and on the compass. Inactive: red/white, still or slow bob. Exactly one active
marker. Keep Fenella's golden recruit marker readable as its own thing, or fold it in
as the active style; note the choice.

**4. Player-proposed solutions.** CLAUDE writes the contract first in
`Docs/Design/QuestBranches.md` (new section "Proposals"), then pings Codex via Dylan.
Shape: the quest screen gets a free-text "Propose a plan" field (gamepad: the
existing note editor pattern). The game sends the quest, giver, motif and the text to
`POST /v1/director/quest/propose`. The model returns a **plan built only from things the
game can spawn and verbs it can check**: a list of steps, each `{verb, target, count,
destination}` from closed enums the client already has (pacify, gather, feed, brush;
rats, goblins, bugs, almonds, celery...; known camps), plus Fenella's reaction line. Client
validates and clamps every field, spawns the targets at the destination, tracks the
steps as the quest objective, and pays the normal reward. Any failure, refusal or
timeout: Fenella says a canned "let's stick to the script" line and the two authored
routes remain. The model never decides rewards or new types. `-nonotes` hides the field.
Codex implements the endpoint in `backend/` next to `quests.py` with the same screen,
limits, budget and cache as branches, plus tests (written in HOLD, run after GO).
Deploying it is Dylan's call after GO.

**5. Relationship levels.** Per villager: a level (e.g. Stranger, Acquaintance, Friend,
Bestie) raised by each completed job, shown on the quest screen and in the journal,
with a short villager line on level-up from `Jokes.md`. Complexity grows with level:
later jobs add steps (pacify then feed then return), larger counts, or a second camp,
using existing verbs and targets only. Rules headless in `JamVillageQuests` with tests
(written in HOLD, run after GO). Session-only like the rest of progress. Proposals (#4) may
pass the level to the backend so plans scale too.

**6. Knight shield.** Codex follows `Docs/GenerationRunbook.md`: a Meshy job matching the
hero's style (`Docs/styles.json`, hero reference in `ImportSource/Meshy/SK_Hero_Rig/`),
a candy-themed knight's shield, low poly per `ASSETS.md`, front facing out. Stop for
Dylan's approval; never self-approve. Declare it in `Docs/Declared.md`. After GO, CLAUDE
imports it and attaches it to the left-hand socket, checks it in a screenshot.

**7. Cooldown graphics.** Find the actual break before changing anything. Path:
`AJamHUD` reads `GetActiveEffectsTimeRemainingAndDuration` by cooldown tag
(`JamHUD.cpp` ~line 324), `UJamHudWidget` sets `Cooldown` on a MID of
`/Game/Art/UI/Materials/M_SkillCooldown` (~line 883, 956). Suspects: the cooldown GE's
duration (SetByCaller or overridden in `ApplyCooldown`) reports 0 or -1, so the ratio is
never set; the MID is replaced when the icon texture loads; the material parameter name
differs from `Cooldown`; the material isn't cooked. Write the hypothesis and evidence
in the progress file. Add a headless test of the ratio if the bug is in the read.

**8. Subtitles over the pause menu.** Captions (and the speaking-portrait box) must
hide, or draw under, any open screen (pause, journal, quest choice, notes). Prefer
hiding the caption view while a screen is open (`AJamHUD` knows when `Screen` is in
the viewport); a caption that expires while paused should not be lost if it is a story
line. Don't touch the caption queue in `JamGameMode.cpp` (Claude's); request it if needed.

## At GO (each thread, in this order, one Unreal job at a time)

1. Codex: compile, `Scripts\run-tests.ps1` (log `EXIT CODE: 0`), backend `pytest`.
2. Claude: imports (voice, shield) in the editor, compile, tests,
   `Scripts\verify_village_quests.ps1 -Mode offline`, screenshots of: arrival, a quest
   marker field (active vs inactive), skill bar mid-cooldown, pause with a caption showing,
   quest proposal screen. Look at each one.
3. Claude: package, smoke, zip (submission candidate). Dylan deploys the backend.
Fix anything that failed before the package.
