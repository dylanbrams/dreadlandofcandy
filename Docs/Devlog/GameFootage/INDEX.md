# GameFootage index (Nov 15, 30 fps re-capture)

Shared by the devlog (`Docs/Devlog/Video/`) and the demo (`Docs/Devlog/Trailer/`). Don't move or re-encode files here;
write derived clips to your own `build/`. The folder is git-ignored; this index is committed.

**Capture:** packaged Win64 Development build of 18:29 (river bridge un-smooshed, foliage fades instead of the camera
lifting), recorded with `Docs/Devlog/Trailer/capture.ps1` (ddagrab of the game window only, GPU encode). 1600x900,
30 fps CFR, no audio, music off in game. Autoplay bot runs: high third-person camera, full HUD, Fenella subtitles.
`old-8fps/` holds the first capture set (choppy, ~8 real fps); `BROLL_INDEX.md` timings refer to those.

Times are from 5 s contact sheets and can be 1-2 s early: scrub the edges. **Avoid** = camera inside foliage,
blurred, or the bot walking into something (Dylan: never show the bot ramming walls or trees).

**Discord pop-ups (privacy).** Desktop notifications (sender and channel names) were drawn over the window's
bottom-right corner during parts of most clips. `toasts.json` here lists the detected ranges per file (seconds,
sampled every 0.5 s). Never use a frame inside a range ±2 s without covering that corner;
`Docs/Devlog/Trailer/build_demo.py` (`mask_toast`) blurs the bottom-right 11.5% x 9.5% of the frame around each
range, and the same treatment is fine for the devlog. Turn on Do Not Disturb before any further capture.

## Scene runs (`-autoplay -class=X`, 80 s each)

All four follow the same route: Scene 1 "A Balanced Diet" (rats) on the yellow road, walk to Fort Toothache in the
Molar Hills, Scene 2 (goblin), walk toward the Almond Affair.

| File | Time | What | Note |
| --- | --- | --- | --- |
| knight-run | 0:00-0:08 | SCENE 1 - TAKE 1 / TAKE 2 slates | |
| | 0:10-0:20 | Lecturing rats: HARRUMPH!, MANNERS!, rat pacified, Pacify 1/3 to 3/3 | best knight combat |
| | 0:20-0:24 | Feed each rat celery, green celery burst | |
| | 0:25 | Blurred pink foliage | avoid |
| | 0:30-0:40 | Walk the cobbled path into the Molar Hills | plain |
| | 0:45 | CUT! "Needs a stronger antagonist. I'm thinking: bigger goblin." | good CUT! |
| | 0:50-0:55 | Brush the very large goblin, toothbrush light | |
| | 0:60-0:70 | Road to the Almond Affair, candy village roofs, red canopy | 0:65 white flare, avoid |
| | 0:75-0:80 | Scene 3 start: lollipop bugs, a storm of word cards (TSK TSK!, MANNERS!, BONK!, "is that CELERY?") | great cards |
| archer-run | 0:05 | PERFECT! AGAIN. card | |
| | 0:10-0:20 | Celery arrows, TH-WACK!, rats pacified | |
| | 0:25-0:40 | Walk into the Molar Hills | 0:30 foliage, avoid |
| | 0:45 | CUT! bigger goblin note | |
| | 0:50-0:55 | Goblin, CELERY! card, green celery splash | |
| | 0:60-0:75 | Candy pines, village, yellow road to the Almond Affair | 0:80 white flare, avoid |
| mage-run | 0:00-0:10 | Slates, Pacify the rats | |
| | 0:15 | Feed each rat celery (prompt "Click / Hold [F] Feed celery") | |
| | 0:20 | Blurred foliage | avoid |
| | 0:25-0:35 | Cobbled Molar Hills | 0:35 grey wall clip, avoid |
| | 0:40 | CUT! bigger goblin | |
| | 0:45-0:50 | Brush the goblin, FIZZ!, MINTY!, Chinese card | |
| | 0:55-0:70 | Candy pines, village roofs, yellow road | 0:70 red canopy fills frame, avoid |
| | 0:75-0:80 | Gather the almonds: peppermint field | |
| barbarian-run | 0:10 | Rolling-pin hit, WHUMP!, starburst | |
| | 0:15 | CUT! "The rats have no motivation. Options, darling." | |
| | 0:20 | Blurred foliage | avoid |
| | 0:25-0:30 | Cobbled path, barbarian with rolling pin | |
| | 0:35 | Grey wall clip | avoid |
| | 0:40 | ACTION! card, "is that CELERY?" | |
| | 0:45-0:55 | Brush the goblin, DOUGH!, flour cloud | 0:50 motion blur |
| | 0:60-0:75 | Candy pine, red tree, yellow road | 0:60 inside foliage, avoid |
| | 0:80 | SCENE 3 - TAKE 1 slate | |

## Director's notes, budget, wrap

| File | Time | What | Note |
| --- | --- | --- | --- |
| typed-note (45 s, knight, typed note "Add more rats and make them sad", backend on) | 0:00-0:15 | Slates, lecture with CLANG! | |
| | 0:20 | CUT! card with the typed note quoted, "She is reading it..." | the typed-note beat |
| | 0:25-0:35 | Scene retitled "A BALANCED DIET, MOURNFULLY": "Feed the five sad rats some celery" 0/3 to 3/3, Chinese card | the note changing the scene |
| | 0:40 | Walk on, foggy | |
| budget-out (40 s, mage, scene 3, `-budgetout`) | 0:00 | Lollipop bugs; a grey placeholder box among the peppermints | boxes small, scrub 0:00-0:05 |
| | 0:05 | CUT! "It's good. Make it more. Just... more." | |
| | 0:10-0:20 | Shoo the many, many bugs 3/8 to 7/8; placeholder boxes on the road | **budget boxes**, high camera |
| | 0:25-0:35 | Premiere card (FILM GROSS / DIRECTOR / HERO / RATS paid in celery), credits | wrap and premiere |
| title-idle (14 s) | all | Black Sugar title screen, menu idle | |

## Village (after the film: `-scene=3 -autoplay -autovillage`, 150 s, backend on)

| File | Time | What | Note |
| --- | --- | --- | --- |
| village-improvise (archer, `-autobranches`) | 0:00-0:20 | Scene 3 bugs, wrap | |
| | 0:25-0:30 | Back to Sugarborough, villagers, "Our film is finished!" | village arrival |
| | 0:35-0:45 | Mayor Bonbon job "A Quiet Constituency", rats fed celery, BRO?!! card | quest giver |
| | 0:50-0:60 | Quest ready, the well, villager houses; Mayor Bonbon talk prompt | |
| | 0:60-0:75 | Goblin job "A More Representative Goblin" | 0:65-0:75 foliage, avoid |
| | 0:80-0:95 | Lecturing goblins, Turkish card "TEMİZ DİŞLER!", return to the square | |
| | 0:100-0:115 | Baker Crumb job "Contains Actual Nuts", rats pacified | quest giver 2 |
| | 2:00 | **Quest offer screen "The Cake Has Teeth"**: original route, other route, Crowd variant, "Fenella is thinking...", Propose a plan | improvise / offer beat |
| | 2:05-2:25 | Playing the chosen route; turn-in, "Baker Crumb has saved you a seat" | |
| village-third (knight, `-autobranch=2`) | 0:25-0:35 | Village arrival, square, Mayor Bonbon | |
| | 0:35-0:45 | **Crowd** route: "Councillors pacified 0/8" at the Riverbank | Crowd swarm |
| | 0:50-0:65 | Riverbank rats, red tree, candy cane marker | 0:50 foliage, avoid |
| | 0:70-0:85 | Turn-in, village houses | |
| | 1:45-1:55 | **Visit** route: "Collect the ceremonial toothbrush from Dr. Molar", toothbrush handed over | Visit step |
| | 2:00-2:30 | Molar Hills cobbles, goblins | mostly foliage, avoid |
| village-propose (knight, `-autopropose="Bribe the rats with snacks so they go home"`) | 0:00-0:20 | Scene 3, CUT! "make it more", shoo bugs | |
| | 0:25-0:35 | Village arrival | |
| | 0:40 | **Offer screen "A Quiet Constituency"**: original, other, "Crowd: the full council", "Fenella is reading your plan..." | proposal beat; pop-up 0:38-0:45, mask |
| | 0:45-0:55 | Mayor Bonbon job at Meadow West, rats | |
| | 0:55 | Offer screen "A More Representative Goblin" with "Fenella is reading your plan..." | |
| | 1:00-1:15 | Goblins brushed (WHEEP! card), village square | 1:05 foliage, avoid |
| | 1:20-1:35 | Baker Crumb, "Contains Actual Nuts", almonds | pop-up 1:20-1:26 |
| | 2:00-2:10 | "The Cake Has Teeth": **GENTLE** step ("one hit and the step starts over") | motion blur |
| | 2:15-2:30 | Turn-in | blurred |

The barbarian attempt of this run got stuck against a house for 2 minutes; it is in `old-8fps/village-propose-stuck.mp4`.
The proposal's server reply ("Fenella is reading your plan...") is visible; whether it turned into a playable route
or a refusal is not clear from the sheets: scrub 0:40-0:60.

## Second batch (`capture-extra.ps1`)

| File | Time | What | Note |
| --- | --- | --- | --- |
| controls-screen (10 s) | all | Controls screen "Know your marks", full key list | |
| village-aerial (15 s) | all | High angled view over the village: cupcake houses, yellow road, square; Fenella "I've assembled these villagers..." | hero small and idle |
| hero-close (60 s, knight) | 0:00-0:03 | Knight walking the yellow road from behind | good hero shot |
| | 0:04-0:08 | PERFECT! AGAIN., SCENE 1 - TAKE 2 | |
| | 0:09-0:15 | Lecture: Pacify the rats, TSK TSK!, BONK!, light flashes | pop-up 0:10-0:15 |
| | 0:19-0:24 | Feed each rat celery, giant celery | |
| | 0:25-0:41 | Walk into the hills | foliage and pop-ups, avoid |
| | 0:43-0:47 | SCENE 2 - TAKE 1, CUT! | |
| | 0:50-0:60 | Brush the goblin, SAYGI! and BONK! cards | |
| tour-low (150 s) | | Tour camera at walking height, no hero, no HUD. 0:10-0:20 yellow road; 0:40-0:45 **river with marshmallows**; 1:30-1:45 village; 1:50-2:00 mint pines and castle gate | 0:05, 0:35, 1:05, 1:20, 2:00, 2:15 inside foliage, avoid; many pop-ups (see toasts.json) |
| tour-high (120 s) | | Tour camera high, no HUD: **best world footage**. 0:00-0:20 canopy and yellow road; 0:25-0:35 **chocolate river from above with marshmallows**; 0:40 **the fixed bridge**; 0:45-0:65 red orchard, hills; 1:15-1:30 forest to village; 1:35-1:45 **village aerial**; 1:50-2:00 mint pines, castle gate | pop-ups 1:34-1:38, 1:45-1:51 |

## Dylan's own recordings

- `LandOfCandyFlythrough.mp4` (4:23, 1280x720, 30 fps): the demo's picks are in `Docs/Devlog/Trailer/DEMO_EDIT.md`.
  River with marshmallows at 0:58-1:01 (pre-fix bridge).
- `DylanPlaythrough.mp4` (4:49, 720p, with audio): see `BROLL_INDEX.md` (typed note 3:20, clock deadline 3:30).
