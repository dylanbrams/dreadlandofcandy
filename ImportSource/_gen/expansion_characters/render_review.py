"""Blender background review: solid geometry makes dark limbs visible."""
import bpy
import sys
from mathutils import Vector
from pathlib import Path

ROOT = Path(__file__).resolve().parents[3]
OUT = Path(__file__).resolve().parent
for name in ("SK_Mage", "SK_Archer", "SK_Barbarian"):
    bpy.ops.wm.read_factory_settings(use_empty=True)
    rigged = '--rigged' in sys.argv
    if rigged:
        source = ROOT / "ImportSource/Meshy" / (name + "_Rig") / ('A_' + name + '_Rig_Idle.fbx')
        if not source.exists():
            continue
        bpy.ops.import_scene.fbx(filepath=str(source), use_image_search=False)
        bpy.context.scene.frame_set(15)
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
    scene.render.filepath = str(OUT / (name + ('_idle_geometry.png' if rigged else '_geometry.png')))
    bpy.ops.render.render(write_still=True)
