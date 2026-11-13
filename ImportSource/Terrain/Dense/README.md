# Dense candy ecology

This pass supersedes the sparse first terrain layout. It was requested by Dylan:
dense low-poly grass and foliage, candy-floss bushes, peppermint trees, PCG and
snow, with clear routes and quest areas.

The actual executed graph is `/Game/World/Dense/PCG/PCG_CandyEcology`.
Seeded, exclusion-tested placement points feed 21 Static Mesh Spawner branches.
The graph was run in a full Unreal editor session, then ClearPCGLink baked its
HISM output into `L_Sugarborough`. The manual/editor-only source volume remains
for editing; the game does not need editor PCG generation.

| Layer | Instances |
| --- | ---: |
| Wind-driven mint, rose and ice grass | 76,148 |
| Peppermint birches, blossom trees and frosted candy-cane firs | 1,050 |
| Cotton-candy thickets | 2,102 |
| Flowers, ferns, jellybean bushes and gummy succulents | 4,113 |
| Shader-animated powdered-sugar snowflakes | 2,590 |
| Total | 86,003 |

Grass is an original 42-triangle tuft per variant. Cotton-candy thickets have a
480-triangle source budget; snowflakes use four triangles. The seven imported
Rodin models retain their 1,398–1,476-triangle source budgets and 1k albedo
textures. The spiky willow is held out. No additional API credits were spent.

Small plants/grass/snow cull between 35 and 60 metres; trees and thickets between
100 and 160 metres. Small foliage casts no dynamic shadow. All vegetation is
nonblocking. This is a performance-conscious starting point, not a benchmark:
the final follow camera and crowded combat still need gameplay testing.

The molar biome has powdered-sugar ground, ice grass, frosted firs and a spatial
snowfall layer. Snow and slow grass wind are animated in material WPO. Snowflakes
have expanded displacement bounds. A daylight ambient cubemap and modest grass
fill avoid the black blade backs seen in the initial iteration.

## See it

`review_maps.json` records current review packages. The new maps are:

- `/Game/World/Review/L_SweetMeadow`
- `/Game/World/Review/L_CandyGrove`
- `/Game/World/Review/L_SugarSnow`
- `/Game/World/Review/L_CandyExplorer`

Explorer uses Unreal's native free-look DefaultPawn: WASD and mouse, with E/Q or
Space/Ctrl for height. It is an environment preview. GAME's follow camera,
quests and Open autoplay are separate O2–O9 work.

The normal main map remains `/Game/World/L_Sugarborough`; the project default
map has not been switched. O1 is now merged: native `-open -notitle` can be
tested on the real map. It must log "terrain map" rather than "flat fallback".

## Rebuild

1. `python -I ImportSource/Terrain/Dense/build_ecology.py`
2. Blender 4.5 background:
   `--python-exit-code 1 --python ImportSource/Terrain/Dense/export_blender.py`.
3. Full editor:
   `-ExecutePythonScript=<absolute path>/Dense/build_unreal.py -unattended -nullrhi`.
   This really executes PCG and bakes the result. Check `engine_report.json`.
4. Python commandlet `Dense/capture_maps.py` creates fresh player-height camera
   copies and the explorer map. Unique names avoid overwriting loaded worlds.
5. `python -I ImportSource/Terrain/Dense/validate_sources.py`, then full-editor
   `review_entry.py` for reload, instance counts and collision checks.
6. Commandlet parent `label_maps.py` refreshes the AlwaysCook label.
7. Parent `run_review.ps1 -View DenseGrove|DenseSnow|Explore` renders current
   maps. Add `-PackagedExe <real exe>` to check the package. Main-map native
   gameplay startup uses `-MainMap -StartPlaying`.
8. Build/tests, package, Backlot smoke and size/CRC verification as recorded in
   `HANDOVER.md` and the delivery reports.

The source generator preserves village/camp/set/giver/signpost exclusions and
road margins. Ground collision remains the existing terrain. Reload validation
confirmed all 86,003 saved instances, 79 ground samples, three hero-sized routes
and twelve boundary checks. The engine build and 57 Jam tests passed.

See `HANDOVER.md` for exact current state, remaining checks and known Unreal
Python API pitfalls. The dense Windows package succeeds; its CRC-verified local
archive is 613,096,338 bytes. Current packaged grove/snow previews are in
`Review/packaged-grove.png` and `Review/packaged-snow.png`. The earlier sparse
world's 575 MB archive and screenshots are historical.
