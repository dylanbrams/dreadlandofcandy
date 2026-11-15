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
