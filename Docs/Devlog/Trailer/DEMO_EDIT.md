# Demo video: edit plan (Nov 15)

About 90 s. Gameplay is the picture; Dylan's devlog lines and Fenella's recorded lines carry it, with text cards between.
Dylan appears as a keyed picture-in-picture (bottom-right, the `key_still.py` matte from `../Video/`) over gameplay,
and full-frame CAM only for the opener and the closer. Music: generated score from `music/` only, ducked under every
voice. Never `theme-song.mp3`.

**Dylan (Nov 15): keep him talking very little.** Use only D1 (open), D4 and D8 (close), about 12 s in all. The rest
below are alternates. An ElevenLabs trailer announcer (`vocals/generate_announcer.py`, lines in its `LINES`) narrates
dead straight, Fenella undercuts him, and text cards fill the rest. Auditions: `vocals/output/announcer/`.

## Devlog lines picked (source: `Docs/Devlog/Video/raw/original/2026-11-15 <take>.mp4`)

| # | Take | In-out (s) | Line | Picture |
| --- | --- | --- | --- | --- |
| D1 | 12-16-09 | 0.6-4.2 | "Build me an MMO in four days, please." | CAM, full frame (cold open) |
| D2 | 12-17-35 | 3.3-11.6 | "I asked Claude for something huge, ChatGPT specs it out, Claude cuts it, makes it doable, and then I'm like, More!" | PIP over the title screen, then a slow flythrough |
| D3 | 12-40-31 | 31.0-44.2 | "Hey look, we've got Skyrim in Candyland with this Elden Ring guy ... brushing the teeth of rats with a giant toothbrush and feeding them celery." | PIP over the hero on a ridge, then rats and celery (trim the stammer mid-line on the word timings) |
| D4 | 13-09-43 | 0.2-4.5 | "Nothing dies. So you lecture, you feed things celery, you brush their teeth." | PIP over knight lecture / celery feed |
| D5 | 13-09-07 | 10.4-14.4 | "Every attack throws comic word cards into the world in several languages" | B-ROLL: comic cards burst, no PIP |
| D6 | 13-10-07 | 0.4-3.1 | "The quests are rewritten while you play." | PIP over a villager quest offer (4 routes) |
| D7 | 13-22-27 | 12.4-17.6 | "...a hot chocolate river. I've never heard of a hot chocolate river." | B-ROLL: river, marshmallows, bridge (flythrough) |
| D8 | 13-29-20 | 32.5-35.9 | "Four days. Four days. One dude. What are you going to do?" | CAM, full frame (closer, before the title) |

Alternates: 13-24-31 9.0-13.9 "Elden Ring with a Sweet Shop" (for D3); 12-18-42 0.8-10.4 "...an MMO in a few days with AI. A lot of AI. All of the AI." (after D1).

## Beat sheet

| Time | Picture | Sound |
| --- | --- | --- |
| 0:00 | D1 CAM | no music |
| 0:04 | Title screen, PIP D2 | `title-theme_take1` horn in |
| 0:13 | Flythrough: Molar Hills, village | music; card *The Dread Land of Candy* |
| 0:20 | Hero on ridge, rats, celery; PIP D3 | music ducked |
| 0:34 | Fenella flies in | `VO_SceneStart_03` |
| 0:38 | Knight lecture, pacified rat; PIP D4 | `boss-battle_take1` kicks in |
| 0:43 | Class montage: archer volley, mage cards, barbarian cyclone; D5 over the cards | music |
| 0:55 | Village quest offer; PIP D6 | `VO_VIL_04` after |
| 1:02 | River flythrough, D7 | music |
| 1:09 | Big fight, CUT! card | `VO_CutPitch_04` |
| 1:14 | Budget-out boxes | music stops, `VO_BudgetOut_02` |
| 1:20 | D8 CAM | silence |
| 1:24 | Title card, *Made for the 2026 Epic AI MegaJam* | `victory_take1` tail |

## Footage still needed

- Gameplay from the packaged build (`capture.ps1`): waiting for Dylan to free the engine. Planned runs: knight, archer,
  mage, barbarian `-autoplay -class=X`; `-autoplay -autovillage`; `-budgetout`; title idle; one `-forcefail` for CUT!.
- Flythrough: Dylan's, in progress. Location TBD.
