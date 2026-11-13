# Mage, Archer and Barbarian

Requested by Dylan on 2026-11-13. These are the three expansion classes in
BUILD_SPEC section 3d. The knight's locked `character` style from
`Docs/styles.json` is applied to each subject in `expansion_characters.json`.
Full prompts and rig briefs are in `Assets/Prompts/3D/SK_{Mage,Archer,Barbarian}*.txt`.
This batch is separate from the original test/batch1/batch2 job list so it
does not change approvals, routing or regenerate any existing assets.

| Character | Design | Height |
| --- | --- | --- |
| Mage | Stern grey-bearded battle mage, charcoal split coat, dark chest armour | 1.90 m |
| Archer | Athletic female ranger, charcoal leather and chainmail, uncovered face | 1.85 m |
| Barbarian | Massive muscular bearded warrior, steel harness, leather trousers | 2.15 m |

Each model requests 40,000 faces, PBR textures and an A-pose. Empty hands,
separated limbs and short fitted clothing support humanoid rigging.
Weapons and effects are separate assets. Skeleton target: `ue5_mannequin`.
Each rig requests Idle (0) and Walk (30), plus any basic walk supplied by Meshy.

Dry run: `python -I Assets/Generation/expansion_characters.py --dry-run`

Generate/resume: `python -I Assets/Generation/expansion_characters.py`

Resume one character: `python -I Assets/Generation/expansion_characters.py --only SK_Barbarian`

The runner records preview and refine task IDs immediately in
`ImportSource/_gen/expansion_characters/SK_*.json` and reuses them after interruption.
It generates models concurrently, then rigs one at a time through the existing
`Scripts/meshy_rig.py`. That client retains rig and animation task IDs in each
output folder's `rig.json`. Inspect failed task errors before retrying; a dead
generation task needs deliberate state repair rather than an automatic new charge.

Outputs: `ImportSource/Meshy/SK_<Class>.glb`, preview PNG and
`ImportSource/Meshy/SK_<Class>_Rig/` containing rigged FBX/GLB and animation FBXs.
Prompts and task IDs are also recorded in `ImportSource/Meshy/manifest.jsonl`.
Rig logs are in `ImportSource/_gen/expansion_characters/`.

Delivery does not constitute visual approval or an Unreal import. GAME must
check deformation, materials, retarget compatibility and hand sockets, then wire
the classes behind the existing `-class=` hook. No Unreal source/content is
changed by this batch.

The first Barbarian generation was incomplete below the knees. Its original
model, rig, preview and state are retained under
`ImportSource/_gen/expansion_characters/rejected_barbarian_v1/`. The corrected
subject explicitly calls for a complete body down to the soles of both boots;
the full replacement prompt is in `Assets/Prompts/3D/SK_Barbarian.txt`.

After delivery, run `python -I Assets/Generation/inspect_expansion_characters.py`
to validate binary formats, triangle counts, skin weights and key bones. The
report is `ImportSource/_gen/expansion_characters/validation.json`. Open
`ImportSource/_gen/expansion_characters/review.html` for textured previews and
posed geometry renders.
