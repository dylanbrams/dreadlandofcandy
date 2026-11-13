"""Run with Blender 3.3: blender.exe -b -P decimate.py."""
import hashlib
import json
from pathlib import Path
import struct
import traceback
import sys
import importlib.util

import bpy
import bmesh
import numpy as np

sys.dont_write_bytecode = True

ROOT = Path(__file__).resolve().parent
SOURCE = ROOT / 'ImportSource'
OUTPUT = SOURCE / '_decimated'
_spec = importlib.util.spec_from_file_location('asset_cleanup', OUTPUT / 'cleanup.py')
cleanup = importlib.util.module_from_spec(_spec)
_spec.loader.exec_module(cleanup)


def exported_topology(path):
    """Weld positions across all material primitives; attribute seams are not holes."""
    data = path.read_bytes()
    length = struct.unpack_from('<I', data, 12)[0]
    doc = json.loads(data[20:20+length]); binary = data[28+length:]
    def accessor(index):
        a = doc['accessors'][index]; v = doc['bufferViews'][a['bufferView']]
        dtype = {5126:'<f4',5125:'<u4',5123:'<u2',5121:'u1'}[a['componentType']]
        width = {'SCALAR':1,'VEC2':2,'VEC3':3,'VEC4':4}[a['type']]
        item = np.dtype(dtype).itemsize
        return np.ndarray((a['count'],width),dtype=dtype,buffer=binary,
            offset=v.get('byteOffset',0)+a.get('byteOffset',0),
            strides=(v.get('byteStride',width*item),item))
    boundaries = nonmanifold = 0
    for mesh in doc.get('meshes', []):
        positions=[]; faces=[]; offset=0
        for p in mesh['primitives']:
            verts=accessor(p['attributes']['POSITION']); positions.append(verts)
            indices=accessor(p['indices']).reshape(-1,3) if 'indices' in p else np.arange(len(verts)).reshape(-1,3)
            faces.append(indices.astype(np.int64)+offset); offset+=len(verts)
        _, mapping=np.unique(np.concatenate(positions),axis=0,return_inverse=True)
        tri=mapping[np.concatenate(faces)]
        edges=np.sort(np.concatenate((tri[:,[0,1]],tri[:,[1,2]],tri[:,[2,0]])),axis=1)
        _, counts=np.unique(edges,axis=0,return_counts=True)
        boundaries+=int(np.sum(counts==1)); nonmanifold+=int(np.sum(counts>2))
    return {'boundary_edges':boundaries,'nonmanifold_edges':nonmanifold}


def digest(path):
    return hashlib.sha256(path.read_bytes()).hexdigest()


def glb_info(path):
    data = path.read_bytes()
    magic, version, size = struct.unpack_from('<III', data)
    assert magic == 0x46546C67 and version == 2 and size == len(data)
    length, kind = struct.unpack_from('<II', data, 12)
    assert kind == 0x4E4F534A
    doc = json.loads(data[20:20 + length])
    triangles = 0
    for node in doc.get('nodes', []):
        if 'mesh' not in node:
            continue
        for primitive in doc['meshes'][node['mesh']]['primitives']:
            assert primitive.get('mode', 4) == 4
            accessor = primitive.get('indices', primitive['attributes']['POSITION'])
            triangles += doc['accessors'][accessor]['count'] // 3
    base_images = set()
    for material in doc.get('materials', []):
        texture = material.get('pbrMetallicRoughness', {}).get('baseColorTexture')
        if texture:
            base_images.add(doc['textures'][texture['index']]['source'])
    for index in base_images:
        assert 'bufferView' in doc['images'][index], 'Base-colour image is not embedded'
    return triangles, len(base_images)


def triangles(mesh):
    mesh.calc_loop_triangles()
    return len(mesh.loop_triangles)


def target_for(path):
    name = path.stem.lower()
    character = name.startswith(('sm_fenella', 'sm_rat', 'sm_goblin'))
    character = character or any(word in name for word in ('npc', 'villager', 'lollybug', 'almond'))
    return 25000 if character else 8000


def process(path):
    original_hash = digest(path)
    _, source_images = glb_info(path)
    bpy.ops.wm.read_factory_settings(use_empty=True)
    bpy.ops.import_scene.gltf(filepath=str(path))
    objects = [obj for obj in bpy.context.scene.objects if obj.type == 'MESH']
    before = sum(triangles(obj.data) for obj in objects)
    target = target_for(path)
    actions = []
    for obj in objects:
        obj.data = obj.data.copy()
        obj.data.transform(obj.matrix_world)
        obj.matrix_world.identity()
        bm=bmesh.new(); bm.from_mesh(obj.data)
        bmesh.ops.remove_doubles(bm,verts=list(bm.verts),dist=max(obj.dimensions)*1e-7)
        assert not any(e.is_boundary for e in bm.edges), 'Source weld left seams'
        assert not any(len(e.link_faces)>2 for e in bm.edges), 'Source weld joined separate surfaces'
        bm.to_mesh(obj.data); bm.free(); obj.data.update()
        actions += cleanup.geometry(obj, path.stem)
    objects = [obj for obj in bpy.context.scene.objects if obj.type == 'MESH']
    cleaned = sum(triangles(obj.data) for obj in objects)
    ratio = min(1.0, target / max(cleaned, 1))
    for obj in objects:
        if ratio == 1.0:
            continue
        # Make shared mesh data independent before applying modifiers.
        uv_count = len(obj.data.uv_layers)
        bpy.context.view_layer.objects.active = obj
        obj.select_set(True)
        modifier = obj.modifiers.new('Triangle budget', 'DECIMATE')
        modifier.decimate_type = 'COLLAPSE'
        modifier.ratio = ratio
        modifier.use_collapse_triangulate = True
        bpy.ops.object.modifier_apply(modifier=modifier.name)
        assert len(obj.data.uv_layers) == uv_count, 'UV layer lost'
        obj.select_set(False)
    for obj in objects:
        actions += cleanup.finish(obj, path.stem)
    destination = OUTPUT / path.relative_to(SOURCE)
    assert destination.resolve() != path.resolve()
    destination.parent.mkdir(parents=True, exist_ok=True)
    staging = destination.with_name(destination.stem + '.pending.glb')
    bpy.ops.export_scene.gltf(
        filepath=str(staging), export_format='GLB',
        export_materials='EXPORT', export_texcoords=True,
        export_normals=True, export_image_format='AUTO',
        use_selection=False, export_extras=True,
    )
    after, output_images = glb_info(staging)
    topology = exported_topology(staging)
    assert topology['boundary_edges'] == 0, 'Export has open edges: ' + str(topology)
    assert topology['nonmanifold_edges'] == 0, 'Export has nonmanifold edges: ' + str(topology)
    assert output_images >= source_images, 'Base-colour texture lost'
    assert digest(path) == original_hash, 'Original file changed'
    assert after <= before, 'Triangle count increased'
    staging.replace(destination)
    cleanup.record(path, destination, before, after, target, actions, topology, original_hash)
    return path.relative_to(SOURCE).as_posix(), before, after, target


def main():
    assert bpy.app.version[:2] == (3, 3), 'Use Blender 3.3'
    paths = sorted(path for folder in ('Hyper3D', 'Fal')
                   for path in (SOURCE / folder).rglob('*')
                   if path.suffix.lower() == '.glb'
                   and not path.stem.lower().endswith('_v1'))
    shard=SOURCE/'Meshy/SM_LollyShard.glb'
    if shard.exists():paths.append(shard)
    if '--only-folder' in sys.argv:
        folder = sys.argv[sys.argv.index('--only-folder')+1]
        paths = [p for p in paths if p.parent.name == folder]
    if '--only' in sys.argv:
        names = sys.argv[sys.argv.index('--only')+1].split(',')
        paths = [p for p in paths if p.stem in names]
    rows, errors = [], []
    for path in paths:
        print('PROCESSING:', path, flush=True)
        try:
            rows.append(process(path))
        except Exception:
            errors.append(str(path))
            traceback.print_exc()
    print('\n{:<48} {:>12} {:>12} {:>10}'.format('File', 'Before', 'After', 'Target'))
    print('-' * 85)
    for row in rows:
        print('{:<48} {:>12,} {:>12,} {:>10,}'.format(*row))
    print('Completed: {}; failed: {}'.format(len(rows), len(errors)), flush=True)
    if errors:
        raise RuntimeError('Failed files: ' + ', '.join(errors))


if __name__ == '__main__':
    main()
