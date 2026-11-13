"""Blender: normalize generated staff/bow and build an unambiguous rolling pin."""
from pathlib import Path
import bpy
from mathutils import Vector

ROOT = Path(__file__).resolve().parents[1]
OUT = ROOT / "ImportSource/CastWeapons"
OUT.mkdir(parents=True, exist_ok=True)

def export(name, height, grip):
    bpy.ops.object.select_all(action="DESELECT")
    meshes = [o for o in bpy.context.scene.objects if o.type == "MESH"]
    for o in meshes:
        o.select_set(True)
    bpy.context.view_layer.objects.active = meshes[0]
    bpy.ops.object.join()
    obj = bpy.context.object
    bpy.ops.object.transform_apply(location=False, rotation=True, scale=True)
    points = [obj.matrix_world @ v.co for v in obj.data.vertices]
    lo = Vector(tuple(min(v[i] for v in points) for i in range(3)))
    hi = Vector(tuple(max(v[i] for v in points) for i in range(3)))
    axis = max(range(3), key=lambda i: hi[i]-lo[i])
    direction = Vector(tuple(1 if i == axis else 0 for i in range(3)))
    rotation = direction.rotation_difference(Vector((0,0,1)))
    for v, p in zip(obj.data.vertices, points):
        v.co = rotation @ p
    points = [v.co for v in obj.data.vertices]
    lo = Vector(tuple(min(v[i] for v in points) for i in range(3)))
    hi = Vector(tuple(max(v[i] for v in points) for i in range(3)))
    pivot = Vector(((hi.x+lo.x)/2, (hi.y+lo.y)/2, lo.z+(hi.z-lo.z)*grip))
    scale = height/(hi.z-lo.z)
    for v in obj.data.vertices:
        v.co = (v.co-pivot)*scale
    obj.matrix_world.identity()
    obj.name = name
    for index, material in enumerate(obj.data.materials):
        material.name = name + "_Material_" + str(index)
    textures=OUT/"Textures"
    textures.mkdir(exist_ok=True)
    for index,image in enumerate(bpy.data.images):
        if image.type == "IMAGE" and image.size[0] > 0:
            image.filepath_raw=str(textures/(name+"_"+str(index)+".png"))
            image.file_format="PNG"
            image.save()
    bpy.ops.export_scene.fbx(filepath=str(OUT/(name+".fbx")), use_selection=True,
        object_types={"MESH"}, add_leaf_bones=False, bake_anim=False, path_mode="COPY", embed_textures=True, mesh_smooth_type="FACE")

for name, height in (("SM_MageStaff",1.8),("SM_ArcherBow",1.4)):
    bpy.ops.object.select_all(action="SELECT")
    bpy.ops.object.delete(use_global=False)
    bpy.ops.import_scene.gltf(filepath=str(ROOT/f"ImportSource/Meshy/{name}.glb"))
    export(name,height,0.5)

bpy.ops.object.select_all(action="SELECT")
bpy.ops.object.delete(use_global=False)
wood = bpy.data.materials.new("Warm oak")
wood.diffuse_color = (0.30,0.12,0.035,1)
wood.use_nodes = True
wood.node_tree.nodes.get("Principled BSDF").inputs["Base Color"].default_value = wood.diffuse_color
wood.node_tree.nodes.get("Principled BSDF").inputs["Roughness"].default_value = 0.48
iron = bpy.data.materials.new("Dull iron collars")
iron.diffuse_color = (0.11,0.12,0.14,1)
iron.use_nodes=True
bsdf=iron.node_tree.nodes.get("Principled BSDF")
bsdf.inputs["Base Color"].default_value=iron.diffuse_color
bsdf.inputs["Metallic"].default_value=0.8
bsdf.inputs["Roughness"].default_value=0.5
for radius, depth, z, mat in ((0.13,0.65,0.5,wood),(0.045,0.2,0.08,wood),(0.045,0.2,0.92,wood),
                              (0.132,0.035,0.19,iron),(0.132,0.035,0.81,iron)):
    bpy.ops.mesh.primitive_cylinder_add(vertices=32,radius=radius,depth=depth,location=(0,0,z))
    obj=bpy.context.object
    obj.data.materials.append(mat)
    bevel=obj.modifiers.new("Rounded edges","BEVEL")
    bevel.width=0.012
    bevel.segments=3
    bpy.ops.object.modifier_apply(modifier=bevel.name)
    for polygon in obj.data.polygons:
        polygon.use_smooth=True
export("SM_BarbarianRollingPin",1.05,0.08)
bpy.ops.wm.save_as_mainfile(filepath=str(OUT/"cast_weapons.blend"))
