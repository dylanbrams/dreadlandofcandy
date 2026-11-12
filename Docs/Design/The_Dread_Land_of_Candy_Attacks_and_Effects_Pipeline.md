Director’s Cut \| Combat Attacks & Effects Pipeline \| Implementation Handoff \| 12 November 2026

Status: all 12 core attacks have conceptual designs. This document records decisions from design discussion, plus proposed implementation contracts. Exact timings, balancing, and Unreal asset names are provisional until verified in the project.

# 1. Creative rules

- Four combat identities: Knight enforces manners; Barbarian enforces food preparation and vegetables; Mage applies catastrophic household hygiene and moderation; Archer weaponizes vegetables and dental hygiene.

- Treat every attack with the full seriousness of epic fantasy. The comedy emerges from disproportionate presentation and parental dialogue.

- Combat reduces Resolve; outcomes pacify rather than kill. Enemy reactions are half the joke.

- Gameplay contact and status are authoritative. Niagara and comic props are cosmetic and may not determine hits.

- Favor legible silhouettes, playable camera distance, short repeatable basics, and decluttered four-player overlap.

# 2. Attack roster & battle cries

| ID  | Attack                     | Primary voice line          | Gameplay role                               |
|-----|----------------------------|-----------------------------|---------------------------------------------|
| K01 | Polite Shield              | EXCUSE ME!                  | Fast shield bash / short push               |
| K02 | Sword of Stern Instruction | ABSOLUTELY NOT!             | One frontal lecture cone                    |
| K03 | Shield of Good Manners     | MIND YOUR MANNERS!          | Moving protective dome / boundary repulsion |
| B01 | Furious Kneading           | BAKE THIS!                  | Fast melee kneading strike                  |
| B02 | Rolling Pin Cyclone        | TBD (no line locked)        | Moving circular multi-pulse whirlwind       |
| B03 | Aggressive Encouragement   | EAT YOUR VEGETABLES!        | One frontal sonic cone                      |
| M01 | A Spoonful of Sugar        | A SPOONFUL OF SUGAR!        | One hitscan, multi-cube cosmetic volley     |
| M02 | Peppermint Popfall         | ONE AFTER DINNER!           | Staggered area bombardment + finale         |
| M03 | Arcane Sanitization        | WASH YOUR HANDS!            | Short sustained frontal soap cone           |
| A01 | Celery Shot                | INCOMING GREENERY!          | One standard ranged hit                     |
| A02 | Celery Volley              | THESE ARE RICH IN VITAMINS! | Ground-targeted area barrage                |
| A03 | Floss Snare                | DID YOU FLOSS?!             | Projectile with area snare/status           |

# 3. Individual attack choreography & assets

## K01 — Polite Shield

Presentation: A politely forceful shield shove; tiny gold shockwave; enemy stumbles backward in embarrassment. Quick return to dignified stance.

Gameplay contract: Single contact / Resolve and optional push at shield notify.

Core effects: Shield impact spark, small geometric ring, shared flustered reaction.

Notes: Short repeatable; full cry only sometimes.

## K02 — Sword of Stern Instruction

Presentation: Draw impossibly giant sword in ominous windup; raise to apex; stop; point sword like a teacher’s pointer; music cuts; shout reprimand. Never swing. Blue/gold geometric lecture shockwaves, formal pre-rendered placards, target straightens or looks chastised.

Gameplay contract: Single authoritative forward cone contact aligned to spoken reprimand; sword collision disabled for offensive damage.

Core effects: Sword-charge, VFX.LectureCone, comic placard mesh system, punctuation debris.

Notes: Illustrative 1.55 s: plant 0–.30, draw .30–.65, pause/point .65–.80, lecture .80–1.15, reaction through 1.55. Timing adjust to animation.

## K03 — Shield of Good Manners

Presentation: Shield slam creates colossal but transparent gold heraldic dome. Boundary rebukes approaches with EXCUSE ME / TOO CLOSE placards. Dome folds away; tiny THANK YOU.

Gameplay contract: Initial radial Resolve pulse; temporary protection for allies inside; crossing enemies repelled according to server rules. Dome follows Knight.

Core effects: Translucent dome mesh/material, boundary pulses, comic placards, soft edge geometry.

Notes: Proposed 3–4 s duration. Keep central area transparent; implement boundary without physics simulation.

## B01 — Furious Kneading

Presentation: Two-handed double-fist dough-kneading strike; flour puff, dough scraps and wet THWUMP; enemy subtle squash-and-stretch. Optional alternating knead pose.

Gameplay contract: Single ordinary melee contact.

Core effects: DoughImpact, flour puff, soft deformation/reaction if existing animation supports it.

Notes: “BAKE THIS!” occasional; reuse melee montage.

## B02 — Rolling Pin Cyclone

Presentation: Start with two rolling pins, then disciplined orbiting inner ring, then escalating chaotic outer cyclone of many tumbling pins and flour. End with outward burst and one stray rolling pin that drops, bounces and rolls away.

Gameplay contract: Character-centered moving area; server pulse schedule and final stronger pulse; cosmetic pins never deal damage.

Core effects: Orbiting-prop mesh/Niagara system, flour cyclone, motion trails, controlled wood-clatter audio loop.

Notes: Approx 2.3 s illustrative; high presentation tier ~20 pins maximum, lower tiers fewer. No networked simulated prop physics.

## B03 — Aggressive Encouragement

Presentation: Exasperated parent roar in one random prerecorded language. Red/orange sonic cone hurls numerous PRE-RENDERED comic bubbles/cards in multiple languages, tumbling end-over-end. Cards briefly face camera then crumple/fragment on hit. Rare emotionally exhausted “Please. Just eat them.” while full catastrophic effect still occurs.

Gameplay contract: One authoritative directional cone hit. Bubble collisions strictly cosmetic.

Core effects: Shared comic projectile pool; VFX.LectureCone chaotic variant; prerecorded voice bank; comic impact scraps.

Notes: Only pre-rendered artwork: no runtime text generation, translation or TTS. Rare variant rate illustrative ~3%, not locked.

## M01 — A Spoonful of Sugar

Presentation: Elegant wrist flick sprays five to six glowing tumbling sugar cubes in a tiny fan, near-converging on resolved impact. Short tink-tink-crunch, sugar sparkle.

Gameplay contract: ONE hitscan trace/contact. All cubes visual only.

Core effects: Small mesh volley, sugar trails, crystalline impact and fragments.

Notes: Proposed .25–.4 s. Short occasional multilingual voice line, not on every basic attack.

## M02 — Peppermint Popfall

Presentation: Ominous red/white portal; fewer giant peppermint meteors than celery barrage, heavy staggered impacts, enormous final peppermint pop. Mage catches a tiny final sweet and responsibly puts it away.

Gameplay contract: Ground-target area, staggered authoritative Resolve pulses and one larger finale.

Core effects: Projectile-rain preset, giant peppermint mesh and spiral trails, candy shard impacts.

Notes: “ONE AFTER DINNER!” primary; alternatives “TOO MUCH CANDY IS UNHEALTHY!” and rare “YOU’VE HAD ENOUGH.” Different rhythm from A02.

## M03 — Arcane Sanitization

Presentation: Apocalyptic handwashing: enormous magical soap/sanitizer dispenser or conjured gel mass, viscous turquoise soap cannon, impossible white foam, iridescent bubbles, enemies come out squeaky clean. Mage finishes with a tiny personally applied sanitizer drop.

Gameplay contract: Short sustained aimable frontal cone; repeated authoritative Resolve pulses; interruption stops stream and sound.

Core effects: MagicCharge, SoapTorrent, FoamBurst, SoapBubble, CleanSparkle, looping audio.

Notes: “WASH YOUR HANDS!” as world-ending incantation; proposed .8–1.2 s active and 3–4 pulses; exact balance open.

## A01 — Celery Shot

Presentation: Quick bow release fires one leafy celery stalk; impact crunch and leaf scatter. Military seriousness.

Gameplay contract: Ordinary ranged hit; existing projectile or trace path as project supports.

Core effects: Celery stalk mesh, leafy impact confetti, short crunch.

Notes: “INCOMING GREENERY!” occasional; minimal new technology.

## A02 — Celery Volley

Presentation: Heroic upward celery shot duplicates into immense rain; rapid crunchy ground impacts leave temporary miniature celery forest; one pathetically late stalk drops. No ranch dressing gag.

Gameplay contract: Ground-target circle telegraph and several authoritative area pulses, not one hit per stalk.

Core effects: Shared rain-of-projectiles system, celery meshes, leaves/dirt, impact audio.

Notes: “THESE ARE RICH IN VITAMINS!” randomly prerecorded language per cast. Proposed 1–1.5 s barrage and ~3–5 pulses.

## A03 — Floss Snare

Presentation: Archer draws magical floss filament from impossibly small dispenser; on impact it unspools into taut glowing web; binds enemies; strands twang; final extremely fast dispenser zip.

Gameplay contract: One projectile impact with small-radius temporary immobilization and modest Resolve effect; server-managed status and expiry.

Core effects: Reusable tether/web primitive, strand meshes/ribbons, twang, retract effect.

Notes: “DID YOU FLOSS?!” in accusatory multilingual delivery. Proposed 2–3 s snare duration; do not simulate a physical web.

# 4. Shared effects pipeline — implementation maturity levels

These are alternative delivery scopes, NOT four mandatory sequential frameworks. Choose the lowest level that reliably ships the game. The recommended jam target is Level 2 for shared plumbing and selected hero attacks, Level 1 for basics; Level 3 is optional post-playable polish.

| Level                          | What exists                                                                                                                                                                                                        | Suitable use                                                             | What NOT to build                                                            |
|--------------------------------|--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------|--------------------------------------------------------------------------|------------------------------------------------------------------------------|
| L0 — Blockout                  | Montage/animation + simple single Niagara spawn on notify; gameplay hit event; placeholder sound; cleanup on cancel.                                                                                               | Prove all 12 abilities work and contact is aligned.                      | No custom editor, dynamic texts, or costly simulated props.                  |
| L1 — Reliable playable         | Per-attack data asset/preset; Start/Release/Contact/End events; known impact point; replicated gameplay result; local cosmetics; voice playback; explicit cleanup and interruption.                                | Jam minimum for every attack; stable multiplayer.                        | Do not make Niagara determine damage; no actor per sugar cube or comic card. |
| L2 — Shared signature pipeline | Small presentation coordinator; reusable primitives: cone, projectile rain, orbiting props, comic cards, foam, shield dome, tether; owner/teammate intensity; audio priority; target reactions; hit/miss variants. | Recommended for B03/K02/B02/M03 and other hero skills once L1 is stable. | No universal visual timeline editor; no procedural localization system.      |
| L3 — Cinematic polish          | Attack-specific camera beats, selective hit-stop, better mesh/orientation motion, voice alternatives, Director’s Cut gags, GPU and multiplayer quality tuning, accessibility options.                              | Only after Friday playable / during polish.                              | No new gameplay dependency on presentation polish.                           |

# 5. Visual intensity tiers — distinct from maturity levels

| Tier   | Owner-view presentation                                                                    | Teammate / crowded screen                                             |
|--------|--------------------------------------------------------------------------------------------|-----------------------------------------------------------------------|
| Low    | Core silhouette + impact + essential cue; sparse meshes and particles.                     | Only readable telegraph, impact and brief signature tell.             |
| Medium | Standard recommended: moderate props, select secondary particles, restrained camera/audio. | Reduce repeated props, bubbles and flour; keep threat cues.           |
| High   | Full comedy escalation, greater prop density, local micro-camera/hit-stop and debris.      | Do not automatically show full high-tier hero effects for every ally. |

Tier is a per-client presentation choice. It must not affect authoritative gameplay, telegraph timing, damage pulses, collision or enemy status. Override per attack when a particular silhouette needs protection.

# 6. Common event and timing contract

- Attack.Start: begin anticipation, audio priority reservation and cosmetic ownership.

- Attack.Release: montage notify to release visual projectile or begin active stream; for physical projectiles, the server projectile controls actual impact.

- Attack.Contact: authoritative hit geometry or projectile contact; resolve Resolve/status once or per designed pulse. Cosmetic animations are cued to match.

- Attack.HitResolved: display target-specific impact/reaction; support miss, blocked, immune and interrupted variants.

- Attack.End / Attack.Cancel: stop emitters and loops, release pooled meshes, detach tethers, fade translucent surfaces; remove gameplay effects only according to authoritative rules.

- A shared authored timing contract bridges montage/notifies, gameplay and cosmetic cues. Avoid animation, server damage and Niagara maintaining competing independent clocks.

- Voice variant and attack seed should be consistently chosen for a cast; cosmetic object positions may be local variation. Never replicate every visual prop.

# 7. Reusable primitives & ownership

| Primitive               | Attack consumers | Implementation note                                                                                             |
|-------------------------|------------------|-----------------------------------------------------------------------------------------------------------------|
| Comic-card projectile   | B03, K02, K03    | Pre-rendered transparent textures; pooled mesh cards or Niagara mesh particles; no runtime multilingual layout. |
| VFX.LectureCone         | B03, K02         | Shared system with barbarian chaotic warm palette versus knight disciplined blue/gold.                          |
| Orbiting props + flour  | B02              | Parameterized locally animated meshes; inner radius communicates hit area.                                      |
| Projectile rain         | A02, M02         | Different density and impact rhythms; target area and seeded timing data.                                       |
| Foam / soap stream      | M03              | Directional sustained attachment; collision/status comes from gameplay.                                         |
| Shield / boundary       | K01, K03         | Transparent center; protective state and boundary checks server-side.                                           |
| Tether / web            | A03              | Few purposeful strands, not physics rope simulation.                                                            |
| Impact/reaction library | All              | Flustered, embarrassed posture, squash/stretch, SqueakyClean, ReluctantlyNourished where supported.             |

# 8. Voice assets & multilingual workflow

- Generate and approve voice lines offline (e.g. ElevenLabs batch); package audio in build. No live TTS, translation, or external API at runtime.

- Select one voice language/recording per cast; store line ID, locale, duration and optionally mapped artwork. Do not assume literal translations retain natural parental tone.

- Use prerecorded, individually pre-rendered artwork for multilingual comic bubbles, independent of spoken locale except optional first large matching bubble.

- Basic-attack full cries trigger occasionally with concurrency limits; hero abilities may have guaranteed signature calls if mix permits.

- Central mix priority: critical game feedback \> local hero cry \> teammate cry \> secondary bubble clatter. Avoid simultaneous wall-of-voices in four-player combat.

- Kneading cry “BAKE THIS!” is now chosen; the older “IT’S NOT GOING TO KNEAD ITSELF!” is retired. B02 battle cry remains undecided.

# 9. Pipeline ownership / minimal asset schema

Suggested per-attack data definition (adapt to actual project conventions): AttackID; Montage; Gameplay geometry and pulse pattern; gameplay event tags; VFX preset IDs; voice pool ID; local camera preset; target reaction ID; audio loops; intensity overrides; interruption cleanup. Keep timing in animation/gameplay-owned notifies and contact events rather than creating a separate authoring timeline.

Suggested system split: gameplay ability owns target validation, authoritative events, Resolve/status; presentation coordinator tracks a cast ID and local lifecycle; primitive effect instances own cosmetics; audio manager owns voice priority/concurrency. If the project already uses Gameplay Ability System / GameplayCues, plug into that rather than introduce a competing ability framework.

# 10. Three-day jam delivery order

| Order                         | Deliverable                                                                                                            | Acceptance test                                                                         |
|-------------------------------|------------------------------------------------------------------------------------------------------------------------|-----------------------------------------------------------------------------------------|
| 1 — Gameplay first            | Every attack fires and applies the intended Resolve/status; placeholder visuals.                                       | Single player and multiplayer contact matches animation; no orphaned effects on cancel. |
| 2 — Signature primitives      | Comic card + lecture cone; projectile rain; orbit props; foam stream; dome; floss tether.                              | Two different attacks reuse common systems with visibly different presets.              |
| 3 — Signature animation/audio | Sword fakeout, catastrophic bubbles, rolling-pin escalation, soap apocalypse, celery rain; licensed/prerecorded cries. | Jokes work from normal game camera and with voice muted.                                |
| 4 — Polish and multiplayer    | Owner/adaptive intensity, culling, camera shakes, debug selectors, cleanup and overlap testing.                        | Four players attacking together remain readable; frame time acceptable.                 |

# 11. Explicitly open decisions

- B02 battle cry not chosen.

- Exact Resolve numbers, cooldowns, range, projectile travel speeds, root motion and montage assets require game-side confirmation.

- Duration, pulse counts, cone angles, dome behavior, status lengths and voice probability noted above are provisional tuning targets.

- Whether A01 uses hitscan or projectile depends on existing Archer gameplay implementation; reuse what is already stable.

- Whether actual project has GAS, Niagara assets, sound concurrency buses, pooled actor helpers, or camera shake presets has not been verified.

- Director’s Cut “ACTION!”, “CUT!”, “RETAKE!” can be later presentation-only interventions; never interrupt authoritative contact.
