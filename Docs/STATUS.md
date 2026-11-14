# STATUS - GAME, Nov 14: batches 1 and 2 green

Read CLAUDE.md and Docs/Design/RELEASE_PLAN.md (Nov 14 section).
Dylan asked to finish existing batches before packaging. No package/ZIP refresh
or upload was performed. Original wishlist/history:
Docs/Archive/STATUS-before-batch0-2026-11-14.md.

## Next task

Dylan's hands-on play / tester feedback, then any batch 3 blockers actually
reported. No new feedback batch exists yet. Quest givers and outer quests remain
cut. Packaging stays deferred until Dylan resumes it. No Backlot tests/fixes.
This is a good thread-reset checkpoint; source and handoff are committed.

## Batch 2

- Q/E turn camera, Mouse 2 drag looks, wheel zooms (600-2200 uu, zoom offset
  persists across scene framing), F uses/takes note, R ultimate, Mouse 3 signature.
  Refuse note moved to X; T writes directions. Keyboard/menu/HUD hints updated.
- Read-only Controls card in Options and Pause, keyboard/mouse and pad variants;
  returning from Controls restores its parent menu and selection. No rebinding.
- Open numeric XP: disable 2, treat 4, collect 2, once-only quest bonus 40.
  Boss disabling also pays XP. Thresholds: 100 / 250 / 450, one point per level;
  starting ranks remain 2 / 2 / 1. Open no longer gets milestone levels.
- Scene-based tests cover both taken/refused note routes: 190 XP = 1.6 levels.
  Runtime taken route verified: quest totals 64 / 116 / 190; final level 8 +60%.
- Editor and Development game builds pass; all 72 Jam tests pass, including
  mappings, yaw/release, zoom bounds/persistence, Controls card, numeric XP.
- Inspected Saved/Screenshots/WindowsEditor/batch2-controls.png,
  batch2-pause.png and batch2-xp.png. Layouts and numeric HUD are readable.
- Saved/Batch2-Editor-Open.log: PASS mode=open main=3 side=0;
  PASS scenes=3 first=1 takes=4 notes=3/0 ending=directors_cut flees=14
  lines=13 skills=12 hits=1 dignityfails=0 time=113.7.

## Batch 1 (72a72be)

Compass label separated from strip; travel skill/XP HUD restored with clickable
controls; Open BudgetOut keeps world/gear materials and grade (the global grey
checker gag caused the grey-screen report); archer bow turns about its long axis.
Cardboard extras and budget audio/effects remain. Screenshots inspected and
three-scene Open autoplay passed; both targets and the then-69 tests passed.
Evidence: Saved/Batch1-Editor-Open.log and batch1-* screenshots.
Item 11 guaranteed direction prompt and batch 0 mouse/character/pose work remain.

## R8 paperwork

Declared 88 imported English Fenella clips and 47 SFX (ElevenLabs), expanded
tools list, updated runtime credits. Dylan confirmed music was ACE Studio with
his vocals; theme-song vocals credited to Dylan. Unrelated action-card source
art/declaration edits were preserved. R8 package/smoke/ZIP remains pending.

## Existing tester ZIP is stale (batch 0)

Builds/ImaginaryComponent_TheDreadLandOfCandy.zip: 721,385,002 bytes.
Its last packaged smoke: Saved/R7Mouse/Packaged-Open.log. Batches 1/2 and credits
are source/editor verified and have NOT been packaged. No git push or upload.
Itch handoff: Marketing/Itch/LUNA_UPLOAD_HANDOFF.md; explicit go per push.

## Hands-on checks still needed

Physical swapped-button/refocus behaviour, right-drag/wheel feel, manual typed
directions, cast visual approval and the reported golden-rain moment in play.
Automated tests/captures do not establish those. Backend redeploy is Dylan's.
Engine handoff: FREE after this checkpoint.
