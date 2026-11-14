# STATUS - Nov 15: Fixes3 #9, #10, #12 audio and afterglow lines

Root cause of #9 and the archer audio in #12: every class played the knight's Sword_Raise/Lecture sounds; the mage, archer and barbarian SFX were imported but never cued. `NotifySkillCast` now plays per-class cues (sugar, soap, peppermint; celery, floss; knead, encouragement, rolling pin); the knight's Last Judgment adds the manners dome. #10: each giver has three afterglow lines after their quests are done (`VillageAfterglow` in `JamVillageFlow.cpp`); random quest counts per villager not done. #3 left as is: all classes share one set of hit shapes in GAS; only cosmetics differ. Build clean, 82 Jam tests pass. Not heard or seen live.

# STATUS - Nov 15: Fixes3 #2, #14, #25 effects pass

Every non-Lecture skill (all classes) now pops a jagged hero shout bubble; pacified nuisances answer back 80% of the time (was 40%); eight new shouts and five replies added to `JamLines.cpp` (not yet mirrored in `Jokes.md`). Every class's attack also spawns `NS_M02_PeppermintPopfall` as an extra layer (#2). Pacified nuisances show a small bobbing mint-green cone arrow until treated (#14). Build clean, 82 Jam tests pass. None of this has been seen live: no screenshot, no repackage. Engine FREE.

# STATUS - Nov 15: generated foliage mixed into original woodland

Replaced 134 existing non-snow Birch/Blossom placements (one in five) with
45 peppermint, 44 candy apple and 45 candy trees. Added 67 berry bushes and
67 succulents at those footprints; all eight FoliageTwo models now appear.
Snow firs and the central square tree are preserved. Crowns/understory are
nonblocking; replacement trunks block Pawn and ignore camera. Build and Jam
tests passed (82/82). Offscreen review inspected:
`Saved/Screenshots/WindowsEditor/foliage-mix.png`; scale and palette blend with
the older woodland. Manual traversal and cooked delivery remain unverified.
No repackage or upload.

# STATUS - Nov 15: Fixes3 #8, #15 and script fixes

Fixes3 #8: after a menu closes, the HUD keeps reclaiming game viewport focus for 0.5 s, so WASD should work after accepting a quest without a click. Fixes3 #15: the walk animation rate now follows ground speed (2.1x at base walk speed, clamped 1.2 to 3) to cut foot sliding. Smoke and zip scripts now pick the renamed TheDreadLandOfCandy.exe launcher. Editor build clean, 82 Jam tests pass. Neither fix has been seen live; no screenshot or repackage since. The Nov 15 ZIP predates these changes. #16 (escalating, losable quests) skipped: the village has no defeat path. Engine FREE.

# STATUS - Nov 15: fresh packaged build

Development package succeeded after renaming a unity-build helper collision in JamVillageQuests.cpp. Packaged Open film autoplay passed all three scenes; packaged village autoplay passed main=3 side=10. Corrected ZIP: Builds/ImaginaryComponent_TheDreadLandOfCandy.zip, 727.3 MiB; launcher and cooked content containers verified. No upload. The smoke and zip scripts select the wrong executable after the launcher rename; the package run used a temporary alias and verified the final archive after removing it. Manual play and upload remain. Engine FREE.

# STATUS - Nov 14: Riverlands map redesign

**GAME redesign:** Dylan authorized a larger, prettier map using the new
terrain assets, a retained hill, river and bridge, relocated quest destinations,
and Fixes3 physical lines 21/23 (items 20/22). This overrides the earlier new
terrain cut.

The saved map now has seven generated village buildings (including two small
houses), a peppermint square tree, an eastern terrain opening and Riverlands.
Playable area grows 58.3%, from 240x240m to 380x240m. The Molar hill and film
sets are retained. Far meadow moves to (22200,0), with riverbank and mint-grove
camps added. Givers move beside their workplaces; authored quest IDs remain
stable. Imported assets and reproducible scripts: `ImportSource/Terrain/Expansion`.

The large-icon skill HUD request was already implemented and is retained.
Class hand/back carry offsets are tightened; live pose inspection remains.
Existing trunk hulls are adopted, with tree HISM collision restricted to Pawn;
new riverland trunks have narrow hidden cylinders. Camera channels ignore
these hulls. This alone does not prove foliage never obscures the hero.

Village review screenshot inspected: `Saved/Screenshots/WindowsEditor/riverlands-village.png`.
Terra's cleaned Bridge126 is imported and fitted to the river. Editor build
succeeded; 82 Jam tests passed with log EXIT CODE 0. Offline village autoplay
passed main=3 side=10. Saved-world route traces pass across the bridge to the
far camp; a simple collision query hits the selected tree trunk, with Pawn
blocking verified on all 17 tree groups. Report: `Expansion/validation.json`.
River preview: `Saved/Screenshots/WindowsEditor/riverlands-river.png`.
Manual crossing, weapon clearance in every animation, camera occlusion and
cooked delivery remain unverified. No package, deployment or upload. Earlier
notes below are historical. Engine is FREE after the final review capture.

## Earlier Fixes3 gameplay checkpoint

**Terrain handoff, Fixes3 #23/#24:** Dylan is assigning a terrain rebuild to a
Sol agent. Tree collision work is stopped and its changes are uncommitted:
`Content/World/L_Sugarborough.umap`, the eleven tree meshes under
`Content/World/Polish/Meshes/`, and
`ImportSource/Terrain/Polish/tree_collision_probe.py`. The meshes have narrow
convex cylinder hulls on their lower trunks. The saved map enables QueryOnly
BlockAll on 17 tree groups (1,026 instances), leaving other foliage nonblocking.
This blocks camera/visibility too; the attempted Pawn-only configuration was not
saved or verified. An initial trace reported a hit but did not prove the hit was
the selected trunk. The terrain agent should review or replace this work, verify
actual pawn collision and clear camera sightlines, and coordinate ownership of
these uncommitted files. Do not assume #23 or #24 is complete.

Fixes3 #25: attack particles spawned outside an active film take no longer get
destroyed on their first tick merely because the shoot is not rolling. Editor
build and 82 Jam tests pass (EXIT CODE 0). Celery Shot visuals, projectile
behavior and speech bubbles still need a live check; no screenshot or village
autoplay was run after this code change. Engine released for terrain.

Fixes3 #19: raised the five village giver candy-cane markers from 320 to 400 uu
above their anchors. Editor build and 82 Jam tests passed (log EXIT CODE 0);
offline village autoplay passed main=3, side=10. The arrival screenshot was
captured but shader compilation obscured visual assessment of giver markers,
so their final appearance still needs a hands-on check. Tree collision (#23)
is still open: the shipped terrain's baked foliage is nonblocking; a targeted
tree-trunk collision pass must preserve clear roads and nonblocking undergrowth.

Fixes3 work is underway. Dylan deferred village terrain and building redesign to
tomorrow. GO and shield approval remain granted; do not deploy or upload without
asking. Engine is FREE. No new package or cooked smoke since P1.

- `61a5c5c`: restore viewport focus after menus, resume directly from an
  automatic level-up sheet, and restore mouse-directed attack aiming in Open.
  Keyboard/gamepad and aiming still need hands-on verification.
- `3c6fdc0`: class weapon carry applies the correct hand/grip and draws during
  village skill animations. Archer attack pose inspected; mage and live village
  combat still need hands-on inspection.
- `fa448d7`: Fenella can be addressed in town during an active errand; nearby
  villagers retain priority. The active-errand interaction needs a manual check.
- `85d7986`: 154 translated attack cries imported. First use of each skill per
  player is English; later cries randomly select a translated recording, with
  English fallback. PlayerState tracks first uses for the current session. Voice
  listening and packaged playback remain unverified.

Latest editor build succeeded; 82 Jam tests passed with log EXIT CODE 0 and the
offline village autoplay passed main=3 side=10. Its log records English first
cries followed by multiple international variants. Captures inspected:
`fixes3-control-check.png`, `fixes3-archer-cast-open.png`,
`fixes3-fenella-arrival.png`, and `fixes3-multilingual-hud.png`.

Next substantial gameplay item: Fixes3 #16. Village enemies are explicitly
peaceful and village mode bypasses the film Dignity/retake path. A safe solution
needs a separate recoverable encore defeat path, scaling, and tests; do not
reuse the film failure path or disturb the ten authored jobs. Also still open:
class-specific attack patterns, bugs eating, effects, mage/archer audio and
hands-on Fixes2 checks. The P1 ZIP predates all Fixes2/3 changes.

## Earlier Fixes2 checkpoint

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
