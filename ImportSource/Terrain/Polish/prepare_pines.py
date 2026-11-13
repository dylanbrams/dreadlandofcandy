"""Two original candy-pine silhouettes to complement the three warped firs."""
import hashlib
import importlib.util
import json
import math
from pathlib import Path
import bpy
from mathutils import Matrix

HERE=Path(__file__).resolve().parent
spec=importlib.util.spec_from_file_location('landmark_meshes',HERE/'prepare_landmarks.py')
p=importlib.util.module_from_spec(spec)
spec.loader.exec_module(p)
p.palette.update(PineMint=(.06,.39,.36,1),PineIce=(.1,.48,.53,1),
    PineFrost=(.91,.94,.88,1),PineTrunk=(.72,.035,.08,1),PineCandy=(.8,.11,.29,1))

def tier(z,radius,height,phase,body,broad=False):
    vertices=[]
    profile=[(.07,1),(.31,.8),(.60,.48),(.85,.18),(1,.07),(.94,0)]
    for r,h in profile:
        for j in range(32):
            a=j*math.tau/32
            ripple=1+.11*math.sin(a*(5 if broad else 3)+phase)
            x=radius*r*ripple*math.cos(a)+23*math.sin(z/190+phase)*h
            y=radius*r*ripple*math.sin(a)+16*math.cos(z/170)*h
            vertices.append((x,y,z+height*h+7*math.sin(a*3+phase)*(1-h)))
    faces=[]
    for ring in range(len(profile)-1):
        for j in range(32):
            a=ring*32+j;b=ring*32+(j+1)%32
            faces.append((a,b,b+32,a+32))
    # The profile runs top to bottom; reverse winding for outward normals.
    faces=[tuple(reversed(f)) for f in faces]
    faces.append(tuple(range(32)))
    faces.append(tuple(reversed(range(160,192))))
    mesh=bpy.data.meshes.new('Swept candy bough')
    mesh.from_pydata(vertices,[],faces)
    obj=bpy.data.objects.new('Swept candy bough',mesh)
    bpy.context.collection.objects.link(obj)
    p.finish_piece(obj,body)
    points=[]
    for j in range(65):
        a=j*math.tau/64
        r=radius*(1+.11*math.sin(a*(5 if broad else 3)+phase))
        points.append((r*math.cos(a),r*math.sin(a),z+height*.065+7*math.sin(a*3+phase)))
    p.tube(points,14 if broad else 10,'PineFrost')

def save(variant,label):
    name='SM_CandyCaneFir_Warp'+str(variant+1)
    bpy.ops.object.select_all(action='SELECT')
    bpy.context.view_layer.objects.active=next(o for o in bpy.context.scene.objects if o.type=='MESH')
    bpy.ops.object.join()
    obj=bpy.context.object
    obj.data.transform(obj.matrix_world)
    obj.matrix_world.identity()
    obj.name=name
    # Keep the same conservative 300 cm foliage exclusion radius.
    assert max(math.hypot(v.co.x,v.co.y) for v in obj.data.vertices)<300
    obj.data.calc_loop_triangles()
    triangles=len(obj.data.loop_triangles)
    assert triangles<=18000
    bounds=[[min(v.co[i] for v in obj.data.vertices),max(v.co[i] for v in obj.data.vertices)] for i in range(3)]
    bpy.ops.wm.save_as_mainfile(filepath=str(HERE/'Blend'/(name+'.blend')))
    original=obj.data
    reflected=original.copy()
    reflected.transform(Matrix.Diagonal((1,-1,1,1)))
    reflected.flip_normals()
    obj.data=reflected
    bpy.ops.export_scene.fbx(filepath=str(HERE/'FBX'/(name+'.fbx')),use_selection=True,
        object_types={'MESH'},apply_unit_scale=True,axis_forward='-Y',axis_up='Z',bake_anim=False,mesh_smooth_type='OFF')
    source=HERE.parent/'Foliage/Clean/SM_Rodin_CandyCaneFir.glb'
    return dict(name=name,category='tree',species='CandyCaneFir',variant=variant,display_name=label,
        triangles=triangles,bounds_cm=bounds,source=str(source.relative_to(HERE.parents[2])),
        source_sha256=hashlib.sha256(source.read_bytes()).hexdigest(),
        construction='Original rounded tier geometry and frosting ribbons',
        procedural_materials={s.material.name:list(p.palette[s.material.name][:3]) for s in obj.material_slots})

new=[]
p.reset()
p.cylinder((0,0,355),34,710,'PineTrunk')
for i,(z,r) in enumerate(zip([115,230,345,460,575,680],[215,188,160,126,92,56])):
    tier(z,r,170 if i<5 else 140,i*.85,'PineMint')
    a=i*1.8
    p.sphere((r*.72*math.cos(a),r*.72*math.sin(a),z+45),(18,18,24),'PineCandy',12,8)
new.append(save(3,'Spiral taffy pine'))

p.reset()
p.cylinder((0,0,205),43,410,'PineTrunk')
for i,(z,r) in enumerate([(100,240),(215,190),(330,125)]):
    tier(z,r,180,i*1.1,'PineIce',True)
    for j in range(4):
        a=j*math.tau/4+i*.7
        p.sphere((r*.7*math.cos(a),r*.7*math.sin(a),z+65),(r*.22,r*.2,36),'PineFrost',16,10)
new.append(save(4,'Marshmallow umbrella pine'))

manifest=json.loads((HERE/'assets.json').read_text())
manifest['assets']=[r for r in manifest['assets'] if r['name'] not in {a['name'] for a in new}]+new
(HERE/'assets.json').write_text(json.dumps(manifest,indent=2)+'\n')
print('NEW PINE VARIANTS PASS '+json.dumps([(r['name'],r['triangles']) for r in new]))
