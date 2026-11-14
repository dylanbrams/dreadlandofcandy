"""Export four replacement tx3 terrain tiles with the east wall opened.

Run from the project root with Blender 4.5 background. Coordinates and FBX
handedness match ImportSource/Terrain/export_blender.py.
"""
import importlib.util
import math
from pathlib import Path
import bpy
from mathutils import Matrix

ROOT = Path(__file__).resolve().parent
TERRAIN = ROOT.parent
OUT = ROOT / "FBX"
OUT.mkdir(parents=True, exist_ok=True)
spec = importlib.util.spec_from_file_location("terrain_sources", TERRAIN / "build_sources.py")
src = importlib.util.module_from_spec(spec)
spec.loader.exec_module(src)

bpy.ops.object.select_all(action="SELECT")
bpy.ops.object.delete(use_global=False)
bpy.context.scene.unit_settings.system = "METRIC"
bpy.context.scene.unit_settings.scale_length = .01
mat = bpy.data.materials.new("Terrain")
mat.use_nodes = True
tex = mat.node_tree.nodes.new("ShaderNodeTexImage")
tex.image = bpy.data.images.load(str(TERRAIN / "Sources" / "T_SugarboroughGround.png"))
tex.image.colorspace_settings.name = "Non-Color"
bsdf = mat.node_tree.nodes.get("Principled BSDF")
mat.node_tree.links.new(tex.outputs["Color"], bsdf.inputs["Base Color"])
bsdf.inputs["Roughness"].default_value = .9

def opening_height(x, y):
    h = src.height(x, y)
    edge = max(abs(x), abs(y))
    eastern_wall = src.smooth(11300, 14750, edge) * (2600 + 600 * math.sin(x / 1800) * math.cos(y / 1600))
    # Clear the eastern wall only through the playable middle; retain its north/south shoulders.
    h -= eastern_wall * (1 - src.smooth(10500, 12000, abs(y)))
    # Blend all residual terrain to the new level extension shelf by x=15000.
    h *= 1 - src.smooth(14000, 15000, x)
    return h

for ty in range(4):
    name = f"SM_EastOpening_3_{ty}"
    verts=[]
    for ix in range(33):
        for iy in range(33):
            x=8000+ix*250
            y=-16000+ty*8000+iy*250
            verts.append((x,y,opening_height(x,y)))
    faces=[]
    for ix in range(32):
        for iy in range(32):
            a=ix*33+iy
            # Preserve build_sources terrain diagonal/winding.
            faces.append((a,a+33,a+34,a+1))
    mesh=bpy.data.meshes.new(name)
    mesh.from_pydata(verts,[],faces)
    mesh.update()
    mesh.materials.append(mat)
    uv=mesh.uv_layers.new(name="UVMap")
    for loop in mesh.loops:
        v=mesh.vertices[loop.vertex_index].co
        uv.data[loop.index].uv=((v.x+16000)/32000,(v.y+16000)/32000)
    obj=bpy.data.objects.new(name,mesh)
    bpy.context.scene.collection.objects.link(obj)
    bpy.ops.object.select_all(action="DESELECT")
    obj.select_set(True)
    bpy.context.view_layer.objects.active=obj
    export=mesh.copy()
    export.transform(Matrix.Diagonal((1,-1,1,1)))
    export.flip_normals()
    obj.data=export
    bpy.ops.export_scene.fbx(filepath=str(OUT/(name+".fbx")),use_selection=True,
        object_types={"MESH"},apply_unit_scale=True,axis_forward="-Y",axis_up="Z",
        bake_anim=False,use_mesh_modifiers=True,mesh_smooth_type="FACE",add_leaf_bones=False)
    obj.data=mesh
    bpy.data.meshes.remove(export)
    zmin=min(v.co.z for v in mesh.vertices); zmax=max(v.co.z for v in mesh.vertices)
    print(f"EXPORTED {name}: 1089 verts, 2048 tris, bounds x=8000..16000 y={-16000+ty*8000}..{-8000+ty*8000} z={zmin:.2f}..{zmax:.2f}")

print("EASTERN OPENING FBX EXPORT PASS")
