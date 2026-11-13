"""Build the handoff from validated exported files and completed visual checks."""
import json,hashlib
from pathlib import Path
OUT=Path(__file__).resolve().parent
ASSETS=OUT.parent
validation=json.loads((ASSETS/'validation.json').read_text(encoding='utf-8'))
results=json.loads((ASSETS/'cleanup_results.json').read_text(encoding='utf-8'))
audit=json.loads((OUT/'clean/audit.json').read_text(encoding='utf-8'))
assert len(validation)==31 and all(row['status']=='PASS' for row in validation)
for row in validation:
    assert hashlib.sha256((ASSETS/row['file']).read_bytes()).hexdigest()==row['output_sha256'],'Validation is stale: '+row['file']
assert len(audit)==31
old=OUT/'REPORT.md'
if old.exists() and not (OUT/'INITIAL_REVIEW.md').exists():
    (OUT/'INITIAL_REVIEW.md').write_text(old.read_text(encoding='utf-8'),encoding='utf-8')
before=sum(row['triangles_before'] for row in results.values())
after=sum(row['triangles_after'] for row in results.values())
fbx=list((ASSETS/'_collision').glob('Fal/*.fbx'))+list((ASSETS/'_collision').glob('Hyper3D/*.fbx'))
for path in fbx:assert path.read_bytes().startswith(b'Kaydara FBX Binary')
text=f'''# Cleaned asset handoff — 2026-11-12

**31 cleaned GLBs are ready for GAME's import pass.** The original 30 Hyper3D/Fal models and batch-2 Meshy candy shard are included. Originals and `_v1` files remain untouched. No commits, paid generation requests, Unreal imports, game-code edits or engine configuration changes were made by this cleanup session.

Batch 2 was already complete: all eight 3D jobs and sixteen UI jobs were marked done. Its eight existing meshes have now been cleaned; generation was not repeated. The old first-pass decimation defect is fixed in the current staged outputs. [Initial review](INITIAL_REVIEW.md) documents the superseded results.

## Validation

- **31/31 PASS** in [independent saved-GLB validation](../validation.json): closed geometric edges across material splits, no edges shared by more than two faces, no collapsed geometric triangles, finite positions/normals/UVs, embedded base-colour images, triangle budgets, and unchanged original SHA-256 hashes.
- Original embedded base-colour image pixels are retained on all 31 models. New cut surfaces use matching solid materials. The rebuilt hat, mirror and megaphone sample the original embedded atlas as paint; their new shapes and UVs are intentional replacements for fused-head/ornament failures, not a claim of identical source UV layout.
- Total geometry: **{before:,} → {after:,} triangles** ({100*(1-after/before):.1f}% reduction). Characters are 25,000 each. Most other models are 8,000; deliberately simple repaired props remain below budget.
- Four-angle renders cover all 31 current outputs (124 images). [Wearable fit previews and ray tests](fit/fit_results.json) check the hat underside, mirror hole, and five clear rays through the gate passage. Both wearable openings and all portal rays pass.
- {len(fbx)} binary FBX companions contain named UCX collision boxes plus neutral preview geometry. Per-asset collision policies and dimensions are in `../_collision/<service>/<asset>.json`. Gate collision has separate sides/header and is checked not to span the passage.
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

'''
for i in range(1,8):text+=f'- [Sheet {i}](clean/sheet_{i:02d}.jpg)\n'
text+='''
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
'''
for rel,record in sorted(results.items()):
    pivot=record['objects'][0]['pivot_purpose']
    actions=' '.join(record['actions'][1:])
    text+=f"| `{rel}` | {record['triangles_before']:,} | {record['triangles_after']:,} | {pivot} | {actions} |\n"
(OUT/'REPORT.md').write_text(text,encoding='utf-8')
(ASSETS/'HANDOFF.md').write_text('''# Cleaned assets

Current report: [_review/REPORT.md](_review/REPORT.md).

Visual GLBs: `Hyper3D/`, `Fal/`, and batch-2 shard in `Meshy/`. Collision companions: `_collision/`. Current preview sheets: `_review/clean/`. Raw originals remain outside this directory.

The first decimation pass is superseded. Follow the current report for validated results, wearable placement, collision import and reproducibility. No Unreal content was imported by ASSETS.
''',encoding='utf-8')
print('Final handoff written:',len(validation),'PASS;',len(fbx),'collision FBX files')
