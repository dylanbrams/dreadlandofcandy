# Candy terrain polish and song landmarks

Main world: `/Game/World/L_Sugarborough`. New runtime content lives under
`/Game/World/Polish`; `review_maps.json` records current camera copies.

## Snow pine and translucent spring refinement

There are now **five snow-pine varieties** and **eleven tree varieties**
overall. The two new original shapes are a tall six-tier spiral taffy pine
(9,556 triangles) and a shorter three-tier marshmallow umbrella pine (7,792
triangles). They retain the same conservative footprint and replace some
existing fir instances. Foliage count and retained transforms stay unchanged.
`PineVarieties` is a separate asset lineup: spiral on the left, umbrella on
the right. It adds no presentation props to the gameplay map.

Fountain crystals now use lit translucency, stronger grazing-angle opacity
and mild refraction. The lemonade has a single translucent surface over an
opaque basin floor, with animated ripple normals. Four partly submerged lemon
wheels float inside the basin; the upright wheel beside it has been removed.
`slice_validation.json` inspects the saved Blender geometry and confirms four
separate rinds, all inside the basin and intersecting the liquid surface.
The former lemon-wheel bluebird is perched on the tallest spring crystal.

All **20 meshes** have three LODs. Current renders are in `Review/`; the three
earlier snow/fountain/spring captures are preserved in `Review/PriorRefinement`.
See `refinement_report.json` for the targeted checks and performance comparison.

Refinement medians on the local GTX 1660 Ti at 1600x1000 were 6.448 ms in
snow, 6.848/6.655 ms at the two fountain angles, and 7.144 ms at the spring.
The longer snow capture had an 8.291 ms p95. The spring still showed frame-time
spikes (50.35 ms p95) in its longer capture; its performance remains unresolved.
These measurements do not establish gameplay or packaged performance.

A preview-only override forcing synchronous shader compilation hit an engine
assertion. It has been removed; final captures used normal compilation and
completed without that assertion. The separate character-session animation
error and the conversation interruptions are not established as the same cause.

This pass adds red/cream diagonal blade stripes in snow, green/cream stripes
in meadow grass, and raspberry/cream stripes in rose patches. UVs keep the
stripes attached to each blade while it sways. Original 42-triangle grass and
anchored wind are retained.

Eleven actual Blender tree meshes introduce bent trunks, twisted crowns, and
different height/width proportions. One subdivision pass smooths the tree
surfaces. The original four-lobed cotton-candy bush gets two subdivisions and
small irregularities. Analytically filtered, intersecting thread patterns
and soft marbling add fiber detail; bush shading uses opaque subsurface.
The blossom material restricts fibers to pink atlas regions and preserves
the trunk. No translucent canopy layers or extra foliage shells are used.

Three 18,000-triangle house meshes derive from the full original Fal candy
house, retaining icing, striped trim, windows and texture detail. Squat,
tall and crooked proportions plus tinted trim vary the six village houses.
Their fronts face inward. Raw generated sources are unchanged; no paid
generation was used. Every new asset has an editable file in `Blend/`.

## Song scenery

Inspired by [Harry McClintock's song](https://www.locustreview.com/online/big-rock-candy-mountain).
These are scenic objects, with shallow decorative liquid surfaces and the
existing terrain underneath. There are no new swimming, drinking, quest or
bird AI systems. Five bluebirds perch around the spring and village fountain.

| Destination | World XY (cm) | Appearance |
| --- | --- | --- |
| Crystal fountain | 650, 750 | Blue sugar crystals, arcing jets, sugar-rock basin |
| Lemonade spring | 4500, 4500 | Translucent lemonade, four floating lemon slices, bubbles and cascades |
| Stew pond | 6500, 7000 | Brown broth, carrots, peas, potatoes, noodles, giant spoon |
| Whiskey run | -6500, -500 | Amber stream through sugar rocks, oversized pouring bottle |

All four destinations preserve the published quest reservations; the three
wilderness destinations also pass the original road/location exclusion rule.
Foliage is cleared around their complete footprints, including tree crown
radius. The village fountain clears giver/signpost reservations.

The remaining **85,091 instances** comprise 75,332 grass, 1,026 trees,
2,071 bushes, 4,072 smaller plants and 2,590 snowflakes. Exactly 912 earlier
instances were removed for landmarks. Every retained transform matches the
previous ecology exactly. The graph still executes and bakes HISM output;
it is now `/Game/World/Polish/PCG/PCG_CandyEcology`.

## Validation and limits

`source_validation.json` checks raw source hashes, FBX signatures, editable
Blender files, finite geometry/placements, exact retained transforms,
triangle budgets and both gameplay and landmark exclusions.
`verification.json` records the reloaded map's actual instances, three LODs
per new mesh, nonblocking vegetation/landmarks, 79 ground samples, three
capsule approaches and twelve perimeter collision samples.

New trees, bushes, houses and landmarks have LOD0/1/2, targeting full, 45%
and 16–18% geometry. Existing HISM culling remains. `performance.json` and
`Review/timings-*.csv` record actual stationary editor-game measurements on
the local GTX 1660 Ti at 1600×1000. This measures a static camera, with
startup samples removed; it does not establish packaged traversal or combat
performance. These content changes have not been repackaged. The prior ZIP
and packaged screenshots describe the previous terrain.

The previous polish pass's grove median frame time was **5.336 ms**, compared with **4.798 ms**
for the earlier grove at the same camera. The other final scenic views had
medians of 5.17–7.438 ms and 95th percentile times up to 8.24 ms. These are
limited static editor measurements, not a full-game frame-rate guarantee.

## Reproduce

1. Blender background: `--python-exit-code 1 --python ImportSource/Terrain/Polish/prepare_blender.py`.
   Then `--python-exit-code 1 --python ImportSource/Terrain/Polish/prepare_pines.py` to add the two original pine shapes to the manifest.
2. Blender background: `--python-exit-code 1 --python ImportSource/Terrain/Polish/prepare_landmarks.py`.
3. Blender background: `--python-exit-code 1 --python ImportSource/Terrain/Dense/export_blender.py` for grass UVs.
4. During an available engine window, full Unreal editor with `-ExecutePythonScript=<absolute path>/Polish/build_unreal.py -unattended -nullrhi`.
5. Full editor with `-ExecutePythonScript=<absolute path>/Polish/verify_unreal.py -unattended -nullrhi` to reload/check and create fresh preview maps/cook references.
6. `python -I ImportSource/Terrain/Polish/validate_sources.py`.
7. `Polish/run_review.ps1 -View DenseGrove|DenseSnow|PolishVillage|CrystalFountain|LemonadeSpring|StewPond|WhiskeyRun` (one name at a time); then `python -I ImportSource/Terrain/Polish/summarize_performance.py`.

The original Dense generator remains available as a previous look. Rebuilding
it replaces the ecology; apply this Polish pass afterward to restore these
variants and landmark clearings. Gameplay code, default maps and shared config
are outside this content pass.
