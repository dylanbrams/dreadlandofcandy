# STATUS (rewrite this file; do not prepend)

Keep this page short: current state, what is unverified, what is next. Session history goes in
`Docs/DEVLOG.md`; the old long-form STATUS is in `Docs/Status/STATUS-archive-to-Nov15.md` (read a
section there only when a task needs it).

## Fixes5 (Nov 15, `Docs/Feedback/Fixes5.txt`)

Done, editor build + Jam tests green, seen in autoplay screenshots (`Saved/Screenshots/WindowsEditor/fixes5*_*.png`):
- #1 comic cards fly in the game world: all 36 multilingual PNGs imported to `/Game/Art/UI/ComicCards`
  (`Scripts/import_comic_cards.py`, 512 px effect textures with mips), material `/Game/FX/M_ComicCard`
  (`Scripts/make_comic_card_material.py`: unlit translucent after DOF, alpha x Fade). `AJamComicCard` is one camera-facing
  quad: readable for 0.25 s, then a 3D tumble while it flies 3-5 m out, pops in, fades. `AJamGameMode::SpawnComicCards`:
  basic 3, signature 5, ultimate an 8-card ring; languages cycle so neighbours differ. Knight MANNERS, barbarian
  VITAMINS, mage/archer BRUSH. Seen at 1080p (`cards3d_c21.png`).
- #2 help: H / F1 / gamepad View opens Controls over a pause, Back resumes. Hints hide 12 s after they last changed;
  the camera tip shows only for the first 40 s and ends "H: all controls".
- #3 Resolve bars over every unpacified nuisance within 2600 uu (`AJamHUD::DrawResolveBars`, canvas).
Not packaged; packaged cook of the cards unverified (smoke test).

**#4 more quest types / directions (done, 86/86 Jam tests green, partly seen in autoplay):** every authored job has a
third route (`ThirdRoute` in `JamVillageQuests.cpp`, lines in Jokes.md "Villagers: third routes", text only). New step
kind Visit (walk to a place; signpost, Dr. Molar, Baker Crumb, orchard and meadow sets) and modifiers Rush (clock per
camp from arrival, x1.5), Gentle (they fight back, one hit restarts the step, x1.3), Crowd (double count, x1.75).
Camps Riverbank and Mint Grove now used. Offer screen takes 4 routes (`QuestBranch3`); backend improvise keeps the
third. Autoplay `-autobranch=2` plays third routes: Crowd, Visit, Rush (no timeout) and Crowd seen paying out over 4
jobs; run stopped when the bot snagged on an edge heading to Mint Grove. Gentle not seen live (autoplay ignores Gentle
hits). Not packaged.
- Player plans (Propose a plan) can now visit places and carry a Rush/Gentle/Crowd twist (contract in
  `QuestBranches.md`, "Fixes5 #4 additions"). Fixed on the way: improvise sent 3 routes (backend wants 2, so it always
  fell back); the backend camp list lacked Riverbank and Mint Grove, so every proposal failed its vocabulary check;
  the backend refused third-route history. Backend committed in its own repo (`backend/`, 9695393, 156 pytest pass),
  **not deployed: Dylan deploys it**; until then proposals still fall back (improvise fix is client-only).
- Seen in a 70 s autoplay shot: camera inside a giant teal candy prop on the way to The Almond Affair.

## Fixes4 #1-8 (Nov 15, `Docs/Feedback/Fixes4.txt`)

Done, committed (8a4ece6, 0c33a50), editor build + 85/85 Jam tests green, **not yet seen live**: title Controls line,
Left Shift = signature, start level 1 with ranks 1/1/1 (XP to 10 = 1080, Lecture max 6, Line max 5, +8% power and
~10% less damage taken per level, village Resolve/damage scale with total jobs), flashes ~1/5 brightness, barbarian ult
loops + spins 4 turns with a bigger 4.2 s flour cloud (FX regenerated), knight attacks throw flying text placards.
Packaged to `Builds/Win64-Development` (log `Saved/pkg-fixes4.log`); no smoke or zip yet. Items 9-10 (river, bridge,
path brush, house on path, tree in road) go to the TERRAIN thread.

## Fixes4 #11 camera (Nov 15)

Follow camera lifts (up to 55 degrees more pitch, fast up / slow down) when a sweep from the hero to the camera hits
anything on Visibility (terrain, buildings) or any of the 18 tall instanced-foliage components (tree canopies, big
gumdrops have no collision, so their instance bounds are checked). Pinned scene shots keep their framing. Hero walkable
floor 52 degrees, step 60 (snagging on banks). Seen in autoplay: same frame went from canopy-filled to a clear view
(`camlift_b.png` vs `camlift_d.png`). Not packaged. Engine/build FREE after this commit.

## Black Sugar UI (Nov 15, after terrain handoff)

Implemented the selected dark candy UI. Editor build passes, all 82 Jam tests pass.
Three UI textures and OFL fonts included; reviewed six initial screens, then fixed
pause clipping/cooldown overlap and reviewed final pause at 1080p and HUD at 720p.
Evidence/handoff: `Docs/Design/CandyUI/IMPLEMENTATION.md`. Engine FREE, no running
editor/game. Existing package/ZIP does not contain this UI; no package or upload
in this task. Full 25-screen sweep and physical gamepad playtest deferred to
conserve Dylan's usage.

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


### Nov 15: generated drifting marshmallows

Replaced the river's 13 cylinder placeholders with three fal-generated,
Blender-cleaned marshmallow variants. Material flow remains +Y at 45 cm/s,
following the sinuous river centerline, with slight bobbing and endpoint wrap.
Whole-object offsets avoid splitting a mesh at the wrap; expanded bounds keep
the animated meshes visible away from their original actor positions. Finish
is matte cream. Sources and cleanup report: Docs/Devlog/Marshmallows.
Timed runtime screenshots: marshmallow-flow-8.png and marshmallow-flow-18.png.
This task changes no C++ and does no packaging. Other worker source edits made
after the previous green build/82 tests are outside this asset verification.
