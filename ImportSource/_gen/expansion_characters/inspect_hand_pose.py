import bpy
from pathlib import Path
from mathutils import Vector
import json

ROOT = Path(__file__).resolve().parents[3]
records = []
for name in ('SK_Hero', 'SK_Knight'):
    bpy.ops.wm.read_factory_settings(use_empty=True)
    bpy.ops.import_scene.fbx(filepath=str(ROOT / 'ImportSource/Meshy' / (name + '_Rig') /
                                         ('A_' + name + '_Rig_SwordRaise.fbx')), use_image_search=False)
    rig = next(o for o in bpy.context.scene.objects if o.type == 'ARMATURE')
    for frame in (1, 5, 10, 15, 30, 45, 60):
        bpy.context.scene.frame_set(frame)
        bones = {}
        for key in ('upperarm_r', 'lowerarm_r', 'hand_r', 'RightHand_End'):
            bone = rig.pose.bones[key]
            matrix = rig.matrix_world @ bone.matrix
            bones[key] = {'head': list(rig.matrix_world @ bone.head),
                          'tail': list(rig.matrix_world @ bone.tail),
                          'rotation': list(bone.rotation_quaternion),
                          'axes': [list((matrix.to_3x3() @ Vector(axis)).normalized())
                                   for axis in ((1, 0, 0), (0, 1, 0), (0, 0, 1))]}
        records.append({'name': name, 'frame': frame, 'bones': bones})
dest = Path(__file__).with_name('hand_pose_inspection.json')
dest.write_text(json.dumps(records, indent=2))
print('Saved', dest)
