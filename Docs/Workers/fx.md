# FX handoff — 12 November 2026

## Current status

The pipeline/specifications, supporting assets and standalone Niagara prototypes
are built. Gameplay integration and final art approval remain separate work.
The stale saved-script repair is verified; the earlier graph-binding diagnosis
was incorrect.

## Deliverables

- [Production specification](../Design/Niagara_Attack_Pipeline.md), linked from both
  original attacks/effects documents; it follows the newer twelve-attack roster
  and the actual solo GAS / single-node-animation architecture.
- Ten original lightweight meshes, four procedural texture masks and six Niagara
  materials, plus source hashes and imported-asset manifests in
  [ImportSource/FX](../../ImportSource/FX/README.md).
- Nine shared primitive assemblies and twelve attack presets, using 33 Niagara
  systems. Read every system reference in `attack_presets.json` for a full attack;
  opening only the main system omits its secondary layer.
- Twelve isolated review maps under `/Game/FX/Attacks/Review`, a review gallery,
  particle-attribute samples and checks that destroying each cast owner destroys
  its Niagara components. The white cylinder is a 180 cm scale marker.
- A sequential rebuild command: `Scripts/build_attack_fx.ps1 -Review`.
  Add `-Sources` to regenerate the procedural sources. Individual commands are
  documented in the source README. The wrapper passed syntax checking; its
  underlying Unreal stages were exercised individually.

## Verified saved-bytecode repair

Claude identified the actual failure: saved assets carried stale compiled
scripts while reporting UpToDate. The graph expressions were present and a
forced in-memory bytecode audit generated correct programs, but that commandlet
did not save them. It therefore could not repair the assets a later render loaded.

Run `Scripts/recompile_fx_systems.py` in a **fresh** PythonScript commandlet with
`-dpcvars=fx.ForceCompileOnLoad=1`. On load, the engine invalidates compile IDs;
PreSave waits for compilation and saves the result. The script checks for
already-loaded systems, save failures and compile errors. This is now a required
step after the generator and before rendering/cooking.

The repair force-compiled and saved all 33 existing systems without rebuilding
their graphs. A fresh B02 capture then produced eight brown pins with XY orbit
radius 225 cm and 32 flour particles at nonzero positions. Fresh captures also
restored the intended ring/dome scales, celery/peppermint rain and comic-card layers.

The cause of the original stale save (mid-edit snapshot versus a cache-key issue)
has not been established. The prior suggestion to remove a final graph rewrite
was not a verified fix. Do not revive the graph-binding diagnosis from old notes.

Sprite billboards now explicitly use FaceCamera / Unaligned in the generator.
The existing sprite renderers were normalized with the repair script's optional
`-fxNormalizeSpriteFacing` engine flag; the cyclone's flour layer is now visible.
K01 uses the specified short shield arc mesh rather than depending on its brief
sprite puff for its primary silhouette. Its capture is still weak or missing
despite valid live attributes; review that basic's timing/readability in the editor
before integration. This is separate from the verified saved-script repair.

## Validation and remaining scope

`Saved/FX/recompile.log` records saved repairs; `Saved/FX/compile.log` and the
bytecode CSV record the separate forced audit. Run
`python Scripts/validate_attack_fx.py` after fresh captures. It checks source
hashes, Python syntax, all 33 systems / 132 nonempty VM programs, repaired-save
evidence, preset references, screenshot freshness and owner cleanup.
[Validation report](../../ImportSource/FX/validation.json).
[Review gallery](../../ImportSource/FX/review.html).

These remain blockouts: cards need approved artwork; flour/foam, trails and
choreography need an art pass. Lifetimes are currently baked from the authored
preset; Duration fades within that cap. Dynamic lifetime binding was not retested
after fixing stale scripts. Tether endpoints, hit events, protective states,
reactions, sword fakeout and audio require GAME hookups. Mid-cast Stop/BudgetOut,
pooled reuse, gameplay-camera movement, crowded-screen performance and packaged
cooking still need gameplay-world validation.

No abilities, default map, gameplay C++, module dependencies or cook settings
were changed. Niagara is enabled by default in this engine. These assets are
unreferenced by the playable flow; the source JSON is not a cooked runtime registry.
Do not integrate until GAME supplies a typed presentation registry and cast-owned
cleanup for every layer, then runs the slice's build, Jam.*, autoplay and package checks.

The checkout contains other workers' edits; never stage those with this work.
No push is authorized. The review workflow exits its editor when it finishes;
coordinate engine ownership with the terrain worker before another engine run.

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
