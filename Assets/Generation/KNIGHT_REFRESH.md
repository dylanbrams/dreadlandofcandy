# Knight continuity refresh

Dylan prefers the aesthetic of the new Mage, Archer and Barbarian and requested
a matching knight on 2026-11-13. `SK_Knight` is a new source delivery; `SK_Hero`
and `SK_Hero_Rig` remain intact for comparison and rollback.

The subject keeps the closed helmet, dark plate, tabard and 2.1 m height, while
asking for broad clean shapes, smooth surfaces, rounded pauldrons, restrained
ornament and satin metal. It uses the same character wrapper, 40k face target,
PBR refinement, Meshy model selection and UE5 mannequin skeleton as the new cast.

Config: `Assets/Generation/knight_refresh.json`.
Full prompt: `Assets/Prompts/3D/SK_Knight.txt`.

Generate/resume:
`python -I Assets/Generation/expansion_characters.py --config Assets/Generation/knight_refresh.json`

The knight gets Idle, Walk, ProudStrut, OneHandUp, SwordRaise, Scrub, Sigh and
the basic walk supplied with rigging. The last three reuse the original
knight's completed text-to-motion task IDs and retarget those motions onto
the new rig rather than regenerating the motion descriptions.

Validate:
`python -I Assets/Generation/inspect_expansion_characters.py --config Assets/Generation/knight_refresh.json --report knight_validation.json`

Outputs: `ImportSource/Meshy/SK_Knight.*` and
`ImportSource/Meshy/SK_Knight_Rig/`. State and review outputs:
`ImportSource/_gen/expansion_characters/`.

This is an ASSETS delivery. GAME must import the new mesh and clips, verify
grip/scale/deformation and replace the runtime hero references. Runtime source
and Content are not changed by the generation runner.

Delivered: 41,119 triangles, 28 joints, nine valid FBXs (rig plus eight
animations) and rigged GLB. Full-body geometry, Idle frame 15 and SwordRaise
frame 30 were rendered and inspected in Blender. See
`ImportSource/_gen/expansion_characters/knight_review.html` for the cast lineup
and original-knight comparison. Meshy used 56 credits; final balance 584.
