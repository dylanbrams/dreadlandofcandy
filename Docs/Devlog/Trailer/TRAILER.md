# The Dread Land of Candy — trailer plan

About 75 seconds. There's no narrator: Fenella's existing voice lines, text cards and gameplay carry it. The joke is the same as the score's: an utterly sincere epic trailer about lecturing rats and feeding them celery.

**Music: generated score only. The theme song is cut; never use `theme-song.mp3`.** All tracks are in `music/`.

- 0:00–0:18 `title-theme_take1.mp3`, from its opening (the rising horn motif).
- 0:18–0:55 `boss-battle_take1.mp3` (or `hard-combat_take1.mp3`), cut on a downbeat.
- 0:55–1:02 hard stop: silence under the button joke.
- 1:02–1:15 `victory_take1.mp3` tail under the title card.

Duck the music about 8 dB under each Fenella line. Pick the music cut points by ear first, then lay the shots onto the beats.

## Beat sheet

| Time | Picture | Sound | Card |
| --- | --- | --- | --- |
| 0:00 | Black. Slow fade up on the Molar Hills at dawn, the camera drifting | Horn motif | *In a land of unspeakable sweetness…* |
| 0:06 | Hero on the ridge, back to camera; cape, sword | Music swells | *…one hero…* |
| 0:10 | Hard cut: Fenella flies into frame | `VO_SceneStart_03`: "Remember: you're grim, you're haunted, you're holding celery." | |
| 0:15 | Close on the hero holding the giant celery | Music beat | *…holding celery.* |
| 0:18 | Rats pour across a field | Boss-battle track kicks in | |
| 0:21 | Knight shout attack, text placards flying | `VO_LectureCast_02`: "The sword is a metaphor! Keep it a metaphor!" | |
| 0:25 | Archer celery-arrow volley | music | |
| 0:27 | Mage card storm | music | |
| 0:29 | Barbarian baker's cyclone, flour cloud | music | |
| 0:31 | A rat sitting, pacified, chastened | `VO_Pacified_04`: "Look at the remorse. You can't direct that. I directed that." | |
| 0:35 | Wide: Sugarborough village from above | music | *An open world.* |
| 0:38 | Hot chocolate river, marshmallows drifting under the bridge | music | *Of chocolate.* |
| 0:41 | Villager handing over a job; quest panel | `VO_VIL_04`: "I walk from this house to that well. Forever. It's a living." | *Quests.* |
| 0:45 | Peppermint rain on a big fight, camera shake | music peak | |
| 0:48 | CUT! card slams on screen mid-swing | `VO_CutPitch_04`: "CUT! Test audiences wanted more rats. I asked two rats." | |
| 0:52 | Same fight, many more rats | music returns, biggest | |
| 0:55 | Budget-out: placeholder box "rats" | **music stops**. `VO_BudgetOut_02`: "Pretend the box is a rat. I believe in you. I have to." | |
| 1:02 | Title card: Black Sugar title screen | `victory_take1.mp3` tail | **THE DREAD LAND OF CANDY** |
| 1:07 | | `VO_Premiere_02`: "Four stars! Out of a number I'll confirm later." | *Made in 4 days for the 2026 Epic AI MegaJam* |
| 1:12 | | | *[itch.io link]* |

Swap any line freely: all of Fenella's recorded lines, with their text, are in `vocals/line_inventory.json`. The files are `vocals/output/fenella/<asset_name>.wav`. Villager lines are in `vocals/output/cast/vil/`. Check the jam card's wording ("4 days") against the jam rules before publishing.

## Recordings to capture

Record from the **packaged build** at 1920×1080, 60 fps, with OBS (game capture). Turn music **off** in the game options and keep SFX on; the trailer music goes on in the edit. Hide the HUD where possible for the scenic shots. Record each shot 2–3 times: at least 8 seconds each, with a second of lead-in and tail.

| # | Shot | How |
| --- | --- | --- |
| 1 | Molar Hills, slow drift, no hero visible | Walk to the hills; slow camera turn with Q/E |
| 2 | Hero on a ridge, back to camera (knight) | Knight; stand still on a hilltop, camera behind |
| 3 | Fenella flying into frame near the hero | Start a scene; her entrance |
| 4 | Giant celery close-up | Feed a rat or bug as a non-goblin class |
| 5 | Rats crossing a field | Start of a rat scene; low camera |
| 6 | Knight shout attack with placards | Knight; shout skill on a group |
| 7 | Archer celery arrow volley | Archer; signature skill (Shift) |
| 8 | Mage card storm | Mage; ultimate (R) |
| 9 | Barbarian baker cyclone + flour cloud | Barbarian; ultimate |
| 10 | A pacified rat, close | Lecture one rat; hold the camera on it |
| 11 | Village from above | Camera pulled up over the square (the follow camera lifts over buildings now) |
| 12 | Hot chocolate river, marshmallows, bridge | Stand on the bank; let marshmallows pass under the bridge |
| 13 | Villager quest hand-off | Talk to a village giver; quest panel up |
| 14 | Peppermint rain on a big fight, camera shake | Any class in a large fight |
| 15 | CUT! mid-action | Fail a take or trigger a director's note mid-fight |
| 16 | Big swarm after the CUT | Retake with more enemies, or the heaviest fight available |
| 17 | Budget-out placeholder boxes | Play until the budget runs out |
| 18 | Black Sugar title screen, untouched | Launch; leave the menu idle 10 s |
| 19 | Spare: walking the hero across the bridge | For transitions |
| 20 | Spare: the level-up fanfare | Level up during a village job |

Shortcuts: `-autoplay` plays unattended; stage it with `-class=knight|archer|mage|barbarian`, and use `-uiscreen=<screen>` to open one screen directly. `Scripts\screenshot.ps1 -At <s> -Name <n>` captures stills for the thumbnail and the itch page.

## Audio to pull

- Fenella: `VO_SceneStart_03`, `VO_LectureCast_02`, `VO_Pacified_04`, `VO_CutPitch_04`, `VO_BudgetOut_02`, `VO_Premiere_02` from `vocals/output/fenella/` (use the .wav).
- Villager: `VO_VIL_04` from `vocals/output/cast/vil/`.
- Music: `title-theme_take1`, `boss-battle_take1` (alternate: `hard-combat_take1`), `victory_take1` from `music/`.
- SFX come from the gameplay recordings; no separate pulls are needed.

## Deliverables

- `Trailer_75s_1080p.mp4` for itch.io and YouTube.
- A 15-second cut (shots 10, 15, 17 + title) for social media.
- A thumbnail: still from shot 9 or 12 with the title.
