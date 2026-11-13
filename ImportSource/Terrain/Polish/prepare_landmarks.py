"""Original Blender meshes inspired by the song's impossible picnic landscape."""
import json
import math
import random
from pathlib import Path
import bpy
from mathutils import Matrix, Vector

HERE=Path(__file__).resolve().parent
records=[]
palette={'Sugar':(.92,.82,.73,1),'Crystal':(.39,.82,.94,1),'Lemonade':(.91,.63,.035,1),
    'LemonRind':(.98,.77,.025,1),'LemonFlesh':(1,.88,.29,1),'Broth':(.28,.075,.018,1),
    'Carrot':(.93,.22,.012,1),'Pea':(.16,.48,.055,1),'Potato':(.86,.64,.29,1),
    'Noodle':(.96,.82,.45,1),'Whiskey':(.48,.14,.018,1),'Metal':(.55,.65,.72,1),
    'Bluebird':(.025,.31,.91,1),'BirdCream':(.91,.82,.55,1),'BirdDark':(.014,.027,.045,1),
    'SpringBed':(.48,.39,.16,1)}

def reset():
    bpy.ops.wm.read_factory_settings(use_empty=True)
    bpy.context.scene.unit_settings.system='METRIC'
    bpy.context.scene.unit_settings.scale_length=.01

def finish_piece(obj,mat):
    obj.data.materials.append(bpy.data.materials.get(mat) or bpy.data.materials.new(mat))
    obj.data.materials[0].diffuse_color=palette[mat]
    for p in obj.data.polygons:
        p.use_smooth=True
    bpy.ops.object.transform_apply(location=False,rotation=False,scale=True)
    return obj

def sphere(loc,size,mat,segments=20,rings=12):
    bpy.ops.mesh.primitive_uv_sphere_add(segments=segments,ring_count=rings,location=loc)
    obj=bpy.context.object
    obj.scale=size
    return finish_piece(obj,mat)

def cylinder(loc,radius,depth,mat,vertices=32):
    bpy.ops.mesh.primitive_cylinder_add(vertices=vertices,radius=radius,depth=depth,location=loc)
    return finish_piece(bpy.context.object,mat)

def tube(points,radius,mat):
    curve=bpy.data.curves.new('Confection ribbon','CURVE')
    curve.dimensions='3D'
    curve.bevel_depth=radius
    curve.bevel_resolution=2
    curve.resolution_u=12
    spl=curve.splines.new('POLY')
    spl.points.add(len(points)-1)
    for p,co in zip(spl.points,points):
        p.co=(*co,1)
    obj=bpy.data.objects.new('Ribbon',curve)
    bpy.context.collection.objects.link(obj)
    bpy.context.view_layer.objects.active=obj
    obj.select_set(True)
    bpy.ops.object.convert(target='MESH')
    finish_piece(obj,mat)
    obj.select_set(False)

def crystal(loc,radius,height):
    verts=[]
    for z,r in [(0,radius*.8),(height*.68,radius),(height,radius*.1)]:
        for i in range(6):
            a=i*math.tau/6
            verts.append((loc[0]+math.cos(a)*r,loc[1]+math.sin(a)*r,loc[2]+z))
    faces=[]
    for ring in range(2):
        for i in range(6):
            j=ring*6+i;k=ring*6+(i+1)%6
            faces.append((j,k,k+6,j+6))
    faces.append(tuple(range(12,18)))
    faces.append(tuple(reversed(range(6))))
    mesh=bpy.data.meshes.new('Sugar crystal')
    mesh.from_pydata(verts,[],faces)
    obj=bpy.data.objects.new('Sugar crystal',mesh)
    bpy.context.collection.objects.link(obj)
    finish_piece(obj,'Crystal')
    for face in obj.data.polygons:
        face.use_smooth=False

def rim(rx,ry,z=0):
    for i in range(22):
        a=i*math.tau/22
        sphere((rx*math.cos(a),ry*math.sin(a),z),(rx*.16,ry*.16,38),'Sugar',12,8)

def disk(rx,ry,z,mat):
    obj=cylinder((0,0,z),1,8,mat,64)
    obj.scale=(rx,ry,1)
    bpy.ops.object.transform_apply(location=False,rotation=False,scale=True)

def liquid_surface(rx,ry,z,mat):
    # One translucent sheet, rather than two stacked sides of a thin cylinder.
    verts=[(0,0,z)]+[(rx*math.cos(i*math.tau/64),ry*math.sin(i*math.tau/64),z) for i in range(64)]
    mesh=bpy.data.meshes.new('Liquid surface')
    mesh.from_pydata(verts,[],[(0,i+1,(i+1)%64+1) for i in range(64)])
    obj=bpy.data.objects.new('Liquid surface',mesh)
    bpy.context.collection.objects.link(obj)
    finish_piece(obj,mat)

def lemon_slice(x,y,radius,tilt):
    before=set(bpy.context.scene.objects)
    cylinder((x,y,26),radius,16,'LemonRind',48)
    cylinder((x,y,34.5),radius*.84,3,'LemonFlesh',48)
    for i in range(8):
        a=i*math.tau/8
        tube([(x,y,37),(x+radius*.79*math.cos(a),y+radius*.79*math.sin(a),37)],1.8,'Sugar')
    cylinder((x,y,37),radius*.08,2,'Sugar',16)
    center=Vector((x,y,26))
    rotation=Matrix.Rotation(tilt,4,'X')
    for obj in set(bpy.context.scene.objects)-before:
        # Apply tilt to the entire slice without separating pulp and rind.
        obj.data.transform(obj.matrix_world)
        obj.matrix_world.identity()
        for v in obj.data.vertices:
            v.co=rotation@(v.co-center)+center

def export(name,category,location,radius,label):
    bpy.ops.object.select_all(action='SELECT')
    bpy.context.view_layer.objects.active=next(o for o in bpy.context.scene.objects if o.type=='MESH')
    bpy.ops.object.join()
    obj=bpy.context.object
    # Bake translations so pivot is at the landmark's authored center.
    obj.data.transform(obj.matrix_world)
    obj.matrix_world.identity()
    obj.name=name
    obj.data.calc_loop_triangles()
    triangles=len(obj.data.loop_triangles)
    bpy.ops.wm.save_as_mainfile(filepath=str(HERE/'Blend'/(name+'.blend')))
    mesh=obj.data.copy()
    mesh.transform(Matrix.Diagonal((1,-1,1,1)))
    mesh.flip_normals()
    obj.data=mesh
    bpy.ops.export_scene.fbx(filepath=str(HERE/'FBX'/(name+'.fbx')),use_selection=True,
        object_types={'MESH'},apply_unit_scale=True,axis_forward='-Y',axis_up='Z',bake_anim=False,mesh_smooth_type='OFF')
    records.append(dict(name=name,category=category,location=location,radius=radius,label=label,
        triangles=triangles,materials=[s.material.name for s in obj.material_slots]))
    print('LANDMARK',name,triangles,flush=True)

def main():
    reset()
    rim(170,170)
    disk(147,147,12,'Crystal')
    for i in range(7):
        a=i*math.tau/7
        crystal((90*math.cos(a),90*math.sin(a),20),19,110+(i%3)*35)
    crystal((0,0,12),35,240)
    for i in range(5):
        a=i*math.tau/5
        tube([(math.cos(a)*(25+100*t),math.sin(a)*(25+100*t),215+80*t-260*t*t) for t in [j/18 for j in range(19)]],4,'Crystal')
    export('SM_CrystalFountain','fountain',[650,750],205,'CRYSTAL FOUNTAIN')

    reset()
    rim(410,335)
    disk(365,290,-10,'SpringBed')
    liquid_surface(365,290,26,'Lemonade')
    for i in range(5):
        crystal((-140+i*58,175+(i%2)*35,15),30,110+i%3*50)
        tube([(-140+i*58,175-120*t,140+60*t-180*t*t) for t in [j/16 for j in range(17)]],6,'Lemonade')
    # Half-submerged horizontal wheels float inside the basin, rather than a
    # single upright lemon beside it. Slightly different tilts catch the waterline.
    for x,y,r,tilt in [(-165,-75,77,.075),(85,-130,91,-.10),(195,65,69,.14),(-30,75,61,-.06)]:
        lemon_slice(x,y,r,tilt)
    for i in range(12):
        a=i*2.4
        sphere((math.cos(a)*210,math.sin(a)*160,27),(9,9,7),'LemonFlesh',12,8)
    export('SM_LemonadeSpring','spring',[4500,4500],550,'LEMONADE SPRING')

    reset()
    rim(560,445)
    disk(510,390,10,'Broth')
    rng=random.Random(1928)
    for i in range(30):
        a=rng.uniform(0,math.tau);r=math.sqrt(rng.random())
        x,y=math.cos(a)*430*r,math.sin(a)*320*r
        mat=['Carrot','Pea','Potato'][i%3]
        if mat=='Carrot':
            carrot=cylinder((x,y,17),rng.uniform(16,29),12,mat,20)
        else:
            sphere((x,y,14),(23,19,13) if mat=='Potato' else (12,12,12),mat,12,8)
    for i in range(7):
        x,y=rng.uniform(-230,230),rng.uniform(-160,160)
        tube([(x+j*12,y+10*math.sin(j),18) for j in range(8)],5,'Noodle')
    spoon=sphere((355,-220,45),(100,62,14),'Metal')
    tube([(400,-220,45),(520,-210,85),(650,-200,150)],15,'Metal')
    export('SM_StewPond','pond',[6500,7000],750,'LAKE OF STEW')

    reset()
    # A meandering amber channel in a shallow rock bed, with a bottle at its head.
    centers=[(-370+i*42,55*math.sin(i*.45),0) for i in range(19)]
    for i,(x,y,z) in enumerate(centers):
        for side in [-1,1]:
            sphere((x,y+side*80,-3),(43,38,27),'Sugar',12,8)
    verts=[]
    for x,y,z in centers:
        verts.extend([(x,y-63,8),(x,y+63,8)])
    mesh=bpy.data.meshes.new('Amber water')
    mesh.from_pydata(verts,[],[(i*2,i*2+1,i*2+3,i*2+2) for i in range(18)])
    obj=bpy.data.objects.new('Amber water',mesh)
    bpy.context.collection.objects.link(obj)
    finish_piece(obj,'Whiskey')
    body=cylinder((-440,0,95),64,180,'Whiskey',32)
    neck=cylinder((-440,0,215),24,70,'Whiskey',24)
    band=cylinder((-440,0,95),65,60,'Sugar',32)
    tube([(-440,0,250),(-410,0,260),(-370,0,200),(-350,0,40)],8,'Whiskey')
    export('SM_WhiskeyRun','stream',[-6500,-500],600,'WHISKEY RUN')

    reset()
    sphere((0,0,18),(16,24,21),'Bluebird')
    sphere((0,-17,43),(15,15,15),'Bluebird')
    sphere((0,-15,15),(13,13,13),'BirdCream')
    for side in [-1,1]:
        wing=sphere((side*16,0,22),(6,21,13),'Bluebird')
        wing.rotation_euler.y=side*.3
        sphere((side*8,-29,47),(3,3,3),'BirdDark',12,8)
    tube([(0,16,20),(0,44,10)],7,'Bluebird')
    bpy.ops.mesh.primitive_cone_add(vertices=12,radius1=6,radius2=0,depth=15,location=(0,-35,40),rotation=(math.pi/2,0,0))
    finish_piece(bpy.context.object,'LemonRind')
    for side in [-1,1]:
        tube([(side*7,0,3),(side*7,0,-9),(side*7,-10,-10)],2,'LemonRind')
    export('SM_Bluebird','bird',[4500,4500],0,'Bluebird')

    (HERE/'landmarks.json').write_text(json.dumps(dict(assets=records,palette=palette),indent=2)+'\n')
    print('LANDMARK BLENDER PASS')

if __name__ == "__main__":
    main()
