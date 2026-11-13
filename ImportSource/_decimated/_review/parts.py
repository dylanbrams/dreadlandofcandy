"""Inspect welded source components and their bounds for targeted cleanup."""
import bpy,bmesh,json,sys
from pathlib import Path
OUT=Path(__file__).resolve().parent
ROOT=OUT.parents[2]
names=sys.argv[sys.argv.index('--only')+1].split(',') if '--only' in sys.argv else None
rows=[]
paths=sorted((ROOT/'ImportSource/Fal').glob('*.glb'))
if '--shard' in sys.argv:paths=[ROOT/'ImportSource/Meshy/SM_LollyShard.glb']
for path in paths:
    if names and path.stem not in names: continue
    bpy.ops.wm.read_factory_settings(use_empty=True)
    bpy.ops.import_scene.gltf(filepath=str(path))
    obj=next(o for o in bpy.context.scene.objects if o.type=='MESH')
    # Bake import transform: the bounds and later cuts are in Blender Z-up metres.
    obj.data.transform(obj.matrix_world); obj.matrix_world.identity()
    bm=bmesh.new();bm.from_mesh(obj.data)
    bmesh.ops.remove_doubles(bm,verts=list(bm.verts),dist=max(obj.dimensions)*1e-7)
    bm.verts.ensure_lookup_table();bm.verts.index_update()
    seen=set();parts=[]
    for vert in bm.verts:
        if vert.index in seen:continue
        stack=[vert];seen.add(vert.index);verts=[]
        while stack:
            v=stack.pop();verts.append(v)
            for e in v.link_edges:
                w=e.other_vert(v)
                if w.index not in seen:seen.add(w.index);stack.append(w)
        low=[min(v.co[i] for v in verts) for i in range(3)]
        high=[max(v.co[i] for v in verts) for i in range(3)]
        parts.append({'vertices':len(verts),'min':low,'max':high})
    row={'name':path.stem,'parts':sorted(parts,key=lambda c:-c['vertices'])}
    rows.append(row);bm.free()
    (OUT/'parts.json').write_text(json.dumps(rows,indent=2),encoding='utf-8')
    print(path.stem,json.dumps(row['parts'][:15]),flush=True)
