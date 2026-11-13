# Hyper3D candy foliage batch

Dylan requested this batch using his existing Hyper3D credits. Generated during
the jam on 2026-11-12 through Hyper3D Rodin Gen-2.5-Low: **eight models, four
credits consumed total**. No paid retries, no use of Tripo/fal, and no balance
queries. Submitted prompts and provider task UUIDs are in
`generation_manifest.json`; the private queue file is git-ignored.

## Review

Open [the labelled contact sheet](Review/contact-sheet.png).
Every source model was rendered and inspected in Blender, without using Unreal.

| Model | Triangles | Intended height | Review |
| --- | ---: | ---: | --- |
| Peppermint birch | 1,438 | 620 cm | Clear striped trunk and mint crown; strong candidate. |
| Licorice willow | 1,460 | 660 cm | More angular and spiky than intended. Hold for review; might suit the Molar hills. |
| Candy-cane fir | 1,464 | 720 cm | Distinct tiered frosting silhouette; strong candidate. |
| Bubblegum blossom | 1,458 | 580 cm | Pink/cream crown and caramel trunk; strong candidate. |
| Jellybean bush | 1,444 | 160 cm | Readable fruit clusters; green is more saturated than the mint palette. |
| Sugar fern | 1,398 | 95 cm | Solid chunky leaves rather than fine fern fronds; small brown root/base remains. |
| Candy flower clump | 1,476 | 80 cm | Three flowers are readable; small brown root/base remains. |
| Gummy succulent | 1,404 | 130 cm | Compact teal/lavender rosette; strong candidate. |

The flower and fern bases can be partly buried during placement; their preview
shows the complete mesh. The willow has thin/pointed leaf shapes. These are
review findings, not a claim that every asset has been approved for the game.
Integration update, Nov 13: seven models are imported under `/Game/World/Dense/`
and placed in the packaged dense world. Willow remains excluded. See
`../Dense/engine_report.json` for runtime integration; `validation.json` checks
offline source files only.
Dylan explicitly asked not to run the engine while another task uses it.

## Files and checks

- `Raw/`: untouched downloaded GLBs.
- `Clean/`: grounded, centimetre-normalized, matte-textured GLBs.
- `FBX/`: Unreal import companions using the terrain pipeline's tested axis/unit convention.
- `Textures/`: one original generated base-colour atlas per asset, capped at 1024 square.
- `Review/`: individual Blender previews and contact sheet.
- `prepared_manifest.json`: measured dimensions, material slots, hashes and intended biomes.
- `validation.json`: passing saved-file checks and actual credit total.

Rodin accepted the 1,500-face request: raw counts were already 1,398–1,476, so
none needed Collapse decimation. Cleanup welded coincident attribute-split
vertices, checked finite geometry/UVs, grounded pivots and preserved raw hashes.
The cleaned materials use the generated base-colour atlas with matte roughness;
unused PBR normal/metallic/roughness maps were omitted. Original full PBR outputs
remain in Raw. Validation checks GLB structure, actual index/triangle counts,
valid indices, finite positions/UVs, material texture references, PNG size,
FBX headers and unchanged raw originals. It does not certify manifold topology,
all-view aesthetics, Unreal materials, camera readability or cooked performance.

## Reproduce without Unreal

`python -I ImportSource/Terrain/Foliage/generate.py` resumes already accepted
jobs and skips downloaded outputs; it does not regenerate finished models.
Do not delete private queue state to force a paid retry.

`& 'D:/Program Files/Blender Foundation/Blender 4.5/blender.exe' -b -t 3 --python-exit-code 1 --python ImportSource/Terrain/Foliage/prepare_blender.py`

`python -I ImportSource/Terrain/Foliage/validate.py`

`python -I ImportSource/Terrain/Foliage/contact_sheet.py`

Generation and Blender preparation are complete. Future engine work should
import to `/Game/World/Foliage/`, preserve the manifest's scale, make foliage
nonblocking, and retain the world's camp/set/giver/road exclusions. Compare at
the final game camera before replacing existing scatter.
