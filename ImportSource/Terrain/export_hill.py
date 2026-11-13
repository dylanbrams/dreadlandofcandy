"""Blender background: re-export only terrain tiles with changed hill geometry."""
import json
from pathlib import Path
import bpy
from mathutils import Matrix

ROOT = Path(__file__).resolve().parent
names = json.loads((ROOT/'hill_sources.json').read_text())['tiles']
bpy.ops.object.select_all(action='SELECT')
bpy.ops.object.delete(use_global=False)
bpy.context.scene.unit_settings.system = 'METRIC'
bpy.context.scene.unit_settings.scale_length = .01
material = bpy.data.materials.new('Terrain')
for name in names:
    verts, faces = [], []
    for line in (ROOT/'Sources'/(name+'.obj')).read_text().splitlines():
        fields = line.split()
        if fields and fields[0] == 'v':
            verts.append(tuple(map(float, fields[1:])))
        elif fields and fields[0] == 'f':
            faces.append(tuple(int(v.split('/')[0])-1 for v in fields[1:]))
    mesh = bpy.data.meshes.new(name)
    mesh.from_pydata(verts, [], faces)
    mesh.update()
    uv = mesh.uv_layers.new(name='UVMap')
    for loop in mesh.loops:
        v = mesh.vertices[loop.vertex_index].co
        uv.data[loop.index].uv = ((v.x+16000)/32000, (v.y+16000)/32000)
    mesh.materials.append(material)
    mesh.transform(Matrix.Diagonal((1,-1,1,1)))
    mesh.flip_normals()
    obj = bpy.data.objects.new(name, mesh)
    bpy.context.collection.objects.link(obj)
    bpy.ops.object.select_all(action='DESELECT')
    obj.select_set(True)
    bpy.context.view_layer.objects.active = obj
    bpy.ops.export_scene.fbx(filepath=str(ROOT/'FBX'/(name+'.fbx')), use_selection=True,
        object_types={'MESH'}, apply_unit_scale=True, axis_forward='-Y', axis_up='Z',
        bake_anim=False, use_mesh_modifiers=True, mesh_smooth_type='FACE', add_leaf_bones=False)
    bpy.data.objects.remove(obj, do_unlink=True)
print('HILL FBX EXPORT PASS')
