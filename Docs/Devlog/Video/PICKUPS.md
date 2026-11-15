# Pickups to record (green screen) and B-roll to capture

Record each pickup as its own short take; say its ID in the first second (or note it) so it can be labeled.
Speak naturally, don't worry about ums: they are cut. Pickup text is a suggestion; use your own words where you'd say it better.
`NARRATIVE.md` section 4 has the "get to the point" retake scripts (A-F). `edit.json` places every pickup, so a new take slots straight in.

## A. Gaps (record these first)

| ID | Where | Frame | Say |
| --- | --- | --- | --- |
| 02-encores | Hook | B-roll | And the villagers keep coming back. Two to four encore jobs each, with twists: a time limit, a crowd, rats that fight back. Then a goodbye. |
| 02-filmshoot | Hook | B-roll | Quests are scenes. Fenella yells CUT! and pitches rewrites. You can type director's notes mid-scene, and the scene changes. And when the budget runs out, the rats are replaced by cardboard boxes. |
| 02-usabletake | Hook | CAM | You're not saving the day. You're trying to get a usable take. |
| 03-declared | Setup | B-roll | Some setup and design work existed before the jam, and it's declared. |
| 03b-warning | Gantt | CAM | A warning before anyone reads it like a leaderboard. |
| 06-playtest | Playtest (optional) | B-roll | This is where a list of finished tasks starts being a game. A green test can't tell me whether an attack feels funny, or whether walking across the river is any fun. |
| 08-three | Sunday | CAM | That's the point where a sensible person stops adding things. Instead, three things happened at once. |
| 08-pbr | Sunday | B-roll | ChatGPT generated fifteen candy PBR materials, and I picked mine from a review page. |
| 08-marshmallows | Sunday | CAM | Thirteen cylinders had been standing in for marshmallows. Now there are three generated ones, drifting downstream. Nobody needed this. |
| 08-blacksugar | Sunday | B-roll | Four UI directions were mocked up. I picked Black Sugar: pewter frames, peppermint enamel, proper serif fonts. I cut the full review short to save usage. The collaborators have a budget too. |
| 08b-fixed | River | B-roll | So: Controls is called Controls now. The flashes are about five times dimmer. The river got carved into a real channel with banks, the bridge stopped being flat, and fourteen hundred and thirty-two things growing in the middle of roads were removed. |
| 09-lookatwhat | Why | B-roll | Look at what's in here. |
| 09-green | Why | CAM | Every test was green when I wrote "the river is terrible." |
| 10-ending | Ending | B-roll | That's the Dread Land of Candy. It's on itch.io, link below. It isn't an MMO. But you can lecture a rat, and your hero shouts in a hundred and fifty-four translations, and I'd call that a start. (Re-record once the final build is submitted.) |
| 11-theme | Post-credits | CAM | And that's why the theme song was cut. |

Optional: cold-open beats 6-10 (each faster): "Make it an open world." "Four classes." "Uncut them." "A hot chocolate river. With marshmallows."

## B. Where you diverged from the script (re-record tighter, ~20-40 s each)

Scripts for each are in `NARRATIVE.md` section 4:
A. Skyrim-in-Candyland dream (section 5) · B. Sunday river/marshmallows/UI run-on (section 8) · C. game-jam history (section 9) · D. QA and other people (section 9) · E. Gantt walkthrough and the $20 plans (3b) · F. Star Wars Galaxies to engineering degree (section 1).

## C. Check while you record (what you said vs the records)

- "enemies sort of have health" (13-09-43): the game says they have Resolve, not health.
- "15 languages" (13-20-55) and "150 battle cries" (13-30-15): the records say 154.
- "four static arenas" (12-40-31): the release has three main scenes.
- Rodin used for "the rat and the NPCs" (13-16-35): the script only claims the Rodin rat.
- The river and bridge arrived Saturday night, not Sunday (13-22-27).
- 13-35-40: the song's problem is the 1928 guitar, not an "AI backing track". Don't say the latter.
- Section 11 uses only the 8 s blooper clip; no other song audio.

## D. Gameplay B-roll to capture (none recorded yet; wait for the graphics fixes)

Packaged build, game window only. Backend live for the AI features (check `readyz`). Game audio up for these.

| Needed for | Shot |
| --- | --- |
| 1 | Slow pan across the Molar Hills |
| 2 | Signature attack with word cards tumbling out; same skill again with a non-English battle cry |
| 2 | Knight placards on a group; Resolve bar emptying; a pacified rat |
| 2 | Offer screen, "Ask Fenella to improvise", the two new routes, playing one |
| 2 | Typing a proposal ("bribe them with snacks so they go home"), the route appearing; a refusal for something silly |
| 2 | A giver's encore offer; a Rush timer; a Crowd swarm |
| 2 | Fenella's CUT! pitch; a typed director's note changing a scene; budget-out cardboard boxes |
| 4 | First hero on screen; autoplay bot run (early captures) |
| 4b / 5 | Rat in game, candy foliage; world and village pans; quest givers; compass |
| 6 | Compass, HUD, controls screen |
| 7 | Attack effects in motion (peppermints, celery arrows, flour cloud, card storm); a village encore |
| 8 | Village from above; marshmallows drifting under the bridge; old title screen then the Black Sugar title |
| 8b | River before/after (same crossing); camera lifting over a canopy |
| 9 | River-before shot; one shot per draft-list item |
| 10 | Title screen; the itch page (once live) |

Stand-ins that already exist: `Docs/Devlog/Tomorrow/assets/` (morning before/after, rat comparison), `CandyPBR/contact.jpg`, `Marshmallows/Marshmallow_contact_sheet.jpg`, `Docs/Design/CandyUI/*.png`, `Docs/Devlog/Screenshots/`, and `D:\OBSVideo\2026-11-14 17-41-16.mp4` (9 min of gameplay with your webcam in the corner, older build; usable cropped as a placeholder if you say so).
