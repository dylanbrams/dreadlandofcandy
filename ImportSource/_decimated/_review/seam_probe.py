"""In-memory test only: weld attribute splits before collapse. Does not export meshes."""
import bpy, bmesh, json
from pathlib import Path
OUT=Path(__file__).resolve().parent
ROOT=OUT.parents[2]
results=[]
for rel,target in [('Fal/SM_Boulder.glb',8000),('Hyper3D/SM_Fenella.glb',25000)]:
    bpy.ops.wm.read_factory_settings(use_empty=True)
    bpy.ops.import_scene.gltf(filepath=str(ROOT/'ImportSource'/rel))
    obj=next(o for o in bpy.context.scene.objects if o.type=='MESH')
    bpy.context.view_layer.objects.active=obj
    bm=bmesh.new(); bm.from_mesh(obj.data)
    extent=max(obj.dimensions)
    bmesh.ops.remove_doubles(bm,verts=list(bm.verts),dist=extent*1e-7)
    bm.to_mesh(obj.data); bm.free(); obj.data.update()
    obj.data.calc_loop_triangles()
    mod=obj.modifiers.new('Probe','DECIMATE'); mod.ratio=target/len(obj.data.loop_triangles)
    mod.use_collapse_triangulate=True
    bpy.ops.object.modifier_apply(modifier=mod.name)
    obj.data.calc_loop_triangles()
    bm=bmesh.new(); bm.from_mesh(obj.data)
    results.append({'file':rel,'triangles':len(obj.data.loop_triangles),'uv_layers':len(obj.data.uv_layers),
                    'boundary_edges':sum(e.is_boundary for e in bm.edges),
                    'nonmanifold_shared_edges':sum(len(e.link_faces)>2 for e in bm.edges)})
    bm.free()
(OUT/'seam_probe.json').write_text(json.dumps(results,indent=2),encoding='utf-8')
print('IN-MEMORY PROBE',results,flush=True)
