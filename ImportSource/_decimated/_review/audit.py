"""Read-only mesh audit and four-angle textured previews; Blender 3.3."""
import bpy, bmesh, json, math, sys
import importlib.util
from pathlib import Path
from mathutils import Vector

OUT = Path(__file__).resolve().parent
ROOT = OUT.parents[2]
SOURCE = ROOT / 'ImportSource' if '--source' in sys.argv else OUT.parent
if '--source' in sys.argv:
    OUT = OUT / 'source'
    OUT.mkdir(exist_ok=True)
if '--output-subdir' in sys.argv:
    OUT=OUT/sys.argv[sys.argv.index('--output-subdir')+1]
    OUT.mkdir(exist_ok=True)
only = sys.argv[sys.argv.index('--only')+1].split(',') if '--only' in sys.argv else None
collision=None
if '--collisions' in sys.argv:
    sys.dont_write_bytecode=True
    spec=importlib.util.spec_from_file_location('collision',ROOT/'ImportSource/_decimated/collision.py')
    collision=importlib.util.module_from_spec(spec);spec.loader.exec_module(collision)

rows = json.loads((OUT/'audit.json').read_text(encoding='utf-8')) if '--merge' in sys.argv and (OUT/'audit.json').exists() else []
for path in sorted(SOURCE.glob('*/*.glb')):
    if path.parent.name not in ('Fal','Hyper3D','Meshy'):continue
    if '.pending' in path.stem:continue
    if only and path.stem not in only: continue
    print('AUDIT', path, flush=True)
    bpy.ops.wm.read_factory_settings(use_empty=True)
    bpy.ops.import_scene.gltf(filepath=str(path))
    meshes = [o for o in bpy.context.scene.objects if o.type == 'MESH']
    points = [o.matrix_world @ v.co for o in meshes for v in o.data.vertices]
    low = Vector(tuple(min(p[i] for p in points) for i in range(3)))
    high = Vector(tuple(max(p[i] for p in points) for i in range(3)))
    center = (low + high) / 2
    extent = max(high - low)
    row = {'file': str(path.relative_to(SOURCE)), 'dimensions_m': list(high-low),
           'bounds_min_m': list(low), 'bounds_max_m': list(high), 'objects': [], 'views': []}
    for obj in meshes:
        mesh = obj.data
        mesh.calc_loop_triangles()
        bm = bmesh.new(); bm.from_mesh(mesh)
        raw_boundary = sum(e.is_boundary for e in bm.edges)
        # glTF splits vertices at UV/normal seams. Weld ONLY this disposable copy.
        bmesh.ops.remove_doubles(bm, verts=list(bm.verts), dist=extent * 1e-6)
        bm.verts.ensure_lookup_table(); bm.verts.index_update()
        seen = set(); components = []
        for v in bm.verts:
            if v.index in seen: continue
            stack = [v]; seen.add(v.index); count = 0
            while stack:
                current = stack.pop(); count += 1
                for edge in current.link_edges:
                    other = edge.other_vert(current)
                    if other.index not in seen:
                        seen.add(other.index); stack.append(other)
            components.append(count)
        uv_degenerate = 0
        uv_outside = 0
        if mesh.uv_layers.active:
            uv = mesh.uv_layers.active.data
            for tri in mesh.loop_triangles:
                a,b,c = [uv[i].uv for i in tri.loops]
                uv_degenerate += abs((b-a).cross(c-a)) < 1e-12
            uv_outside = sum(any(x < -1e-5 or x > 1.00001 for x in loop.uv) for loop in uv)
        row['objects'].append({'name': obj.name, 'triangles': len(mesh.loop_triangles),
            'raw_boundary_edges': raw_boundary, 'welded_boundary_edges': sum(e.is_boundary for e in bm.edges),
            'welded_nonmanifold_shared_edges': sum(len(e.link_faces)>2 for e in bm.edges),
            'inconsistent_winding_edges': sum(e.is_manifold and not e.is_contiguous for e in bm.edges),
            'zero_area_faces': sum(f.calc_area() < extent*extent*1e-12 for f in bm.faces),
            'components_vertex_counts': sorted(components, reverse=True),
            'uv_layers': len(mesh.uv_layers), 'uv_degenerate_triangles': uv_degenerate,
            'uv_outside_01_loops': uv_outside, 'materials': [m.name for m in mesh.materials if m],
            'origin_world_m': list(obj.matrix_world.translation),
            'images': [{'name': n.image.name, 'size': list(n.image.size)} for m in mesh.materials if m and m.use_nodes
                       for n in m.node_tree.nodes if n.type == 'TEX_IMAGE' and n.image]})
        bm.free()
    if collision:row['collision']=collision.prepare(path,meshes)
    scene = bpy.context.scene
    scene.render.engine = 'BLENDER_EEVEE'
    scene.eevee.taa_render_samples = 12
    scene.eevee.use_gtao = True
    scene.eevee.gtao_distance = extent * .1
    scene.world = bpy.data.worlds.new('Review world')
    scene.world.color = (.35,.35,.35)
    scene.render.resolution_x = 400; scene.render.resolution_y = 400
    scene.render.resolution_percentage = 100
    scene.render.image_settings.file_format = 'PNG'
    scene.view_settings.view_transform = 'Standard'
    scene.view_settings.look = 'Medium High Contrast'
    if '--output-subdir' in sys.argv:scene.view_settings.exposure=-.8
    for pos, energy, size in [((2,-3,4),500,3),((-3,-1,2),250,3),((0,3,3),400,2)]:
        lamp = bpy.data.lights.new('Review area', 'AREA'); lamp.energy=energy*extent*extent; lamp.size=size*extent
        light = bpy.data.objects.new('Review area',lamp); scene.collection.objects.link(light)
        light.location = center + Vector(pos)*extent
        light.rotation_euler = (center-light.location).to_track_quat('-Z','Y').to_euler()
    cam_data=bpy.data.cameras.new('Review camera'); cam=bpy.data.objects.new('Review camera',cam_data)
    scene.collection.objects.link(cam); scene.camera=cam; cam_data.type='ORTHO'; cam_data.ortho_scale=extent*1.45
    cam_data.clip_start = max(extent*.001,.0001); cam_data.clip_end=extent*100
    for index, direction in enumerate(((1,-2,.65),(-1,2,.65),(2,1,.3),(.2,-.3,3))):
        cam.location=center+Vector(direction).normalized()*extent*4
        cam.rotation_euler=(center-cam.location).to_track_quat('-Z','Y').to_euler()
        image=OUT/(path.parent.name+'_'+path.stem+'_'+str(index)+'.png')
        scene.render.filepath=str(image); bpy.ops.render.render(write_still=True)
        row['views'].append(image.name)
    rows=[existing for existing in rows if existing['file']!=row['file']]
    rows.append(row);rows.sort(key=lambda item:item['file'])
    (OUT/'audit.json').write_text(json.dumps(rows,indent=2),encoding='utf-8')
print('AUDIT COMPLETE',len(rows),flush=True)
