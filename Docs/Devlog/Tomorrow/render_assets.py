"""Render source assets for the devlog comparison with Blender 3.3.

Run from this folder: blender -b --factory-startup --python render_assets.py
The first three are original rat candidates; the Tripo tree is a cleaned export,
included as a separate example rather than a same-prompt comparison.
"""
import bpy
from mathutils import Vector
from pathlib import Path

HERE = Path(__file__).resolve().parent
ROOT = HERE.parents[2]
OUT = HERE / 'assets'
OUT.mkdir(exist_ok=True)
ITEMS = [
    ('rat_meshy', ROOT / 'ImportSource/Meshy/SM_Rat.glb'),
    ('rat_rodin', ROOT / 'ImportSource/Hyper3D/SM_Rat_Rodin.glb'),
    ('rat_hunyuan', ROOT / 'ImportSource/Fal/SM_Rat_Fal.glb'),
    ('tree_tripo', ROOT / 'Assets/DylanGenerationFoliageTwo/Clean/SM_CandyAppleTree.glb'),
]
for name, path in ITEMS:
    bpy.ops.object.select_all(action='SELECT')
    bpy.ops.object.delete(use_global=False)
    bpy.ops.import_scene.gltf(filepath=str(path))
    meshes = [o for o in bpy.context.scene.objects if o.type == 'MESH']
    bpy.context.view_layer.update()
    points = [o.matrix_world @ Vector(corner) for o in meshes for corner in o.bound_box]
    lo = Vector(tuple(min(p[i] for p in points) for i in range(3)))
    hi = Vector(tuple(max(p[i] for p in points) for i in range(3)))
    center = (lo + hi) / 2
    size = max(hi - lo)
    bpy.ops.object.camera_add(location=center + Vector((2.1, -3.4, 1.8)).normalized() * size * 2.6)
    camera = bpy.context.object
    camera.rotation_euler = (center - camera.location).to_track_quat('-Z', 'Y').to_euler()
    camera.data.type = 'ORTHO'
    camera.data.ortho_scale = size * 1.55
    bpy.context.scene.camera = camera
    for pos, energy, scale in [((1, -3, 4), 800, 4), ((-3, -1, 2), 500, 3), ((0, 3, 3), 700, 3)]:
        bpy.ops.object.light_add(type='AREA', location=center + Vector(pos) * size)
        light = bpy.context.object
        light.data.energy = energy * size * size
        light.data.size = scale * size
        light.rotation_euler = (center - light.location).to_track_quat('-Z', 'Y').to_euler()
    scene = bpy.context.scene
    scene.render.engine = 'BLENDER_EEVEE'
    scene.render.resolution_x = 700
    scene.render.resolution_y = 700
    scene.render.resolution_percentage = 100
    scene.world.color = (0.16, 0.11, 0.18)
    scene.render.image_settings.file_format = 'PNG'
    scene.render.filepath = str(OUT / (name + '.png'))
    bpy.ops.render.render(write_still=True)
