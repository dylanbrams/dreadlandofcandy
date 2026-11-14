# Riverlands expansion

Nov 15 foliage mix: `intersperse_foliage.py` replaces one in five existing
Birch/Blossom instances with generated peppermint, candy apple and candy trees
(134 total), retaining their positions. Fir/snow groups and the square landmark
are preserved. Narrow hidden Pawn-only trunks retain collision; decorative
crowns and undergrowth do not block the camera. Berry bushes and succulents
complete the eight-model foliage set. The script guards against applying twice.
`review_foliage_mix.py` corrects the initial understory alternation, writes final
counts to `foliage_mix.json`, and creates a separate camera review map.

The original Molar hill and three film sets are retained. The eastern boundary
opens into a rolling meadow and a north/south mint river, with one walkable
bridge. Runtime bounds are (-12000,-12000) to (26000,12000), 58.3% more area.
The far meadow camp moves to (22200,0), beyond the crossing, with two additional
camp destinations on the riverbank and in the mint grove. Givers now stand near
their new village workplaces. Camp identities remain stable for authored jobs.

`export_extension.py` and `export_opening.py` run in Blender 4.5 background.
`build_expansion.py` runs via Unreal's Python commandlet, replacing the eastern
ground tiles and preserving the original hill. New geometry uses world vertex
coordinates plus a +4000 actor X offset for the riverlands meshes only. The
opening tiles retain original world coordinates. Sources and cleaned user
models remain untouched. Existing imports are reused, except Riverlands terrain
which reimports to support height refinement. Other changed sources require
explicit reimport before rebuilding the map.

Buildings use the seven generated village models in `Assets/DylanGeneration`.
Six foliage types use `Assets/DylanGenerationFoliageTwo/LowPoly`, with textures.
The crossing uses Dylan's generated peppermint-railed Bridge126, cleaned by
Terra in `Assets/DylanGenerationFoliageThree/Clean`. It is fitted separately
along each axis to span the channel and meet the banks. The procedural wafer
bridge source remains available but its actor is replaced by the generated one.

`run_review.ps1 -Mode validate` performs saved-world physics queries.
Modes `village` and `river` capture offscreen review-map screenshots. These
camera maps use GameModeBase; gameplay remains on L_Sugarborough. Validation
does not establish manual traversal, gameplay performance, or cooked delivery.
