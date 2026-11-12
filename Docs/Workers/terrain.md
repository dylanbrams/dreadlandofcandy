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
