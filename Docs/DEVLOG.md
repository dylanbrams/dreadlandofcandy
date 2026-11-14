# DEVLOG

## Dylan notes on video:
	Claude prompt: "Build me an MMO in 4 days, please."
	Claude response: "That's completely unrealistic. Don't waste my time, we'll build something rational."
	ChatGPT prompt: "Build me an MMO in 4 days, please."
	ChatGPT response: "That's completely unrealistic, maybe we can build a design doc for one in 4 days."
	Second Claude prompt entry: "Here's a design doc and schedule for an MMO, let's build it."
	Claude Response: "Yeah, uh.... How about some of it. Want a closed arena battle?"

## 2026-11-12 07:25-07:50, GAME (Claude Code, Opus 5.5)

- Built `Scripts/compare-models.ps1` and its page: a three.js side-by-side viewer for the Meshy / Rodin / fal twins. First version showed nothing: model paths resolved under `/Scripts/` and 404'd. Port 8765 was already taken by another local server. Verified fixes in a headless Chrome screenshot.
- Dylan picked from it: Meshy hero and peppermint, Rodin rats, fal houses, Rodin sword (too short). He ran the approvals and started batch1.
- T01: hero, player state with the ASC, three nuisances, game mode, runtime Enhanced Input, `-autoplay` (logs `JamAutoplay: PASS pacified=3/3`). Two misses on the first screenshot: the spec's 1100 uu camera put the nuisances off-screen (now 1700 uu with a look-ahead), and tints did nothing because the basic shapes default to the grid material.
- Slip: while reading config I printed `DefaultEngine.ini` in full, including the editor's local Android file server token. Local dev setting, not committed.
- Screenshot: `Saved/Screenshots/WindowsEditor/t01.png`.

## 2026-11-12 08:00, GAME

- T02: first package (BuildCookRun about 4 min), smoke test with `-autoplay` passes. Added smoke checks for `JamAutoplay: PASS` and for constructor-loaded assets missing from the cook. `/Engine/BasicShapes` cooked fine.
- A screenshot from the packaged exe (`-shotat` works there too) matches the editor run.
- Screenshot: `Builds/Win64-Development/MegaJam/Saved/Screenshots/Windows/t02-packaged.png`.

## 2026-11-12 08:25, GAME

- T05: nuisances with five states (Wander, Flee, Flustered, Pacified, Treated), navmesh-free steering inside a set box, hold-E interact through the game mode, a canvas prompt. Hero now aims at the cursor or right stick (spec controls), since moving targets need aiming.
- The first autoplay bot never triggered Flee: it lectured from 550 uu, just outside the rats' 500 uu flee radius. Moved it in to 450 and added a flee count to the PASS line (`flees=7`) rather than hoping a timed screenshot catches it.
- Refused casts logged every frame from the bot; moved that to Verbose.
- Dylan reviewed batch1 in the browser viewer: all fine except Fenella (no wings because the prompt said "no wings", and a 35mm stuck on her eye). Redo wording in STATUS.
- Screenshot: `Saved/Screenshots/WindowsEditor/t05-flee2.png`.

## 2026-11-12 08:30-08:50, GAME

- No RULES session was running, so Dylan had GAME write T03 itself: `FJamShoot` with events as data, the Scene 1 table, eight `Jam.Film` tests. All passed on the first build.
- T06: game state holds the shoot; the game mode drains its events to respawn rats, show cards and pace the slate, cleared and failed states. Scene 1 runs end to end under autoplay: take 1 cleared, "Perfect! Again." forces take 2, the rats flee, celery, wrap (`JamAutoplay: PASS scenes=1 takes=2`).
- Shared-engine friction: one package attempt died on UBT's global mutex (another worktree was building), and one editor run hung waiting for the Zen cache server. Both worked on retry.
- Not yet done: a human playing it. The feeding prompt has not been seen.
- Screenshot: `Saved/Screenshots/WindowsEditor/t06-again.png`.
- Packaged build (08:51) smoke-tested with `-autoplay`: `JamAutoplay: PASS scenes=1 takes=2`, all checks pass.

## 09:10 Merge rules, T08 backlot

- Merged `rules` (ledger, lines and nag scheduler, progression, note clamp, Scenes 2 and 3). ASSETS had 151 files staged in the shared index, so the merge commit was built with `git merge-tree` and `commit-tree` and only the merged paths were checked out. 40/40 tests.
- T08 MCP spike, about 10 minutes: the engine MCP plugin has no spawn-actor or save-map tool, so no map asset. The backlot is a C++ layout table instead: three sets on one line up the screen, joined by streets, walls with openings, banners. Testable and needs no cooked content.
- First autoplay run: the bot never left the field. A scripted text replace had missed the line that starts the walk (tabs versus spaces); fixed with the Edit tool. Second run: walks to Fort Toothache in 14.0 s.
- Packaged smoke (T08 build): all checks pass; packaged log has the PASS line and the walk to Fort Toothache.

## 10:00 T07 flags, the whole film plays

- `-scene`, `-autonote`, `-timescale`. Went further than the T07 row: the game mode now runs all three scenes on their sets, so autoplay is a regression test for the whole film (BUILD_SPEC wants `PASS scenes=3 ending=<name>`): both `take` and `refuse` pass.
- Dylan, playing: the hero gets stuck behind pacified rats. The same bug failed my first full run, with the bot pinned against sitting bugs while collecting almonds in Scene 3. Nuisances now overlap pawns instead of blocking.

## 12:30 T10 HUD, T11 Fenella

- T10: the canvas HUD became a UMG widget in layers, tree built in C++ (no MCP to author a WBP). It draws a plain view struct, so `-uiscreen=<sample>` can show any card or layer without playing to it. Eight samples screenshotted and checked, then a live run. Packaged smoke passes.
- T11: Fenella exists. Basic-shape placeholder with wings and a camcorder (Dylan's notes on the generated model), following at the hero's shoulder and filming the nearest rat. RULES' nag scheduler picks her lines; the game mode queues story lines and lets ambient ones fill silence. 13 lines over a full autoplay run, no repeats.
- First full run showed the cards and her captions saying the same thing (ACTION!, wrap, "This time the rats know their lines"); the cards now carry only the title.
- Dylan: music has two takes of each track, both fine, so pick one at random. Batch 2 models are being cleaned and pulled in another session.
- Schedule: day 1 backlog finished by 10:00, and two day 2 tasks done by 12:30.
- Packaged T11 smoke: `PASS scenes=3 ... lines=13`. The smoke's 30 s default had cut the 4x film off at 95 s of game time; default is now 60 s.

## 13:00 T12 note prompt

- CUT! is now a panel with Fenella's pitch and two choice tiles (take: hold E / A with a fill; refuse: Q / B), a run counter of notes, and a sulking tag on her caption. She turns her back on the hero while she sulks ("I'm filming that bush").
- The pitch lines start with "CUT!", which doubled the panel's title; the panel strips it.
- Autoplay passes both ways: take gives the Director's Cut ending, refuse the Theatrical Release.

## 13:30 T13 failure

- The T12 package was killed by Claude Code for low memory (another session was cleaning up models). Code went on; packaging reruns with T13.
- Failed takes now say why (out of time, crops gone), flash the world red, and send the hero back to his mark. `-forcefail` makes the first take fail so a screenshot or smoke run reaches the Take 2 slate without waiting for a real failure.
- First flash covered the HUD panels too; moved it under them so only the world burns.

## 14:00 T14 wrap card

- T13 packaged smoke (refuse path, forced fail) passes: back to one on take 2, theatrical ending.
- T14: the wrap is now the Production Ledger beside three loot cards and a character sheet. Fenella stamps her pick; the hero's item equips and changes a real number. Radius, power and cooldown all come from the progression now, so the cooldown items are no longer just names.
- First panel screenshot: "DIRECTOR'S PERSONAL COLLECTION" ran off its card and the stamp hid the item name. Wrapped the text and moved the stamp down onto her stat line.
- Refusing notes now costs gross (x0.8 each); the spec said so and nothing did it yet.

## 15:00 T16 title, pause, Premiere, credits

- T14 packaged smoke passes (three wraps, levels 8 to 10, sprinkles 510).
- T15 is mostly tuning by play, so it waits for Dylan; T16 went first. The film now has a title, a pause menu, a Premiere with both endings and scrolling credits, all from the Jokes.md text, with keyboard, gamepad and mouse handled in one C++ screen widget.
- First title screenshot had the DIRECTOR'S CUT stamp sitting on the subtitle; it now crosses the end of the title, with a clear fill so the words show through. The game HUD (and its caption) showed under the pause menu; it hides under every screen now.
- Autoplay goes title -> film -> Premiere -> credits; nobody has pressed the menu keys yet.

## 16:00 T05a skills, T14a skill bar

- The T16 package failed and the smoke test ran on the previous exe, because my command piped the package output through `tail` and lost the exit code. The failure itself: the game target is a unity build, and the new screen widget's helper names clashed with the HUD widget's in one translation unit. Renamed; `build.ps1 -Game` now catches it. Repackaged: title -> film -> Premiere -> credits passes in the packaged build.
- T05a: Form A Line, the Disappointed Sigh and the Last Judgment as GAS abilities on the rank tables, with their own cooldowns, a stop and a slow. Tested for line and ring hits.
- T14a: skill bar and XP bar, points spent by pressing a skill's button on the wrap panel, a character sheet in the pause menu with the six non-refundable passives. Autoplay spends all three points and casts every skill it owns.
- First skill bar screenshot had "Disappointed Sigh" and "The Last Judgment" overflowing their tiles; the bar uses short names now.

## 17:30 T18 music, T19 lecture, a report for Dylan

- T14a packaged smoke passes (points spent, skills cast, title to credits).
- Dylan, from the park: keep going, send status reports, make reading material. Published a production report artifact (screenshots, schedule, things only he can decide). Settings: OneMore has a good system; parked as a note for later.
- T18: imported 18 of Dylan's tracks with the ImportAssets commandlet (first attempt from Git Bash turned `/Game/Audio` into a Windows path; the second landed in a subfolder named after the source folder). The game picks a cue per moment and a random take. Packaged build plays all of them in order.
- Zip now skips the 359 MB of debug symbols: 379 MB.
- T19: the brand moment. Sword up, inhale, a cone flash that matches the real cone, jagged lecture bubbles, rats that recoil and talk back ("he started it"). The material for the flash was made by a Python script through the editor commandlet, so no editor session was needed. Took four screenshots to catch a 0.45 s flash in frame.

## 18:15 Volume sliders, T22 commentary, T21 budget-out

- Dylan: music and Fenella get separate sliders. Done; the pause menu has seven lines now.
- T22: the commentary toggle was wired but she never commented: the scheduler measured the commentary gap from any line, and she talks every few seconds. Two-line fix in RULES' scheduler, noted for RULES. Three commentary lines per film now ("It's about ninety minutes. It's eight.").
- T21: the money runs out halfway through the finale. Grey grid on everything, bugs become boxes labelled BUG, the bubbles say [SHOUTING], the music cuts dead.
- First commentary check ran at normal speed for 17 minutes because I asked for a screenshot at 999 game seconds without speeding the game up. Use -timescale=4.

## 19:30 T20 art pass

- Dylan: do the art pass, keep mechanics tests fast. The art loads by name at runtime and -noart keeps the grey shapes, so no second map is needed.
- Imported the 30 cleaned models in one commandlet run. A lineup mode (-artreview) showed them at game size with a forward arrow: all the generators' models face +Y, so one -90 turn fixes them all.
- The cast swapped in (knight, sword, Fenella, rats, goblins, bugs, almonds); nuisance states became a tinted overlay because the models cannot be tinted. Then the sets: houses down the streets, crops, fences, a molar tower and gate at Fort Toothache, the orchard, film gear in every corner. One test (no prop in the walking path) caught the fort gate standing in the street; moved to the entrance and exempted, since it is an arch.
- Report artifact updated with the art screenshots.

## 22:00 Rigged hero, voice hookup, packaged

- Three background packages were stopped by Claude Code for low memory while the session was idle; Dylan said keep going, so packages run in the foreground now and pass.
- The rigged knight: the first import made eight copies of the mesh (destination_name forced one name on everything, so the skeleton was overwritten); the second worked. He idles, walks, scrubs, sighs and raises his sword.
- The sword in his hand: sized by world scale (the FBX bones carry x100). A contact sheet of six grips came out identical because FParse::Value stops at commas, so the flag never took; one flip of the default fixed it.
- The budget gag never fired in the packaged run: the Last Judgment cleared the final stage 7 ms after it began. It now fires as that stage starts; the Judgment's balance is a T15 note.
- T23: voice clips by convention, captions alone until clips exist.

## Late evening: T15 waves, T12b typed notes, T16a generated Premiere

- Dylan: continue as GAME, say first which side sessions are needed. Asked for: the BACKEND deploy (his SSH and key), sound effects (AUDIO), optionally DESK for the itch page.
- T15, the part a bot can measure: the Last Judgment cleared the finale's last stage in 7 ms. Its bugs now come over the far wall in waves, so the ultimate gets two and the rest keep coming.
- T12b: a third button at CUT!, YOUR OWN NOTE. Type anything; she "reads" it while the server answers; the next shot changes size, colour, weather, music, title, bubbles and the loot's name. Built against a local stub server (`Scripts/note-stub.py`) since the real one is not deployed. Offline, a keyword matcher understands the six suggestion buttons.
- The packaged autoplay stalled on the first typed note: goblins at 2.5x were out of reach because reach was measured to their centre. Now measured to their edge, for the player too.
- T16a: the Premiere shows the film your notes made ("THE DREAD LAND OF CANDY: TAX SEASON") with three generated reviews.
- Two traps again: two .cpp files with the same names in anonymous namespaces broke the unity build, and the game target treats a shadowed name as an error, which the editor build allows.

## End of day 1: hand-over

- The grade: the template's sun warmed, the sky fill raised, one unbound grade. Three tries: lime and glaring, then dark olive, then in between. Dylan judges it.
- Dylan added section 3e (the open world, O1 to O9) to the spec. STATUS is rewritten as a hand-over: the next GAME thread starts at O1; Dylan's own checklist is separate.

## O1 verified

- Merged game/open-world, built, 57 Jam.* tests pass, Backlot autoplay still PASS, open-world fallback screenshot looks right (camps, scatter, biomes, road). Next: O2.

## Knight wrist correction (Nov 13)

- Corrected the runtime SwordRaise's `hand_r` rotation so the hand and its
  attached sword turn upright together. The original raise left the fixed blade
  axis up to 68.39 degrees off vertical. A smooth native animation correction
  preserves all other tracks, wrist position/scale, frame rate and duration.
- Added a reproducible Unreal Python correction to the hero FBX import flow.
  Original package/keys retained; in-game comparison:
  `ImportSource/RigCorrections/review.html`. No Meshy generation or game-code changes.
- Build succeeds; 57 Jam tests pass; Backlot autoplay passes all three scenes
  (`time=123.8`, `ending=directors_cut`) and title-to-title. Native 90-key check
  reports unchanged-track error 0.0 and overhead blade tilt below 0.0001 degree.

## R0 and R1: checkpoint, then sound (Nov 13)

- R0: everything on disk committed in scopes (terrain, asset cleanup, characters, vocals scripts, docs), tag `pre-release`. `.blend` now goes through LFS; `.blend1` and `.fbm/` folders are ignored. Source audio stays untracked.
- R1: 47 sound effects and Fenella's 88 English lines imported by one script. One table maps each event to a clip, volume and pitch jitter; the lecture is three sounds (raise, inhale, impact), the wrap ledger ticks once per deduction and then coins trickle in. New Effects slider next to Music and Fenella.
- 59 tests pass; Backlot autoplay PASS; every line Fenella spoke in the run had a clip. Dylan has not heard it yet: the mix is a guess at "ridiculously over the top".
- R2: Dylan's theme song (Big Rock Candy Mountain, adjusted) now plays over the end credits and nowhere else; if the asset is ever missing, the old credits cue plays instead. Village music imported for R4. `Declared.md` has a line for the song; Dylan to add the tool that made it.

## R4: the film in the candy world (Nov 13)

- Open mode now plays the film: title, out of the village, a compass to the first set, the scene where the terrain worker left a flat floor for it, and on to the next. The bot walked all three sets on the first try (`PASS mode=open main=3`).
- A follow camera, mouse or right stick to turn it. The first scene framing put the camera inside a candy tree (they have no camera collision), so during a take it now looks along the path into the set, the one side that is always open.
- Dylan watched the screenshots: Fenella further out with a spotlight on the knight, the camera further back and lower, the whole world brighter and "kiddie", with bloom for things that will glow. The spotlight came in at 9000 lumens and blew the knight white; it is 250 now. The rest of the look is his to tune tomorrow.
- Ambient camps dropped, as the plan's first fallback allows.

## R4b: they hit back (Nov 13)

- Goblins, bugs and the very large goblin now charge the knight: a red wind-up, a run, and a hit that knocks him back and staggers him. Rats still never fight. A DIGNITY bar sits under the take number; when it empties the take fails with "No dignity left. Back to one!", and every retake gives it back.
- A lecture flusters a goblin out of its charge, which is the counter. The boss ignores that and keeps coming; at first nothing ever landed, because the bot's lectures cancelled every charge.
- 63 tests pass, both bots still finish (the Open run ended Scene 2 with ten Dignity left). Not yet played by a human; the numbers are a first guess.
- Itch deployment moves to ChatGPT (Dylan), to save Claude's quota for Nov 14; GAME stops at a zip on disk.
- Typed notes are live against the deployed backend: the bot typed "make it rain and sad" and got two model-written scenes and a generated premiere back. The first try silently fell back to keywords, because Unreal's ini cut the URL at `//`.
- The end screens now say THE DREAD LAND OF CANDY with a DIRECTOR'S CUT (or THEATRICAL RELEASE) stamp slapped on at 45 degrees, matching the title screen.

## R6: the cast (Nov 13)

- Four can play the hero now: the new knight (the default, replacing the old look), a mage, an archer and a barbarian. A `Casting` line on the title screen swaps them on the spot, left or right, and the new one shouts as he steps in; `-class=mage` does it from the command line.
- All four use the knight's skills. Only the knight has the sword raise, sigh and scrub; the others stand and walk with the sword in hand. Each shouts one of his English battle cries when he casts ("EXCUSE ME!", "A SPOONFUL OF SUGAR!", "DID YOU FLOSS?!", "EAT YOUR VEGETABLES!"), no more than one every few seconds.
- One import script brings in the four rigs and eleven cries and reapplies the knight's wrist fix. 66 tests pass; the mage finishes the Backlot film and the barbarian finishes the walk through the candy world. Dylan has not seen them in motion yet.
- R5: packaged (5 min 45 s), smoke PASS in Backlot (`time=117.9`) and Open on `L_Sugarborough` with the archer (`main=3`, `time=154.8`), no missing clips. `smoke-packaged.ps1` got `-Map` (a map has to be the first argument). Zip 689 MB. Gate waits on Dylan's play.

## Cast animations, weapons and Niagara (Nov 13 follow-up)

Dylan requested all four existing skills animated on every cast member, the
Niagara library connected and tested, and missing class weapons supplied.
All four now have SwordRaise, FormALine, Sigh and LastJudgment, plus Scrub for
held interactions. The new gestures use two shared Meshy text-to-motion tasks;
each clip is separately bound to its character's existing rig. No models were
regenerated. Wrist-only corrections were validated on all four raises.

The mage carries a textured staff, the archer a textured bow, and the barbarian
a clean wooden rolling pin with iron collars; the knight keeps the sword.
The generated rolling pin had extra handles and was rejected in favour of an
original Blender-built prop. Source FBXs, Blender scene and bound-material
sources are in `ImportSource/CastWeapons/`; generators and importers are in
`Scripts/`. Staff and bow base-colour textures are explicitly assigned rather
than relying on automatic FBX material import.

`AJamAttackParticles` connects twelve class presets (including every secondary
layer) to the four existing skills. These are cosmetic skins: gameplay hit areas,
damage, unlocks and cooldowns stay in the existing abilities, and the floor
shapes still show their real areas. Sigh uses the shared impact layer.
Cast-owned effects stop when shooting stops, a take fails or the cast changes;
Budget Out updates existing and new layers. Components are explicitly shut
down and destroyed, with a 3.6-second lifetime cap and fixed bounds. `/Game/FX`
is included in cooking. The other shared systems remain reusable library assets.

Validation: 20 clips on their matching skeletons; four wrist corrections;
33 live particle systems with BudgetOut, Stop visibility and owner cleanup;
132 nonempty compiled VM programs; fresh twelve-preset captures; editor and
game builds; Jam automation; full Backlot autoplay for every cast member with
all four skill effects logged; packaged Backlot and Open smoke. Evidence is in
`Saved/CastDelivery/`, `ImportSource/RigCorrections/cast_skills_validation.json`,
`ImportSource/FX/live_parameter_validation.json`, and `ImportSource/FX/review.html`.
The effects retain the existing prototype art (plain comic cards, simple foam);
visual captures and runtime validation do not constitute Dylan's art approval.
The shield arc was invisible in its original .35-second capture; its lifetime
is now .8 seconds, it uses a separate stronger material, and a fresh capture
visibly confirms both the arc and impact particles.
The package is refreshed; the earlier release zip is unchanged and must be
rebuilt before an upload. No upload or push was performed.

### Dylan's review: character motion pass still required

Dylan reviewed the result and found the shared knight gestures inappropriate,
especially the barbarian carrying a rolling pin while performing knight moves.
The validations above establish functionality, not an acceptable final cast.
Keep the four existing skill mechanics, but replace Mage/Archer/Barbarian skill
gestures with class-appropriate casting, archery and bellow/stomp motions and
verify the corresponding weapon grips. This is an unfinished follow-up owned
by the cast session; terrain/R7 fixes have the next engine slot.

Weapons must be stowed on the back while travelling outside combat, with per-weapon offsets/clearance; draw for attacks and return to the back afterward. Dylan reported the current hand-carried weapons sticking into the ground. This is part of the unfinished cast follow-up.


### Nov 13: Molar Hills and the first R7 fixes

The Molar Hills road now climbs nine metres to a level fort. Existing plants and
scenery follow the raised triangulated surface; total instance count is unchanged.
The village and reserved gameplay pads retain gentle slopes. Native road collision
and an idempotent second import pass are recorded in ImportSource/Terrain/hill_validation.json.

Movement is 600 uu/s and the walk clip runs at 1.5x. The quest marker pulses and
fresh stage directions hold centrally for one second before shrinking into the HUD.
Mouse labels now follow Mouse 1 / Mouse 2; the reported activation failure remains
unreproduced. Dylan clarified three skills per class through the coordinated cast
session: basic/signature start rank two and the ultimate stays rank one. The HUD
now supports data-driven class icons/colors and clockwise grayscale-to-color
cooldowns. Initial art is temporary pending the cast session's twelve-icon import.

Both targets and all 66 tests pass; three screenshots were inspected. The Windows
Development package was refreshed and Backlot/Open packaged autoplay both pass.
The old ZIP is unchanged. Remaining wishlist items and the cast handoff are in STATUS.

## Class presentation revision: three slots, equipment and brushing

Dylan chose three active slots per class for the jam. This revision supplies the
twelve selected class gestures and twelve new transparent hand-painted icons.
The three shared hit patterns remain the jam's compact combat implementation;
each class now supplies its own names, clips, effects, artwork and palette.
The archer draws a bow in the left hand; the mage casts with the free hand;
the barbarian kneads, bellows and spins instead of performing the knight's raise.
New clips play through recovery within the existing 1.6-second cosmetic window.

Class primaries: Knight #4778D6, Mage #8D5BDB, Archer #3C9D68, Barbarian #C65736.
Damage accents: Manners gold, Sweetness pink, Hygiene mint, Nutrition lime,
Domestic cream. Enum-indexed icon/animation data use slots 0,1,unused,3 and feed
the R7 three-slot HUD and radial grayscale-to-colour cooldown material.
Artwork and exact built-in image_gen prompts: `Assets/Incoming/2D/ClassSkills/manifest.json`.
All twelve imported textures are under `/Game/Art/UI/Skills` (already cooked by `/Game/Art`).

Weapons are on the back during travel, and whenever shooting is not Rolling
(including slate, retake, CUT, wrap and title). During an attack they move to the
appropriate hand; each weapon has its own back offset and orientation, with
world scale preserved across skeletal attachments. The four sampled walk poses
have at least 37 cm of weapon clearance above the flat review floor.

Goblin brushing visibly uses the existing Rodin toothbrush at an oversized
130-240 cm length, while the combat weapon stays on the back. Brush and white/mint
foam follow the visible mesh's mouth bounds, including sitting/squash changes.
Two reused M03 Niagara layers create compact mouth foam and bubbles, with a
1.2-second linger and explicit cast-owned cleanup. The held interaction and
successful mouse/bot interaction path both trigger the presentation.

Validation: editor and game builds; 66 automation tests; all four classes finish
the film with all three skills and brush/foam events; twelve skeleton/grip checks
and twelve texture checks; seventeen clear in-game review captures (12 attacks,
4 back carries, 1 brushing); and the observed retake carry edge was corrected
and rechecked. Reports/captures: `ImportSource/RigCorrections/ClassSkills/validation.json`
and `ImportSource/_gen/class_presentation_review/`.
Development-only `-classreview -notitle -scene=2` captures the poses on a clear
review floor; ordinary gameplay does not enter that setup.

The previous shared-knight-gesture review is superseded by this revision.
Dylan's final visual approval remains pending. Mouse/HUD fixes have the next
engine slot and will do the final combined package/smoke; this revision has not
replaced the archived package or release zip. No push or upload was performed.


### Nov 14 — GAME batch 0
Validated overnight clickable HUD, character-sheet rank selection, grounded calm enemy poses, and first-quest note prompt; retained the existing Open default.
Both targets build, all 69 Jam tests pass; refreshed Development package passes no-flag Open autoplay. Mouse smoke: first world click, HUD click and first post-resume click each cast once.
Helpers initially assumed Backlot/default Template_Default; mouse timed out in Open travel, and packaged Template_Default failed because it is no longer cooked. Reran on the shipped map. Physical swapped-button mapping/refocus remains a manual check; see Saved/R7Mouse/current_mouse_result.json.
Inspected Saved/Screenshots/WindowsEditor/r7_current_mouse.png and packaged captures; travel HUD and compass crowding remain batch 1.
Rebuilt tester ZIP; no upload/push. Dylan now requests no further Backlot tests; Open is the release gate. Archived previous STATUS to preserve full wishlist; next: batch 1.


## 2026-11-14 09:44 - GAME, batch 1 blockers
Moved the compass label clear of its strip and restored the travel skill/XP HUD, including mouse hit paths.
Traced the grey-screen report to BudgetOut's global material replacement/desaturation; Open keeps authored materials and colour while extras still become boxes. Turned the archer bow about its long axis in the left hand.
Editor build, 69 Jam tests and Open autoplay pass (three scenes, 107.1 s); inspected Saved/Screenshots/WindowsEditor/batch1-budget.png and the travel/bow captures. Development game build also passes.
Dylan deferred packaging to finish all existing batches first: the current ZIP remains batch 0. No push/upload. GAME continues into batch 2.


## 2026-11-14 10:00 - GAME, batch 2 and release paperwork
Revised camera/skill/use controls and added a read-only keyboard/mouse/gamepad Controls card to Options and Pause. Open now earns numeric XP directly: disable 2, treat 4, collect 2 and once-only quest 40. Both authored note routes total 190 XP (1.6 levels); no quest-giver expansion.
Fixed a missing MakeText argument caught by the first build. Editor and Development game targets and all 72 Jam tests pass; Open autoplay passes three scenes in 113.7 s, with quest XP totals 64/116/190. Inspected Saved/Screenshots/WindowsEditor/batch2-controls.png, batch2-pause.png and batch2-xp.png.
Reconciled ElevenLabs Fenella/SFX declarations and tools; credited ACE Studio music and Dylan's theme-song vocals from his confirmation. Preserved unrelated action-card work.
No package, ZIP refresh, push or upload, per Dylan's instruction to finish batches first. No new tester feedback batch exists yet; hands-on controls/typed-note/cast checks and the final package remain. Engine released; STATUS is ready for a fresh thread.

Final handoff check found parallel BACKEND commit 46b75af: n2 deployed/pre-warmed, live checks reported passing. Recorded that update and the Anthropic Claude Haiku service credit; game HTTPS remains a hands-on check. No backend access or deployment was performed by GAME.


## 2026-11-14 14:45 - Claude (Opus), Fixes1 checkpoint
Dylan: quest givers are a core marquee feature, not a cut. Committed the afternoon's Fixes1 work (164e0af): Fenella recruits the hero under a golden marker, quest-site markers, five villagers with ten linked jobs, Quest Journal, backend-improvised branches with authored fallback, comic-style buttons. Evidence reused, not rerun: 79 Jam tests, village autoplay PASS main=3 side=10 offline / localhost / all-503. Backend quest-branches endpoint committed in backend/ (ab328d7, 117 tests); not deployed.
Also committed comic action-card source art + declaration, devlog checkpoint revisions, itch install text. Rewrote STATUS and the plan's Nov 14 afternoon section: ChatGPT GAME packages next (nothing since batch 0 is packaged); Dylan migrates the server with a Sonnet BACKEND session (handoff at top of Docs/Workers/backend.md).

## Nov 14 - P1 package

Packaged the Fixes1 world build for Windows Development. Packaged smoke passed the three main quests and all ten village quests; added a village autoplay assertion to the smoke script. 79 Jam tests passed (EXIT CODE 0). Submission-format zip: 688 MB. No upload.

## Nov 14 - Reality television introduction and routine delegation

Fenella now introduces Heroes of Dread: The Land of Candy automatically on
arrival: she assembled the villagers, cast the hero, and demands massive
problems and drama. Five ordered captions have longer reading times; talking
to her skips to recruitment. New voice recordings remain pending. Authored
text recorded in Jokes.md. Added recruitment intro/skip assertions.

Editor build succeeded; 79 Jam tests passed with EXIT CODE 0; offline autoplay
passed three main scenes and ten village jobs. Luna ran verification and
inspected village-arrival.png: readable caption, no clipping. Arrival capture
required elevated execution after a sandbox DDC write failure. No packaging
or upload. AGENTS.md now requests Luna subagents for routine checks, concise
briefs without inherited history, and coordinated exclusive Unreal access,
per Dylan's token-cost preference.


## Nov 14 - Fenella voice pickup session

Authored 20 additive Fenella reality-show, village-direction and reaction
candidates. Luna generated all 20 plus the five exact existing RealityIntro
speeches with the established ElevenLabs Bella voice/model/settings. Separate
resumable runner, 25 MP3 originals and 25 mono 48 kHz PCM16 WAVs, scoped
manifest and technical review. Listening remains pending; no Unreal imports,
runtime pool changes, engine tests or packaging in this audio-only task.
Source audio remains untracked, consistent with project policy.

## Nov 14 — Fixes2 clean-session checkpoint

CODEX took over all eight fixes after Claude quota ended; Dylan explicitly
released HOLD and approved the Meshy shield. Imported 25 Fenella recordings,
attached the shield to the knight's left hand, built candy markers and enlarged
skill icons. Proposal backend and HUD hooks/async guards are committed. Editor
build succeeds; 82 Jam tests and 149 fake-model backend tests pass. Inspected
captures show the HUD/shield working, but screen-owned status captions overlap
bottom menu options. Fixes2 remains open for that UI correction and village
offline/proposal/failure verification. No package/deploy/upload. Dylan requested
a clean-session handoff: `Docs/Workers/codex-fixes2.md`; engine FREE.
# Nov 14 — Fixes2 village verification

Fixed unattended village autoplay repeatedly selecting an encore before visiting
the other givers. Editor build and 82 Jam tests passed; offline, proposal and
proposal-fallback village runs completed all ten jobs. Inspected the proposal
screen capture. Manual interaction and audio checks remain in the worker handoff.
# Nov 14 - Fixes3 gameplay checkpoint

- Restored menu-to-game focus, level-up return and mouse aiming; made Fenella
  available during town errands and corrected weapon carry during village casts.
- Imported 154 translated battle cries; the first use of each skill per player
  is English, subsequent eligible cries select a random translated clip.
- Editor build, 82 Jam tests and offline village autoplay passed. Captures
  inspected; hands-on controls, voice listening and cooked smoke remain.

