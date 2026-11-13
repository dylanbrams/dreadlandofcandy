"""Blender background review: solid geometry makes dark limbs visible."""
import bpy
import sys
from mathutils import Vector
from pathlib import Path

ROOT = Path(__file__).resolve().parents[3]
OUT = Path(__file__).resolve().parent
names = ("SK_Mage", "SK_Archer", "SK_Barbarian")
if '--names' in sys.argv:
    names = sys.argv[sys.argv.index('--names') + 1].split(',')
clip = sys.argv[sys.argv.index('--clip') + 1] if '--clip' in sys.argv else 'Idle'
frame = int(sys.argv[sys.argv.index('--frame') + 1]) if '--frame' in sys.argv else 15
for name in names:
    bpy.ops.wm.read_factory_settings(use_empty=True)
    rigged = '--rigged' in sys.argv
    if rigged:
        source = ROOT / "ImportSource/Meshy" / (name + "_Rig") / ('A_' + name + '_Rig_' + clip + '.fbx')
        if not source.exists():
            continue
        bpy.ops.import_scene.fbx(filepath=str(source), use_image_search=False)
        bpy.context.scene.frame_set(frame)
    else:
        bpy.ops.import_scene.gltf(filepath=str(ROOT / "ImportSource/Meshy" / (name + ".glb")))
    meshes = [o for o in bpy.context.scene.objects if o.type == "MESH"]
    depsgraph = bpy.context.evaluated_depsgraph_get()
    corners = [o.matrix_world @ Vector(v) for obj in meshes
               for o in [obj.evaluated_get(depsgraph)] for v in o.bound_box]
    low = Vector(tuple(min(v[i] for v in corners) for i in range(3)))
    high = Vector(tuple(max(v[i] for v in corners) for i in range(3)))
    center = (low + high) / 2
    size = high - low
    bpy.ops.object.camera_add(location=center + Vector((0, -size.z * 2, size.z * 0.10)))
    camera = bpy.context.object
    camera.rotation_euler = (center - camera.location).to_track_quat('-Z', 'Y').to_euler()
    camera.data.type = 'ORTHO'
    camera.data.ortho_scale = max(size.z, size.x) * 1.2
    scene = bpy.context.scene
    scene.camera = camera
    scene.render.engine = 'BLENDER_WORKBENCH'
    scene.display.shading.light = 'STUDIO'
    scene.display.shading.color_type = 'SINGLE'
    scene.display.shading.single_color = (0.6, 0.65, 0.7)
    scene.display.shading.show_shadows = True
    scene.display.shading.show_cavity = True
    scene.display.shading.background_type = 'WORLD'
    scene.world = bpy.data.worlds.new('Review')
    scene.world.color = (0.15, 0.15, 0.15)
    scene.render.resolution_x = 640
    scene.render.resolution_y = 800
    scene.render.resolution_percentage = 100
    suffix = '_' + clip.lower() + '_geometry.png' if rigged else '_geometry.png'
    scene.render.filepath = str(OUT / (name + suffix))
    bpy.ops.render.render(write_still=True)
