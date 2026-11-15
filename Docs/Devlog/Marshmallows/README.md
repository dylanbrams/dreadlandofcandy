# Marshmallow mesh delivery

Three FAL Hunyuan 3D v3.1 Pro text-to-3D meshes, generated with PBR enabled. FAL request IDs, prompts, result sizes, and queue URLs are recorded in the matching `*.metadata.json` files and `queue.json`. Each request was submitted once; total expected generation cost at the helper's listed rate ($0.375 + $0.15 PBR) is $1.575.

## Import assets

- `MM_CreamClassic_clean.fbx` — softly irregular cream marshmallow, close to a short upright pillow.
- `MM_TwistedCream_clean.fbx` — more deeply waisted/twisted silhouette.
- `MM_SquashedCream_clean.fbx` — broad, low, side-lying silhouette.

Each FBX is one static mesh, normalized to a 22 cm longest dimension, smooth shaded, normal corrected, and decimated to 4,800 triangles. No disconnected components or loose vertices were present. `cleanup_report.json` records bounds and source/final triangle counts. The FBXs embed generated materials; Unreal may replace them with the project's cream material.

`Marshmallow_contact_sheet.jpg` shows the FAL source previews. Blender's cleaned-mesh renders are the matching `*_clean.png` files. Those renders are overexposed because the generated GLB material is very bright under the Blender preview lights; judge color/material in Unreal. The supplied FAL thumbnails have clearer surface shading.

The source GLBs are preserved beside the cleaned FBXs for provenance. Blender cleanup script: `clean_export.py`.
