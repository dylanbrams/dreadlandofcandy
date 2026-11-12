# Asset generation prompt handoff

Prepared 2026-11-12 by ASSETS. **Prompts only. No 3D, rigging, animation or 2D generation has been run.** Nothing here is an approved asset or evidence of visual quality.

- [3D prompts and animation brief](3D/README.md): Meshy hero, creatures, props and particle meshes; Rodin sword and toothbrush; hero rig and motion requests.
- [2D prompts](2D/README.md): individual images, with aspect, transparency and reference instructions per job.
- [Machine-readable full prompts](prompts.json): all jobs, stages, priorities, settings and anchors.
- Sources: [jobs](../Generation/jobs.json), [3D style locks](../../Docs/styles.json), [asset specification](../../Docs/Design/ASSETS.md), [generation runbook](../../Docs/GenerationRunbook.md).

The current job list and runbook supersede the older sheet instructions in GenerationMap/GenerationRouting and the superseded portion of ASSETS. Generate **one image per job**, not a sheet. Knight and Fenella UI have separate style anchors. Fixed-word speech bubbles, cardboard flats rendered from meshes, fonts, engine terrain/materials/Niagara, audio and Blender primitives remain in their existing lanes; they do not need new image or Meshy prompts.

## How to use later

The editable source is `Assets/Generation/jobs.json` (subjects and 2D styles/rules), with 3D wrappers in `Docs/styles.json`. Regenerate this fileset offline after edits with `python -I Assets/Generation/export_prompts.py`. The exporter reads these two JSON files only and writes prompt documents; it never invokes a generation client, reads credentials or marks jobs done.

Each `.txt` is self-contained. For manual 3D generation, paste its full text. The existing clients add style words themselves, so keep their input as the **subject only** with the matching `--type`. The runner settings (triangle target or Rodin tier) are in the indexes. Polycount, material slots, pivots and exact dimensions require cleanup and inspection; prose cannot guarantee them.

When generation is authorized, begin with stage `test`: hero, sword, rat, house front, peppermint, the Lecture icon and the blank clapper icon. Hero rigging depends on the hero mesh. Initial anchors need no image reference. Dylan approves each style group after review, with 3D checked in the game camera, before its later batches. Preparation does not unlock those gates. Follow the runbook for generation, ingest, cleanup and declaration; do not infer permission to run from this handoff.

For later 2D jobs, attach the approved matching-layer anchor: `T_Skill_Lecture` for knight art, `T_Card_Clapper` for Fenella art. The expected file is `Assets/Incoming/2D/<anchor>/<anchor>.png`. Preserve Fenella's cream skin, lavender bob, berry-red beret, mint jacket and toy black-and-white camera across mesh, portraits and billboard. The three villagers share cream skin, short brown hair and a peach tunic, distinguished by accessories. Once neutral portraits exist, use them as additional identity references when generating variants manually; the existing runner supplies only the layer anchor. Check identity consistency explicitly.

Save later 2D results as `Assets/Incoming/2D/_drop/<id>.png` for the runbook's ingest step. All icons, stamps, portraits, wings and frames use transparent backgrounds. `full` paper textures and posters are **opaque edge-to-edge**; do not ask for transparency on those despite the runbook's blanket transparency sentence. A frame has both an empty transparent centre and transparent exterior. Keep live UI text out of artwork; only stamp/poster jobs carry the quoted words. Clock face uses tick marks instead of numerals to comply with its icon rule.

## Review and cleanup targets

| Group | Budget after cleanup | Texture | Review and placement |
| --- | --- | --- | --- |
| Hero | 30k–50k triangles | 2K | 2.1 m tall, feet at ground, separated limbs suitable for rigging, empty hands, no cape, no candy ornament. Rig and animation success remains untested. |
| Equipment | Sword 10k–20k; toothbrush 5k–10k | 2K | Pivot at grip, consistent hand socket orientation, complete exposed handle; toothbrush bristles legible from camera. |
| NPC | Rat 3k–5k; goblin/villager 4k–6k; Fenella 3k–5k; bug 2k–4k; almond 1k–2k | 1K | Grounded characters pivot at feet; flying ones at body centre. Check scale 0.5–3 and all note tints. Fenella camera pose takes precedence over a generic A-pose; no NPC skeleton required. |
| Terrain pieces | Use per-job target, roughly 100–4k triangles | 512 | Ground placement pivots at bottom centre. Fence spans 400 uu and joins cleanly. Facades must have flat backs; door arch must admit hero. Held accessories need attachment pivots. |
| Particle meshes | Peppermint 150–400; stanchion 200–400; shard 50–150 triangles | none or 256 | Tiny readable silhouette. Candy/shard pivots at centre; stanchion at base. Engine supplies ropes and motion. |
| 2D | Default 1024 square; wide/tall as indexed | Ingest default cap 1024; explicit exceptions as indexed | Look at small UI size, inspect alpha/edges and exact permitted spelling. No unintended lettering, duplicate items, icon frames or scenery. |

Hero should read as grim dark fantasy against the bright, matte candy world. Check rat at scale 1 and 3 with pink/grey tints; check signs, chair backs, boxes and clapper faces are blank for runtime text. Goblin teeth need a separable surface/material assignment during cleanup for the optional clean-teeth effect. The requested clapper hinge gap also needs cleanup if a moving top arm is wanted. Preserve source files if reworking any output.

Engine-only work and import stay with GAME. No builds or Unreal editor are needed to review this prompt package. Declarations are added when assets are actually generated or acquired, not when prompts are prepared.
