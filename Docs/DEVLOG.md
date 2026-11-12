# DEVLOG

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
