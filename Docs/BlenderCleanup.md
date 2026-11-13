# Blender cleanup for generated 3D models

## Current project pipeline (validated 2026-11-12)

Use the **project's Blender 3.3 pipeline** for the current Hyper3D/Fal assets and batch-2 Meshy shard. The older Cafeteria Battler pipeline described below is historical reference, uses Blender 4.5, and was not copied or run for this cleanup.

- [Current asset handoff, results and previews](../ImportSource/_decimated/_review/REPORT.md).
- [ASSETS worker handoff](Workers/assets.md) records the completed work and remaining GAME integration.
- Raw originals: `ImportSource/{Hyper3D,Fal,Meshy}`. Cleaned outputs: `ImportSource/_decimated/<same service>/<same name>.glb`. Never replace raw originals.
- Processing: root `decimate.py` and `ImportSource/_decimated/cleanup.py`. Validation, collision and review helpers live under `_decimated/`.

From the repo root, using the installed Blender **3.3.0**:

```powershell
# Rebuild one asset from its untouched source, including its recorded cleanup.
& 'C:\Program Files\Blender Foundation\Blender 3.3\blender.exe' -b --python-exit-code 1 -P decimate.py -- --only SM_Signpost

# Independent validation of all staged outputs (normal Python, NumPy and Pillow).
python ImportSource/_decimated/validate.py

# Refresh all four-angle previews and UCX collision companions.
& 'C:\Program Files\Blender Foundation\Blender 3.3\blender.exe' -b --python-exit-code 1 -P ImportSource/_decimated/_review/audit.py -- --output-subdir clean --collisions
python ImportSource/_decimated/_review/sheets.py clean

# Validate wearable openings and the gate passage without rerendering.
& 'C:\Program Files\Blender Foundation\Blender 3.3\blender.exe' -b --python-exit-code 1 -P ImportSource/_decimated/_review/fit_check.py -- --checks-only

# Regenerate the final handoff after validation and previews are current.
python ImportSource/_decimated/_review/final_report.py
```

Running `decimate.py` without filters rebuilds all 31 staged assets, not just plain decimation: it reapplies asset-specific cuts, reconstructed parts, pivots and material regions. `-- --only-folder Fal` restricts the source service. Names ending in `_v1` are skipped. Characters, including Almond and LollyBug, use a 25k budget; other meshes use an 8k ceiling and simple rebuilt props remain lighter. This is for the listed **static** meshes, not the rigged hero or animation files.

After changing only selected assets, previews can be refreshed with `audit.py -- --output-subdir clean --collisions --merge --only SM_Signpost`. Finish one audit process before starting another that writes the same `audit.json`; otherwise a later writer can discard another run's rows. Re-run saved-file validation after geometry changes. The final report checks output hashes against validation to reject stale results.

Batch 2's eight 3D and sixteen UI jobs were already complete when cleanup began. No generation was repeated or API credit spent. The runner's separate old Sword `redo` state does not mean batch 2 is unfinished: the staged sword's proportions were fixed from the existing source. Check state and actual files before starting any paid generation.

## Problems solved and checks to retain

1. **Weld attribute splits before Collapse.** GLB imports duplicate vertices at UV/normal seams. Decimating those copies independently created open edges in every first-pass output. Preserve per-corner UV data while welding source positions, then decimate. On these normalized assets, BMesh welding at `max(obj.dimensions) * 1e-7` worked; the tighter `1e-8` trial left splits on dense Rodin meshes. This is an observed tolerance for this data, not a universal setting: verify closed edges and avoid merging nearby separate surfaces.
2. **Validate the saved GLB, not just Blender memory.** Count geometric edges after exact-position welding across *all material primitives of each mesh*. Attribute splits alone are not holes. The first in-memory probe and UV-layer checks were insufficient. Require zero boundary edges, zero edges with more than two incident faces, no collapsed exported triangles, and finite positions/normals/UVs. Some raw sources contain collapsed triangles; report those as source defects without relaxing the requirements for cleaned outputs.
3. **Texture presence does not prove preservation.** Decode embedded base-colour images and compare pixel hashes, using unique images rather than material counts because material-region copies can share one atlas. A retained UV layer does not certify interpolation or seam appearance. Inspect renders too. Deliberately rebuilt accessories reuse the source atlas as paint with new UVs; that is documented as a replacement, not identical source mapping.
4. **Check shapes visually after every kind of repair.** Horizontal head-removal cuts damaged the hat brim; hollowing the fused headband left fragments; a cone subtraction damaged the megaphone body. Small procedural reconstructions were cleaner for those parts. Replacing the clapper slate removed embedded arm stubs that cropping alone missed. Ground cuts can leave thin rims; the signpost ultimately used texture-colour sampling to find the top of its green terrain base. Remove exposed detached debris selectively, not every small component.
5. **Use one closed arched cutter for the gate.** Sequential box/cylinder subtractions at their tangent join produced nonmanifold edges. A single extruded arch profile passed export checks. Confirm a passage with rays through the saved mesh and separate collision sides/header; a convincing front view alone does not establish clearance.
6. **Preserve deliberate hollows and thin parts.** Do not blanket voxel-remesh or fill openings in wings, bristles, hat underside, mirror hole, megaphone cone, chair frame or open box. Closed surface topology can still describe a hollow cup or perforated solid. Refresh split normals after topology changes and inspect both sides.
7. **Fit accessories on the actual character.** Standalone renders hid an oversized hat and loose headband. Measure the head at the attachment height, then render front/back assemblies. The final report records tested Villager mounts, ground/grip/flight origins and unit conventions. A ground-origin mesh needs an offset when parented to a capsule-centred actor.
8. **Keep appearance and collision handoffs distinct.** The 12 UCX FBXs contain neutral preview materials; GLBs carry appearance. Actor capsules, held objects and particles have explicit policies instead of unnecessary blocking hulls. Generator-normalized gate dimensions need a scale adjustment before player traversal; see the report. Collision files were prepared in Blender, not validated through Unreal import here.
9. **Make headless errors observable.** Blender 3.3 can exit successfully after a Python exception unless invoked with `--python-exit-code 1`. `read_factory_settings(use_empty=True)` can also leave `scene.world` as `None`; create a world before setting its colour for renders. Import/export specular-conversion warnings occurred on these generator materials; assess actual output and validation rather than treating either a warning or a successful process exit as the verdict.

The completed checks do not certify exhaustive self-intersection tests, UV overlap, deformation, gameplay material behaviour, camera readability or packaged performance. Only GAME opens/imports into Unreal in the shared checkout. Originals, provenance and the superseded first-pass review remain available; current visual evidence is `_review/clean/`.

## Historical reference: Cafeteria Battler cleanup

The sections below describe a different pipeline. Its Blender 4.5 requirement and untested status apply to that pipeline, **not** the current Blender 3.3 scripts above. Anything copied from another project during the jam must be recorded in `Docs/Declared.md`; no such copying was used for this cleanup.

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
