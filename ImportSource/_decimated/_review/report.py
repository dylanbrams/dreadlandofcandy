import json
from pathlib import Path
out=Path(__file__).resolve().parent
audit=json.loads((out/'audit.json').read_text())
comparisons={r['file']:r for r in json.loads((out/'comparison.json').read_text())}
notes={
'SM_Boulder': 'Face and pink pedestal are present. Remove the pedestal if this should sit naturally in terrain; face is an art choice, not a topology defect.',
'SM_CandyCrop': 'Brown ground patch and gumdrops attached to the stems. Keep only if the whole crop patch is wanted; otherwise separate the plant and set its ground pivot.',
'SM_CandyFence': 'Small white feet under the posts. No obvious silhouette failure at review resolution. Use box collision on posts/rails and check end-to-end tiling; do not block intended openings with one hull.',
'SM_CandyTree': 'Grass/soil platform around trunk. Remove or retain deliberately; collision should follow trunk rather than canopy/platform detail.',
'SM_CardboardBox': 'Open top and flaps survive, but the box has a face and label/pink details. Preserve the opening during repair. Use simple wall collision only if the interior must remain accessible.',
'SM_Celery': 'Face and bundled stalks remain. Decide whether the face belongs; thin leaves need game-camera readability checks. A voxel solid rebuild risks joining the leaves.',
'SM_Clapperboard': 'Arms, gloves, legs and shoes make this a character-shaped prop. Remove/separate them if a plain clapperboard is required. Split the upper clapper and give it a hinge pivot only if it must animate.',
'SM_Crook': 'Pink pedestal and pale collar are present. Remove pedestal, close the cut, and place origin at the hand grip. Preserve the hook opening.',
'SM_DirectorsChair': 'Seat, back and crossed frame read well in these views. Keep openings between legs. Folding would need separate parts and hinge pivots; unnecessary for a static set prop.',
'SM_FortGate': 'BLOCKER: knight figure occupies the arch. Remove it, patch the exposed attachment, and verify clear player width/height. Use separate tower/wall collision shapes so the doorway remains passable.',
'SM_Hat_Straw': 'BLOCKER: includes a full head and neck. Isolate hat, repair its underside, fit it to the actual Villager head and set an attachment pivot. Thin straw fringe is vulnerable to decimation.',
'SM_HeadMirror': 'BLOCKER: includes head/neck. Isolate band and mirror, repair attachment surfaces, preserve mirror hole, fit to Villager and set an attachment pivot.',
'SM_HouseFront_Fal': 'Full house with deep roof, side walls, chimney and ground platform, rather than a shallow film-set facade. Needs substantial separation/cutting if the facade brief remains. Cutting must preserve a useful back and thickness.',
'SM_LeafMegaphone': 'Small smiling object inside the mouth and a handle are visible. Remove the interior ornament if unwanted, preserve the hollow mouth, and set a grip pivot. Avoid solid remeshing that fills the cone.',
'SM_LightStand': 'Long trailing cable reaches well beyond tripod. Separate/remove if unwanted; exclude it from gameplay collision. Pivot at tripod feet; actual light/beam requires an Unreal light or effect.',
'SM_Molar': 'Face, pink pedestal and steps are present. For a tooth pillar, remove the stand if unwanted. Independent plaque/tooth colour changes need a material mask or separate region; they cannot be assumed from the single material.',
'SM_Peppermint_Fal': 'Red pedestal and thick bowl-like candy rim remain. Remove stand if used as a loose particle. Consider the already much smaller Meshy variant described in the handoff instead of spending 8k triangles per particle.',
'SM_Rat_Fal': 'Tail protrudes prominently and arms lie close to sides. Preserve tail tip and ear rims. Static actor can use capsule collision; articulated motion would need a separate rigging task.',
'SM_Signpost': 'Grass/soil tile is attached. Separate if it should plant directly in terrain. Blank front is suitable for a separate text/decal surface; board pivot and readable text size need placement checks.',
'SM_Table': 'BLOCKER for table-only use: all four chairs are included. Separate/remove chairs and patch contact cuts; give separated furniture its own ground pivot and simple collision. Nine UV loops extend outside 0-1: inspect sampler/padding before changing them.',
'SM_Almond': 'NPC budget corrected to 25k. Added arms and mouth remain, as already noted in handoff. One collapsed UV triangle needs inspection after corrected decimation. Keep current silhouette unless an art change is requested.',
'SM_Fenella': 'Thin limbs, four wing lobes, face and held camcorder survive at thumbnail scale. Wings and limbs become more jagged than the source; prioritize repair here. White wing streaks also exist in the original render, so do not label all bright wing marks as new cracks. Preserve hovering pose and wing thickness; inspect at actual game-camera size.',
'SM_Goblin': 'Grin and yellow teeth read clearly. Teeth are not independently addressable through separate materials. Add a tooth/plaque mask or regions for brushing/gameplay tint, and a body mask for boss tint if the tunic should stay cream.',
'SM_HouseFront_Rodin': 'Shallow facade is much closer to the brief than fal house; measured depth is about 0.27 m for a roughly 1.9 m tall normalized mesh. Both opposing views look facade-like/textured. A render does not certify exact back planarity: inspect/flatten or rebuild back only if the film-set reveal requires it.',
'SM_LollyBug': 'NPC budget corrected to 25k. Has insect abdomen, antennae and broad wings rather than a literal stick body. Keep unless an art revision is wanted. If wings must move separately, isolate them or supply a vertex-colour deformation mask and wing-root pivots.',
'SM_Peppermint_Rodin': 'Four wrapper tips and hollow-looking torn ends remain. Excessive complexity for tiny particles; prefer simpler variant or make a dedicated particle LOD. Do not fill intentional wrapper openings indiscriminately.',
'SM_Rat_Rodin': 'Tail curls upward behind body with pale tip; inspect silhouette against head at game angle. One shared nonmanifold edge appears in the tolerance-welded diagnostic and needs confirmation after the decimation redo; it could be a near-contact merge.',
'SM_Sword': 'Blade silhouette survives but is narrow relative to the oversized brief. Three near-zero-area faces flagged after diagnostic weld. Review blade edge/tip after redo, normalize scale, and put origin at grip for the hero socket; check hands/attack reach in Unreal.',
'SM_Toothbrush': 'Bristle rows survive but have fine ragged edges; thin shaft must survive at game size. Two near-zero-area faces flagged after diagnostic weld. Grip pivot/socket orientation and a tooth-contact point need in-engine setup.',
'SM_Villager': 'Body is usable as a static actor silhouette. Accessories need fitting to this actual head/body, not the unrelated heads generated with Hat/HeadMirror. Feet need a consistent placement pivot and actor capsule.',
}
text='''# Decimated mesh review — 2026-11-12

Reviewed all 30 current Hyper3D/Fal outputs with four textured Blender 3.3 views each (120 images), geometry/UV diagnostics, and a source/export connectivity and base-colour pixel comparison. `_v1` and Meshy assets are outside this review. This is a review, not an Unreal acceptance test. Originals were not modified; no commit was made.

## Priority 0: redo the first decimation pass

**All 30 outputs gained open geometric edges. Do not treat this batch as production-ready.** All source primitives have zero boundary edges after exact-coordinate welding; exported results have 2,691–23,334. The first script decimated glTF attribute-split vertices independently. UV layers and image data survived, but geometry continuity did not. Earlier UV-layer validation was insufficient.

The count is geometric boundary edges, not a count of visible holes or texture seams. A separate Blender check welds near-coincident vertices at one millionth of maximum extent; the large boundary counts persist. Ordinary glTF UV/normal splits alone do not explain this. Disconnected components may include legitimate parts and are not automatically junk to delete.

In-memory tests on source **Boulder** and **Fenella** weld before Collapse, reaching **8,000** and **25,000** triangles respectively with **zero boundary edges**, **zero edges shared by more than two faces**, and one retained UV layer. See [seam_probe.json](seam_probe.json). This validates a candidate geometry correction only: it has not been applied to the 30 exported GLBs, and texture interpolation/custom-normal shading must still be checked after re-export. Rebuild from originals; welding the already cracked outputs is not sufficient.

Do not apply blanket hole filling, voxel solidification, or automatic deletion of small components: wing sheets, bristles, wrapper folds, furniture gaps, accessory holes, and the megaphone opening can be intentional. Restore connected source geometry first, then apply asset-specific cleanup.

## Textures, UVs and shading

- All 30 exported embedded base-colour images are **pixel-identical** to their sources. See [comparison.json](comparison.json). Image preservation does not prove that UV interpolation on reduced triangles matches the original surface.
- One UV layer remains on every reviewed mesh. Almond has one near-zero-area UV triangle; Table has nine UV loops slightly outside 0–1. These are inspection flags, not evidence of visible failure by themselves. No exhaustive UV-overlap test was performed.
- Five near-zero-area geometric faces are flagged after the temporary tolerance weld: Sword 3, Toothbrush 2. No inconsistent shared-edge winding was found. Rat_Rodin has one shared nonmanifold edge in that same temporary diagnostic; confirm it on the corrected geometry.
- Texture images are 2048 or 4096 pixels square. Choose streaming/compression and smaller textures/LODs based on screen size, especially small particles; the requested 8k/25k mesh targets are retained here, not final performance certification.
- Most large surfaces read coherently in these renders, but small views and clipped pale highlights can hide subtle seams. Fenella source and reduced renders were compared directly: the source also has bright wing streaks. The reduced wings/limbs are more jagged. Texture seam sign-off requires close views after geometry correction, including the actual Unreal material and mip levels.

## Per-asset cleanup beyond the common decimation defect

“Remove” below identifies work needed for the stated functional use; previously accepted decorative/style choices remain art decisions. Known handoff issues were checked against the current rendered meshes. Every row also requires Priority 0 correction.

| Asset | Findings / next work |
| --- | --- |
'''
for row in audit:
    name=Path(row['file'].replace('\\','/')).stem
    text+=f"| `{row['file']}` | {notes[name]} |\n"
text+='''
## Placement and Unreal work

- Fal meshes have their minimum Z near zero and origins at zero: they already approximately sit on the ground. Rodin meshes are centered vertically, with most minimum Z values near −0.95 m. Standardize placement deliberately; do not blindly move every origin. Set weapon/accessory pivots at grips or attachment points, ground props at the base, and Fenella at the intended flight anchor.
- These are generator-normalized dimensions, not a consistent real-world scale. Set character height first, then weapon reach, accessory fit, furniture dimensions and gate clearance. GLB uses metres; verify Unreal's import scale in a scratch scene before broad import.
- All reviewed GLBs are static (no skins or animation clips), and every mesh uses one material. The current brief permits static NPC movement; rigging is only needed if articulated motion is desired. Teeth/plaque, boss tint, wing deformation and independently moving clappers require masks, vertex data, separate parts or additional material regions.
- Set simple gameplay collision intentionally: actor capsules for NPCs, boxes for furniture/fences, separate gate sides/top, and simple weapon traces. Do not accept a gate-spanning convex hull or collision on every leaf/bristle/cable. No Unreal collision assets or gameplay fitting were created or tested here.
- Recheck normals/tangents, material alpha/two-sided settings, mip seams, LOD popping and actual camera readability after corrected export/import. Test self-shadowing on thin wings and bristles. No exhaustive self-intersection test, deformation test, UV-overlap certification, baked-lightmap validation or packaged performance test was performed.

## Evidence

- [Detailed Blender diagnostics](audit.json) — UVs, dimensions, origins, connected components, geometry flags.
- [Source/export comparison](comparison.json) — exact-weld boundary counts, texture pixel hashes, skin/animation counts.
- [Candidate correction test](seam_probe.json) — no exported mesh changes.
- Direct matching-camera comparisons: [Fenella](SM_Fenella_comparison.jpg), [Sword](SM_Sword_comparison.jpg). Source views are on the left, current decimated views on the right.
- Four-angle contact sheets (oblique front, opposing side, side, overhead):
'''
for i in range(1,7): text+=f'  - [Sheet {i}](sheet_{i:02d}.jpg)\n'
text+='''
## Measured geometry

Source exact-weld boundary count is zero for every row. Export exact-weld and temporary tolerance-weld counts differ slightly because the latter also merges very close points. Neither method treats mere duplicated UV/normal vertices as holes if their geometric edges still match.

| File | Triangles | Export boundary edges (exact weld) | Boundary edges (tolerance weld) | Dimensions XYZ (m) | Minimum Z (m) |
| --- | ---: | ---: | ---: | --- | ---: |
'''
for row in audit:
    c=comparisons[row['file']]
    triangles=sum(o['triangles'] for o in row['objects'])
    exact=sum(p['exact_weld_boundary_edges'] for p in c['decimated']['primitives'])
    welded=sum(o['welded_boundary_edges'] for o in row['objects'])
    dims=' × '.join(f'{v:.3f}' for v in row['dimensions_m'])
    text+=f"| `{row['file']}` | {triangles:,} | {exact:,} | {welded:,} | {dims} | {row['bounds_min_m'][2]:.3f} |\n"
text+='\nLollyBug and Almond were corrected from the first pass’s prop classification to the requested 25k NPC target before this review. Their reviewed outputs still have the common seam-connectivity defect. The decimation script’s classifier now includes them; its geometry step still needs the Priority 0 correction before reuse.\n'
(out/'INITIAL_REVIEW.md').write_text(text,encoding='utf-8')
print('Initial review:',out/'INITIAL_REVIEW.md')
