# BUILD SPEC: The Dread Land of Candy, Director's Cut

Written 2026-11-11 (design review, before the jam). **This file governs.** Where it conflicts with the three briefs in this folder or with older text in `STATUS.md`, this file wins. Dylan can overrule anything here; a session cannot.

Do not read the briefs by default. The master brief is about 40k tokens, most of it describes an MMO we are not building, and it will pull you toward that scope. Open a brief only when a task below names a section. Jokes and all authored text come from `Jokes.md`.

## 1. What the review found

1. **The briefs describe a different game.** Four classes, six NPCs, a quest generator, stash, FastAPI, Postgres, listen servers. The theme appendix sits on top of that as a layer. A judge plays for five to ten minutes; none of the MMO machinery is visible in that time.
2. **The build order had nothing on screen.** State machine, ledger, overlays, faerie, voice: no hero, camera, input, target or map anywhere in the list. Day 1 could have ended with passing tests and no game.
3. **There was no ending.** An endless quest loop has no arc. "Level design and game progression" is one of seven judged criteria.
4. **Nothing could fail,** so RETAKE!, a core theme joke, would never be seen. No player health, no timer, targets that stand still.
5. **The currency bought nothing,** so the 40% / x1.25 / x1.5 tuning problem was a balance exercise with no stakes.
6. **CUT! was a menu comparing multipliers.** The choice has to change what the player sees and what Fenella does, or it is not a mechanic.
7. **Audio had no music plan,** and audio is a judged criterion.
8. **Art risk was unmanaged.** A rigged, animated AI-generated hero is the least proven step in the pipeline and had no fallback.

## 2. How it is judged (from the jam page, checked Nov 11)

Scored 1 to 5 on each: **Ambition and innovation, Theme, Fun factor, Gameplay, Visuals, Level design and game progression, Audio.** Each is worth the same, so an untouched category costs as much as a weak core loop.

Submission needs: Windows Development build zip `TeamName_GameName.zip` under 1 GB, a **30 to 60 second gameplay video link**, tools used, team details. Deadline Nov 15, 5:59 PM PST; late entries are disqualified.

Prizes worth a small effort: **Solo Driver** (solo developer), **Glass Box** (best dev log, needs a link: see `DEVLOG.md` in section 9), **Hyper3D pool** (at least one shipped asset made with Rodin via `Scripts/rodin.py`, listed under tools used).

## 3. The game we ship

A complete eight-minute comedy with a beginning and an end. Solo, offline, one map, one hero.

**Flow:** Title -> opening narration (interrupted) -> Scene 1 -> walk -> Scene 2 -> walk -> Scene 3 -> Premiere (ending card) -> Credits -> back to title.

**Verbs:** move, up to four skills (section 3c; Scene 1 starts with the Lecture alone), Interact (E), and answering the note (take it, refuse it, or type your own). Nothing else.

**Camera and controls (locked):** fixed three-quarter top-down camera on a spring arm, no camera control. Keyboard and mouse: WASD moves, hero faces the cursor, LMB / RMB / Q / R are skills 1 to 4, E interacts, Esc pauses. Gamepad: left stick moves, right stick aims (falls back to move direction), RT / RB / LB / Y are skills 1 to 4, A interacts, Start pauses. Top-down hides weak animation, keeps the cone readable and keeps Fenella in frame.

**The map:** Sugarborough as a film backlot. Three sets (a field, Fort Toothache's one room, an orchard) joined by a short village street. Each set is a flat arena with blocking edges, so nuisances steer without a navmesh. Walking between sets takes about 15 seconds and is where signs, the plywood house backs and the deleted-scene door live. One level, no streaming, no travel.

**Nuisances:** one actor class, three skins (rat, goblin, lollypop bug). States: Wander, Flee (from the hero inside a radius), Flustered (brief stop after a hit), Pacified (sits, awaits treatment), Treated (done). Parameters: resolve, speed, flee radius. They must move, or aiming the cone is not a game.

**Failure:** every stage with a failure condition uses one of two: a countdown ("we're losing the light") or a damage meter (crops eaten). Failure is a failed take: RETAKE!, take number goes up, stage resets. No death, no game over.

### The three scenes

| | Scene 1: A Balanced Diet | Scene 2: Fort Toothache | Scene 3: The Almond Affair (finale) |
| --- | --- | --- | --- |
| Teaches | Lecture, interact, RETAKE | The note (CUT!), failure by timer | Everything, then the budget runs out |
| Stages | Pacify 3 rats -> feed each celery | Pacify 3 goblins -> brush each | Disperse 5 bugs -> collect 4 almonds -> final stage |
| Scripted beat | Take 1 of stage 1 succeeds; Fenella says "Perfect. Again." and forces Take 2, where the rats flee. Once only. | CUT! before the brushing stage: "needs a stronger antagonist" | CUT! before the final stage; halfway through, "we're out of money": materials drop to grey checker, music becomes humming (see Jokes.md) |
| Fail | none (cannot fail) | countdown | crop/almond meter |

**The note (this is CUT!):** at one marked transition per scene (from Scene 2 on), Fenella yells CUT! and pitches a rewrite of the next stage. Two buttons: **Take the note** or **Refuse**. A note changes parameters only (count, resolve, speed, timer, one boss-sized target) plus text and the lecture bubble pool. Taking it makes the scene bigger and sillier, raises the gross, and pleases her. Refusing keeps the stage, lowers the gross, and she sulks for the rest of the scene (different line pool, camera turned away). Track `NotesTaken` and `NotesRefused`.

**The wrap card and ledger:** after each scene, a "That's a Wrap!" card: gross, then **itemized deductions** drawn from a table (Director's fee, Producer's fee "also me", Catering "there was no catering, that costs extra", Miscellaneous), then Hero's Cut. The joke is the line items, so there is no balance to tune. Rules that stay, as a pure tested function: the hero's cut never drops below the floor, deductions never exceed gross minus floor, the outcome commits once.

**Loot without an inventory:** the wrap card shows three items with full dark-fantasy names. Fenella keeps the best one ("reserved for the sequel", stamped across it). The hero's item auto-equips and changes one of three real numbers: Lecture radius, Lecture cooldown, move speed. The change must be visible (a bigger cone is). No inventory screen, no stash, no equipment slots.

**Money:** "sprinkles". It is the score. Nothing is bought.

**The ending:** the Premiere card shows the film's total gross, Fenella's total and the hero's. Ending title depends on notes: mostly taken = "THE DIRECTOR'S CUT" (her vision, her name in every credit), mostly refused = "THE THEATRICAL RELEASE" (she disowns it; credited as Alan Smithee's cousin). Then credits, which also carry the real tool and asset declarations.

**Kept as one-liners, not systems:** level 7 (a line and a HUD number), reputation (a "+7 Approval" toast), the Candy Rock (the skin of the one dialogue widget: the villager phones you while standing in front of you).

### Not building (do not start any of these without Dylan saying so)

(Dylan, Nov 12 evening: the open world, the follow camera and the quest generator are now being built as section 3e. This list still describes Backlot mode, and section 3e ends with its own list.)

Multiplayer features (sessions, host/join, replication code, RPCs): not built, but see "Multiplayer-ready rules" in section 5, which every task follows. Any backend use other than the typed-notes feature in section 3a (no accounts, no player persistence). Save games. Quest generator, seeds, stage weights. Classes other than the knight (in the shipped flow; section 3d allows them as expansion work behind `-class=`). Inventory, stash, equipment UI, vendors. Reputation bands. Minimap, overworld map. Composure, Energy, resistances, any status other than Flustered. GAS beyond what section 3c needs (no new attribute sets, GameplayCues or cost effects). Skill trees, respec, talents. Auto-cast. Level 1 restart. Procedural terrain.

(Dylan, Nov 11: no multiplayer feature work in the jam plan, but keep the preparation: no architectural choice that makes adding it miserable later.)

## 3a. The ambition feature: the player types the director's note

Dylan's pitch for "Ambition and innovation" (Nov 11): an RPG the player directs in their own words, which shifts per player and reacts to typed text. This is the feature that carries that score, and it is the theme taken literally: the player gives the Director's notes.

**In play:** at every CUT! prompt there is a third option beside Take and Refuse: **Give your own note**, a text box (80 characters). "Make the rats enormous and ashamed." "This scene is about taxes now." Fenella reads it ("Bold. Wrong. I love it."), and the next stage is reshot to match: the targets change size, number, speed and toughness, the weather and music mood change, and the knight's lecture bubbles, the nuisances' replies, the scene title and the loot name are rewritten to fit ("DECLARE YOUR MOLARS."). Gamepad players, and anyone offline, get three suggested notes as buttons instead of a text box.

**Per player:** every note is kept in a log on the PlayerState and sent with later requests, so direction carries forward (a film that became about taxes in Scene 2 is still about taxes in Scene 3). At the Premiere the log is sent once more and comes back as the film's title, a tagline and three review quotes. No two players get the same film.

**How it stays safe to ship** (the rule from the old briefs holds: a language model never decides rules, completion or rewards):

- The model fills in a fixed form and nothing else. Knobs, each with a hard range: count, resolve multiplier, speed multiplier, target scale, time limit, target kind (from the kinds that set can spawn), tint, weather (none, rain, sprinkle-snow, fog), music mood. Text, each with a length cap: scene title, Fenella's reaction, three to five lecture bubbles, two or three nuisance replies, objective line, loot name and flavour.
- `JamNoteClamp` (headless, tested) turns any response into a valid `FJamNoteDef` or rejects it: clamps every knob, truncates every string, drops anything not in the form, and checks the stage is finishable in that set. The game then treats it exactly like an authored note. Gross is computed by the client from the knobs, never by the model.
- Six-second timeout, covered by Fenella "reading" the note. On timeout, error, rejection or no network: a small local keyword matcher (bigger, more, faster, sad, rain, and so on) picks the closest authored note, and failing that she says she can't read the handwriting and uses her own. The game is complete and funny with the server unplugged; `-autoplay` and the smoke test run with it off (`-nonotes`).
- No keys in the client. The client knows one URL; the server holds the model key, rate-limits per session and per address, refuses abusive input and is told to keep everything in the game's tone. The title screen says in one line that typed notes are sent to a server.
- The server has to stay up for the whole judging period. That is Dylan's to guarantee, and the reason the offline path must be good.

**Backend (Dylan, Nov 11):** a **second Gunicorn service on its own port** on the existing AWS box, with a **second database on the existing Postgres server**. OneMore's word server is the pattern to copy (a typed player answer sent from Unreal to FastAPI, judged by a language model, returned as structured JSON: `/judge-definition`, and `WordServerClient` on the Unreal side) but its service and database are not touched. Two endpoints: `POST /v1/director/note` and `POST /v1/director/premiere`. Anything copied gets a `Docs/Declared.md` line. This lane does not compile Unreal, so it runs in parallel with the code session. The stream is live: that session never prints environment variables, keys, connection strings or config files.

**Model and spend (Dylan, Nov 11): Claude Haiku, and it must be hard to spend money.** Target: about 50 dollars covers a great deal of play.

- **The database answers first.** Cache key: prompt version, scene, stage, motif, and the note after normalising (lower case, trimmed, whitespace collapsed, punctuation stripped). An identical request returns the stored response and never reaches the model. The three suggested-note buttons per prompt are pre-warmed, so they are always hits.
- **Carry-forward without breaking the cache:** the model returns a `motif` of at most three words with each note ("taxes"). Later requests send the current motif, not the whole notes log, so the key stays small and repeats still hit.
- Note input hard-capped at 80 characters on the client and again on the server; output capped by `max_tokens` sized to the form.
- **A daily request budget in the database.** When the day's model calls reach the limit, the endpoint serves cache hits only and tells the client to fall back. Spend is then bounded by design, not by hope. Also set a spend limit on the API key itself.
- Per-session and per-address rate limits.
- The premiere call is one uncached request per finished run; it counts against the same budget and has the authored endings as its fallback.
- The B1 session loads the `claude-api` skill for the current Haiku model id, structured output and pricing, and sizes the daily budget from real prices, not from memory.

**Gate:** typed notes work in a packaged build against the live server by the end of day 2, or the text box ships hidden and the suggested-note buttons stay.

## 3c. Levels, stats and skills (Dylan, Nov 11: standard RPG progression, MOBA-simple)

The hero levels up and the player puts a point into a skill, the way a MOBA does it. One hero, the knight.

**Levels:** start at level 7, end at level 10. XP comes from pacifying, treating and wrapping; thresholds are set so the levels land on cards: level 8 on Scene 1's wrap, level 9 on Scene 2's wrap, level 10 at the CUT! before Scene 3's final stage. Each level gives **one skill point**, spent on the card where it is earned (blocking, gamepad-friendly, never mid-fight). An XP bar and the level sit on the HUD.

**The six points already spent:** the skill panel shows levels 1 to 6 pre-spent on passives that do nothing (Brooding III, Cape Maintenance II, Standing I; see `Jokes.md`). That is the level-seven joke as a UI element.

**Skills** (each is the existing area ability with a different shape from the AoE library, a rank table, and at most the one status):

| Slot | Skill | Shape | Does | Ranks |
| --- | --- | --- | --- | --- |
| 1 | The Lecture | cone | Resolve damage | 1 to 4: damage and radius up. Owned at start |
| 2 | Form A Line! | long narrow line | lower damage, Flusters (stops) targets | 1 to 3: longer stop. Unlocked with a point |
| 3 | The Disappointed Sigh | ring around the hero | small damage, slows fleeing targets | 1 to 3: bigger ring. Unlocked with a point |
| 4 | The Last Judgment | large ring, long cooldown | heavy damage; the full sword fakeout plays here at its grandest | Unlocks automatically at level 10, for the finale |

Three points across levels 8 to 10, so the player cannot have everything: that is the build choice. The Lecture alone must still be able to finish every stage.

**Stats** (four, all real, shown on a small character sheet on the wrap card): Lecture Power, Radius, Cooldown Recovery, Move Speed. Loot changes them; ranks change skills.

**How it is built:** `FJamProgression` is headless (xp, level, points, ranks, stat totals from base plus loot plus ranks) with tests, and lives on the PlayerState. Skill rank is the GAS ability level; each ability reads its numbers from a rank table. Flustered is one duration effect that grants a tag the nuisance already reacts to. Spending a point goes through a request function like every other input. The autoplay bot spends points by a fixed policy.

## 3b. Widening after the slice works

The slice in section 3 is the floor, built so that more content is data, not code. Cheap to add once Scenes 1 and 2 work, in this order of value:

- More typed-note knobs and text slots (each is a field, a clamp and a test).
- More scenes: a scene is a table, a set and lines. About half a day each with art. Candidates from the old briefs: Mind the Rows (marching children, interact and follow), The Condensed Treacle Errand (collect in a cave).
- More nuisance skins, authored notes, Fenella lines, loot names, street gags.

Expensive, and still not in the plan: new verbs, inventory screens, save games, multiplayer features. Each of those is a system, not content. More classes were on this list until Nov 12; they are now expansion work under section 3d.

## 3d. After the slice: the expandable game (Dylan, Nov 12)

The jam slice in section 3 ships first and stays protected. The longer goal is an open-world, multi-class game, and the slice is its first demo. From now on, new combat systems are built class-agnostic and data-driven, with the knight as their first consumer. This section overrules "Classes other than the knight" in section 3, for expansion work only.

**Order and gates**

- Slice tasks (section 7) always come before expansion tasks. Expansion drops first in the cut order (section 8).
- Expansion never breaks the slice: `JamAutoplay: PASS` and all `Jam.*` tests stay green after every expansion commit, and the daily package and smoke test still pass.
- Anything not finished by the end of day 3 (Nov 14) stays behind a dev flag and out of the shipped flow.

**The combat system** (new folder `Source/MegaJam/*/Combat/`, tests `Jam.Combat.*`; owned by the FX worker, `WORKERS.md`)

- **Attack definitions:** one C++ table row per attack: id, class, archetype, shape and pulse pattern, wind-up / active / recovery times, status, presentation preset, bubble pool, voice pool, animation clip. Timing comes from this row and timers, not montage notifies (the hero plays single-node clips; there are no montages).
- **Archetypes, in build order:** instant area (exists: `UJamGA_AoeAttack`), pulsed area (on the caster or on the ground), projectile, sustained cone. Each is headless-testable hit logic first. Cosmetic objects never decide hits.
- **Events:** each cast emits Start, Release, Contact, HitResolved, End or Cancel as compact data (cast id, attack id, seed, positions, hit list). No replication code and no GameplayCues yet; the event struct is shaped so it can become a cue's parameters later.
- **Presentation coordinator:** consumes those events and drives primitives (cone, projectile rain, orbiting props, foam, dome, tether, comic bubbles). Cosmetic only; it never feeds back into rules. Primitives are C++ (procedural or static meshes, HUD bubbles) unless GAME authors a Niagara asset in the editor; the primitive's interface hides which. Cancel must stop every loop, emitter and bubble a cast started.
- **First task:** the knight's four skills as table rows and through the coordinator, with no change in behaviour. Done when the autoplay PASS line, `Jam.Gas.Skills` and the T19 screenshots are unchanged.
- Things to remember from the slice: the budget-out switch (T21) must also grey anything a primitive spawns afterwards; `-noart` must still work; anything loaded by path goes in `DirectoriesToAlwaysCook`; distances are Unreal units, not metres.

**Classes**

A class is a set of attack rows, a hero mesh on the mannequin skeleton, and its clips. Mage, Archer and Barbarian are reached with `-class=<name>` only. No class select in the shipped flow unless Dylan says so.

**Source documents**

`The_Dread_Land_of_Candy_Attacks_and_Effects_Pipeline.md` is the roster and the contract; `The_Dread_Land_of_Candy_Attacks_and_VFX_Design.txt` is an inventory and mood reference (most of its entries are brainstorm, not commitments). Where they disagree the pipeline document wins; where either disagrees with this file, this file wins. Ignore their replication, owner/teammate tier and runtime-localization parts for now. Neither is evidence that anything exists. Shouted lines go into `Jokes.md` before they go into code.

**Still not building:** replication, sessions, inventory, backend beyond typed notes. (The quest generator was on this list until the evening of Nov 12; it is now section 3e.)

## 3e. The open world (Dylan, Nov 12 evening)

The slice was finished on day 1, and Dylan's verdict is that three arenas, a fixed camera and one quest are too small. Days 2 and 3 widen it into the game he pitched: a village in the middle, three candy biomes around it, a third-person camera, villagers who hand out generated quests, and the three film scenes as the main questline. This section overrules, for open-world work: "one map", the locked camera in section 3 and answer 6 in section 11, "Quest generator" and "Minimap" in "Not building", and rule 8 in section 9.

**Who builds what.** Claude quota is nearly spent for the week, so the work is split by kind, not by size:

- **GAME (Claude) builds the framework only**, in the order O1 to O9 (section 7), and stops. Framework means: the types, the game loop, the flags, the tests, and placeholder numbers that make the autoplay bot pass. GAME does not tune, dress or write content beyond what a test needs. RULES is off, so GAME also writes the headless parts under `Film/` and `World/` itself.
- **TERRAIN (ChatGPT)** makes the terrain models, the scatter palettes, the PCG graph and the map (`Docs/Design/TerrainTask.md` is its brief; the hand-over contract below wins where they differ).
- **FX (ChatGPT)** continues the combat and attack effects under section 3d.
- **TUNING (ChatGPT, Nov 13 to 15)** changes numbers and table rows on the tuning surface below, and nothing else.

**Protecting what works**

- Before O1, tag the last green slice commit `slice-day1`. That build is shippable as it stands.
- The world has two modes. `Backlot` is the slice, unchanged. `Open` is this section. `-open` selects Open, `-backlot` selects Backlot; until the gate below, no flag means Backlot.
- Both modes keep their autoplay line green after every commit: `JamAutoplay: PASS scenes=3 ...` in Backlot, `JamAutoplay: PASS mode=open main=3 side=<n> ...` in Open. The smoke test runs both.
- **Gate, end of Nov 14:** if the packaged Open build passes its smoke test and Dylan prefers it, the default flips to Open and Backlot stays behind `-backlot`. Otherwise the slice ships and Open stays behind `-open`.

**The world as data** (new folder `Source/MegaJam/*/World/`, headless, tests `Jam.World.*`)

- `EJamBiome`: `Village`, `Meadow` (the field), `Molars` (Fort Toothache's hills), `Orchard`. Display names come from `Jokes.md`.
- `FJamLocation`: id, kind (`Village`, `Giver`, `Camp`, `FilmSet`, `Landmark`), biome, position (x, y; z is found by tracing down), radius, yaw, and for camps the kinds it can spawn and its capacity.
- `FJamBiomeDef`: nuisance kinds, tint, music cue, default weather (none, rain, sprinkle-snow, fog: the typed-note weather, reused), grade offsets for `JamLook`.
- `JamWorld::Def()` returns the bounds, the biomes and the locations from one table, `Private/World/JamWorldTable.cpp`. Everything else asks `JamWorld` where things are; nothing reads `JamBacklot::Sets()` directly in Open mode. The three film sets are `FilmSet` locations with the same floor sizes as the backlot.
- Tests: every location is inside the bounds; the village and three film sets exist; each biome has at least two camps and one giver; camps do not overlap each other or the village; every camp's capacity is at least 3.
- Starting size (tune later): playable square 24,000 uu, village at the origin with a 2,500 uu radius, biomes as three wedges around it, film sets 5,000 to 7,000 uu out, camps 1,200 uu across.
- **Flat fallback:** on the template map, Open mode builds the world from the table alone: ground plane, biome-tinted floor patches, the existing houses, trees, crops, molars and fences placed by a seeded scatter. This is what GAME develops against and what ships if the terrain does not arrive.

**Hand-over contract with TERRAIN**

- TERRAIN owns `/Game/World/` (map `/Game/World/L_Sugarborough`, PCG graphs, palettes, terrain meshes), `ImportSource/Terrain/`, the rows of `JamWorldTable.cpp` (positions, radii, biomes; not the struct), and `Docs/Workers/terrain.md`. It may also add the PCG plugin to the `.uproject` and its folders to `DirectoriesToAlwaysCook`, saying so in its notes. It touches nothing else under `Source/`, `Config/` or `Content/`.
- The village centre is at the world origin. Terrain meshes block the `Pawn` and `WorldStatic` channels; scatter does not block pawns unless it is as big as a house. Camps and film sets sit on ground flat enough to walk (under 15 degrees) across their radius.
- Launching `MegaJam.exe /Game/World/L_Sugarborough -open` must work: when Open mode starts on any map other than the template, the game skips the flat fallback and only spawns gameplay actors at the table's locations.
- Scatter props 500 to 1,500 triangles with 1k textures, set pieces up to 8,000; the zip stays under 1 GB (379 MB before terrain).
- The PCG result must exist in a cooked build: baked into the map, or generated at runtime and proven by `smoke-packaged.ps1`. A world that only exists in the editor is not delivered.

**Camera and controls (Open mode)**

- Third-person follow camera on the existing spring arm: the arm uses the controller's rotation, collision test on, about 500 uu long and 15 degrees down to start. Mouse or right stick turns the camera; the cursor is hidden during play and returns for screens.
- Movement is relative to the camera's yaw. The hero turns to face where they walk, and turns to the camera's yaw when a skill is cast, so skills aim where the player looks. Skill and interact buttons do not change.
- `AJamHero` gets `EJamCameraMode { Fixed, Follow }`; Backlot uses `Fixed` exactly as now. `SetAimOverride` also turns the camera in `Follow`, so the bot films what it does.
- `-camyaw=<deg> -campitch=<deg> -camdist=<uu>` pin the camera for repeatable screenshots. Sensitivity and invert-Y go in `UJamSettings` and the pause menu.
- Fenella keeps her shoulder offset; she must stay in frame at the default distance.

**Quests** (headless, under `Film/`, tests `Jam.Film.Quest.*`)

A quest is a scene. Everything a quest needs to run already exists: `FJamShoot` plays any `FJamSceneDef`, with the slate, ACTION!, CUT!, RETAKE! and the wrap. Fenella films every quest, which keeps the theme.

- `FJamQuestTemplate`: id, allowed biomes, one to three stage patterns (verb, kinds, count range, resolve and speed ranges, chance and length of a countdown), whether it carries a note, title patterns, giver line ids, gross tier.
- `JamQuestGen::Make(seed, giver, camp, hero level, motif)` returns an `FJamSceneDef` and an `FJamQuestInfo` (title, the giver's pitch, tier). Pure and deterministic. Verbs stay Pacify, Interact and Collect; no new verbs.
- Tests: the same inputs give the same quest; every quest is finishable in its camp (count within capacity, kinds the camp spawns); a giver never offers the same template twice running; a thousand seeds use every template; difficulty rises with level inside fixed bounds.
- `FJamQuestLog` (on the PlayerState): per quest `Offered -> Accepted -> Shooting -> Done`. One active quest at a time. The main questline is three fixed quests wrapping `JamScenes::Scene1()` to `Scene3()` at the film sets, gated by level (7, 8, 9).
- Text comes from `JamLines` and `Jokes.md` patterns. The generator fills slots; it does not invent sentences.
- **Typed requests (O8):** at a giver, "Ask for something else" opens the existing note editor. The text goes to the existing note endpoint with the quest's first stage as context, comes back through `JamNoteClamp`, and replaces that stage. No backend change. Offline, the keyword matcher answers, as at CUT!.

**The Open game loop** (`AJamGameMode`, still the only writer of shared state)

1. Title, then the hero stands in the village square. Fenella pitches the film and a marker points to the field.
2. Main quest 1 is Scene 1 as built, forced retake included. Its wrap opens the world: givers get offers.
3. `AJamVillager` (the `SM_Villager` model) stands at each `Giver` location with a marker over their head when they have an offer. Interact opens the Candy Rock dialogue: title, pitch, Accept, Not now, Ask for something else.
4. Accepting puts a compass marker and distance on the HUD. Entering the camp's radius starts the shoot there; stage actors spawn inside the camp's bounds and are leashed to them, as nuisances already are to a set.
5. The wrap pays out through the existing ledger and loot at the quest's tier. There is no walk back to hand in; the giver reacts with an approval toast the next time the hero passes.
6. Idle camps keep a few of their biome's nuisances wandering when the hero is within 6,000 uu and remove them beyond 9,000. Pacifying them gives XP and nothing else.
7. Scene 3's wrap leads to the Premiere and credits exactly as now, so the game still ends. Side quests are optional breadth on the way.

Also needed: nuisances, pickups and villagers find the ground by tracing down, so uneven terrain works; XP continues past level 10 on a second threshold table (cap 14, one point a level, existing rank caps) used only in Open mode; a retake returns the hero to the camp's edge, not a set mark.

**Presentation hooks** (GAME wires them, TUNING fills them): music cue, weather and grade change on entering a biome; the HUD gains a compass marker, a quest tracker line under the objective, the giver marker and the dialogue screen (`-uiscreen=giver|tracker|compass`). No minimap.

**Classes.** Section 3d stands. GAME adds one hook (O9): `FJamClassDef` (hero mesh, clips, four attack ids) with the knight as row 0, read by `AJamHero` and the skill bar, so a class is a row plus assets.

**The tuning surface.** Every number and row a tuner may change lives at the top of one of these files, each guarded by a bounds test. TUNING edits these and nothing else, and after each change runs the build, `run-tests.ps1` and both autoplay modes.

| File | Holds |
| --- | --- |
| `Private/World/JamWorldTable.cpp` | Locations, camp kinds and capacity, biome definitions (shared with TERRAIN) |
| `Private/Film/JamQuestTemplates.cpp` | Quest templates, count and difficulty ranges, tiers |
| `Private/Film/JamScenes*.cpp`, section 10 numbers | The three film scenes, nuisance stats |
| `Private/Film/JamProgression.cpp` | XP thresholds, rank tables, loot stat sizes |
| `Private/Game/JamOpenTuning.h` | Camera distance and pitch, ambient spawn counts and ranges, compass range, fallback scatter density |
| `Private/Film/JamLines*.cpp` | Lines, after they are in `Jokes.md` |

GAME lists the real file names in `STATUS.md` when O-tasks create them; where a name above differs from what was built, STATUS is right.

**Still not building:** save games, inventory or vendors, a minimap or map screen, turn-in walks, new stage verbs, navmesh pathfinding, world streaming, replication.

## 4. Theme checklist (the "Theme" score)

Cheap, always-on framing that tells a judge in ten seconds what this is:

- `SCENE 2 - TAKE 3` slate label on the HUD at all times; clapper card on ACTION!
- REC dot and running timecode in a corner; letterbox bars slide in for cards
- Fenella in frame, with a caption bubble for every line (audio optional)
- CUT! as a huge jagged bubble; RETAKE! with a visible world reset ("back to one")
- Production notes as captions ("This walk was restored for the Director's Cut")
- **Director's Commentary toggle** (a key and a pause-menu option): a second line pool where she talks over play. Same scheduler, one extra trigger. High theme value for a few hours' work.

## 5. Architecture

Three layers. Dependencies point downward only.

**A. Headless rules** (`Source/MegaJam/*/Film/`): plain C++ structs, no `UWorld`, no UObjects, all under `Jam.Film.*` tests.

- `FJamStageDef`: id, verb (`Pacify | Interact | Collect`), target kind, count, resolve multiplier, speed multiplier, time limit (0 = none), fail meter (0 = none), objective text.
- `FJamNoteDef`: id, pitch line id, replacement `FJamStageDef`, gross multiplier, bubble pool override.
- `FJamSceneDef`: id, title, stages, index of the transition that has a note (or none), the note, `bForcedRetakeOnFirstStage`.
- `FJamShoot`: the state machine. States `Slate -> Rolling -> StageCleared -> (NotePrompt) -> ... -> Wrap`, plus `FailedTake -> Slate`. Inputs: `Action()`, `ReportProgress(n)`, `ReportFail()`, `AnswerNote(bTake)`, `Retake()`. Output: a queue of events (`StageStarted`, `Progress`, `StageCleared`, `NotePitched`, `TakeFailed`, `Wrapped`) that the world layer drains. Events as data, not delegates, so tests assert on them.
- `JamLedger::Compute`: pure function, inputs gross, notes taken, floor, seed; outputs line items, hero's cut, which loot index she keeps.
- `FJamNagScheduler`: as master brief C.7 (global gap, per-trigger cooldown, no-repeat window, deterministic, silent during prompts), plus the `commentary` and `sulking` pools.
- `JamLines`: every authored line as a C++ table (`id, trigger, text`). **Text lives in C++, not in assets or loose files**: nothing to cook, nothing to load, a test can check every id is unique and every trigger has lines. Voice-over is found by convention: `/Game/Audio/VO/VO_<LineId>`, loaded softly, missing means text only.
- The three scenes are authored `FJamSceneDef` tables in C++. A test proves each stage is finishable: count is no more than what its set spawns.

**B. World** (`Source/MegaJam/*/Game/`):

- `AJamGameMode`: the only place that mutates the shoot. Drives the `FJamShoot`, drains its events, spawns and resets stage actors at `AJamSetMarker` points, parses flags.
- `AJamGameState`: holds the `FJamShoot` and the shared, readable state (scene, take, stage progress, meter, notes taken/refused). UI reads from here.
- `AJamPlayerState`: per-player state (sprinkles, equipped item and its stat bonuses) and the hero's ASC.
- `AJamHero : ACharacter`: avatar for the PlayerState's ASC, spring-arm camera, input. Create Enhanced Input actions and the mapping context in C++ at runtime so there are no input assets to cook or lose; verify this against the 5.8 headers before relying on it, and add the `EnhancedInput` module.
- `AJamNuisance : APawn`: ASC, Resolve, the five states above, an interact entry point. Reports pacified/treated to the game mode, which calls `ReportProgress`.
- `AJamFaerie`: cosmetic only. Interpolates to a shoulder offset, bobs, faces what she is filming. No collision, no skeleton.
- The existing GAS layer carries Resolve and the skills. Extend it only as section 3c says: three more area abilities, ability level as rank, one Flustered effect.

**Multiplayer-ready rules** (cost almost nothing now, save a rewrite later; write no replication code, RPCs or session code during the jam):

- Shared game state changes in one place, the game mode, which is what a server would be. Nothing else writes to the shoot, progress, meters or rewards.
- Player input never changes game state directly. The pawn or controller calls a request function (`RequestLecture`, `RequestInteract(Target)`, `RequestAnswerNote(bTake)`) that validates and then acts. Later these become server RPCs with the same bodies.
- Shared state lives on the GameState, per-player state on the PlayerState. No gameplay singletons, no statics holding game state, no game state on widgets or on the GameInstance.
- No `GetPlayerController(0)`, `GetPlayerCharacter(0)` or `GetFirstPlayerController` in gameplay code. Take the instigator or owner as a parameter. (The autoplay bot and UI setup may use the local player.)
- Cosmetics (VFX, sound, bubbles, captions, cards) are triggered from events carrying compact data (ids, positions, seeds) and never feed back into rules. Fenella's lines are chosen by the scheduler from a seed, so two machines would pick the same line.
- Nuisances and stage actors are spawned and reset only by the game mode.
- UI reads state and sends requests; it holds no authoritative values.
- One wrinkle to remember: the note is one shared decision. In co-op the host would answer it; ledger and loot are already per PlayerState.

**C. Presentation** (`Source/MegaJam/*/UI/`, `/Game/UI/`): one HUD widget with layers (slate label and REC, objective and meter, bubble layer, card layer for ACTION!/CUT!/RETAKE!/Wrap, note prompt), plus Title, Pause, Premiere, Credits screens. Follow the `CLAUDE.md` UI rule (WBP layout, C++ behaviour). If authoring a WBP through MCP costs more than 30 minutes on day 1, build that widget's tree in C++ instead and tell Dylan.

**Flags, day one:** `-autoplay` (a bot walks to the nearest objective target, lectures, interacts; answers notes per `-autonote=take|refuse`; logs `JamAutoplay: PASS scenes=3 ending=<name>` or `FAIL <why>`), `-scene=<n>` (start at a scene), `-uiscreen=<name>` (open one screen with sample data), `-timescale=<x>`. The autoplay log line goes into `smoke-packaged.ps1`. This is the regression test for the whole game; a task that breaks it is not done.

**Cooking:** anything loaded by path (`/Game/Audio/VO`, `/Game/UI`, any data) goes in `DirectoriesToAlwaysCook` the same day it is added. Change `GameDefaultMap` when the real map exists.

## 6. Art and audio plan

**Style (Dylan, Nov 11; locked in `Docs/styles.json`):** the hero and the hero's gear are **dark fantasy, high poly**. The environment, NPCs, nuisances and effects are **bright, kiddie, low poly**. The clash is the joke (master brief Appendix B: do not candy-stripe the knight). Generated 3D through the pipeline in `Docs/GenerationRouting.md` is the plan: Meshy for characters and NPCs, Rodin for the hero's gear and props.

**Order of work, so there is always something shippable:**

1. Debug shapes with labels (day 1; the game is built on these).
2. Generated 3D, one approved test asset per type before batching.
3. First fallback (Dylan, Nov 11), if the generators do not deliver: **bought or free packs from Fab, Synty or similar**. A Synty-style low-poly pack suits the bright kiddie side; a rigged dark-fantasy knight with animations from Fab would also remove the hero rigging risk. Rules: Dylan chooses and downloads (the engine's Fab plugin is disabled in the `.uproject` because it breaks UBT on this install, so assets come in through the launcher or a manual import, not the in-editor plugin); every pack gets a line with its licence in `Docs/Declared.md` under third-party assets and in the submission; import only the meshes used, since the zip must stay under 1 GB; at least one shipped prop still comes from Rodin for the Hyper3D pool.
4. Last fallback, for single missing pieces late on: **cardboard flats** (a 2D image on a plane with a plywood edge and a stick), which the film-with-no-budget fiction excuses. A few deliberate flats as gags (`Jokes.md`) are fine either way.

**The hero** is the riskiest asset. Time-box the Meshy rig-and-animate route to two hours on day 2. If it fails: a rigged knight from Fab; failing that, a static knight mesh that glides and bobs, with one Fenella line about the animation budget. Needed animations if it works: idle, walk, one sword-raise. Set the camera close enough that a high-poly hero is worth having (see section 11, answer 6).

**Lecture presentation is the brand moment and gets real time on day 3:** sword up (0.3 s, no swing), inhale, cone flash, two or three jagged bubbles from `Jokes.md`, targets recoil. Total wind-up under 0.4 s so it still plays well.

**Fill `Docs/styles.json` before generating anything in bulk,** approve one test asset per type, then batch. Make at least one shipped prop with Rodin.

**Audio (a judged criterion, start day 2, not day 4):**

- Music via ACE Studio: (a) cheerful village loop, (b) pompous dark-fantasy sting that plays when the sword comes up and cuts off dead when the lecture starts, (c) a credits song with vocals.
- Eight sounds: inhale, lecture whump, bubble pop, pacified squeak, clapper, CUT! horn, coin trickle (small), UI click.
- Fenella's voice: tabled (Dylan, Nov 11). Captions ship regardless and the hookup (T23) is by line id, so clips can arrive late. If ACE Studio cannot do speech, the fallback is a paid speech generator, chosen later; 30 to 40 clips from `Jokes.md`, most-heard triggers first.
- One master volume slider in the pause menu.

## 7. Task backlog

**Capacity (Dylan, Nov 11): about 9 to 10 hours a day of steering, uneven.** The plan was drawn for 12, so it is already cut to fit: T22 (commentary) and the T24 street gags beyond signs and plywood backs are **stretch**, done only if day 3 is ahead. Gate: if Scenes 1 and 2 are not playable start to finish in a packaged build by the end of day 2, Scene 3 is cut and day 3 goes to making two scenes and the Premiere good. Tasks marked Sonnet are headless and safe to leave running unattended; queue those for when Dylan is away and keep the Opus, engine-facing tasks for when he can look at screenshots.

Take the first unblocked task. One task per session. Sizes are guesses. "Model" is a suggestion: Sonnet for well-specified headless code and text, Opus for anything that touches the engine, the editor, packaging or debugging.

**Day 1 (Nov 12): a game on screen, packaged.**

| ID | Task | Model | Done when |
| --- | --- | --- | --- |
| T01 | Hero on screen: `AJamHero`, top-down camera, runtime-created input, Lecture on LMB against 3 static test nuisances on the template map; `AJamGameMode` as default | Opus | Screenshot shows hero, nuisances, a debug cone; pacified count logged |
| T02 | Package now: `package.ps1`, `smoke-packaged.ps1` pass with T01 | Opus | Packaged exe runs, log clean |
| T03 | `FJamShoot` + scene/stage/note structs + tests (master brief C.10 A to C, plus forced retake, once-only wrap) | Sonnet | `Jam.Film.Shoot.*` pass |
| T04 | `JamLedger::Compute` + tests (floor, cap, idempotent, deterministic) | Sonnet | `Jam.Film.Ledger.*` pass |
| T05 | `AJamNuisance` with the five states and steering inside a set's bounds; interact (E) with a prompt | Opus | Screenshot: rats fleeing; pacify then interact logs Treated |
| T06 | Wire `FJamShoot` into the game mode: Scene 1 playable end to end with debug shapes and plain text HUD (objective, Scene/Take, wrap text) | Opus | A human can play Scene 1 to the wrap |
| T07 | `-autoplay`, `-scene`, `-autonote`, `-timescale`; add the PASS line to the smoke test | Opus | `JamAutoplay: PASS scenes=1` in editor `-game` and packaged |
| T08 | MCP spike, 30 minutes: can the editor MCP place actors and save a map? Record the answer in STATUS. If yes, block out `/Game/Maps/L_Sugarborough` (three sets and the street); if no, spawn the blockout from a C++ layout table | Opus | Blockout exists; hero can walk set to set |

**Day 2 (Nov 13): the whole film, ugly.**

| ID | Task | Model | Done when |
| --- | --- | --- | --- |
| T09 | `JamLines` table from `Jokes.md` + `FJamNagScheduler` + tests (C.10 E) | Sonnet | `Jam.Film.Nag.*` pass; every trigger has 3+ lines |
| T10 | HUD layers: slate label, REC and timecode, objective and meter, card layer (ACTION!/CUT!/RETAKE!), bubble layer | Opus | `-uiscreen` screenshots of each card |
| T11 | `AJamFaerie` + captions from the scheduler | Opus | Screenshot with her and a caption; never over the objective |
| T12 | Note prompt (CUT!) and its consequences: variant stage, sulk pool, counters; gamepad usable | Opus | Autoplay passes with `-autonote=take` and `=refuse` |
| T13 | Failure: countdown and meter, TakeFailed, RETAKE! with world reset | Opus | A forced-fail flag shows Take 2 |
| T14 | Wrap card: itemized ledger, three loot cards, "reserved for the sequel", auto-equip changes a real stat | Opus | Screenshot; cone visibly larger after a radius item |
| T15 | Scenes 2 and 3 as data, their sets, the scripted beats | Opus | Autoplay `scenes=3` |
| T16 | Title, Pause (resume, volume, commentary, quit), Premiere with both endings, Credits | Opus | Full run title to title, keyboard and gamepad |
| T17 | Asset lane (Dylan plus a side session): fill `styles.json`, hero test asset, rat, goblin, one house, one Rodin prop | either | One approved asset per type in the game camera |
| T18 | Audio lane (Dylan): the three music pieces, first eight sounds | Dylan | In the build, with the volume slider |

Progression (section 3c), day 2:

| ID | Task | Model | Done when |
| --- | --- | --- | --- |
| T04a | `FJamProgression` + rank tables + tests (thresholds land on the cards, points never exceed levels, stats total correctly) | Sonnet | `Jam.Film.Progression.*` pass |
| T05a | Skills 2 to 4 as area abilities with rank tables, the Flustered effect, input slots | Opus | `-abilitytest`-style scene shows each shape; tests for line and ring hits |
| T14a | Skill bar with cooldowns and ranks, XP bar, point spend on the wrap and CUT! cards, character sheet, the pre-spent passives panel; autoplay spends points | Opus | Screenshot of each; autoplay ends at level 10 |

Typed notes (section 3a), days 1 and 2. B-tasks run in the backend repo in parallel with the Unreal session:

| ID | Task | Model | Done when |
| --- | --- | --- | --- |
| B1 | `POST /v1/director/note`: request and response schema, prompt, server-side validation, rate limit, tests with twenty canned notes including hostile ones | Opus | Deployed; curl returns a valid form inside 5 s |
| B2 | `POST /v1/director/premiere`: notes log in; title, tagline, three review quotes out | Sonnet | Deployed; canned logs give usable text |
| T12a | `JamNoteClamp` + keyword fallback matcher + tests (garbage JSON, out-of-range knobs, over-long text, unfinishable stage) | Sonnet | `Jam.Film.NoteClamp.*` pass |
| T12b | Note client (HTTP, timeout, `-nonotes`), text box and suggestion buttons on the CUT! prompt, waiting state, notes log on the PlayerState, apply cosmetic knobs (scale, tint, weather, mood, bubble pools). Depends on T12 | Opus | Typed note visibly changes the next stage in a packaged build; unplugged run still passes |
| T16a | Premiere uses the generated title and reviews, falls back to the authored endings | Opus | Both paths screenshotted |

End of day 2: package, smoke, and **have someone who has not seen it play it**. Write down where they got lost.

**Day 3 (Nov 14): make it look, sound and feel like the pitch. Feature freeze at the end of the day.**

| ID | Task | Model | Done when |
| --- | --- | --- | --- |
| T19 | Lecture presentation: sword-up fakeout, cone VFX, jagged bubbles, recoil, sting cut-off | Opus | Looks right in a 5-second capture at game camera |
| T20 | Art pass: hero in, nuisance skins, flats for villagers and houses, set dressing, lighting and one colour grade | Opus + Dylan | Three set screenshots Dylan approves |
| T21 | Finale beat: budget runs out (material swap, labels, humming) | Opus | Screenshot before/after |
| T22 | Director's Commentary pool and toggle | Sonnet then Opus | Toggle works mid-scene |
| T23 | Voice-over hookup by line id; missing clips fall back to text | Opus | Packaged build plays clips; smoke passes with the VO folder removed |
| T24 | Street gags from `Jokes.md` (signs, plywood backs, deleted-scene door), opening narration interrupt, gag credits | Opus | Dylan picks which; each is a screenshot |
| T25 | Fixes from the playtest: first-minute clarity, prompts, pacing | Opus | Second outside playtest finishes without help |

**Open world (section 3e), from the evening of Nov 12. GAME takes these in order, ahead of T24 and T25, and stops after O9.** Each ends with the build, `Jam.*` tests, both autoplay modes and a screenshot. Each is a commit, so the work can stop after any of them and leave a working game. Numbers are placeholders that make the bot pass; tuning is not GAME's job.

| ID | Task | Done when |
| --- | --- | --- |
| O1 | Tag `slice-day1`. World mode switch (`-open`, `-backlot`). `World/` types, `JamWorldTable.cpp` with a first layout, `Jam.World.*` tests. Flat fallback world built from the table. Write the table's shape into `Docs/Workers/terrain.md` for TERRAIN | `-open` screenshot from above shows village, three tinted biomes, film sets and camps; Backlot autoplay unchanged |
| O2 | Follow camera, camera-relative movement, cast-faces-camera, `-camyaw` / `-campitch` / `-camdist`, sensitivity and invert-Y settings; bot turns the camera | Screenshot behind the hero in the village with Fenella in frame; Scene 1 passes by bot in `-open` |
| O3 | `FJamQuestTemplate`, `JamQuestGen`, `FJamQuestLog`, four starter templates, tests | `Jam.Film.Quest.*` pass |
| O4 | Open game loop: main questline at the film sets, `AJamVillager` givers, accept, camp start with leashed spawns, wrap at tier, ground tracing, ambient camps, Open-mode XP table; bot plays it | `JamAutoplay: PASS mode=open main=3 side=2` in the editor `-game` |
| O5 | HUD and screens: compass marker and distance, quest tracker, giver marker, Candy Rock dialogue (keyboard, gamepad, mouse) | `-uiscreen=giver`, `tracker`, `compass` screenshots |
| O6 | Biome hooks: music, weather and grade on entering a biome; kinds per biome from the table | Three screenshots, one per biome, visibly different |
| O7 | Package and smoke `-open`; add the Open PASS line and a cooked-world check to `smoke-packaged.ps1`. If TERRAIN's map exists: launch on it, fix what ground tracing and collision get wrong | Packaged Open run passes on the flat fallback (and on the map, if delivered) |
| O8 | Typed quest requests at a giver, through the existing note client and clamp | Stub-tested: a typed request visibly changes the quest; offline run passes |
| O9 | `FJamClassDef` hook with the knight as row 0; `-class=` picks a row | Knight unchanged in both autoplay modes and the T19 screenshots |

After O9, GAME writes the real tuning-surface file names into `STATUS.md` and hands over to TUNING.

**Day 4 (Nov 15): no features.** Bugs, tuning, audio levels, a clean-machine unzip test, the 30 to 60 second video (cut: sword fakeout, CUT! prompt, ledger, budget running out), declarations, itch page. **Upload by 2:00 PM PST**; the last four hours are spare.

## 8. Cut order when behind

Drop from the top. 0) All expansion work (sections 3d and 3e): it goes before anything in the slice does. Within 3e, drop from the end: O9, O8, O6, then TERRAIN's map (ship the flat fallback); if O4 is not green by the end of Nov 13, Open mode stays behind `-open` and the slice ships. 1) Commentary track. 2) Street gags beyond signs. 3) Skill 3, the Disappointed Sigh (two skills and the ultimate still give a choice). 3a) Second ending (ship one Premiere card). 4) Scene 3's almond-collect stage. 5) Loot stat effects (names only). 6) Voice-over (captions only). 7) Generated 3D for anything but the hero (flats). 8) Scene 3 entirely: two scenes and a Premiere is still a complete film.

Typed notes are outside this order: they are decided by their own gate at the end of day 2 (section 3a), and if they pass they rank above everything in the list except Scene 3 entirely.

Never cut: the sword fakeout, the forced "Perfect. Again.", one CUT! choice, the itemized ledger, an ending card, music, the packaged smoke test.

## 9. Session protocol

1. Read `CLAUDE.md`, `Docs/STATUS.md`, this file. Not the briefs.
2. Take one task. If it needs a decision this file does not make, pick the cheaper option, write the choice in STATUS, keep going.
3. Finish with: build, `run-tests.ps1` green, `-autoplay` PASS, a screenshot you looked at. Say which of these you did not do.
4. Rewrite `Docs/STATUS.md` in place (what exists, what is broken, next task). Keep it under 80 lines; it is a snapshot, not a history.
5. Append three to six lines to `Docs/DEVLOG.md`: date and time, what was built, what went wrong, one screenshot path. This is the Glass Box entry and the source for the submission's tool list. Be candid; failures make a better log.
6. Anything brought in from another project or generated by a tool: a line in `Docs/Declared.md` the same session.
6a. **Commit at the end of every task** once the build and tests pass (Dylan, Nov 11), and at any green checkpoint inside a long task. Small commits are the rollback points. Never push without Dylan asking. Never commit the `Config/DefaultEngine.ini` token diff. The whole jam is streamed: nothing with a secret is ever printed, opened or committed.
7. **One code session per checkout, and one editor.** Two sessions building in the same folder lock each other's binaries. Parallel work is split into workers with their own checkout or repo and their own folders: see `WORKERS.md`. Only the GAME worker opens the editor, imports assets, or writes `STATUS.md` and `DEVLOG.md`; the others keep notes in `Docs/Workers/<name>.md`.
8. Do not add a system that is not in section 3 or 5. If something seems to need one, stop and write the question in STATUS.
9. Never mark a task done on the strength of tests alone when its "done when" is a screenshot or a played run.

## 10. Starting numbers (placeholders; tune by playing, not by argument)

| Thing | Value |
| --- | --- |
| Lecture | 800 uu cone, 35 degree half angle, 60 Resolve, 1.5 s cooldown (as built) |
| Rat | 100 Resolve (two lectures), speed 300, flees inside 500 uu |
| Goblin | 150 Resolve, speed 350, flees inside 600 uu |
| Bug | 60 Resolve (one lecture), speed 450, erratic |
| Boss-sized note target | 480 Resolve (eight lectures), speed 250, does not flee |
| Hero | speed 600 |
| Scene 2 countdown | 60 s; Scene 3 meter fails at 10 crops eaten |
| Interact | hold E 0.6 s, inside 150 uu |
| Target scene length | Scene 1: 90 s. Scene 2: 2 min. Scene 3: 3 min |

## 11. Needs Dylan (answers go in STATUS.md under "Decisions")

Answered Nov 11:

1. **Multiplayer:** no feature work, keep the preparation (section 5, "Multiplayer-ready rules").
2. **Solo.** Team name **Imaginary Component**; zip is `ImaginaryComponent_<GameName>.zip` (game name defaults to `TheDreadLandOfCandy` until Dylan says otherwise). Eligible for Solo Driver.
3. **Hours:** 9 to 10 a day, uneven. Plan cut to fit in section 7.
4. **Voice:** tabled; fallback is a paid generator, decided later. Captions ship.
5. **Art:** dark fantasy high poly for the hero, bright kiddie low poly for everything else, through the generators. In `styles.json`.

6. **Camera:** Dylan considered a Fortnite-style over-the-shoulder camera and left the choice to whichever is easiest. Decision: **fixed three-quarter camera, pulled in close** (start at about 1100 uu arm length, 50 degree pitch, no player camera control; tune in T01 so the hero fills roughly a sixth of the screen height). Reasons: every screenshot shows the whole set from a known angle, which is how sessions verify their work; no camera collision, mouse-look, sensitivity or invert settings to build; Fenella and her camera stay in frame; nuisances cannot hide behind the player; strafing and backpedal animations are not needed. Do not revisit. **Revisited by Dylan, Nov 12 evening:** this holds for Backlot mode only; Open mode uses the follow camera in section 3e.

7. **Backend:** second Gunicorn service and second Postgres database; Haiku; database cache first; about 50 dollars must go a long way (section 3a).
8. **Commits:** regularly, at the end of each green task (section 9).
9. **RPG:** levels, stats and MOBA-style skill points (section 3c). One hero.
10. **Game name: The Dread Land of Candy.** Zip: `ImaginaryComponent_TheDreadLandOfCandy.zip`. "Director's Cut" stays as the stamp on the title screen.
11. **Dev log:** yes. The jam is streamed; `Docs/DEVLOG.md` is the written log and carries the stream links. Outside playtester for the end of day 2: Dylan is looking for one.

Open, only Dylan can answer:

6. Is the forced "Perfect. Again." retake in Scene 1 acceptable? It is the best theme joke per hour and it makes the player repeat 20 seconds.
7. Is the hero's one spoken word at the Premiere in or out (`Jokes.md`, set piece 12)?
8. Which jokes you veto. Read `Jokes.md` once and strike what you dislike before T09 turns it into code.
9. Who is the outside playtester at the end of day 2 and day 3?
10. Where the dev log is published (itch devlog, a gist, a repo page) and what records the video.
11. Do you have a gamepad plugged in for testing? If not, gamepad support is untested and should say so.
12. Rodin and Meshy credit balances (`Scripts/check-balances.py`) and whether the Meshy rigging route works on one test character.

## 12. Still missing from the design (nobody has decided these; cheapest defaults given)

- **First 30 seconds of teaching.** Default: three on-screen prompts in Scene 1 only (move, lecture, hold E), each dismissed by doing the thing. No tutorial screen.
- **What Fenella looks like.** Default: glowing billboard sprite, beret, toy camera; a flat like the villagers.
- **What the knight looks like** beyond "dark and grim". Needs one reference sentence before generation.
- **Replay.** Default: credits return to the title; no save, no scene select.
- **Settings.** Default: volume and commentary only. No graphics options, no rebinding.
- **Performance target.** Default: 60 fps at 1080p on your machine; Lumen and Nanite off unless they are free.
- **Accessibility.** Default: every voiced line captioned; "reduce shake" is skipped unless there is shake.
- **Hero's name and the film's in-fiction title.** Default: the hero is "Talent"; the film is the game's title.
