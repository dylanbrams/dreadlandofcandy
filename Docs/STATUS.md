# STATUS - GAME, Nov 14: P1 packaged and smoked

**P1 handoff:** `Builds/ImaginaryComponent_TheDreadLandOfCandy.zip` (688 MB;
under 1 GB). This includes Fixes1: Fenella recruitment and markers, ten village
jobs and quest branches, and comic buttons. Packaged shipped-map smoke passed:
`JamAutoplay: PASS mode=open main=3 side=0`, `JamAutoplay: PASS scenes=3`,
`JamVillageAutoplay: PASS main=3 side=10`; all smoke checks passed. Development
package succeeded; 79 Jam tests passed with log EXIT CODE 0. No upload.

**Next:** Dylan plays this package and records BLOCKER / SHIP / NICE feedback in
`Docs/Feedback/Fixes2.txt`; GAME then takes Fixes2. Engine handoff: FREE.

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
