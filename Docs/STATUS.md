# STATUS — GAME, Nov 14: batch 0 green

Read CLAUDE.md and Docs/Design/RELEASE_PLAN.md (Nov 14 section).
Previous detailed STATUS and Dylan's complete numbered wishlist:
Docs/Archive/STATUS-before-batch0-2026-11-14.md.

## Next task

Batch 1 BLOCKERs: 5 compass target label overlap; 15b Open HUD absent during
travel; 13 golden rain/grey screen (investigate BudgetOut world-material gag);
17 archer bow grip. Item 11 was included in the overnight work and is now verified
by scene finishability tests and the Open autoplay run; manual direction entry
still needs Dylan's play. Do not repeat batch 0 or start quest-giver expansion.

## Batch 0 result

- Verified and checkpointed overnight mouse/HUD, character-sheet and pose work.
- Items 2/12/16: clickable skills, prompts, note choices, wrap continue and pause;
  cursor stays visible; empty HUD space forwards gameplay input; GameAndUI mode.
  Slate hit-path test passes. Runtime mouse evidence/limits are in DEVLOG.
  Physical swapped-button mapping and first click after refocus need Dylan's play.
- Item 7: character sheet offers basic/signature ranks, disables ultimate spending;
  opens on level-up with points (except autoplay). Rule/menu tests pass.
- Item 14: squash/recoil anchors mesh feet, scaled ground offsets corrected;
  calm settle/breathing replaces sinking. GroundedPose test passes.
- Item 11: first scene now always offers the note prompt before its second stage.
- Existing overnight Open-default source/config change retained and packaged;
  no-flag packaged launch explicitly verified on L_Sugarborough. Backlot: -backlot.
- Camera remains middle-mouse drag until batch 2 controls revision.
- Editor and Development game builds pass; all 69 Jam tests pass.
- Screenshots inspected; no-flag Open packaged headless smoke passes.
- Dylan (Nov 14): no further Backlot tests. Open is the release gate.

## Tester build 1 handoff (no upload performed)

ZIP: Builds/ImaginaryComponent_TheDreadLandOfCandy.zip
Size: 721,385,002 bytes (721.4 MB); rebuilt Nov 14 after batch 0.
Includes class presentations/back carries/brushing plus overnight input fixes.
Evidence: Saved/Batch0-Package.log, Saved/Automation/RunTests.log,
Saved/R7Mouse/Packaged-Open.log. Visible smoke ended via viewport-close event;
the headless Open rerun supplies the valid autoplay result.

```
JamAutoplay: PASS mode=open main=3 side=0
JamAutoplay: PASS scenes=3 first=1 takes=4 notes=3/0 ending=directors_cut flees=7 lines=11 skills=12 hits=1 dignityfails=0 time=104.7
```

Itch session: follow Marketing/Itch/LUNA_UPLOAD_HANDOFF.md; Dylan's explicit go
is required for this push. No git push or itch upload by GAME.

## Remaining scope

Batch 2: Q/E camera, right-drag, wheel zoom, F use, R ult, middle skill 2;
read-only controls card; numeric XP tuned to 1.6 levels across three quests.
Quest NPCs/repeatable outer quests remain cut unless Dylan decides otherwise.
Backend redeploy remains Dylan's task. Cast visual approval remains pending.
R8: final declarations/credits/tools, package/smoke/zip and submission handoff.
Known prior wishlist details and validation history remain in the archive above.

Engine handoff: FREE after batch 0. Next session starts at project root.
