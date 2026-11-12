# Sugarborough terrain sources

Original geometry and deterministic placement authored during the jam, 2026-11-12.
No purchased assets, generated bitmap art, or paid model generations used in this pass.
All source files and output stay inside TERRAIN's ownership surface.

## Reproduce

1. `python -I ImportSource/Terrain/build_sources.py`
2. `& 'D:/Program Files/Blender Foundation/Blender 4.5/blender.exe' -b -t 4 --python ImportSource/Terrain/export_blender.py`
3. During TERRAIN's exclusive engine window, run the project with `-run=pythonscript -script="<absolute path>/ImportSource/Terrain/build_level.py" -unattended -nosplash -nullrhi`. PCG and Python editor plugins must be enabled. This creates `/Game/World/L_Sugarborough`, meshes, materials and three PCG palette graphs. **Executed successfully.**
4. Run the project with `-ExecutePythonScript="<absolute path>/ImportSource/Terrain/review_entry.py" -unattended -nosplash -nullrhi`. This uses a full editor session because commandlets do not create a physics scene. Exact imported XY/scale, nonblocking scatter, and 79 ground collision/slope checks pass. Review maps are saved under `/Game/World/Review`; the script exits the editor afterward.
5. `& ./ImportSource/Terrain/run_review.ps1 -View Village` captures the actual level in-engine. `-View Overview`, `Meadow` and `Fort` select other camera maps. A `-PackagedExe <path>` argument runs the equivalent packaged check.
6. Run the project commandlet with `-run=pythonscript -script="<absolute path>/ImportSource/Terrain/label_maps.py" -unattended -nosplash -nullrhi`. This creates `/Game/World/PAL_Sugarborough`, an AlwaysCook primary asset label with explicit references to the main map and four review maps. Including a directory alone did not include its maps in this project's packaged build; the first packaged launch caught this. The cook label keeps the correction within TERRAIN's owned content.
7. `/Game/World` is included in `DirectoriesToAlwaysCook`, and runtime `PCG` is enabled (both explicitly permitted by BUILD_SPEC 3e). The current default map is unchanged. Package through `Scripts/package.ps1`, then verify a fresh packaged render.
8. GAME merges and verifies its O1 framework before the `-open` gameplay check. Never merge GAME's branch just to run the terrain importer. Once integrated, package and launch `MegaJam.exe /Game/World/L_Sugarborough -open`.

`Review/overview.png`, `village.png` and `meadow.png` are **Blender previews**. `Review/unreal-village.png` is the inspected editor game render. `Review/packaged-village.png` and `packaged-overview.png` are inspected renders from the actual Windows package (1600x1000). The PCG graphs carry the exact world-space seed points grouped by biome and mesh. The map's persistent actors bake those placements so shipping does not depend on editor PCG generation. Do not run those graphs on top of the already baked actors.

## Contract

Village `(0,0)`, 24,000 cm playable square, biome bearings 0/120/-120 degrees. Locations mirror `game/open-world` commit `442057a` without modifying that branch. All nine camps, three film squares (including corners), five givers, signpost and village are reserved. If GAME changes the world table, update `locations()` and regenerate; never silently use an outdated manifest.

Terrain is sixteen 2,048-triangle tiles. Twelve original prop meshes are each under 1,500 triangles; one original 1024-square ground texture supplies the biome colours and smooth paths. No prop textures are required. Scatter is nonblocking; buildings, fort dressing and the outer mountains block. Continuous hidden perimeter walls supplement the steep visible mountains. The cave is a decorative entrance for later content.

The source validator measures the height function and generated mesh triangles touching reserved areas. It also checks prop budgets and scatter exclusions. `engine_validation.json` records passing imported-scale checks, 79 ground samples, three hero-capsule approaches and twelve boundary traces. `review_run.json` records a fresh pass; the wrapper keeps Python alive and allows the loaded map's physics components to register before querying them. It clears the prior result at startup so a failed check cannot leave an apparently current success report.

Editor compile and all 54 `Jam.*` tests pass. Normal Windows packaging passes with the AlwaysCook label. The packaged main map launches with `-open`; the village and full-world camera maps render correctly. Packaged Backlot autoplay passes all three scenes and the 70-second smoke checks. `check_package.py` creates a local validation zip, excludes PDBs and runtime Saved directories, verifies CRCs and records the size: **574,901,156 bytes**, below the stricter decimal 1 GB limit. No upload was performed.

Full Open gameplay is **not verified**: GAME's O1 framework remains on its unmerged branch, and O2–O9 are separate GAME work. Launching the map is not evidence that those quests or the follow camera exist in main. `delivery_report.json` records this distinction. Performance still needs a human playthrough at the final follow camera.

The Y-axis reflection is compensated in the FBX exporter; collision flags and section collision are set before rebuilding each mesh's physics data.

## fal investigation (2026-11-12)

Verified from fal's own pages:

| Option | Published base price | Output / tradeoff |
| --- | --- | --- |
| [Hunyuan 3.1 Rapid text-to-3D](https://fal.ai/models/fal-ai/hunyuan-3d/v3.1/rapid/text-to-3d) | $0.225/model | Textured OBJ + MTL + texture. Fixed polygon output; still needs local reduction and conversion. Existing `Scripts/fal3d.py` expects GLB, so cannot consume this textured output unchanged. |
| [Hunyuan 2 Mini-Turbo](https://fal.ai/models/fal-ai/hunyuan3d/v2/mini/turbo) | $0.08/model | Image input, untextured white GLB. Suitable for silhouettes recoloured with our candy palette; needs a source image and reduction. |
| [Hunyuan Pro](https://fal.ai/hunyuan-3d) | $0.375/model | Textured GLB; optional features cost extra. The existing 40k-face pipeline adds custom-face-count expense. |

[Pro's older endpoint schema](https://fal.ai/models/fal-ai/hunyuan3d-v3/text-to-3d/api) exposes `LowPoly`, but its face-count floor is 40,000. That is still far above this project's 500–1,500-triangle scatter budget. [fal's family page](https://fal.ai/hunyuan-3d) quotes $0.75 for smart retopology, which can cost more than the generation itself. Cheap generation does not guarantee cheap runtime geometry.

Recommendation for later unique landmarks: one Rapid sample, reduce locally in Blender, then compare at the game camera before batching. For repeated gumdrops, candy trees, mountains, gates and wafer pieces, the original parametric meshes here give exact budgets, editable silhouettes, shared materials, and no API expense. No fal balance queries were made (Dylan's recorded preference).
