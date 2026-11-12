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
