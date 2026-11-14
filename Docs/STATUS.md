# STATUS (rewrite this file; do not prepend)

Keep this page short: current state, what is unverified, what is next. Session history goes in
`Docs/DEVLOG.md`; the old long-form STATUS is in `Docs/Status/STATUS-archive-to-Nov15.md` (read a
section there only when a task needs it).

## Where things stand (Nov 15, ~01:45)

- Deadline: **Nov 15, 5:59 PM PST**. Submission ZIP: `Builds/ImaginaryComponent_TheDreadLandOfCandy.zip`
  (package -> smoke -> zip scripts all find the renamed `TheDreadLandOfCandy.exe`). No upload without Dylan.
- Editor build clean, 82 Jam tests pass. A fresh package/smoke/zip run was started at ~01:45 with everything
  below; check `Saved/pkg-nov15.log`, `Saved/smoke-nov15.log`, `Saved/zip-nov15.log`.
- Map: Riverlands expansion plus the generated candy foliage mix (see archive, Nov 14/15 terrain entries).
- Attack FX are generated: edit recipes in `Scripts/build_niagara_attacks.py`, run `Scripts/build_attack_fx.ps1`
  (its last step fails on stale review screenshots unless run with `-Review`; everything before it is the real check).

## Fixes3 (`Docs/Feedback/Fixes3.txt`), by item

Done in code, **none yet seen live** unless noted:
1 giant celery when feeding non-goblins · 1b comic jagged objective/caption panels, wobbling yellow shout bubbles ·
2 per-class Niagara juice (peppermint shower + giants homing onto nearest nuisances, celery arrow volley, flour clouds,
card storms), camera shake, per-class hit words · 4 bugs seek pickups and munch with "nom" bubbles ·
5 battle cries (English first per player, then 14 languages) · 6 Fenella reachable in town · 7 level-up returns to play ·
8 viewport refocus after menus · 9/12 per-class skill SFX (were never cued) · 10 per-giver afterglow lines ·
11 mayor megaphone, scout crook beside her, baker chef hat · 14 arrow over pacified nuisances · 15 walk rate follows speed ·
19 givers' candy canes raised · 20 big-icon skill HUD · 21 village buildings/peppermint tree · 22 weapon offsets tightened ·
23/24 trunk collision Pawn-only, camera ignores trunks · 25 attack bubbles.

Open: 3 classes share hit shapes (GAS) · 10b random quest count per villager · 12 archer voice recordings themselves
(mapping is right; may need regenerating) · 13 cobblestone path · 16 escalating/losable post-quests (no village defeat
path) · 17 barbarian rig + its Niagara · 18 mage staff not following animation.

## Next

1. Dylan playtests the fresh package: attack FX feel (flour density, arrow orientation, peppermint giants), bubbles,
   bugs munching, villager props, WASD after quest accept, walk speed.
2. Tune FX recipes from that playtest, repackage, smoke, zip, then Dylan uploads.
