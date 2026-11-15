# STATUS (rewrite this file; do not prepend)

Keep this page short: current state, what is unverified, what is next. Session history goes in
`Docs/DEVLOG.md`; the old long-form STATUS is in `Docs/Status/STATUS-archive-to-Nov15.md` (read a
section there only when a task needs it).

## Where things stand (Nov 15, ~08:30 JST / Nov 14 15:30 PST)

- Deadline: **Nov 15, 5:59 PM PST**. Submission ZIP: `Builds/ImaginaryComponent_TheDreadLandOfCandy.zip`
  (package -> smoke -> zip scripts all find the renamed `TheDreadLandOfCandy.exe`). No upload without Dylan.
- Editor build clean, 82 Jam tests pass. Packaged, smoked (PASS, 0 errors) and zipped with everything below,
  including #3, #16, 10b and the review fixes (737.7 MB, logs `Saved/pkg-nov15c.log`, `Saved/smoke-nov15c.log`,
  `Saved/zip-nov15c.log`). Not yet playtested by Dylan in that build.
- Village autoplay run to the end (editor -game, `-autoplay -autovillage`): `JamVillageAutoplay: PASS main=3 side=10`.
  Pass the two flags as one `-Extra "-autoplay -autovillage"` string; an array is joined with a comma and ignored.
- #12 bow: class review log shows the archer's bow on `hand_l` for every skill (mage/knight `hand_r`). `-classreview`
  screenshots need `-notitle` too, or they capture the title screen.
- Codex (TERRAIN) has the engine to redo the terrain and the cobblestone village path (#13). Also asked: make the
  Molar Hills landmarks block the Camera channel (white tooth fills the camera near Fort Toothache).
- Map: Riverlands expansion plus the generated candy foliage mix (see archive, Nov 14/15 terrain entries).
- Attack FX are generated: edit recipes in `Scripts/build_niagara_attacks.py`, run `Scripts/build_attack_fx.ps1`
  (its last step fails on stale review screenshots unless run with `-Review`; everything before it is the real check).

## Fixes3 (`Docs/Feedback/Fixes3.txt`), by item

Done in code, **none yet seen live** unless noted:
1 giant celery when feeding non-goblins · 1b comic jagged objective/caption panels, wobbling yellow shout bubbles ·
2 per-class Niagara juice (peppermint shower + giants homing onto nearest nuisances, celery arrow volley, flour clouds,
card storms), class-coloured light flashes (second flash on ultimate landings), camera shake, per-class hit words ·
4 bug stages plant 7 candy crops with the first wave; bugs eat them (shrink, then *BURP*), else munch pickups ·
Fenella skips lines naming the wrong creature for the current stage (`FJamNagScheduler::AvoidWords`) ·
5 battle cries (English first per player, then 14 languages) · 6 Fenella reachable in town · 7 level-up returns to play ·
8 viewport refocus after menus · 9/12 per-class skill SFX (were never cued) · 10 per-giver afterglow lines ·
11 mayor megaphone, scout crook beside her, baker chef hat · 14 arrow over pacified nuisances · 15 walk rate follows speed ·
19 givers' candy canes raised · 20 big-icon skill HUD · 21 village buildings/peppermint tree · 22 weapon offsets tightened ·
23/24 trunk collision Pawn-only, camera ignores trunks · 25 attack bubbles.

Done after the last package (not in the ZIP yet): 3 per-class hit shapes (`Gas/JamClassShape.h`, ground flash matches) ·
16 repeat errands (relationship level 2+) fight back, harder per level, Dignity meter shows, defeat restarts the step
(`VillageDanger`, `VillageDefeat`); autoplay damage x0.25. Village autoplay (420 s cap) turned in 8/10 with 1 hit taken;
not run to completion. 18 staff already attaches to hand_r on every mage cast (log), likely fixed by 3c6fdc0.

Nov 15 morning (Claude): 10b done — each giver has 2 authored jobs plus 2-4 seeded encores, then only their
afterglow (`FJamVillageQuestLog::EncoresLeft`, tested). Code review of ec25d5e~5..HEAD: seven fixes in `e92d51f`
(town errands treatable while Fenella's town prompt is up, Collect steps never inherit danger, defeat resets step
progress, hit words no longer erase nuisance replies, Fenella's creature filter matches whole words, bugs give up
unreachable food after 6 s and only target food inside their set, attack particles sized to the class hit shape).
Not fixed (style): launcher name hardcoded in three scripts, magic 26000 east edge, per-class timing tables.
Barbarian: Niagara presets fire (log: NS_B01/B02/B03), stills look fine; "poorly rigged" needs Dylan's eye in motion.
Seen once in barbarian autoplay: a white mesh (likely a Molar Hills tooth landmark) fills the camera briefly on the
walk to Fort Toothache; camera probe does not hit it.

Not in the ZIP yet: 26 encore EXP (`247af6f`): past level 10 XP keeps counting, every 200 is an encore level
(+5% power, LevelUp sound + Fenella line, no menu); HUD shows "LV 10  ENCORE n" and "ENCORE EXP x / 200". Tested; not seen live.

Open: 12 archer voice recordings themselves (mapping is right; may need regenerating — paid generation, Dylan) ·
13 cobblestone path (terrain/texture work, Dylan's texture credits) · 17 barbarian rig (asset).

## Next

0. After Codex's terrain commit: repackage, smoke, zip again (GAME waits for the engine).

1. Dylan playtests the fresh package: attack FX feel (flour density, arrow orientation, peppermint giants), bubbles,
   bugs munching, villager props, WASD after quest accept, walk speed.
2. Tune FX recipes from that playtest, repackage, smoke, zip, then Dylan uploads.


## Nov 15: candy environment pass (ChatGPT)

Generated/reviewed 15 fal PATINA seamless 1K PBR sets (3 variants each: rock
candy, peanut brittle, peppermint, chocolate mint, mint grass). All 75 map PNGs
validated. Sources/seeds/review: Docs/Devlog/CandyPBR; selected choices recorded.
Applied blended paving + mint ground to original terrain actor overrides and
Riverlands. Added 36 trees, 73 shrubs, 23 eastern backdrop mountains, 13 drifting
3D marshmallows and 30 faint steam cards; river is opaque glossy hot chocolate.
Scripts: Docs/Devlog/apply_candy_environment.py. Inspect fresh review captures in
Saved/Screenshots/WindowsEditor/candy-{chocolate,village}.png. Runtime review logs
have no persisted material compile failures; apply-time missing-input warnings
occur while rebuilding graphs. No packaging or upload in this task. Packaged
delivery, manual traversal and gameplay performance remain unverified.
