# STATUS - Nov 14: Fixes2 checkpoint for a clean session

**Fixes2 verification update:** Screen-caption layout is fixed and visually checked
in the latest worker handoff. Autoplay no longer loops on the first repeatable
encore before visiting other givers. Editor build and 82 Jam tests pass; offline,
proposal and proposal-fallback village runs all pass main=3, side=10. Proposal
and fallback used the local stub, not the live backend. Manual interaction,
visual and audio checks remain in `Docs/Workers/codex-fixes2.md`; Fixes2 is not
yet fully verified. No new package, deploy or upload. Engine is FREE.

Dylan requested a fresh-session handoff. **GO has been given; CODEX owns all
eight Fixes2 rows. Engine is FREE.** Read `Docs/Workers/codex-fixes2.md` for the
current commits, verified evidence and exact next chunk. Editor build passes;
82 Jam tests and 149 backend tests pass. Voice imports and approved shield are
committed; large skill icons are committed and visually checked.

**Fixes2 is not done:** the screen-owned caption still overlaps bottom menu
buttons, and offline/proposal/fallback village verification remains. Fix that
UI chunk next. Round three is waiting in Fixes3.txt; do not start it yet.
No new package, backend deploy or upload. Older HOLD/P1 notes below are historical.

## Historical P1 snapshot

**Nov 14 voice pickup delivery:** Wrote 20 additional Fenella reality-show,
village and hero-reaction candidates, and Luna generated those plus the five
exact arrival speeches with ElevenLabs/Bella. All 25 MP3/WAV pairs are in
`vocals/output/fenella/`; scoped manifest:
`vocals/output/fenella_pickups_2026-11-14/manifest.json`. Source and production
notes: `vocals/Fenella_NewLines_2026-11-14.md`. Listening review and Unreal
import remain pending; the 20 candidates have no runtime hooks. No game code,
engine run or package changed in this audio-only task.

**Nov 14 follow-up:** Fenella's arrival introduction now establishes her
reality show, *Heroes of Dread: The Land of Candy*, the assembled villagers,
the hero's casting, and her demand for massive drama. Five ordered captions
play automatically; talking to her skips ahead. New voice recordings are now
generated, awaiting import (see delivery above). Editor build and all 79 Jam tests passed (EXIT CODE 0); offline
autoplay passed all three main scenes and ten village jobs. Luna inspected
`Saved/Screenshots/WindowsEditor/village-arrival.png`: readable, no clipping.
This change is
not yet in the P1 package below. `AGENTS.md` records Dylan's preference for
Luna subagents for routine tests and checks.

**P1 handoff:** `Builds/ImaginaryComponent_TheDreadLandOfCandy.zip` (688 MB;
under 1 GB). This includes Fixes1: Fenella recruitment and markers, ten village
jobs and quest branches, and comic buttons. Packaged shipped-map smoke passed:
`JamAutoplay: PASS mode=open main=3 side=0`, `JamAutoplay: PASS scenes=3`,
`JamVillageAutoplay: PASS main=3 side=10`; all smoke checks passed. Development
package succeeded; 79 Jam tests passed with log EXIT CODE 0. No upload.

**Next:** Fixes2 is split between CLAUDE (GAME) and CODEX in
`Docs/Feedback/Fixes2-plan.md`. **HOLD is on:** code and compile only, no tests,
game runs, editor or packaging until Dylan says GO. Engine handoff: FREE.

Read CLAUDE.md, then Docs/Design/RELEASE_PLAN.md ("Nov 14 afternoon" section).
Previous STATUS (batches 1 and 2): git history at 3cae0ec. Wishlist archive:
Docs/Archive/STATUS-before-batch0-2026-11-14.md.

**Quest givers are a core marquee feature (Dylan, Nov 14).** Every earlier
"quest givers cut" line in the plan or briefs is void. Protect the village loop
in every fix and every package.

## Next task (GAME, ChatGPT)

1. **Done: Package, smoke, zip (P1).** `Scripts\package.ps1`, then
   `Scripts\smoke-packaged.ps1` on the shipped map, then
   `Scripts\zip-submission.ps1 -TeamName ImaginaryComponent -GameName TheDreadLandOfCandy`.
   Add a smoke check that the packaged village autoplay reaches
   `JamVillageAutoplay: PASS main=3 side=10` (the comic widgets, villagers and
   quest markers are all new since the last package; cooked-only failures show here).
   Hand-off line here: zip path, size, PASS lines. No upload; ChatGPT itch pushes on Dylan's go.
2. **Dylan plays** the packaged build, writes numbered BLOCKER / SHIP / NICE lines
   in `Docs/Feedback/Fixes2.txt`.
3. **Fixes2**, BLOCKER then SHIP, until feature freeze **11 PM PST**. Each fix:
   build, `Scripts\run-tests.ps1` (expect 79+, check the log's
   `EXIT CODE: 0`, not only the shell exit), `Scripts\verify_village_quests.ps1 -Mode offline`,
   commit. Repackage once at the end of the evening: that zip is the submission candidate.

Never run Backlot (`-backlot`): the enlarged mountain blocks its third arena.

## What the game is now

Hero spawns facing Sugarborough. Fenella waits under a bouncing golden marker,
recruits the hero, follows and directs all three film-set quests (compass +
marker at the active set). The wrap says FILM COMPLETE and returns to the
village. Five villagers (Mayor Bonbon, Baker Crumb, Gardener Sprig, Dr. Molar,
Scout Taffy) give ten linked jobs: pacify, gather, pacify-then-feed/brush.
Rewards need a return visit, pay sprinkles + 40 XP once. Pause > Quest Journal.
Each villager job offers two authored routes or "Ask Fenella to improvise"
(`POST /v1/director/quest/branches`, falls back to authored on any failure).
Premiere and credits via Fenella, returning to the same session. Comic-style
jagged buttons on wrap, notes, menus and quest choices. Progress is session-only.

Design/contract: `Docs/Design/QuestBranches.md`, `Docs/Design/Village_QuestHooks.md`.
Fixes1 detail: `Docs/Feedback/Fixes1-progress.md`, `Docs/Feedback/TESTING_HANDOFF.md`.

## Evidence (Fixes1, commit 164e0af)

- Editor build 11:56; no source changed after it. Game `.exe` is from 10:00: stale, P1 rebuilds it.
- 79 Jam tests pass (`Saved/Automation/RunTests.log`, EXIT CODE 0).
- Backend 117 tests pass (`backend/` commit ab328d7, fake model).
- `JamVillageAutoplay: PASS main=3 side=10` in `Saved/Automation/Fixes1-village.log`
  (authored), `Fixes1-backend.log` (localhost branches), `Fixes1-fallback.log` (all 503).
- Screenshots inspected: `fixes1-arrival.png`, `fixes1-questbranches.png`, `village-journal.png`.

## Backend

Live server (`api.imaginarycomponent.com`) runs `3362c19`/n2: notes and premiere.
It does **not** have the quest-branches endpoint (game falls back silently on 404).
Dylan is migrating the server with a Sonnet session: `Docs/Workers/backend.md`
("Nov 14 migration"). GAME does not touch the backend or the box. When BACKEND
reports the new URL/endpoint live, GAME checks the game's own HTTPS once
(typed note + one improvise) and changes `BaseUrl` in `Config/DefaultGame.ini` (`[/Script/MegaJam.JamNoteClient]`, keep it quoted) if the host changed; `-noteurl=<base>` tests a host without editing config.

## Release state

- P1 tester ZIP is on disk (688 MB); Fixes1 is packaged and smoked.
- No git push, no itch upload since last night's tester build.
- `Docs/Devlog/PersonalNotes.txt` is Dylan's, untracked on purpose. `music/`,
  `vocals/output/*` are source audio, untracked. Never commit the
  `DefaultEngine.ini` Android SecurityToken block.

## Hands-on checks still needed (Dylan)

Recruitment and village loop feel; whether villagers and markers read at a
glance; comic buttons with mouse and pad; swapped-button/refocus; right-drag and
wheel feel; typed directions; golden rain; cast look; game HTTPS to the live backend.

Engine handoff: FREE.
