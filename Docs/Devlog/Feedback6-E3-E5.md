# Feedback6 E3-E5 progress

- E4: added a pool of up to 12 cool blue fireflies near the riverbank. Positions are traced to the ground; the pool is collision-free and distance culled.
- E3: first implementation adds an amber bakery light, four periodic flour puffs, and a swinging sign, using bakery mesh bounds.
- Editor build passed and 86 Jam tests passed with E3/E4. A subsequent small sign rotation correction rebuilt successfully. The village capture feedback6-bakery.png is blocked by a roof; bakery placement and fireflies still need visual inspection.
- E5: existing striped grass already has root-masked wind; tree materials lack a verified wind mask. A tree pass needs root anchoring, bounds changes and near/far review.

- Runtime log feedback6-bakery-close.log reports bakery found and 12 fireflies. The close capture shows a warm window; the sign and flour remain obscured. feedback6-fireflies.png shows small light points at the riverbank; animation has not been assessed in motion.
- E5 is not implemented: striped grass already sways, while tree wind needs a per-instance root mask and bounds verification. No tree material was changed.

## E5 offline preparation

Claude has the engine. `Scripts/prepare_foliage_wind.py` is a proposed narrow
pass for Polish peppermint birches, bubblegum blossoms and spun-sugar bushes;
its default is inspection only. It has passed Python syntax checking but has
not been run in Unreal, and its editor API, per-instance transform, instance
scale versus bounds extension, material compilation and visual motion must be
verified before applying. It stops if it cannot inspect existing WPO, or if a
target material already has WPO. Collision is untouched. Existing striped
grass is excluded. After the engine is handed back, inspect roots and near/far
motion and confirm culling; do not claim E5 complete until those checks pass.

## E5 applied and reviewed

Applied a restrained 8 cm X / 3 cm Y world-space sway to the Polish
peppermint birches, bubblegum blossoms and spun-sugar thicket. The weight is
derived from per-instance local height, with the lowest 8 cm fixed and a
quadratic ramp. Seven mesh assets have 60 cm horizontal bounds extensions;
the source ecology instance scales are at least 0.65 for bushes and 0.8 for
the affected trees, so those extensions exceed the maximum displacement.
Collision and the other foliage types are unchanged.

The first runtime render caught a missing Transform Position input and used
fallback materials. `Scripts/repair_foliage_wind.py` connected it, and the
generator was corrected to check the connection. Subsequent DenseGrove and
DenseMeadow review renders report no material compilation failures and show
normal colours. Grove captures at 30 and 32 seconds show changed canopy and
bush positions with grounded trunks. These are stills, not a continuous
motion or performance test; inspect culling during normal play. Review images:
`ImportSource/Terrain/Polish/Review/polish-DenseGrove.png` and
`polish-DenseMeadow.png`.

No-build Jam suite after the material changes: 86 passed, 0 failed (Saved/Automation/RunTests.log). No packaging or continuous motion playtest was performed.
