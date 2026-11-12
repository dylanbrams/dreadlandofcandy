"""Blender background: export FBXs and render the actual offline world for review.

blender -b -t 4 --python ImportSource/Terrain/export_blender.py
Only creates files under ImportSource/Terrain. Does not use Unreal.
"""
import json
import math
import sys
from pathlib import Path
import bpy
from mathutils import Vector, Matrix

ROOT = Path(__file__).resolve().parent
DATA = json.loads((ROOT / "world_manifest.json").read_text(encoding="utf-8"))
bpy.ops.object.select_all(action="SELECT")
bpy.ops.object.delete(use_global=False)
scene = bpy.context.scene
scene.unit_settings.system = "METRIC"
scene.unit_settings.scale_length = .01
mats = {}
for name, rgb in DATA["palette"].items():
    mat = bpy.data.materials.new(name)
    mat.diffuse_color = (*rgb, 1)
    mat.use_nodes = True
    mat.node_tree.nodes.get("Principled BSDF").inputs["Base Color"].default_value = (*rgb, 1)
    mat.node_tree.nodes.get("Principled BSDF").inputs["Roughness"].default_value = .85
    mats[name] = mat
ground = bpy.data.materials.new("Terrain")
ground.use_nodes = True
tex = ground.node_tree.nodes.new("ShaderNodeTexImage")
tex.image = bpy.data.images.load(str(ROOT / "Sources" / "T_SugarboroughGround.png"))
tex.image.colorspace_settings.name = "Non-Color"
ground.node_tree.links.new(tex.outputs["Color"], ground.node_tree.nodes.get("Principled BSDF").inputs["Base Color"])
ground.node_tree.nodes.get("Principled BSDF").inputs["Roughness"].default_value = .9
mats["Terrain"] = ground
meshes = {}
(ROOT / "FBX").mkdir(exist_ok=True)
for record in DATA["meshes"]:
    vertices, faces, material_names, face_materials = [], [], [], []
    current = None
    for line in (ROOT / "Sources" / (record["name"] + ".obj")).read_text().splitlines():
        fields = line.split()
        if not fields:
            continue
        if fields[0] == "v":
            vertices.append(tuple(map(float, fields[1:4])))
        elif fields[0] == "usemtl":
            current = fields[1]
            if current not in material_names:
                material_names.append(current)
        elif fields[0] == "f":
            faces.append(tuple(int(i.split("/")[0]) - 1 for i in fields[1:]))
            face_materials.append(material_names.index(current))
    mesh = bpy.data.meshes.new(record["name"])
    mesh.from_pydata(vertices, [], faces)
    mesh.update()
    if record["name"].startswith("SM_Terrain"):
        uv = mesh.uv_layers.new(name="UVMap")
        for loop in mesh.loops:
            v = mesh.vertices[loop.vertex_index].co
            uv.data[loop.index].uv = ((v.x + 16000) / 32000, (v.y + 16000) / 32000)
    for name in material_names:
        mesh.materials.append(mats[name])
    for poly, index in zip(mesh.polygons, face_materials):
        poly.material_index = index
    obj = bpy.data.objects.new(record["name"], mesh)
    scene.collection.objects.link(obj)
    bpy.ops.object.select_all(action="DESELECT")
    obj.select_set(True)
    bpy.context.view_layer.objects.active = obj
    # Unreal's FBX import changes handedness by negating Y. Compensate in the
    # exported geometry while retaining native world coordinates in the preview.
    export_mesh = mesh.copy()
    export_mesh.transform(Matrix.Diagonal((1, -1, 1, 1)))
    export_mesh.flip_normals()
    obj.data = export_mesh
    bpy.ops.export_scene.fbx(filepath=str(ROOT / "FBX" / (record["name"] + ".fbx")), use_selection=True,
        object_types={"MESH"}, apply_unit_scale=True, axis_forward="-Y", axis_up="Z", bake_anim=False,
        use_mesh_modifiers=True, mesh_smooth_type="FACE", add_leaf_bones=False)
    obj.data = mesh
    bpy.data.meshes.remove(export_mesh)
    meshes[record["name"]] = mesh
    if not record["name"].startswith("SM_Terrain"):
        bpy.data.objects.remove(obj, do_unlink=True)

if "--export-only" in sys.argv:
    print("TERRAIN FBX EXPORT PASS")
    sys.exit(0)

for p in DATA["placements"]:
    obj = bpy.data.objects.new(p["mesh"], meshes[p["mesh"]])
    scene.collection.objects.link(obj)
    obj.location = p["location"]
    obj.scale = p["scale"]
    obj.rotation_euler.z = math.radians(p["yaw"])

world = bpy.data.worlds.new("Candy sky")
world.use_nodes = True
world.node_tree.nodes["Background"].inputs[0].default_value = (.48, .68, .85, 1)
world.node_tree.nodes["Background"].inputs[1].default_value = .65
scene.world = world
sun_data = bpy.data.lights.new("Warm sunshine", "SUN")
sun_data.energy = 2.0
sun_data.angle = math.radians(15)
sun = bpy.data.objects.new("Warm sunshine", sun_data)
scene.collection.objects.link(sun)
sun.rotation_euler = (.4, -.6, -.4)
camera_data = bpy.data.cameras.new("Review")
camera = bpy.data.objects.new("Review", camera_data)
scene.collection.objects.link(camera)
scene.camera = camera
scene.render.engine = "CYCLES"
scene.cycles.samples = 24
scene.cycles.use_denoising = True
scene.render.resolution_x = 1600
scene.render.resolution_y = 1000
scene.render.resolution_percentage = 100
scene.view_settings.view_transform = "Standard"
scene.render.image_settings.file_format = "PNG"
scene.render.film_transparent = False
(ROOT / "Review").mkdir(exist_ok=True)
for name, eye, target, lens in [
    ("overview", (25000, -29000, 29000), (0, 0, 0), 44),
    ("village", (3800, -4500, 2600), (0, 0, 150), 42),
    ("meadow", (1700, -350, 220), (6500, 0, 220), 26),
]:
    camera.location = eye
    camera.rotation_euler = (Vector(target) - camera.location).to_track_quat("-Z", "Y").to_euler()
    camera_data.type = "PERSP"
    camera_data.lens = lens
    camera_data.clip_end = 200000
    scene.render.filepath = str(ROOT / "Review" / (name + ".png"))
    bpy.ops.render.render(write_still=True)
    if "--overview-only" in sys.argv:
        break
print("TERRAIN FBX EXPORT AND OFFLINE PREVIEW PASS")
