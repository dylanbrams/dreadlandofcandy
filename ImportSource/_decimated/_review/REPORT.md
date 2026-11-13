# Cleaned asset handoff — 2026-11-12

**31 cleaned GLBs are ready for GAME's import pass.** The original 30 Hyper3D/Fal models and batch-2 Meshy candy shard are included. Originals and `_v1` files remain untouched. No commits, paid generation requests, Unreal imports, game-code edits or engine configuration changes were made by this cleanup session.

Batch 2 was already complete: all eight 3D jobs and sixteen UI jobs were marked done. Its eight existing meshes have now been cleaned; generation was not repeated. The old first-pass decimation defect is fixed in the current staged outputs. [Initial review](INITIAL_REVIEW.md) documents the superseded results.

## Validation

- **31/31 PASS** in [independent saved-GLB validation](../validation.json): closed geometric edges across material splits, no edges shared by more than two faces, no collapsed geometric triangles, finite positions/normals/UVs, embedded base-colour images, triangle budgets, and unchanged original SHA-256 hashes.
- Original embedded base-colour image pixels are retained on all 31 models. New cut surfaces use matching solid materials. The rebuilt hat, mirror and megaphone sample the original embedded atlas as paint; their new shapes and UVs are intentional replacements for fused-head/ornament failures, not a claim of identical source UV layout.
- Total geometry: **5,800,166 → 339,166 triangles** (94.2% reduction). Characters are 25,000 each. Most other models are 8,000; deliberately simple repaired props remain below budget.
- Four-angle renders cover all 31 current outputs (124 images). [Wearable fit previews and ray tests](fit/fit_results.json) check the hat underside, mirror hole, and five clear rays through the gate passage. Both wearable openings and all portal rays pass.
- 12 binary FBX companions contain named UCX collision boxes plus neutral preview geometry. Per-asset collision policies and dimensions are in `../_collision/<service>/<asset>.json`. Gate collision has separate sides/header and is checked not to span the passage.
- These are Blender/file checks. Unreal import, camera acceptance, actor socket hookups, material parameter behaviour and packaged performance remain GAME's integration checks. No exhaustive self-intersection, UV-overlap or deformation certification is claimed. Some intentionally separate manufactured parts overlap where attached.

## What changed

- Weld source attribute splits **before** Collapse, preserving per-corner UVs. Reject bad exported candidates before replacing a staged GLB; rebuild from originals rather than welding the broken first pass.
- Refresh split normals after topology changes; retain the textured silhouette of thin limbs, wings, bristles, leaves and intentional openings.
- Remove the gate's knight, four table chairs, clapperboard limbs/stubs, the shard's white stick, trailing lamp cable, unwanted ground/pedestal geometry and exposed loose remnants. Patch cut surfaces.
- Rebuild a complete hollow straw hat and continuous head mirror band/disc, fitted on the actual Villager. Rebuild the ornament-filled megaphone as an open leaf cone with handle/veins. Retain the lamp housing/barn doors and supply a clean cable-free tripod.
- Lengthen/broaden the sword blade relative to the grip while retaining its original overall height and texture atlas. This addresses the recorded short-blade complaint without another generation request.
- Supply ground-contact, grip, particle-centre and wearable attachment origins. The NPC classifier includes LollyBug and Almond. Approved character faces, body silhouettes and the approved full fal cottage remain; there was no blanket redesign to match older prompts.
- Goblin has separate `Teeth` and `Skin` material regions; Molar has a named `Enamel` material; LollyBug has a texture-derived `Wings` region. Original paint remains. GAME can build material instances/masks from these; wing deformation still needs a suitable blended root mask if used.

## Preview sheets

Each row shows an oblique front, opposing side, side and overhead view. Camera framing is per model; these sheets do not imply a common world scale.

- [Sheet 1](clean/sheet_01.jpg)
- [Sheet 2](clean/sheet_02.jpg)
- [Sheet 3](clean/sheet_03.jpg)
- [Sheet 4](clean/sheet_04.jpg)
- [Sheet 5](clean/sheet_05.jpg)
- [Sheet 6](clean/sheet_06.jpg)
- [Sheet 7](clean/sheet_07.jpg)

Wearable assembly: [hat front](fit/SM_Hat_Straw_0.png), [hat back](fit/SM_Hat_Straw_1.png), [mirror front](fit/SM_HeadMirror_0.png), [mirror back](fit/SM_HeadMirror_1.png).

## Import and placement

Use the GLBs in `ImportSource/_decimated/{Hyper3D,Fal,Meshy}` for appearance. Do not import `_review` as content. Original raw assets still live one directory above and must not be overwritten. The staged names are unchanged.

Use `_collision/<service>/<asset>.fbx` for its named UCX collision, or its JSON box data. Those FBX files intentionally carry **neutral preview materials**, not the GLB appearance. Retain the GLB materials when bringing in collision; avoid a whole-mesh auto hull over the gate, open box or furniture gaps. Files are prepared, not certified through an Unreal import on this session.

All dimensions/metadata are in metres with Blender Z up. Verify the engine's centimetre conversion on one import. Source scales are retained except the reconstructed wearables, fitted to Villager. Ground pivots need a negative capsule-half-height mesh offset when attached to a capsule-centred actor. The current GAME nuisance capsule is radius 40 cm / half-height 70 cm; a 1.9 m NPC would use roughly 0.737 uniform scale to fill its 1.4 m height. Choose actual species sizes in the game camera.

For the cleaned Villager at scale 1, place Hat at local **(0, -0.03, 1.898) m** and HeadMirror at **(0, -0.03, 1.65) m** in Blender's axes; the previews use these exact transforms. Convert axes when setting Unreal sockets/components. Parenting both to the Villager and scaling the parent keeps the fit.

The gate's unscaled passage is approximately **0.23 m wide × 0.285 m high** because the generator normalized the whole castle. Use about **7.72×** uniform scale for a 2.20 m high / 1.78 m wide passage, and scale its collision with it. This requires an actual player traversal test by GAME.

Weapon/Crook/Megaphone origins are at grips; Fenella uses a flight anchor. NPCs/ground dressing use ground contact. Shard uses a particle centre. Actor capsules, held objects, tiny particles and flying Fenella have explicit collision policies rather than unnecessary blocking hulls.

## Files and reproducibility

- [Cleanup results](../cleanup_results.json): source hashes, per-file actions, pivots and triangle counts.
- [Final validation](../validation.json) and [render diagnostics](clean/audit.json).
- `decimate.py` at repo root plus `ImportSource/_decimated/cleanup.py` implement the processing. Run Blender **3.3**: `blender.exe -b --python-exit-code 1 -P decimate.py`.
- Optional filters: append `-- --only-folder Fal`, or `-- --only SM_Hat_Straw,SM_HeadMirror`. Default processes the 30 current Hyper3D/Fal models plus the batch-2 Meshy shard; `_v1` is excluded.
- `python ImportSource/_decimated/validate.py` independently validates saved outputs with NumPy/Pillow.
- `blender.exe -b --python-exit-code 1 -P ImportSource/_decimated/_review/audit.py -- --output-subdir clean --collisions` refreshes four-angle previews and collision companions; `python ImportSource/_decimated/_review/sheets.py clean` refreshes contact sheets.
- `fit_check.py` renders wearable fits and tests openings/passage; `-- --checks-only` runs its ray tests without rendering. Initial/rejected diagnostic artifacts are historical; `clean/` is the current visual review.

## Per-file results

| File | Source triangles | Cleaned triangles | Pivot | Work performed |
| --- | ---: | ---: | --- | --- |
| `Fal/SM_Boulder.glb` | 40,000 | 8,000 | ground contact | Removed generated ground/pedestal geometry and capped exposed contact surfaces. Set origin at ground contact; retained source scale for explicit engine fitting. |
| `Fal/SM_CandyCrop.glb` | 40,012 | 8,000 | ground contact | Removed generated ground/pedestal geometry and capped exposed contact surfaces. Set origin at ground contact; retained source scale for explicit engine fitting. |
| `Fal/SM_CandyFence.glb` | 40,000 | 8,000 | ground contact | Removed generated ground/pedestal geometry and capped exposed contact surfaces. Set origin at ground contact; retained source scale for explicit engine fitting. |
| `Fal/SM_CandyTree.glb` | 40,000 | 8,000 | ground contact | Removed generated ground/pedestal geometry and capped exposed contact surfaces. Set origin at ground contact; retained source scale for explicit engine fitting. |
| `Fal/SM_CardboardBox.glb` | 40,000 | 8,000 | ground contact | Set origin at ground contact; retained source scale for explicit engine fitting. |
| `Fal/SM_Celery.glb` | 40,008 | 8,000 | ground contact | Set origin at ground contact; retained source scale for explicit engine fitting. |
| `Fal/SM_Clapperboard.glb` | 40,000 | 8,000 | ground contact | Retained textured striped clapper/hinge and rebuilt the blank slate without limbs, stubs or floating glove fragments. Set origin at ground contact; retained source scale for explicit engine fitting. |
| `Fal/SM_Crook.glb` | 40,000 | 7,998 | hand grip | Removed generated ground/pedestal geometry and capped exposed contact surfaces. Set origin at hand grip; retained source scale for explicit engine fitting. |
| `Fal/SM_DirectorsChair.glb` | 40,000 | 8,000 | ground contact | Set origin at ground contact; retained source scale for explicit engine fitting. |
| `Fal/SM_FortGate.glb` | 39,992 | 8,000 | ground contact | Removed embedded knight and opened a continuous arched passage through the gate. Set origin at ground contact; retained source scale for explicit engine fitting. |
| `Fal/SM_Hat_Straw.glb` | 39,998 | 2,400 | Villager head-top mount | Rebuilt a complete broad-brim hollow straw hat without the head, using the embedded straw atlas and a Villager-sized crown. Set origin at Villager head-top mount; retained source scale for explicit engine fitting. |
| `Fal/SM_HeadMirror.glb` | 40,000 | 1,164 | Villager head-centre mount | Rebuilt a continuous hollow headband and perforated mirror disc without the head, sized to the actual Villager head. Set origin at Villager head-centre mount; retained source scale for explicit engine fitting. |
| `Fal/SM_HouseFront_Fal.glb` | 40,000 | 8,000 | ground contact | Removed generated ground/pedestal geometry and capped exposed contact surfaces. Set origin at ground contact; retained source scale for explicit engine fitting. |
| `Fal/SM_LeafMegaphone.glb` | 40,000 | 456 | hand grip | Rebuilt the rolled leaf as a continuous hollow cone with handle and raised veins, without the interior face/ornament. Set origin at hand grip; retained source scale for explicit engine fitting. |
| `Fal/SM_LightStand.glb` | 40,000 | 8,000 | ground contact | Retained textured lamp housing/barn doors, removed wired lower assembly, and rebuilt a clean cable-free tripod and pole. Set origin at ground contact; retained source scale for explicit engine fitting. |
| `Fal/SM_Molar.glb` | 40,000 | 8,000 | ground contact | Removed generated ground/pedestal geometry and capped exposed contact surfaces. Named enamel material for independent plaque/treatment material setup. Set origin at ground contact; retained source scale for explicit engine fitting. |
| `Fal/SM_Peppermint_Fal.glb` | 40,000 | 8,000 | ground contact | Removed generated ground/pedestal geometry and capped exposed contact surfaces. Set origin at ground contact; retained source scale for explicit engine fitting. |
| `Fal/SM_Rat_Fal.glb` | 40,000 | 25,000 | ground contact | Set origin at ground contact; retained source scale for explicit engine fitting. |
| `Fal/SM_Signpost.glb` | 40,000 | 8,000 | ground contact | Removed generated ground/pedestal geometry and capped exposed contact surfaces. Set origin at ground contact; retained source scale for explicit engine fitting. |
| `Fal/SM_Table.glb` | 40,000 | 8,000 | ground contact | Removed all four separate chair components; retained only the table and its four legs. Set origin at ground contact; retained source scale for explicit engine fitting. |
| `Hyper3D/SM_Almond.glb` | 500,000 | 25,000 | ground contact | Set origin at ground contact; retained source scale for explicit engine fitting. |
| `Hyper3D/SM_Fenella.glb` | 500,000 | 25,000 | flight anchor | Set origin at flight anchor; retained source scale for explicit engine fitting. |
| `Hyper3D/SM_Goblin.glb` | 500,000 | 25,000 | ground contact | Separated texture-derived material regions without changing atlas pixels: {"Skin": 15990, "Teeth": 1890} Set origin at ground contact; retained source scale for explicit engine fitting. |
| `Hyper3D/SM_HouseFront_Rodin.glb` | 500,000 | 8,000 | ground contact | Set origin at ground contact; retained source scale for explicit engine fitting. |
| `Hyper3D/SM_LollyBug.glb` | 500,000 | 25,000 | ground contact | Separated texture-derived material regions without changing atlas pixels: {"Wings": 4887} Set origin at ground contact; retained source scale for explicit engine fitting. |
| `Hyper3D/SM_Peppermint_Rodin.glb` | 500,000 | 8,000 | ground contact | Set origin at ground contact; retained source scale for explicit engine fitting. |
| `Hyper3D/SM_Rat_Rodin.glb` | 500,000 | 25,000 | ground contact | Set origin at ground contact; retained source scale for explicit engine fitting. |
| `Hyper3D/SM_Sword.glb` | 500,000 | 8,000 | hand grip | Lengthened and broadened blade relative to grip, retaining original overall height and atlas UVs; no paid regeneration. Set origin at hand grip; retained source scale for explicit engine fitting. |
| `Hyper3D/SM_Toothbrush.glb` | 500,000 | 8,000 | hand grip | Set origin at hand grip; retained source scale for explicit engine fitting. |
| `Hyper3D/SM_Villager.glb` | 500,000 | 25,000 | ground contact | Set origin at ground contact; retained source scale for explicit engine fitting. |
| `Meshy/SM_LollyShard.glb` | 156 | 148 | ground contact | Removed the white lollipop stick, keeping the pink low-poly candy fragment. Set origin at ground contact; retained source scale for explicit engine fitting. |
