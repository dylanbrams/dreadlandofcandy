# Assets

Written 2026-11-11 from `BUILD_SPEC.md` (sections 3, 3a, 3c, 6), `Jokes.md`, `NoteForm.md`, `Docs/GenerationRouting.md` and `Docs/styles.json`. This is the list the ASSETS worker generates from and the GAME worker builds from. Nothing here has been generated yet, and nothing here is evidence that anything works.

## How to read this

- **Tool.** `Meshy` and `Rodin` are generation runs through `Scripts/meshy.py` / `Scripts/rodin.py` with the `--type` from `styles.json`. `Engine` means **the GAME worker builds it in Unreal** (terrain, Niagara, materials, post-process, anything made of basic shapes): those rows are a build spec, not a prompt. `2D` means Codex image sheets (template in `GenerationRouting.md`) or a script. `Blender` means a scripted clean-up or a trivial shape made by script (`Docs/BlenderCleanup.md`), owned by ASSETS. `ACE` is ACE Studio (AUDIO worker, Dylan).
- **Rig.** `Rigged` = skeletal mesh with animations. `Static` = static mesh; any motion is procedural in C++ (bob, lean, squash, waddle) or material world-position offset. Only the hero is planned as rigged.
- **Tris.** Rough budget after clean-up. Hero side is high poly by design (dark fantasy); everything else is low poly (bright kiddie). Texture sizes: hero 2K, hero gear 2K, nuisances and NPCs 1K, props 512, particle meshes none or 256.
- **Pri.** `must` = the slice in section 3 or one of the five must-ship set pieces (1 to 5) needs it. `should` = theme checklist, set pieces 6 to 18 that are cheap, or typed-note knobs. `stretch` = only if day 3 is ahead.
- **Fallback.** The cheapest thing that still ships. Order of fallbacks follows section 6: generated, then a Fab/Synty pack Dylan picks, then a cardboard flat or debug shape that the no-budget fiction excuses.

## Reuse rules (read before generating anything)

One mesh with tint and scale beats two meshes. Every bright-side asset uses one master material with a `Tint` parameter, so the note form's `tint` knob (`none, pink, mint, gold, grey, purple`) and the `scale` knob (0.5 to 3.0) are free on every nuisance and prop.

| One asset | Is reused as |
| --- | --- |
| `SM_Rat` | every rat; the boss-sized rat (scale 2.5 to 3.0); Gary (one rat with a tint); rats on break; the late rat |
| `SM_Goblin` | every goblin; the "stronger antagonist" boss goblin (scale + `purple` tint) |
| `SM_Villager` | Farmer, Dental Marshal, Almond Herder (one attached hat/prop each, three tints); the T-posed villager in the deleted scene; the source render for the villager cardboard flats |
| `SM_HouseFront` | every house on the street (tint, scale, mirror); its back is the plywood material on the same mesh's back faces or one engine plane |
| `SM_CandyCrop` | Scene 1 field crops, Scene 3 orchard undergrowth, the crop meter's "eaten" state (scaled to 30% and tinted `grey`) |
| `SM_CandyTree` | Scene 3 almond trees, Scene 1 field border, street background |
| `SM_CandyFence` | blocking edge of Scenes 1 and 3 and the street |
| `SM_Molar` | Fort Toothache pillars, battlements, the fort gate's flanking towers, a sign-topper on the street |
| `SM_CardboardBox` | lost-property box, celery crate, RAT boxes in the budget beat, set-edge clutter |
| `SM_DirectorsChair` | the two-inch DIRECTOR chair and the full-size TALENT chair (scale) |
| `SM_Signpost` | every sign; text is a `TextRender` component, so no sign textures |
| `SM_Celery` | the treat for rats, the "Celery of Unreasonable Authority" loot (wrap card uses its 2D icon), celery chunk particles (scaled down) |
| `SM_Peppermint` | pacify burst, hit sparks, Last Judgment rain, coin-like reward burst on the wrap card |
| `SM_BubbleJagged` / `SM_BubbleRound` | every comic word bubble in Niagara; jagged for the default and budget pools, round for the "sadder" pool |
| `M_Candy_Master` | every bright-side mesh; one `Budget` switch from `MPC_Global` turns them all grey checker for set piece 3 |

## Hero

| Asset | Needed by | Tool | Rig | Tris | Pri | Fallback |
| --- | --- | --- | --- | --- | --- | --- |
| `SK_Hero` the knight, empty-handed, A-pose | everything; set piece 1 | Meshy (`--type character`), then Meshy rig + animate (2-hour time-box, day 2) | Rigged | 30k to 50k | must | Rigged dark knight from Fab; then a static knight mesh that glides and bobs plus one Fenella line about the animation budget |
| Hero animations: idle, walk, sword-raise (0.3 s, held) | movement, Lecture wind-up, set piece 1 | Meshy preset animations | Rigged | n/a | must (idle, walk), should (sword-raise) | Sword-raise faked by rotating the sword socket in code; walk faked by bob |
| `SM_Sword` The Unforgiving Blade of the Last Judgment, oversized | Lecture wind-up, Last Judgment, set piece 1, loot card | **Rodin** (`--type equipment`) | Static, attached to hand socket | 10k to 20k | must | Fab sword; then an engine cylinder-and-cube sword with `M_Hero_Master` |
| `SM_Toothbrush` Toothbrush of Uncompromising Governance, dark-fantasy, hero-held | Scene 2 brushing interact, loot card | **Rodin** (`--type equipment`) | Static, swapped onto the hand socket during the brush interact | 5k to 10k | should | Sword stays in hand and the brush interact is just the hold-E bar; a Fenella line covers it |
| `T_HeroCutout` the stunt double: one render of the hero on a key colour | set piece 11 (failed take) | Engine (high-res screenshot of `SK_Hero` via `screenshot.ps1` on a green plane), keyed by script | Static (plane with `M_Cardboard_Flat`) | 2 | should | A grey silhouette texture on the same plane |

Hero test prompt sentence (section 12 asks for one before generation): *"a towering knight in scorched black plate armour with a tattered dark tabard, closed great helm with a narrow visor slit, broad pauldrons, gauntlets, no cape, empty hands"*. "No cape" pays off the Cape Maintenance passive.

## Fenella (NPC, cosmetic, no skeleton)

Section 5: `AJamFaerie` interpolates, bobs and faces what she films. No collision, no skeleton.

| Asset | Needed by | Tool | Rig | Tris | Pri | Fallback |
| --- | --- | --- | --- | --- | --- | --- |
| `SM_Fenella` faerie producer: beret, toy camera held up, tiny body, big head | in frame always; captions | Meshy (`--type npc`) | Static; bob and turn in code | 3k to 5k | must | Section 12 default: glowing billboard sprite (2D) with beret and toy camera, on `M_Cardboard_Flat` with an emissive rim |
| `SM_FenellaWing` one wing plane, mirrored for the pair | flapping (and the ledger's "wing wear and tear") | Engine (plane + 2D wing texture, translucent) | Static; flap is a sine rotation in code | 2 each | should | No wings; she glows |
| `SM_LeafMegaphone` rolled-up leaf | CUT! moments, ledger joke | Meshy (`--type terrain_piece`) | Static, attached | 300 | stretch | None; the CUT! bubble carries it |

## Nuisances

One actor class, three skins. All static with procedural motion: waddle (roll oscillation), bob, squash on hit (Flustered), sit = squash Z to 0.7 and tip back 15 degrees (Pacified). They must read at note scale 0.5 to 3.0 and in all six tints, so keep base colours light and let `Tint` multiply.

| Asset | Needed by | Tool | Rig | Tris | Pri | Fallback |
| --- | --- | --- | --- | --- | --- | --- |
| `SM_Rat` giant candy-field rat, chunky, cute, on hind legs | Scene 1; Gary; boss rat; set pieces 2, 14, 15 | Meshy (`--type npc`) | Static | 3k to 5k | must | Synty-style animal from a pack; then a capsule with ears and a `RAT` label (which is also the budget joke) |
| `SM_Goblin` plaque-toothed goblin, big grin, very visible teeth | Scene 2; boss goblin for the note | Meshy (`--type npc`) | Static (stretch: Meshy rig for a walk; humanoid, so it qualifies) | 4k to 6k | must | Pack goblin; then `SM_Villager` tinted green with a `GOBLIN` label |
| `M_GoblinTeeth` dirty-to-clean parameter on the teeth material slot | Scene 2 brush interact (teeth go from yellow to sparkling) | Engine (scalar `Clean` on a material instance) | n/a | n/a | should | No change; the sparkle Niagara alone shows "brushed" |
| `SM_LollyBug` lollypop bug: lolly head on stick body, little wings | Scene 3 | Meshy (`--type npc`) | Static; wing jitter by WPO, erratic path in code | 2k to 4k | must (if Scene 3 ships) | Engine sphere on a cylinder with `M_Candy_Master` swirl; reads as a lolly anyway |
| `SM_Almond` almond with stubby legs and two dot eyes, one of the herd | Scene 3 collect stage ("one of them lies about being counted") | Meshy (`--type npc`) | Static; hop in code | 1k to 2k | must (if Scene 3 ships) | Engine ellipsoid with `M_Candy_Master` tint `gold` and eye decal |
| Budget-beat stand-ins: boxes labelled RAT / GOBLIN / BUG / ALMOND | set piece 3 | Engine (`SM_CardboardBox` + `TextRender` label) | Static | reuse | must | Engine cube, grey checker, label |

## NPCs (villagers)

Set piece 17 says every villager is the same flat. So: one generated villager, three dressings for the named ones, and cardboard flats of it for the crowd.

| Asset | Needed by | Tool | Rig | Tris | Pri | Fallback |
| --- | --- | --- | --- | --- | --- | --- |
| `SM_Villager` round, sincere candy villager, A-pose | Farmer, Dental Marshal, Almond Herder; deleted-scene T-pose; flat source | Meshy (`--type npc`) | Static; idle bob, talk wobble in code | 4k to 6k | must | Pack villager; then a cardboard flat (2D) |
| Hats/props: straw hat (Farmer), dentist's head mirror (Marshal), crook (Herder) | tells the three apart | Meshy (`--type terrain_piece`), one run each | Static, attached to a head socket made in Blender | 300 to 800 each | should | Tint alone tells them apart, plus a name caption |
| `T_VillagerFlat` + `T_VillagerFlat_Moustache` | the extras along the street; set piece 17 | 2D: render of `SM_Villager` keyed by script; moustache drawn on with Codex or by hand | Static (plane, `M_Cardboard_Flat`) | 2 each + stick | should | One flat, moustache skipped |
| `SM_CandyRock` the phone rock, held by the speaking villager | set piece 9; dialogue | Reuse `SM_Boulder` scaled to hand size | Static, attached | reuse | should | UI skin only (the rock is mostly a dialogue widget) |

## Scene 1: A Balanced Diet (the field)

Terrain and layout are built by GAME in the engine; this is the spec.

| Asset | Needed by | Tool | Rig | Tris | Pri | Fallback |
| --- | --- | --- | --- | --- | --- | --- |
| Field arena: flat ground about 3000 x 2400 uu (tune against the 800 uu Lecture and 500 uu rat flee radius), `M_Ground` tinted candy-meadow green-pink, three to five `AJamSetMarker` spawn points | the whole scene | Engine: flat plane or flat Landscape, no sculpting; blocking edges are `SM_CandyFence` with collision | n/a | n/a | must | Default floor with grid material |
| `SM_CandyCrop` lollipop-and-gumdrop crop plant, one mesh | crop rows (dressing), the crops the rats eat | Meshy (`--type terrain_piece`) | Static, instanced (ISM) | 500 to 1.5k | must | Engine cone + sphere in `M_Candy_Master` |
| `SM_CandyFence` candy-cane fence segment, 400 uu | arena edges in Scenes 1 and 3 and the street | Meshy (`--type terrain_piece`) | Static, instanced | 300 to 800 | must | Engine cubes striped by the material |
| `SM_CandyTree` round candy-floss tree | border, orchard, street | Meshy (`--type terrain_piece`) | Static, instanced | 1k to 2k | should | Engine sphere on cylinder |
| `SM_Celery` a chunky celery stick with leaves | feed interact; Treated rats hold it; craft services; loot | Meshy (`--type terrain_piece`) | Static | 300 to 800 | must | Engine cylinder, tint green |
| Craft services table: `SM_Table` at small scale, `SM_Celery` on it, sign CRAFT SERVICES (CELERY) | set piece 15 | Meshy table (`--type terrain_piece`) + reuse | Static | 300 to 600 | should | Engine cube table |
| `SM_Boulder` rounded candy boulder | field dressing, Candy Rock, Brooding cliff gag | Meshy (`--type terrain_piece`) | Static, instanced | 500 to 1k | should | Engine sphere scaled |

## Scene 2: Fort Toothache (one room)

| Asset | Needed by | Tool | Rig | Tris | Pri | Fallback |
| --- | --- | --- | --- | --- | --- | --- |
| Fort room: square interior about 2400 x 2400 uu, walls of `SM_Molar` pillars with flat plaque-stained wall panels between, open roof so the fixed camera sees in, one doorway from the street | the whole scene | Engine: engine cubes/planes for walls and floor, `M_Tooth` on walls, `M_Ground` tinted pale-pink tile for floor | n/a | n/a | must | Four grey walls with a FORT TOOTHACHE sign |
| `SM_Molar` giant molar, roots down | pillars, battlements, gate towers | Meshy (`--type terrain_piece`) | Static | 800 to 1.5k | must | Engine cylinder with a rounded top |
| `M_Tooth` enamel with a `Plaque` scalar (yellow-green blotches by noise) | fort walls, molars | Engine | n/a | n/a | should | Flat white tint |
| `SM_FortGate` molar-shaped castle gate front, seen from the street | street side of the fort, sign "FORT TOOTHACHE. Est. whenever. Brushed: never." | Meshy (`--type terrain_piece`) | Static | 2k to 4k | should | `SM_HouseFront` scaled up, flanked by two `SM_Molar` |
| Countdown dressing: a big wall clock flat ("we're losing the light") | timer fail state reads in the world, not only on the HUD | 2D flat on `M_Cardboard_Flat`, hands rotated in code | Static | 2 + 2 hands | stretch | HUD timer only |

## Scene 3: The Almond Affair (orchard, finale)

| Asset | Needed by | Tool | Rig | Tris | Pri | Fallback |
| --- | --- | --- | --- | --- | --- | --- |
| Orchard arena about 3600 x 3000 uu: rows of `SM_CandyTree` tinted almond-blossom pink, crop patches of `SM_CandyCrop` (10 count toward the meter), an almond pen in one corner | the whole scene | Engine: flat ground, ISM rows, `SM_CandyFence` edges; the pen is four fence segments | n/a | n/a | must (if Scene 3 ships) | Scene 1's field re-dressed and re-tinted |
| Almond pen sign "COUNT THEM" | collect target | Reuse `SM_Signpost` | Static | reuse | should | Objective text only |
| Budget-runs-out swap: every material to grey checker, labels on boxes, music to humming | set piece 3 | Engine: `MPC_Global.Budget` drives `M_Candy_Master` and `M_Hero_Master`; actors swap to labelled boxes | n/a | n/a | must | Override material on every primitive at runtime (slower to write, same look) |

## The street (between sets) and set dressing

About 15 seconds of walking per segment at hero speed 600 (so up to 9000 uu; shorten if it is empty). Signs, plywood backs and the deleted-scene door live here.

| Asset | Needed by | Tool | Rig | Tris | Pri | Fallback |
| --- | --- | --- | --- | --- | --- | --- |
| Street: straight dirt road, `M_Ground` tinted caramel, houses either side, gaps to walk behind them | joins the three sets | Engine | n/a | n/a | must | Plain floor corridor |
| `SM_HouseFront` gingerbread-candy cottage facade, one face only, flat back | street, set piece 6 | Meshy (`--type terrain_piece`) | Static | 2k to 4k | must | Synty house; then a cardboard flat of a house (fully excused by the joke) |
| Plywood back: struts and the stencil THIS SIDE BACK | set piece 6, "Location Scout" toast | Engine: `M_Plywood` on the back plane, struts are engine cubes, stencil is a `TextRender` | Static | ~100 | should | Back plane only, no struts |
| `SM_Signpost` wooden sign on a post | all signs in `Jokes.md` | Meshy (`--type terrain_piece`) | Static | 200 to 500 | must | Engine cube on a cylinder; the words are the joke, not the sign |
| `SM_DirectorsChair` | DIRECTOR (two inches tall) and TALENT, DO NOT SIT | Meshy (`--type terrain_piece`) | Static | 500 to 1k | should | Engine boxes |
| `SM_CardboardBox` | lost property (one gauntlet, one celery), celery crate, budget boxes | Meshy (`--type terrain_piece`) | Static | 100 to 300 | must (budget beat) | Engine cube, `M_Cardboard_Flat` |
| `SM_Clapperboard` | slate on ACTION!, Fenella's hand or a stand by each set | Meshy (`--type terrain_piece`) | Static; the top arm is a separate piece if the mesh allows, snapped in code | 300 to 800 | should | UI clapper card only (it is already on the HUD) |
| `SM_LightStand` film light on a tripod | set edges, sells the backlot | Meshy (`--type terrain_piece`) | Static | 500 to 1k | should | Engine cylinder + cone, with a real spotlight |
| Boom mic: pole and fuzzy mic | set piece 13 | Engine (cylinder + capsule with a fuzzy-tinted material) | Static, dips in code | ~200 | stretch | Skip the gag |
| Deleted scene room: untextured box room behind a door marked DELETED SCENE (ajar), `SM_Villager` in its reference pose, a floating cube labelled DRAGON (TODO) | set piece 7 | Engine (default grid material on everything, `TextRender` label) | Static | reuse | stretch | Door only, locked, with the sign |
| Gauntlet in the lost-property box | street gag | Reuse a gauntlet split off `SK_Hero` in Blender, or skip | Static | reuse | stretch | Only the celery in the box |

## Props for the hero's interactions (summary)

| Interaction | Prop | From |
| --- | --- | --- |
| Lecture (all ranks) | `SM_Sword` raised, bubbles fly | Rodin + Niagara |
| Feed rat (Scene 1) | `SM_Celery` appears in the rat's paws | Meshy |
| Brush goblin (Scene 2) | `SM_Toothbrush` in the hero's hand, foam on the goblin | **Rodin** + Niagara |
| Collect almond (Scene 3) | `SM_Almond` hops after the hero into the pen | Meshy |

**Rodin assets:** `SM_Sword` (must) and `SM_Toothbrush` (should). The sword is the one that must ship for the Hyper3D pool; it is on screen for the whole game.

## Niagara effects

Built by GAME in the engine. This is the spec, not a prompt. Several use **custom meshes** through the Niagara mesh renderer; those meshes are listed under "Particle meshes" below and come from Meshy, Blender or engine shapes. One colour ramp for all effects (taken from the `M_Candy_Master` palette), hard-edged sprites, one SubUV grid size (4x4 or 5x4) everywhere.

**The comic word bubbles.** The Lecture streams bubble meshes from the sword tip along the cone at the targets. Each bubble mesh uses `M_Bubble`, which picks a cell from `T_BubbleWords`, an atlas of the fixed words (the 13 default lecture lines, the 4 "sadder" lines, the 3 budget lines). Text the player or the model writes (typed-note bubbles) cannot be baked into an atlas, so: the two or three *readable* bubbles per Lecture are world-space widgets or `TextRender` components fed from the active bubble pool, and the Niagara stream behind them carries atlas words when the pool is authored and grawlix cells (`#@!*`, `!!`, `?!`) when it is a typed pool. Rendering arbitrary text into a render target for Niagara is stretch.

| Effect | Needed by | Custom mesh | Pri | What it is | Fallback |
| --- | --- | --- | --- | --- | --- |
| `NS_LectureStream` | every Lecture; set piece 1 | `SM_BubbleJagged`, `SM_BubbleRound` | must | 6 to 10 bubbles per cast, burst from the sword tip, fly along the cone in 0.4 s with a little spin and squash, pop at the cone edge or on a target; round bubbles and lower-case cells when the "sadder" pool is active | Sprite bubbles with the same atlas |
| `NS_LectureCone` | every Lecture | none (engine cone mesh or a ribbon fan) | must | 0.15 s additive flash in the cone shape, sized by the Lecture radius stat so a loot radius change is visible | Decal flash on the ground |
| `NS_PeppermintBurst` | Pacified ("REFORMED"), strong hits ("EXCELLENT ADVICE!") | `SM_Peppermint` | must | 8 to 15 spinning peppermints pop up and bounce once on the ground, then shrink | Sprite starburst |
| `NS_Flustered` | Flustered status (Form A Line!, hits) | `SM_QuestionMark` | should | 2 or 3 small question marks orbiting the head, plus a sweat-drop sprite | One sprite "?" above the head |
| `NS_FormALine` | skill 2 | `SM_Stanchion` | should | Queue-barrier posts pop up along the line with a velvet-rope ribbon between them, hold 0.5 s, sink | Ground decal line |
| `NS_DisappointedSigh` | skill 3 | none | should | A slow grey puff ring expanding from the hero, low and heavy, slightly blue | Expanding ring decal |
| `NS_LastJudgment` | skill 4 and the finale | `SM_Peppermint`, `SM_BubbleJagged` | should | Sky darkens (post-process + directional light lerp, driven by code), shaft of light on the raised sword, huge shockwave ring, then a rain of peppermints and one enormous bubble | Shockwave ring only |
| `NS_FaerieTrail` | Fenella always | none | should | Sparkle ribbon behind her, pink-gold | No trail |
| `NS_CelerySnack` | feed interact, rats on break | `SM_Celery` at 0.15 scale | should | Green chunks fly from the rat's mouth while it chews | Generic crumb sprites |
| `NS_BrushFoam` + `NS_ToothTing` | brush interact | none; the "ting" is a 4-point star sprite | should | Foam bubbles (translucent spheres) at the goblin's mouth during the hold, then one bright star "ting" on the teeth | Sparkle only |
| `NS_BugScatter` | bug dispersed | `SM_LollyShard` | should | Wrapper-bright shards and a puff as the bug zips off-set | Puff sprite |
| `NS_AlmondPickup` | almond collected / counted | none | should | Small gold sparkle and a "+1" sprite | none |
| `NS_CropEaten` | crop meter tick | `SM_Celery` scaled tiny, or engine cubes in candy tints | should | Crumb puff at the crop as it shrinks to its eaten state | none |
| `NS_ClapperDust` | ACTION!, CUT! | none | should | Chalk-dust puff where the slate snaps on screen | none |
| `NS_Retake` | RETAKE! "back to one" | none | should | Quick white-streak rewind sweep across the set as actors snap back | Screen flash only |
| `NS_LevelUp` | level 8, 9, 10 | `SM_Peppermint`, `SM_Sprinkle` | should | Spotlight cone on the hero, confetti of sprinkles and peppermints | Confetti sprites |
| `NS_SprinkleShower` | wrap card gross, coin trickle | `SM_Sprinkle` | should | Sprinkles (the money) pour in the UI scene or in front of the camera | UI-only number tick |
| `NS_Weather_Rain` | note `weather: rain` | none | should | GPU sprites in a box around the camera, light streaks | none (knob ignored, caption only) |
| `NS_Weather_SprinkleSnow` | note `weather: sprinkle_snow` | `SM_Sprinkle` | should | Slow tumbling sprinkles in a box around the camera | Rain system re-coloured |
| Fog | note `weather: fog` | n/a | should | Not Niagara: Exponential Height Fog density lerp | none |
| `NS_BudgetCut` | set piece 3, the moment materials go grey | none (engine cubes) | stretch | A puff of grey cubes from each actor as it becomes a box | Instant swap |
| `NS_LateRat` | set piece 14 | none | stretch | Dust trail behind the sprinting rat | none |

### Particle meshes (for the Niagara mesh renderer)

| Asset | Used by | Tool | Tris | Pri | Fallback |
| --- | --- | --- | --- | --- | --- |
| `SM_BubbleJagged` comic starburst bubble with a short tail, flat front face UV'd 0 to 1 for the word atlas, thin extrusion and an outline lip | `NS_LectureStream`, `NS_LastJudgment` | Blender script: extrude a 2D starburst outline (generators make lumpy bubbles) | 100 to 300 | must | Camera-facing sprite with the same atlas |
| `SM_BubbleRound` rounded speech bubble, same UV rule | sadder pool | Blender script | 100 to 300 | should | `SM_BubbleJagged` with a round-cell texture |
| `SM_Peppermint` red-and-white swirl disc in a twisted wrapper | bursts, level-up, Last Judgment | Meshy (`--type particle_mesh`) | 150 to 400 | must | Engine cylinder flattened with a swirl texture (2D) |
| `SM_Sprinkle` single rounded sprinkle | money, sprinkle-snow, level-up | Engine capsule, tinted per particle | ~50 | should | Sprite |
| `SM_QuestionMark` chunky 3D "?" | Flustered | Blender script (text extrude) | 100 to 200 | should | Sprite |
| `SM_Stanchion` queue post with a round base | Form A Line! | Meshy (`--type particle_mesh`) | 200 to 400 | should | Engine cylinder + disc |
| `SM_LollyShard` broken lollipop shard | bug scatter | Meshy (`--type particle_mesh`) or a Blender-fractured `SM_LollyBug` head | 50 to 150 | stretch | Engine cubes in candy tints |

## Materials

All built by GAME in the engine. Two masters, a handful of utilities, one global parameter collection.

| Asset | Needed by | Pri | Spec |
| --- | --- | --- | --- |
| `MPC_Global` | budget beat, weather, sulking | must | Scalars: `Budget` (0 normal, 1 grey checker), `Wetness`, `SulkDesaturate` |
| `M_Candy_Master` + one MI per asset | every bright-side mesh | must | Unlit-looking matte lit: base colour texture times `Tint` (six note tints as a lookup), no specular, optional vertex-colour mode for untextured props; `Budget` lerps to a world-aligned grey checker; `Wetness` darkens slightly |
| `M_Hero_Master` | hero, sword, toothbrush | must | Standard PBR for the generated textures, desaturation scalar, same `Budget` checker lerp (the hero goes grey too) |
| `M_Ground` | three sets and the street | must | World-aligned tiling noise in two tints per set; no Landscape layers needed |
| `M_Cardboard_Flat` | every flat: villagers, hero cutout, Fenella fallback, wall clock | must | Masked, texture parameter, darker cardboard edge band from a second UV or a slight bevel mesh; two-sided, back face shows plain cardboard |
| `M_Plywood` | house backs, struts | should | Tiling wood grain (2D or engine noise), `THIS SIDE BACK` via `TextRender`, not texture |
| `M_Tooth` | fort, molars | should | Enamel white with a `Plaque` noise blend |
| `M_Bubble` | bubble meshes and sprites | must | Unlit, black outline, white fill, atlas cell from a Niagara dynamic parameter; lower-case/round variant by switch |
| `M_ConeFlash` | Lecture cone | must | Additive unlit, edge fresnel, fades by particle age |
| `M_Label` | budget boxes, DRAGON (TODO) | must | Grey checker with a `TextRender` on top; nothing new if `M_Candy_Master`'s checker branch is reused |
| `PP_FilmGrade` | whole game | should | One colour grade (warm, slightly lifted blacks), light vignette and film grain; `SulkDesaturate` pulls saturation when Fenella sulks. Letterbox bars are UI, not post-process |
| `PP_LastJudgment` | skill 4 | stretch | Darken and cool the frame for 1.5 s |

## UI art (2D)

Layouts are WBPs (CLAUDE.md UI rule); this is the art that goes in them. Text stays live text in UMG wherever possible (lines change with typed notes), so most of this is frames and stamps, not lettering.

**Two UI layers (Dylan, Nov 11).** The *knight layer* is dark and sharp: black iron, hard angular edges, blackletter, desaturated with blood-red and dull-gold accents. It covers the skill bar and icons, XP bar, level-up card, loot cards and rarity frames, the title logo frame and the frame of the director's-notes card. The *Fenella layer* is film production: paper, stamp ink, clapper black-and-white, scrappy. It covers captions, the notes card's paper, ACTION!/CUT!/RETAKE!/Wrap cards, stamps, ledger, portraits, the Candy Rock and the Premiere. Fenella-layer art has thick dark outlines and sits on a dark scrim, so it reads over the bright world. The notes card is where they meet: her paper in his iron frame. Per-sheet layers are in `Docs/GenerationMap.md`.

### Handoff: 2D is generated by Dylan in Codex (ChatGPT images) from a prompt fileset

No session calls an image API. The ASSETS worker writes the prompts as files; Dylan pastes them into Codex and drops the results back in the repo.

- **Prompts:** one paste-ready text file per sheet in `Assets/Prompts/2D/`, named `NN_<sheet>.txt` in generation order (`01_skill_icons.txt`, `02_stamps.txt`, ...). Each file is complete on its own: the shared 2D style paragraph at the top (kept identical in every file), then the sheet prompt from the template in `GenerationRouting.md` (grid size, cell list, flat key colour), then one line saying which approved image to attach as the style reference. No file depends on reading another.
- **Index:** `Assets/Prompts/2D/README.md` lists each file, the assets it produces (names from this document), cell order, target cell size, and status (`todo`, `sent`, `approved`, `redo`).
- **Results:** Dylan saves what Codex returns to `Assets/Incoming/2D/<sheet>/` with the same `NN_<sheet>` name. The ASSETS worker then keys out the flat colour, slices the grid into named PNGs by the README's cell order, and records the sheet in `Docs/Declared.md`. GAME imports from there as with meshes.
- **Redo:** a rejected sheet gets its prompt edited in place and its status set to `redo`; the old image is kept beside it as `NN_<sheet>_v1.png` so the declaration stays accurate.
- **Also uses this lane:** flat textures for the cardboard flats when a mesh render will not do, Fenella's billboard fallback, wing texture, and any flipbook textures for Niagara.

| Asset | Needed by | Tool | Pri | Fallback |
| --- | --- | --- | --- | --- |
| Fonts: one heavy comic all-caps (bubbles, cards), one blackletter or gothic serif (title, loot names), one plain sans (captions, menus) | everything | Google Fonts (OFL), declared in `Docs/Declared.md` | must | Engine Roboto for all three |
| Title logo THE DREAD LAND OF CANDY + DIRECTOR'S CUT rubber stamp | title screen | 2D (logo as live text in the gothic font over a Codex grunge frame; the stamp as a Codex image) | must | Live text, stamp as rotated red text with a border |
| Stamps sheet: DIRECTOR'S CUT, RESERVED FOR SEQUEL, PRODUCER'S PICK, EVIDENCE, REFORMED | title, loot cards, pacify | 2D, one sheet | must (RESERVED), should (others) | Rotated red text in a border |
| Card art: clapper slate (ACTION!), jagged CUT! burst, RETAKE! panel, That's a Wrap! paper | theme checklist, set pieces 2 and 5 | 2D, one sheet | must | Plain coloured panels with live text |
| Ledger paper (lined, coffee ring) | wrap card | 2D | should | Plain cream panel |
| Loot icons, 12, dark-fantasy painted (sword, pauldrons, greaves, helm, doomplate, gauntlets, toothbrush, cloak, damp napkin, amulet, ring, celery) | wrap card loot | 2D, one 4x4 sheet (4 cells spare) | must (3 at least) | Renders of the actual meshes for sword, toothbrush, celery; name text alone for the rest |
| Rarity frames, 5 (Common to Director's Personal Collection) | loot cards | 2D or a UMG border with five tints | should | Tinted UMG border |
| Skill icons, 4 (Lecture, Form A Line!, Disappointed Sigh, Last Judgment) + 3 greyed passives (Brooding, Cape Maintenance, Standing) | HUD skill bar, skill panel, level-up card | 2D, one sheet | must (4 skills) | Letters on circles |
| Fenella portrait (head, beret, toy camera; one neutral, one sulking) | caption bubble, note prompt | 2D (or a render of `SM_Fenella`) | must | Her name in the caption |
| Villager portraits, 3 (or one, tinted) | Candy Rock dialogue | 2D or a render of `SM_Villager` | should | Name only |
| Candy Rock phone frame | dialogue widget skin, set piece 9 | 2D | should | Plain dialogue panel |
| Note prompt: director's-notes card, Take / Refuse / Give your own note buttons, 80-char text box | sections 3 and 3a | UMG styling, one 2D paper texture | must | Default UMG styling |
| HUD bits: REC dot, timecode, slate label, XP bar, crop meter icon, timer icon | theme checklist, HUD | UMG shapes + 2 small icons from the skill sheet | must | UMG shapes only |
| Premiere card: marquee poster frame, two ending posters (THE DIRECTOR'S CUT, THE THEATRICAL RELEASE) | ending | 2D | should | Live text on black |
| Rating card ("rated S for Sugar") | before title | UMG text | should | none needed |
| Narration background (black, solemn) | set piece 4 | UMG | must | n/a |
| Achievement toast frame | fake toasts | UMG | stretch | Caption |
| Loading tip panel | transitions | UMG | stretch | Caption |

## Audio

Owned by AUDIO (Dylan, ACE Studio). Files land in `Assets/Incoming/Audio/`. VO by convention at `/Game/Audio/VO/VO_<LineId>` (tabled).

| Asset | Needed by | Tool | Pri | Fallback |
| --- | --- | --- | --- | --- |
| `MUS_Village_Cheerful` loop | default mood, street, Scenes 1 to 3 | ACE | must | Silence plus SFX; or a CC0 loop, declared |
| `MUS_Village_Epic` loop (same theme, brass and drums) | note `mood: epic` | ACE (Add A Layer on the cheerful loop) | should | Cheerful loop |
| `MUS_Village_Sad` loop (same theme, slow, minor) | note `mood: sad`, sulking | ACE | should | Cheerful loop pitched down in a Sound Cue |
| `MUS_SwordSting` pompous dark-fantasy swell, cut dead on the first word | set piece 1, every Lecture wind-up (short version) and Last Judgment (long version) | ACE | must | One orchestral hit SFX |
| `MUS_FenellaHumming` the village theme hummed badly | set piece 3 | ACE (vocal synth) or Dylan humming into a mic | must | Music stops; silence is also funny |
| `MUS_Credits` song with vocals | credits | ACE | should | Cheerful loop |
| SFX: inhale, lecture whump, bubble pop, pacified squeak, clapper, CUT! horn, coin trickle, UI click | section 6's eight | ACE or a CC0 pack | must | jsfxr-style synthesised blips |
| SFX: rat squeak, goblin grumble, bug buzz, almond boing, celery crunch, brush scrub, tooth ting, stamp thud, record scratch (budget), rock ringtone | nuisances, interacts, set pieces 3, 9, 10 | ACE or CC0 pack | should | Reuse the eight with pitch variation |
| VO: Fenella, 30 to 40 clips, most-heard triggers first | captions | Tabled; paid speech tool later | stretch | Captions only (they ship regardless) |

## Triangle budget on screen

With the camera at about 1100 uu and 50 degrees, a frame shows one set: hero 50k, sword 20k, up to 8 nuisances at 6k (48k; note `count` caps at 8), 3 villagers or Fenella about 15k, instanced dressing about 100k, particles about 20k. Under 300k triangles, well inside 60 fps at 1080p without Nanite. Texture memory and the 1 GB zip cap matter more than triangles: keep texture sizes to the rule at the top and import only meshes the game uses.

## Generation order

### Day 1 (Nov 12): one test asset per type, approved in the game camera

Purpose: lock `styles.json` once per type and answer section 11 question 12 (does the Meshy rig route work). Run 1 to 4 in parallel lanes (A Meshy, B Rodin, D Codex); nothing goes to batch until Dylan approves the test asset for that type.

1. **Hero test** (`character`, Meshy): `SK_Hero` with the prompt sentence above. Start first; it is the riskiest and the rig check depends on it. Then submit it to Meshy rigging the same day, before day 2's time-box, just to learn whether it is accepted.
2. **Sword** (`equipment`, **Rodin**): `SM_Sword`. Also try sending the Rodin `.glb` to Meshy's rigging endpoint as a probe (`GenerationRouting.md` open question), using the sword only to see if the upload path works.
3. **Rat** (`npc`, Meshy): `SM_Rat`. Approve it at scale 1 and 3, tinted `pink` and `grey`.
4. **House front** (`terrain_piece`, Meshy): `SM_HouseFront`. Check the back face is flat enough for the plywood back.
5. **Peppermint** (`particle_mesh`, Meshy): `SM_Peppermint`. Small and fast; confirms the particle style.
6. **Skill icon sheet** (2D, Codex): ASSETS writes the whole `Assets/Prompts/2D/` fileset and README on day 1; Dylan runs `01_skill_icons.txt` first (the four skill icons plus the three passives). It locks the 2D style; attach it to every later sheet.
7. **Bubble meshes** (Blender script): `SM_BubbleJagged`, `SM_BubbleRound`, and the word atlas `T_BubbleWords` (script renders the fixed words in the chosen comic font). Not generated, so it can run any time; hand to GAME with the end-of-T08 import.

GAME, meanwhile, builds on debug shapes (section 6 order of work) and needs none of these on day 1.

### Day 2 (Nov 13): the batch

In this order, so the scenes that must ship get art first. Lanes in brackets; Meshy runs up to 10 at once.

1. **Hero rig and animate** (A), two-hour time-box: idle, walk, sword-raise. Decide the fallback (Fab knight, or static glide) at the end of the box, not later.
2. **Scene 1 set** (A): `SM_CandyCrop`, `SM_CandyFence`, `SM_Celery`, `SM_Villager`, `SM_Fenella`.
3. **Scene 2 set** (A): `SM_Goblin`, `SM_Molar`. (B): `SM_Toothbrush`.
4. **Scene 3 set** (A): `SM_LollyBug`, `SM_Almond`, `SM_CandyTree`.
5. **Street and theme props** (A): `SM_Signpost`, `SM_CardboardBox`, `SM_Clapperboard`, `SM_DirectorsChair`, `SM_Boulder`.
6. **Particle meshes** (A): `SM_Stanchion`; `SM_QuestionMark` (Blender).
7. **2D sheets** (D): stamps; card art (clapper, CUT!, RETAKE!, wrap paper); loot icons; Fenella portrait. After the first sheet, attach it as the style reference.
8. **Audio** (AUDIO, in parallel all day): cheerful loop, sword sting, the eight SFX, Fenella humming.

Day 3 (for reference): `SM_FortGate`, `SM_LightStand`, `SM_Table`, villager hats, villager and hero flats, the epic and sad loops, the credits song, the extra SFX, skill polish and anything that failed (replaced from Fab or Synty packs, chosen by Dylan). Every generated or bought asset gets a line in `Docs/Declared.md` the same session.
