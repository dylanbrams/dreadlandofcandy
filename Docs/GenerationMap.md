# Generation map (2D and 3D)

Written 2026-11-11. **To run generation, hand an agent `Docs/GenerationRunbook.md`.** Every asset below is already a job with its prompt in `Assets/Generation/jobs.json`, and `Scripts/gen_batch.py` runs them: 3D in parallel through the Meshy and Rodin APIs, 2D through the agent's own image generation (Codex/ChatGPT). This page is the human-readable map. Condensed from `Docs/Design/ASSETS.md` (the full list, with fallbacks and triangle budgets) and `Docs/styles.json` (the locked style words). Music is done; voice is tabled. Nothing here has been generated yet.

**Priority:** `MUST` = the game does not ship without it (or its fallback). `SHOULD` = makes it good. `STRETCH` = only if ahead.

## The whole map at a glance

| Provider | What | Count | When |
| --- | --- | --- | --- |
| **Meshy** (3D) | The hero, every candy creature and villager, every candy prop and set piece, small particle meshes | 1 hero + 6 creatures + about 20 props + 3 particle meshes | Test assets Nov 12, batch Nov 13 |
| **Meshy rigging** | Hero only: rig, then idle / walk / sword-raise | 1 | Probe Nov 12, two-hour time-box Nov 13 |
| **Rodin** (3D) | The hero's two hand props (sword, toothbrush). Needed for the Hyper3D prize | 2 | Sword Nov 12, toothbrush Nov 13 |
| **Rodin, if picked** | Any mesh group Dylan moves off Meshy after the morning comparison (rat, house front and peppermint are made on both). 0.5 credits a model, so props on Rodin leave Meshy's 1,100 credits for the hero, rig and creatures | up to ~23 | From batch1 |
| **fal** (Hunyuan3D v3.1 Pro) | Same routing. Tested Nov 12 with a throwaway model: works, about $0.53 a model at 40k faces, so the $30 balance covers about 57 | up to ~57 | From batch1 |
| **Tripo** | Same routing, if its API balance is topped up (it is 0 now) | - | - |
| **ChatGPT images** (2D) | UI art: icons, stamps, cards, portraits, posters, paper textures | 11 sheets | Skill icons Nov 12, the rest Nov 13 |
| **Google Fonts** | Three fonts (pick, don't generate) | 3 | Any time |
| **Not you** (Claude in Blender / Unreal / scripts) | Bubble meshes, word atlas, ground, materials, all Niagara effects, cardboard flats made from renders | - | Claude does these |

## The two styles (paste these to ChatGPT)

The clash is the joke: the hero and his gear are grim dark fantasy, everything else is a bright kiddie candy world. Keep these words identical across every prompt of a type; vary only the subject.

| Type | Style words |
| --- | --- |
| **character** (hero) | *Dark fantasy hero, high poly, highly detailed,* [subject] *, grim and imposing, heavy worn dark metal armour, aged leather, desaturated palette, realistic proportions, strong readable silhouette, neutral A-pose, full body, no gore, no baked lighting* |
| **equipment** (sword, toothbrush) | *Dark fantasy, high poly, highly detailed,* [subject] *, worn dark metal, aged leather, desaturated palette, oversized and menacing, single object, no baked lighting* |
| **npc** (creatures, villager, Fenella) | *Cute low-poly cartoon character for a children's game,* [subject] *, bright saturated candy colours, chunky rounded shapes, big head, simple friendly face, flat hand-painted colours, matte, neutral A-pose, full body, no baked lighting* |
| **prop** (everything in the world) | *Cute low-poly cartoon game prop,* [subject] *, bright saturated candy colours, chunky rounded shapes, flat hand-painted colours, matte, single object, no baked lighting* |
| **particle** (tiny effect meshes) | *Cute low-poly cartoon game prop,* [subject] *, bright saturated candy colours, very simple chunky shape, flat colours, matte, single small object, no baked lighting* |

The scripts add these words themselves (`--type`), so `jobs.json` holds the subject only. To make something by hand in a web app, paste the full sentence.

Creatures and props get recoloured and resized in the game (pink, mint, gold, grey, purple; half size to triple size), so ask for **light base colours** on creatures.

---

## 1. Meshy (3D)

### Hero (character type, then rigging)

| # | Asset | Subject for the prompt | Pri |
| --- | --- | --- | --- |
| H1 | `SK_Hero` the knight | *a towering knight in scorched black plate armour with a tattered dark tabard, closed great helm with a narrow visor slit, broad pauldrons, gauntlets, no cape, empty hands* ("no cape" is a joke payoff; empty hands because the sword is separate) | MUST |
| H2 | Hero rig + animations | Meshy auto-rig on H1 (UE5 mannequin skeleton), then library presets **Idle (0), Casual_Walk (30), Proud_Strut (114), Cheer_with_One_Hand_Up (306)**. Then text-to-motion clips: **SwordRaise** (raise overhead and hold), **Scrub** (brushing), **Sigh**. The library has no real sword raise (102 Sword_Judgment is a jumping spin attack), so SwordRaise is generated from a description, with 306 as the backup | MUST (idle, walk) |

Target 30k to 50k triangles, 2K textures. If rigging fails: a rigged dark knight from Fab.

### Creatures and characters (npc type, static, no rig)

| # | Asset | Subject | Pri |
| --- | --- | --- | --- |
| N1 | `SM_Rat` | giant chunky cute candy-field rat standing on its hind legs | MUST |
| N2 | `SM_Goblin` | goblin with a huge grin and very visible yellow plaque-covered teeth | MUST |
| N3 | `SM_Villager` | round, sincere candy villager, plain clothes (one mesh is reused for every villager) | MUST |
| N4 | `SM_Fenella` | tiny faerie film producer: big head, small body, beret, holding a toy film camera up to her eye, no wings (wings are added in the engine) | MUST |
| N5 | `SM_LollyBug` | bug whose head is a swirl lollipop on a stick body, little wings | MUST (Scene 3) |
| N6 | `SM_Almond` | an almond with stubby legs and two dot eyes | MUST (Scene 3) |

Targets 1k to 6k triangles, 1K textures.

### Props and set pieces (prop type)

**Scene 1, the candy field**

| # | Asset | Subject | Pri |
| --- | --- | --- | --- |
| P1 | `SM_CandyCrop` | crop plant made of lollipops and gumdrops | MUST |
| P2 | `SM_CandyFence` | candy-cane fence segment, long and straight | MUST |
| P3 | `SM_Celery` | chunky celery stick with leaves | MUST |
| P4 | `SM_CandyTree` | round candy-floss tree | SHOULD |
| P5 | `SM_Boulder` | rounded candy boulder | SHOULD |
| P6 | `SM_Table` | small folding table | SHOULD |

**Scene 2, Fort Toothache**

| # | Asset | Subject | Pri |
| --- | --- | --- | --- |
| P7 | `SM_Molar` | giant tooth, a molar standing on its roots, used as a castle pillar | MUST |
| P8 | `SM_FortGate` | castle gate front shaped like a molar, flanked by tooth towers | SHOULD |

**The street between sets (it's a film backlot)**

| # | Asset | Subject | Pri |
| --- | --- | --- | --- |
| P9 | `SM_HouseFront` | gingerbread candy cottage, **front facade only**, flat back (it's a film set) | MUST |
| P10 | `SM_Signpost` | blank wooden sign on a post (text added in the engine) | MUST |
| P11 | `SM_CardboardBox` | plain cardboard box, open top | MUST |
| P12 | `SM_DirectorsChair` | director's folding chair, canvas seat and back | SHOULD |
| P13 | `SM_Clapperboard` | film clapperboard slate | SHOULD |
| P14 | `SM_LightStand` | film studio light on a tripod | SHOULD |
| P15 | Villager hats, 3 runs | straw farmer's hat; dentist's head mirror; shepherd's crook | SHOULD |
| P16 | `SM_LeafMegaphone` | megaphone made from a rolled-up leaf | STRETCH |

Targets 100 to 4k triangles, 512 textures.

### Particle meshes (particle type)

| # | Asset | Subject | Pri |
| --- | --- | --- | --- |
| X1 | `SM_Peppermint` | red-and-white swirl peppermint in a twisted wrapper | MUST |
| X2 | `SM_Stanchion` | queue barrier post with a round base | SHOULD |
| X3 | `SM_LollyShard` | broken lollipop shard | STRETCH |

---

## 2. Rodin (3D, equipment type)

Rodin is here for quality on the two things the camera sees most on the hero, and because at least one shipped prop must come from Rodin for the Hyper3D prize.

| # | Asset | Subject | Pri |
| --- | --- | --- | --- |
| R1 | `SM_Sword` The Unforgiving Blade of the Last Judgment | a comically oversized two-handed greatsword, dark and grim, long straight blade | MUST |
| R2 | `SM_Toothbrush` Toothbrush of Uncompromising Governance | a toothbrush built like a dark-fantasy weapon: forged iron handle, leather grip, menacing | SHOULD |

Targets 10k to 20k (sword), 5k to 10k (toothbrush), 2K textures.

---

## 3. ChatGPT images (2D sheets)

**Rules for every sheet** (tell ChatGPT these once):

- One sheet per row below, items on an even grid, one item per cell, same scale and angle.
- **Flat solid `#00FF00` background**, no gradient or shadow (Claude keys it out and slices the grid). Do not ask for transparency.
- **No text in the images** unless the row says so: the game draws text live.
- Put the same 2D style paragraph at the top of every prompt. After sheet 1 is approved, attach it as the style reference to every later sheet.
- Save each result to `Assets/Incoming/2D/NN_<sheet>/` (e.g. `Assets/Incoming/2D/01_skill_icons/01_skill_icons.png`). Claude slices them.

**The UI has two layers (decided Nov 11), and every 2D sheet belongs to one:**

- **Knight layer, dark and sharp.** This is how the hero sees himself: a grim RPG. Black iron, hard angular edges, blackletter feel, desaturated, with blood-red and dull-gold accents. It covers skill icons, loot icons, rarity frames, the title logo frame and the frame of the director's-notes card. The clash with the candy world is the joke. *2D style paragraph: "painted dark-fantasy game UI art, black iron and aged leather, sharp angular edges, desaturated, heavy shadows, accents of blood red and dull gold".*
- **Fenella layer, the film production.** This is a children's film made on no budget: paper, rubber-stamp ink, clapper black-and-white, slightly scrappy and handmade. It covers stamps, film cards, the ledger, the call sheet, portraits, the phone frame and posters. *2D style paragraph: "bright bold-outlined cartoon, thick dark outlines, candy colours, paper and cardboard textures, slightly scrappy handmade film-production props".*
- **Readability.** The world is bright and busy, so Fenella-layer pieces need a **thick dark outline** baked into the art. The game will also sit them on a dark scrim. Plain cream paper with no outline washes out on a pink meadow.
- **Where the layers meet:** the director's-notes card is Fenella's paper card held in a dark iron frame, because it's where her notes reach the knight.

Sheets 01 and 02 each lock one layer: approve them first, then attach **01 to every later knight sheet and 02 to every later Fenella sheet** as the style reference. Never attach a sheet from the other layer.

| # | Sheet | Cells | Layer | Pri |
| --- | --- | --- | --- | --- |
| 01 | **Skill icons** | 4 skills: Lecture (raised sword with a speech bubble), Form A Line! (queue posts and rope), Disappointed Sigh (grey sigh cloud), Last Judgment (sword in a beam of light). 3 passives, greyed: Brooding (hunched helm), Cape Maintenance (empty cape hook), Standing (planted boots). 256 px cells | knight | MUST (do first, locks the knight layer) |
| 02 | **Film cards** | clapper slate (blank, for ACTION!), jagged comic burst (for CUT!), rewind panel (for RETAKE!), crumpled call-sheet paper (for That's a Wrap!). Thick dark outlines. Text is added live | Fenella | MUST (do second, locks the Fenella layer) |
| 03 | **Rubber stamps** | DIRECTOR'S CUT, RESERVED FOR SEQUEL, PRODUCER'S PICK, EVIDENCE, REFORMED. Red ink, grungy, slightly rotated. **Text allowed here** | Fenella | MUST |
| 04 | **Loot icons** (4x4, 4 spare) | sword, pauldrons, greaves, helm, chest plate ("doomplate"), gauntlets, toothbrush, cloak, damp napkin, amulet, ring, celery stick. All painted as grim legendary loot, including the napkin and celery | knight | MUST (at least sword, toothbrush, celery) |
| 05 | **Fenella portraits** | head and shoulders: tiny faerie producer, beret, toy film camera. One neutral, one sulking. Plus one full-body glowing version as a billboard fallback, and one faerie wing (single, translucent). Thick dark outlines | Fenella | MUST |
| 06 | **Knight frames** | title logo frame (wide, black iron, sharp, room for the title text in the middle); director's-notes card frame (dark iron frame with an empty slot where a paper card sits); 5 rarity card borders, rising in menace: Common, Uncommon, Rare, Legendary, Director's Personal Collection | knight | MUST (title and notes frames), SHOULD (rarity) |
| 07 | **Paper** | director's-notes index card (fits the frame from 06); lined ledger paper with a coffee ring. Outlined edges | Fenella | MUST (notes card), SHOULD (ledger) |
| 08 | **Premiere posters** | movie marquee frame; poster "THE DIRECTOR'S CUT"; poster "THE THEATRICAL RELEASE" (posters may carry their title text) | Fenella, a bit epic | SHOULD |
| 09 | **Villager portraits** | the candy villager as Farmer (straw hat), Dental Marshal (head mirror), Almond Herder (crook) | Fenella | SHOULD |
| 10 | **Candy Rock phone frame** | a candy boulder used as a mobile phone, with a screen area left empty | Fenella | SHOULD |
| 11 | **Odds and ends** | wall clock face with no hands, plus separate hour and minute hands; a drawn-on villager moustache | Fenella | STRETCH |

## 4. Google Fonts (pick three, OFL licence)

1. Heavy comic all-caps (bubbles, cards), e.g. Bangers.
2. Blackletter or gothic serif (title, loot names), e.g. UnifrakturMaguntia or Pirata One.
3. Plain sans (captions, menus), e.g. Nunito.

---

## Not on your list (Claude makes these)

Comic speech-bubble meshes, the "?" mesh and the word atlas (Blender script); ground, all materials and the grey "budget ran out" look (Unreal); every Niagara effect; sprinkles, boom mic, plywood backs, budget-label boxes (engine shapes); the cardboard villager and hero cutouts (renders of the meshes, keyed by script); rating card, narration screen, HUD shapes (UMG).

## Suggested order

**Nov 12 (day 1), before leaving at noon:** start the five test assets, one per type, and the first 2D sheet. Each takes minutes, so they can finish while you're at the park.

1. H1 hero (Meshy). Then submit it to Meshy rigging the same day just to learn if it's accepted.
2. R1 sword (Rodin).
3. N1 rat (Meshy).
4. P9 house front (Meshy).
5. X1 peppermint (Meshy).
6. 2D sheets 01 (skill icons) and 02 (film cards) in ChatGPT, one per UI layer.

Approve each in the game camera before batching its type; that's the style lock.

**Nov 13 (day 2), the batch, in this order:** hero rig and animations (two-hour time-box) · Scene 1: P1, P2, P3, N3, N4 · Scene 2: N2, P7, R2 · Scene 3: N5, N6, P4 · street: P10, P11, P13, P12, P5 · X2 · 2D sheets 03 to 07.

**Nov 14 (day 3), if there's time:** P8, P14, P6, P15, sheets 08 to 11, and replacements from Fab or Synty for anything that failed.

Every generated or bought asset gets a line in `Docs/Declared.md` the same day.
