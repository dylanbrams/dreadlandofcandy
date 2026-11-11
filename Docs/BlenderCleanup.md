# Blender cleanup for generated 3D models

How Cafeteria Battler cleaned up AI-generated meshes, where the scripts are, and how to use them here.
Nothing has been copied into this project yet. Anything copied in during the jam gets a line in
`Docs/Declared.md` the same day; copying before Nov 12 makes it pre-jam work.

## Where it lives

All under `D:\Source\Unreal\CafeteriaBattler\`:

| File | What it is |
| --- | --- |
| `Tools/GenRunner/blender/cleanup_mesh.py` | **The one to reuse.** Headless Blender script: raw mesh in, game-ready low-poly `.glb` out. About 870 lines, takes plain file paths, no project coupling. |
| `Tools/GenRunner/blender/cleanup_batch.py` | Batch runner. Tied to that project's manifest and `art/generation/` folders, so **not reusable as is**. Read it for the pattern: run Blender per file in parallel, skip up-to-date outputs, build a before/after contact sheet. |
| `Tools/GenRunner/blender/cleanup_overrides.json` | Per-asset extra flags with a `why` (example: `--no-sheets` for models with genuinely flat parts). |
| `Tools/GenRunner/blender/decimate_preview.py` | Simpler script: decimate and render previews only. Good for a quick "how low can it go". |
| `Tools/GenRunner/blender/build_gloves.py` | Builds simple parts procedurally instead of generating them. Shows the pattern for things that scan badly. |
| `Docs/AssetPipeline.md` | Why the pipeline looks like this: budgets, routes, licences. |

## Blender

Use **4.5 LTS** (what the script was tested on): `D:\Program Files\Blender Foundation\Blender 4.5\blender.exe`.
The older installs (2.92, 3.3, 3.4, 3.6) will not work: the script uses 4.x calls such as
`shade_smooth_by_angle`.

## Running it on one model

```powershell
& "D:\Program Files\Blender Foundation\Blender 4.5\blender.exe" -b --factory-startup `
  --python D:\Source\Unreal\CafeteriaBattler\Tools\GenRunner\blender\cleanup_mesh.py -- `
  ImportSource\Meshy\vending.glb  ImportSource\Clean\vending.glb  2000  ImportSource\Clean\vending_report `
  --no-ground --no-sheets --no-chop
```

Arguments after `--`: input `.glb`, output `.glb`, triangle budget, a folder for previews and the report.

Output: the low-poly `.glb` (textures embedded), the baked PNGs (basecolor, roughness, normal) for
hand fixes, `raw_*.png` and `clean_*.png` previews from three angles, and `report.json`
(triangle counts, silhouette overlap `silhouette_iou_mean`, colour difference `colour_diff_mean`).
Look at the previews before trusting the numbers; an IoU near 1.0 and a low colour difference means
the low mesh matches the original.

## What it does

1. **Weld and strip junk.** Welds UV-seam splits, deletes the floor-shadow slab some generators build
   under the object, deletes flat thin sheets (backdrops, puddles), drops floating specks.
2. **Rebuild as a solid.** Voxelises the surface, flood-fills from outside, and treats everything
   unreachable as solid. This seals holes and tunnels that make decimation get stuck. It can also chop
   a plate off the bottom.
3. **Decimate** by collapse to the triangle budget, closing harder if the topology blocks it.
4. **Unwrap and bake.** Smart UV, then bakes colour, roughness and a tangent-space normal map from the
   cleaned high-poly mesh onto the low one (Cycles, GPU if available).
5. **Export and report.**

Useful options: `--texture 1024|512` (bake size), `--shading flat|smooth`, `--no-normal-map`,
`--no-ground`, `--no-sheets`, `--no-chop`, `--chop F`, `--keep-area` (floating specks).

## Triangle budgets used before

Units 1,000-2,000; elites 2,000-3,000; bosses 3,000-6,000; small parts and projectiles 50-300;
512 px textures for parts at or under 400 tris, 1024 px otherwise.

## Caveats for this jam

- **It was built for TRELLIS.2 output.** The ground-slab, hollow-shell and metallic/alpha fixes target
  that model's artifacts. Meshy, Tripo and Rodin output is different, so start with
  `--no-ground --no-sheets --no-chop`, and turn a pass on only if the preview shows the problem.
- **The solid rebuild is always on.** It makes a closed solid and fills hollows, so it suits props and
  characters but not anything with a deliberate opening or very thin parts. If a generator's output is
  already clean and textured, skip Blender and just decimate (`decimate_preview.py` or Meshy's own
  `target_polycount` / Tripo's `face_limit`).
- **Untested on this machine for this project.** Do one model end to end before relying on it, and
  time it. The Cafeteria Battler runs were done on rented GPUs; local Cycles baking on this PC may be
  slow with the low free RAM noted earlier.
- **Rigged or animated models:** the script joins meshes and drops everything else. It is for static
  meshes.

## Getting a model into Unreal

1. Generate with `Scripts/meshy.py`, `rodin.py` or `tripo.py`: raw `.glb` lands in `ImportSource/<Service>/`.
2. Run the cleanup above; check `clean_*.png` against `raw_*.png`.
3. Import the cleaned `.glb` through Interchange into `/Game/...`. Try one in a scratch copy of the
   project first. Imported assets should be cooked normally; anything loaded by path at runtime needs
   `DirectoriesToAlwaysCook` (see `CLAUDE.md`).
4. Keep the manifest line from the generator: it is the record for `Docs/Declared.md` and the
   "Tools you used" field.
5. Commit the imported `.uasset` (LFS), not the raw `.glb` (gitignored).

## Licence and disclosure notes

The Cafeteria Battler pipeline used open-weights models and tracked licences per model. Here the
generators are commercial services (Meshy, Hyper3D, Tripo): check each one's output licence on the
account the credits came from (Meshy's free plan is CC BY 4.0; paid plans include commercial use) and
note it in `Docs/Declared.md` under third-party assets.
