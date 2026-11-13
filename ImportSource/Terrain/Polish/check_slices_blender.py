"""Inspect saved spring geometry: four separate rinds, inside and half submerged."""
import json
from pathlib import Path
import bpy

HERE=Path(__file__).resolve().parent
bpy.ops.wm.open_mainfile(filepath=str(HERE/'Blend/SM_LemonadeSpring.blend'))
obj=next(o for o in bpy.context.scene.objects if o.type=='MESH')
slot=next(i for i,s in enumerate(obj.material_slots) if s.material.name=='LemonRind')
indices={v for f in obj.data.polygons if f.material_index==slot for v in f.vertices}
edges={i:set() for i in indices}
for edge in obj.data.edges:
    a,b=edge.vertices
    if a in indices and b in indices:
        edges[a].add(b);edges[b].add(a)
components=[]
unseen=set(indices)
while unseen:
    stack=[unseen.pop()];component=[]
    while stack:
        i=stack.pop();component.append(i)
        adjacent=edges[i]&unseen
        unseen.difference_update(adjacent);stack.extend(adjacent)
    components.append(component)
assert len(components)==4,len(components)
bounds=[]
for component in components:
    vertices=[obj.data.vertices[i].co for i in component]
    zmin=min(v.z for v in vertices);zmax=max(v.z for v in vertices)
    assert zmin<26<zmax,(zmin,zmax)
    assert all((v.x/365)**2+(v.y/290)**2<1 for v in vertices),'Slice outside basin'
    bounds.append([zmin,zmax])
report=dict(status='passed',slices=4,inside_basin=True,all_straddle_surface=True,water_z=26,rind_z_bounds=bounds)
(HERE/'slice_validation.json').write_text(json.dumps(report,indent=2)+'\n')
print('FLOATING LEMON SLICE CHECK PASS '+json.dumps(report))
