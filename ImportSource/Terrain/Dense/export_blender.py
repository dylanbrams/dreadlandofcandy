"""Export original grass, cotton-candy thickets and snowflake meshes for Unreal."""
import json
from pathlib import Path
import bpy
from mathutils import Matrix

HERE = Path(__file__).resolve().parent
data = json.loads((HERE / "ecology.json").read_text())
(HERE / "FBX").mkdir(exist_ok=True)
bpy.ops.object.select_all(action="SELECT")
bpy.ops.object.delete(use_global=False)
bpy.context.scene.unit_settings.system = "METRIC"
bpy.context.scene.unit_settings.scale_length = .01
for record in data["meshes"]:
    vertices, faces, slots, indices = [], [], [], []
    mat = None
    for line in (HERE / "Sources" / (record["name"] + ".obj")).read_text().splitlines():
        fields = line.split()
        if not fields:
            continue
        if fields[0] == "v":
            vertices.append(tuple(map(float, fields[1:4])))
        elif fields[0] == "usemtl":
            mat = fields[1]
            if mat not in slots:
                slots.append(mat)
        elif fields[0] == "f":
            faces.append(tuple(int(i.split("/")[0]) - 1 for i in fields[1:]))
            indices.append(slots.index(mat))
    mesh = bpy.data.meshes.new(record["name"])
    mesh.from_pydata(vertices, [], faces)
    for name in slots:
        mesh.materials.append(bpy.data.materials.get(name) or bpy.data.materials.new(name))
    for poly, index in zip(mesh.polygons, indices):
        poly.material_index = index
    colors = mesh.color_attributes.new(name="CandyWind", type="FLOAT_COLOR", domain="CORNER")
    top = max(v[2] for v in vertices)
    for loop in mesh.loops:
        weight = max(0, vertices[loop.vertex_index][2] / max(top, 1))
        value = .6 + .4 * weight
        colors.data[loop.index].color = (value, value, value, weight)
    if "Grass" in record["name"]:
        uv = mesh.uv_layers.new(name="CandyBladeUV")
        for loop in mesh.loops:
            # Each six-vertex blade gets its own ribbon coordinates. This keeps
            # diagonal candy stripes attached to the blade during wind/yaw.
            vertex = loop.vertex_index
            uv.data[loop.index].uv = (vertex % 2, vertices[vertex][2] / 55)
    mesh.transform(Matrix.Diagonal((1, -1, 1, 1)))
    mesh.flip_normals()
    obj = bpy.data.objects.new(record["name"], mesh)
    bpy.context.scene.collection.objects.link(obj)
    bpy.ops.object.select_all(action="DESELECT")
    obj.select_set(True)
    bpy.context.view_layer.objects.active = obj
    bpy.ops.export_scene.fbx(filepath=str(HERE / "FBX" / (record["name"] + ".fbx")),
        use_selection=True, object_types={"MESH"}, apply_unit_scale=True,
        axis_forward="-Y", axis_up="Z", bake_anim=False, mesh_smooth_type="FACE")
print("DENSE FOLIAGE FBX PASS")
