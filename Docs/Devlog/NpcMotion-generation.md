# NPC rig and motion generation

Generated after the user authorized bounded Meshy NPC rig work. The three sources are the approved current NPC GLBs listed in the generation manifests and used by the project’s `SM_Villager`, `SM_Goblin`, and `SM_Rat` art names. Each source was copied through a Blender 3.3 decimation export into a separate rig-input GLB; source files were not overwritten.

## Local rig inputs

`Scripts/prepare_npc_rigs.py` imports each source, applies a collapse decimator while retaining UVs and materials, and exports to `ImportSource/Meshy/NpcRigInputs/`.

| Rig | Source GLB | Source triangles | Output triangles | Rig input | Bytes |
| --- | --- | ---: | ---: | --- | ---: |
| Villager | `ImportSource/Hyper3D/SM_Villager.glb` (Rodin manifest) | 500,000 | 24,999 | `ImportSource/Meshy/NpcRigInputs/Villager.glb` | 7,880,844 |
| Goblin | `ImportSource/Hyper3D/SM_Goblin.glb` (Rodin manifest) | 500,000 | 25,000 | `ImportSource/Meshy/NpcRigInputs/Goblin.glb` | 8,911,392 |
| Rat | `ImportSource/Hyper3D/SM_Rat_Rodin.glb` (selected Rodin rat) | 500,000 | 25,000 | `ImportSource/Meshy/NpcRigInputs/Rat.glb` | 8,739,148 |

Blender command (hidden background process):

```powershell
& 'C:\Program Files\Blender Foundation\Blender 3.3\blender.exe' -b --python-exit-code 1 -P Scripts/prepare_npc_rigs.py
```

## Meshy jobs

Each job used the local reduced GLB, UE5 mannequin skeleton, the specified physical height, preset action `0` (Idle), and preset action `30` (Casual Walk) labelled `Walk` to match the expected runtime asset names. The jobs ran sequentially. Each completed rig plus both animation exports and reported 11 consumed Meshy credits (33 total).

| Rig | Height | Command | Output directory |
| --- | ---: | --- | --- |
| Villager | 1.7 m | `python -I Scripts/meshy_rig.py --glb ImportSource/Meshy/NpcRigInputs/Villager.glb --name SK_Villager_Rig --height 1.7 --action 0:Idle --action 30:Walk` | `ImportSource/Meshy/SK_Villager_Rig/` |
| Goblin | 1.3 m | `python -I Scripts/meshy_rig.py --glb ImportSource/Meshy/NpcRigInputs/Goblin.glb --name SK_Goblin_Rig --height 1.3 --action 0:Idle --action 30:Walk` | `ImportSource/Meshy/SK_Goblin_Rig/` |
| Rat | 0.8 m | `python -I Scripts/meshy_rig.py --glb ImportSource/Meshy/NpcRigInputs/Rat.glb --name SK_Rat_Rig --height 0.8 --action 0:Idle --action 30:Walk` | `ImportSource/Meshy/SK_Rat_Rig/` |

Every output directory contains `<name>.fbx`, `<name>.glb`, `A_<name>_Idle.fbx`, `A_<name>_Walk.fbx`, the supplied basic-walk FBX, and `rig.json` task state. FBX byte sizes for the required outputs:

| Rig | Skeletal mesh | Idle | Walk |
| --- | ---: | ---: | ---: |
| Villager | 8,165,980 | 8,309,676 | 8,313,564 |
| Goblin | 9,217,596 | 9,360,732 | 9,364,636 |
| Rat | 9,037,788 | 9,180,780 | 9,184,828 |

An initial villager request against the unmodified 500,000-triangle source was rejected by Meshy with HTTP 400 (rig input limit 380,000 triangles); it produced no rig task or output. The local decimation above was added in response. The three reduced-input jobs succeeded. No Unreal import, animation review, or gameplay integration was performed here.

## GAME integration and review

The primary session imported all three rigs and Idle/Walk/BasicWalk against their
own skeletons with `Scripts/import_npc_rigs.py`. Root motion is locked and imported
textures are capped at 1024 for cooking. Import log: `Saved/NpcMotion/import.log`.
Runtime checks mesh/clip skeleton compatibility, retains procedural/static fallback,
and drives walk speed from actual distance moved; collision and actor locations stay
owned by the existing gameplay code. Bugs and almonds keep procedural animation.

Editor build and 86/86 Jam tests passed before the review helper; a final editor
rebuild including the helper and another session's pending HUD source also passed.
Village and first-two-scene runs completed without runtime asserts; both scenes
wrapped. Some captures were obscured by scenery or landed after combat had ended.
The final unobstructed lineup uses real NPC classes, including their runtime rig
setup: template map, `-backlot -artreview=motion`, captured as
`Saved/Screenshots/WindowsEditor/npc-motion-lineup.png` and inspected. Rat, goblin,
bug, almond and villager are coloured and grounded. Baker headwear is still attached
to the body pivot and can drift alongside the animated head; bone/socket fitting is
the next polish item, documented in Fixes5 #23. No claim of a full manual animation
deformation or performance sweep. No package/cook/upload in this task.

Engine released to Dylan's waiting Claude at 14:10 +02:00. The latest Group A source
and review helper have not had a second full test run in this session.
