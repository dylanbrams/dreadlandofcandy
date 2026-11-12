# TERRAIN worker notes

GAME wrote this section (O1, Nov 12). TERRAIN owns the file from here: append your notes below the line.

## The world table (what the game reads)

The game reads world layout only from `Source/MegaJam/Private/World/JamWorldTable.cpp`. The struct types are in `Source/MegaJam/Public/World/JamWorld.h` and belong to GAME. TERRAIN may edit the **rows** in the table (positions, radii, biomes, wedge yaws, sizes) and nothing else under `Source/`. BUILD_SPEC section 3e, "Hand-over contract", wins where this file and `TerrainTask.md` differ.

These files live on the branch `game/open-world` until GAME merges O1 into `main`. Do not edit them before that merge.

### Coordinates

- Units are Unreal units (cm). +X is north, +Y is east, and yaw 0 points along +X.
- The village square is at the origin `(0, 0)`. The playable square runs from -12,000 to +12,000 on both axes (24,000 uu).
- The table holds x and y only. The game finds z by tracing down onto your terrain, so heights are yours.

### Biomes

| Biome | Wedge centre yaw | Kinds | Placeholder name |
| --- | --- | --- | --- |
| Village | the disc at the origin, radius 2,500 | none | Sugarborough |
| Meadow | 0 (north) | rats, almonds | The Gumdrop Meadow |
| Molars | 120 (south-east) | goblins | The Molar Hills |
| Orchard | -120 (south-west) | lollipop bugs, almonds | The Lollipop Orchard |

Outside the village disc, a point belongs to the biome whose wedge centre is closest to its bearing from the origin, so each wedge is 120 degrees wide. Paint the biomes to match. If you move a wedge, change its yaw in the table and the tests still hold.

### Locations (current rows)

All positions are `Polar(yaw, distance)` from the origin.

| Id | Kind | Biome | Bearing, distance | Radius |
| --- | --- | --- | --- | --- |
| Village | Village | Village | origin | 2,500 |
| Signpost | Landmark | Village | 60, 500 | 100 |
| GiverMayor, GiverBaker | Giver | Village | 180, 700 and -60, 900 | 150 |
| SetField | FilmSet (Scene 1) | Meadow | 0, 6,000 | square, 1,800 side |
| SetFort | FilmSet (Scene 2) | Molars | 120, 6,500 | square, 1,400 side |
| SetOrchard | FilmSet (Scene 3) | Orchard | -120, 6,000 | square, 2,000 side |
| GiverMeadow / Molars / Orchard | Giver | its biome | wedge yaw ±20, 3,200 | 150 |
| Camp* (3 per biome) | Camp | its biome | about 4,200 near the road, 8,000 to 9,800 far out | 600 (1,200 across) |

A film set's `Radius` is half the side of its square floor. The set faces the village, so the hero walks in from the near edge.

### What the terrain must give these locations

- **Flat enough to walk (under 15 degrees)** across each camp's and each film set's full radius. Film sets want a nearly flat floor because the scene's walls and actors sit on it.
- Clear of scatter. No tree or rock inside a camp's or set's radius, and none inside a 300 uu radius round a giver or the signpost.
- Terrain meshes block the `Pawn` and `WorldStatic` channels. Scatter does not block pawns unless it is house-sized.
- The village disc stays walkable and readable from a third-person camera about 500 uu behind the hero.

### Rules the tests check (`Jam.World.*`; a row change that breaks one is rejected)

- Every location, plus its radius, lies inside the bounds, and every id is unique.
- Each giver, camp and film set stands inside its own biome, by the wedge rule above.
- There is exactly one village, at the origin, and one film set per scene, each 5,000 to 7,000 uu out and at least the size of its backlot set.
- Every biome has at least one giver. Every wild biome has at least two camps.
- Each camp holds at least 3 (`Capacity`), spawns only its biome's kinds, and overlaps neither another camp, the village, nor a film set.

### Launching on your map

`MegaJam.exe /Game/World/L_Sugarborough -open`. On any map other than the template, the game skips its flat fallback and only spawns gameplay actors (hero, villagers, camps, set walls) at the table's locations. On the template map it builds a flat stand-in world from the table, so the game never waits for the terrain.

---

## TERRAIN active: 2026-11-12 (Codex)

Dylan confirmed: keep the compact 24,000 uu world; walk into outdoor film locations; add a cave entrance for later, without a playable interior. Another worker currently owns Unreal. TERRAIN is preparing geometry and deterministic scatter offline and will not launch Unreal or build until the engine is handed over.

**Request to GAME / FX:** when your current engine work reaches a safe checkpoint, record engine availability here (or tell Dylan). TERRAIN needs an exclusive import, map-authoring, screenshot and packaging window. Do not merge O1 on TERRAIN's behalf until GAME is ready to verify it; terrain follows the published table without modifying Source.

Implementation sources and manifests will live in `ImportSource/Terrain/`; all imported assets, materials, placement palettes and the map will live in `/Game/World/`. Village at origin, three readable paths to film sets, flat reserved gameplay areas, candy mountain perimeter with physical collision, and a cave mouth in the outer hills.

### Offline checkpoint

- `ImportSource/Terrain/build_sources.py`: 16 ground tiles, 12 original prop meshes, 1k ground atlas, 520 excluded/seeded scatter placements, six village cottages, well, fort towers/gate, 48 perimeter mountains and cave entrance. Full placement data in `world_manifest.json`.
- Source validation: 32,768 terrain triangles; every prop under 1,500 triangles; all 19 published locations reserved; maximum generated triangle slope touching village/camp/film footprints **9.32 degrees**, below the 15-degree limit.
- `export_blender.py` exports FBX and offline previews. `build_level.py` is prepared to import into `/Game/World/`, author PCG graphs, bake map actors and create `L_Sugarborough`. **Importer not yet run; map not yet delivered.** No source code or shared configuration changed.
- `ImportSource/Terrain/README.md` records reproduction, remaining verification and fal price/output research. Rapid $0.225 textured OBJ versus Mini-Turbo $0.08 untextured image-to-GLB; neither removes the need to reduce meshes. No API spending or balance requests.
- Waiting for an explicit engine handover through this file or Dylan. GAME's O1 is still unmerged, so packaged `-open` gameplay also awaits GAME verification. Do not count Blender previews as an engine or packaging pass.

### Engine window granted

Dylan explicitly handed the engine to TERRAIN in chat. TERRAIN now owns the import/review window; other workers should hold engine work until the next handover here. Added runtime `PCG` to `MegaJam.uproject` as permitted by 3e. GAME's Source and branch remain untouched.

### Engine released for FX compile (2026-11-12, 19:44 local)

**The engine is free for FX.** Dylan requested the FX compile window and TERRAIN has finished its current render and stopped all Unreal processes it started. TERRAIN will not start builds, tests or packaging until FX hands the engine back. No GAME branch merged.

Delivered on disk: `/Game/World/L_Sugarborough`, 28 meshes, 15 materials, one 1k texture, three PCG graphs, and four separate camera review maps. Added `/Game/World` to `DirectoriesToAlwaysCook` in `DefaultGame.ini`; added runtime PCG to the `.uproject`. Neither the default map nor GAME Source changed.

Engine verification: exact imported XY bounds and centimetre scale pass for all 16 ground tiles. All 520 scatter actors have collision disabled. **79 physics traces** across every location and camp/set edges/corners hit terrain; maximum measured hit slope **0.171 degrees**. `engine_validation.json` records every trace. Height-function and triangle-intersection slope bounds also pass (9.32 degrees maximum touching a reserved footprint).

Fixed during verification: Blender-to-Unreal Y reflection; collision flags must be set BEFORE rebuilding mesh collision, not afterward; the Python commandlet has no physics scene, so actual traces run via `review_entry.py` in a full editor session. Lighting is movable with precomputed lighting disabled. Final in-engine village screenshot: `Saved/Screenshots/WindowsEditor/terrain-village.png` (1600x1000). Offline overview: `ImportSource/Terrain/Review/overview.png`.

**Still required after FX returns the engine:** editor build, `Jam.*` tests, terrain cook/package and packaged launch/screenshot, plus Backlot regression. `-open` integration additionally waits for GAME's O1 branch merge and verification. Terrain is not yet declared package-proven. The current terrain work is ready for a scoped checkpoint commit; no unverified completion claim.

### Compile/validation window resumed

Dylan asked TERRAIN to continue the compile and checks. No Unreal or build process was running at handover. TERRAIN now holds the engine for editor build, tests, packaging and smoke checks. GAME's O1 remains unmerged, so this pass can prove terrain cooking/rendering and Backlot regression, while full Open gameplay remains a GAME integration dependency.

### Verified delivery checkpoint (2026-11-12, 20:53 UTC)

**TERRAIN's build/render/smoke processes are finished.** Other workers can use the engine; do not stop an unrelated FX commandlet if one is already active.

- Editor compile succeeded; all **54 `Jam.*` tests pass**, zero failed. No GAME Source changed or branch merged.
- Fresh full-editor physics pass: **79 ground samples**, all **three hero-sized capsule approaches**, **12 boundary traces**, 16 correctly scaled/oriented terrain tiles, and 520 nonblocking scatter actors. `review_run.json` and `engine_validation.json` hold the current results. Map loading requires a few editor ticks before physics checks; the wrapper explicitly stays alive and clears stale reports.
- Normal Windows Development package succeeded. First packaged capture exposed missing map packages despite `DirectoriesToAlwaysCook`. Fixed **inside owned content** with `/Game/World/PAL_Sugarborough`: AlwaysCook, recursive, with five explicit map references. No extra map-cooking config or packaging-script changes were needed. `label_maps.py` reproduces it after building the main and review maps.
- Actual packaged `/Game/World/L_Sugarborough -open` launch passed. Packaged Village and Overview captures inspected: `ImportSource/Terrain/Review/packaged-village.png` and `packaged-overview.png`. Baked scatter, roads, houses, fort, cave mouth and perimeter are present.
- Packaged Backlot survived the **70-second smoke test**, with all seven checks green. Autoplay: `PASS scenes=3 first=1 takes=4 notes=2/0 ending=directors_cut flees=72 lines=15 skills=16 time=126.5`.
- Local validation archive: `Builds/TerrainValidation.zip`, **574,901,156 bytes** (575 MB decimal), CRC checked, under 1 GB. Excludes debug symbols and runtime Saved directories. Nothing uploaded or pushed.

**GAME integration remains:** merge/build O1 and continue O2–O9. The terrain follows O1's published table; main currently has no Open framework. Packaged map launch/render passes must not be called a passing Open quest autoplay. Keep the current default map until BUILD_SPEC's gate is met. Final third-person camera readability and a human playthrough remain for GAME/TUNING once that framework exists. Terrain sources, runtime assets, cook label, validation tools and evidence are ready for the scoped terrain commit.

### Hyper3D foliage expansion active

Dylan explicitly requested additional candy foliage and trees using his Hyper3D credits. TERRAIN is generating eight candidates in `ImportSource/Terrain/Foliage/`: four trees and four smaller plants. Current official API supports `quality_override=1500`; Gen-2.5-Low, Raw mesh, PBR/low textures. Expected cost four credits total. Raw files and public provenance are preserved; private queue subscription keys are git-ignored. No automatic paid retries. No world assets or existing scatter replaced yet. Unreal is in use by another worker, so generation and Blender review are proceeding offline.

### Hyper3D foliage expansion delivered offline

**Dylan explicitly instructed TERRAIN not to run Unreal; no engine launched for this batch.** All eight jobs succeeded at 0.5 credits each, **four credits total**. Prepared eight GLBs, eight FBXs, eight 1k base-colour textures, individual previews and a labelled contact sheet under `ImportSource/Terrain/Foliage/`.

Models: peppermint birch, licorice willow, candy-cane fir, bubblegum blossom, jellybean bush, sugar fern, candy flower clump and gummy succulent. Actual source/output counts **1,398–1,476 triangles**: the API honoured the 1,500-face target, so no Collapse reduction was necessary. Original files/hashes and full submitted prompts retained. Final saved-file validation passed all eight (GLB/index structure, finite positions and UVs, texture references/size, raw hashes and FBX signatures). Material preparation keeps one generated base-colour atlas and matte roughness; original full PBR output stays in Raw.

Visual review: birch, fir, blossom and succulent are strongest. Willow is noticeably spiky/angular; hold for review before scattering. Fern is chunky rather than fernlike; fern/flowers retain small brown root shapes to bury or trim at placement. Contact sheet: `ImportSource/Terrain/Foliage/Review/contact-sheet.png`. More details in that folder's README.

**Not imported, not scattered, not included in the current 575 MB package.** Current world and prior packaged validation remain unchanged. Engine integration should target `/Game/World/Foliage/`, keep scatter nonblocking and retain all location/road exclusions. Approval and in-game visual checks remain for the next engine window; this task's requested generation and offline preparation are complete. No C++ build/test/package run was needed or attempted for these source-only additions.
