# Dense candy-world handover

## Superseding art pass - Nov 13

Current terrain work is in `../Polish/README.md`. It adds striped grass,
eleven smoother tree variants (five snow pines), fibrous cotton-candy materials, three
detailed candy-house variants and four song-inspired landmarks with five
bluebirds. Main map remains `/Game/World/L_Sugarborough`. The current baked
graph is `/Game/World/Polish/PCG/PCG_CandyEcology`, with **85,091 instances**;
912 older instances were cleared for the new landmark footprints.

Use `../Polish/review_maps.json` for current camera maps and
`../Polish/Review/` for current editor-game captures. Editable Blender files
are in `../Polish/Blend/`. All 20 new meshes have three LODs. Source hashes,
retained transforms, quest/road/landmark exclusions and saved-map collision
checks pass. `../Polish/performance.json` records limited stationary
measurements; it is not a gameplay or packaged benchmark.

The latest refinement adds two original pine silhouettes, lit translucent
fountain crystals, and four partly submerged lemon slices in a translucent
spring surface. `../Polish/refinement_report.json` records the targeted checks.

The package and ZIP described below predate this art pass. No new packaging,
push, upload or paid generation is claimed. The remainder of this document
is the previous dense pass's history. Rebuilding that original Dense pipeline
replaces the newer foliage, so apply the Polish pipeline afterward.

## User direction and authority

Dylan wants a compact, lush low-poly candy world: dense grass and foliage,
candy-floss thickets, peppermint trees, PCG and snow. Village at origin, three
biomes, outdoor fort, future cave entrance. He explicitly granted TERRAIN the
engine window after the earlier request to keep it closed. No push or upload
is authorized.

## Current state

- Main map: `/Game/World/L_Sugarborough`. Default project map unchanged.
- PCG graph: `/Game/World/Dense/PCG/PCG_CandyEcology`.
- Graph actually executed and its HISM output baked via ClearPCGLink:
  **86,003 instances / 21 components**: 76,148 grass, 1,050 trees, 2,102
  candy-floss thickets, 4,113 smaller plants, 2,590 animated sugar snowflakes.
- Seven Hyper3D models are now imported and placed. The spiky willow was held
  out. The earlier batch cost exactly four credits; this density pass spends no
  additional generator credits.
- New original meshes: three 42-triangle grass tufts, a 480-triangle candy-floss
  thicket and a four-triangle snowflake. Wind and snowfall use material WPO.
- Powdered-sugar ground covers molar country. A daylight ambient cubemap and
  gentle grass emissive fill replace the initially black-looking blade backs.
- Main PCG source volume is editor-only/manual; baked instances persist without
  runtime PCG generation. All foliage is nonblocking and distance-culled.
- Reload checks passed: all 86,003 instances, 79 ground samples, three
  hero-sized approaches, twelve boundary samples. See parent
  `engine_validation.json` and `review_run.json`.
- Editor build and all **57 Jam tests passed** (log
  `Saved/Logs/DenseEcology-Tests.log`). GAME merged O1 while this work was in
  progress: main head was `47187bf`. O2 follow camera and O4 quests/bot are
  still GAME work. Do not claim an Open quest autoplay pass.

## Visual evidence and preview maps

`Dense/review_maps.json` is authoritative:

- DenseMeadow: `/Game/World/Review/L_SweetMeadow`
- DenseGrove: `/Game/World/Review/L_CandyGrove`
- DenseSnow: `/Game/World/Review/L_SugarSnow`
- Explore: `/Game/World/Review/L_CandyExplorer`

Explorer uses the native DefaultPawn free-look controls: WASD/mouse, E/Q or
Space/Ctrl for vertical movement. It is an environment preview, not a new
character or quest implementation.

The grove was inspected in-engine and copied to `Dense/Review/editor-grove.png`.
Other editor screenshots are in `Saved/Screenshots/WindowsEditor/` named
`terrain-densemeadow.png`, `terrain-densesnow.png`.

The first dense meadow showed black/chunky grass. This was corrected by finer
blades, slow wind, slight emissive fill and ambient cubemap lighting. Do not use
the first capture as current evidence. The latest grove view is the best
close-up of the revised vegetation.

## Verified delivery checkpoint - Nov 13

The resumed snow capture and normal Windows package succeeded. Actual packaged
Grove, Snow and Explorer captures were inspected; native Open startup reports
the terrain map and all 19 locations. Current camera in native Open mode is
still the old fixed camera: GAME O2 must replace it. This is not a passing
Open quest loop or manual playthrough.

Packaged Backlot's 70-second smoke passes all seven checks with three-scene
autoplay PASS. Archive `Builds/TerrainValidation.zip`: **613,096,338 bytes**,
CRC verified, below 1 GB. Nothing uploaded or pushed. Current evidence is in
`Review/packaged-grove.png`, `packaged-snow.png`, `packaged-explorer.png` and
`packaged-main.png`. Parent `delivery_report.json` records checks and limits.

TERRAIN releases the engine after this checkpoint. Read worker notes before
any new launch. A fresh context can start with: "Continue TERRAIN from
ImportSource/Terrain/Dense/HANDOVER.md. Review the packaged grove and snow
captures, then improve traversal/readability with the current GAME framework."
Next meaningful work is human visual/traversal feedback, density tuning and
validation with GAME's follow camera once O2 lands. Preserve village, road,
camp and film-set exclusions. Never claim current native quests exist.

## Reproduction and known API traps

1. `python -I ImportSource/Terrain/Dense/build_ecology.py`
2. Blender 4.5 background, `--python-exit-code 1 --python .../Dense/export_blender.py`.
3. Full Unreal editor, not commandlet:
   `-ExecutePythonScript=.../Dense/build_unreal.py -unattended -nosplash -nullrhi`.
   This runs the PCG executor and bakes instances.
4. Commandlet `Dense/capture_maps.py` creates fresh camera copies and an
   explorer map. Fresh unique package names avoid overwriting loaded worlds.
5. Full editor `review_entry.py` runs collision checks. It keeps Python alive
   across ticks and waits for map physics registration. Dense validation skips
   resaving the old sparse review maps; those packages may already be loaded.
6. Commandlet parent `label_maps.py` refreshes AlwaysCook label.

VertexColor RGB output and unary Sine/Frac input pins use the empty string.
PCG graph nodes are read via `get_editor_property("nodes")`; there is no Python
`get_nodes()`. Remove nodes one at a time: reflected `remove_nodes()` treats its
non-const array reference as an output and accepts no arguments.
Blender FBX export mirrors Y and flips winding to compensate for Unreal import.
Keep this convention. PCG graph rebuilding is independent of generation API
credits. Dense source/engine JSON reports must reflect the current pass, not old
successful runs.

Source gameplay code, shared config and default maps were not changed by this
dense pass. Other workers' edits remain in this shared checkout. Read
`Docs/Workers/terrain.md` for engine ownership before launching anything.
