# ASSETS handoff

Updated 2026-11-12. Current instruction from Dylan: **prepare and write prompts only; do not generate through Meshy, Rodin or any 2D image tool.** This overrides the execution steps in the ASSETS start prompt and GenerationRunbook for this session.

## Completed

- Refined `Assets/Generation/jobs.json` subjects for all 30 model jobs and character/texture prompts that needed explicit identity, blank text surfaces, clean silhouettes or transparency constraints. Existing job IDs, routing, stages, priorities, dependencies, preset animation IDs and model settings retained.
- Preserved `Docs/styles.json` unchanged. Hero stays dark fantasy without cape or candy decoration; candy-side meshes stay low poly and tintable. Fenella's shared appearance is cream skin, lavender bob, berry-red beret, mint jacket and a toy black-and-white camera. Villagers share a cream-skinned round face, short brown hair and peach tunic, with separate hats/tools.
- Corrected the clock's numerals/no-numbers contradiction by using twelve tick marks. Clarified opaque full-canvas paper/poster rules. Revised sword-raise motion to a 0.3-second raise followed by a hold, and kept brush props separate from animation generation.
- Wrote 83 self-contained text files: 28 Meshy model prompts, 2 Rodin model prompts, 1 hero rig/animation brief, 52 individual image prompts. Indexes include stages, priorities, settings, transparency and style anchors. Start at `Assets/Prompts/README.md`.
- Added an offline exporter at `Assets/Generation/export_prompts.py`. It reads only jobs/style JSON and writes prompt documents. This lets prompt changes be re-exported without generation, approval changes or service calls.

## Validation

Offline validation passed: all 83 files match source records, all 52 image prompts and 30 full model prompts match the existing runner/client formatting exactly, all prompt-index links resolve, original job IDs/order and routing/dependencies/stages remain intact. Longest Meshy prompt is 545 characters, below the client's 800 limit. Model arguments unchanged; only the two described text-to-motion descriptions changed in the rig arguments.

No Unreal build or editor run was needed for prompt-only work. No generation clients or image tools ran, no balance/API requests were made, no assets were generated or ingested, no approval state was written, and no generated-asset declarations were added. Visual quality, rig acceptance, animation quality and usable material slots remain untested.

## Next authorized session

Wait for Dylan to authorize generation. Then follow `Docs/GenerationRunbook.md`, starting at the test stage and reviewing each type before batching. The test items are hero, sword, rat, house front, peppermint and the two separate UI anchors (Lecture and clapper). A prepared prompt is not an approved asset.

The runbook/jobs replace the older sheet layout in GenerationRouting/GenerationMap and the superseded sheet subsection in ASSETS. Use one image per job. Paper/poster jobs are opaque; the runbook's blanket instruction to request transparency needs that exception. GAME/DESK can reconcile those shared docs later; this session leaves them untouched.

GAME imports only after generation and cleanup. Handoff review targets and placement notes are in `Assets/Prompts/README.md`: inspect flat facade backs, tint/scale readability, blank runtime-lettering surfaces, separated goblin teeth surfaces and hand grips. Exact mesh dimensions, pivots, material assignments and triangle budgets need cleanup; prompts cannot guarantee them. Extra identity references can be attached manually for portraits once approved; the existing runner attaches only each UI layer's anchor.
