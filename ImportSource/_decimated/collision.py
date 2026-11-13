"""Prepare UCX FBX companions without opening or modifying Unreal content."""
import bpy,json,math
from pathlib import Path
from mathutils import Vector
OUT=Path(__file__).resolve().parent


def prepare(path,meshes):
    name=path.stem
    points=[o.matrix_world@v.co for o in meshes for v in o.data.vertices]
    low=Vector(tuple(min(p[i] for p in points) for i in range(3)))
    high=Vector(tuple(max(p[i] for p in points) for i in range(3)))
    dims=high-low;mid=(high+low)/2;w,d,h=dims
    boxes=[]
    def box(center,size):
        assert min(size)>0
        boxes.append({'center_m':list(center),'size_m':list(size)})
    actors=('SM_Almond','SM_Goblin','SM_LollyBug','SM_Villager','SM_Rat_Rodin','SM_Rat_Fal')
    if name in actors:
        policy='Actor capsule; static mesh has no blocking collision. Current GAME nuisance capsule is radius 40 cm, half-height 70 cm; fit mesh scale and ground offset to that actor.'
    elif name=='SM_Fenella':policy='No collision: flying director visual.'
    elif name in ('SM_Hat_Straw','SM_HeadMirror','SM_Sword','SM_Toothbrush','SM_Crook','SM_LeafMegaphone','SM_Celery','SM_CandyCrop','SM_Clapperboard','SM_LollyShard') or 'Peppermint' in name:
        policy='No static blocking collision. Use attachment socket/interaction overlap or weapon traces as appropriate.'
    else:
        policy='Import the supplied UCX shapes; do not generate a single hull that fills functional gaps.'
        if name=='SM_FortGate':
            left=(-.115-low.x);right=high.x-.115
            box(((low.x-.115)/2,mid.y,h/2),(left,d,h))
            box(((high.x+.115)/2,mid.y,h/2),(right,d,h))
            box((0,mid.y,(.295+high.z)/2),(.23,d,high.z-.295))
        elif name=='SM_Table':
            thickness=h*.09
            box((mid.x,mid.y,high.z-thickness/2),(w,d,thickness))
            for x in (-1,1):
                for y in (-1,1):box((mid.x+x*w*.39,mid.y+y*d*.39,(h-thickness)/2),(w*.10,d*.10,h-thickness))
        elif name=='SM_CardboardBox':
            walltop=h*.65;thickness=min(w,d)*.045
            box((mid.x,mid.y,thickness/2),(w,d,thickness))
            for x in (-1,1):box((mid.x+x*(w-thickness)/2,mid.y,walltop/2),(thickness,d,walltop))
            for y in (-1,1):box((mid.x,mid.y+y*(d-thickness)/2,walltop/2),(w,thickness,walltop))
        elif name=='SM_CandyFence':
            for x in (-1,1):box((mid.x+x*w*.44,mid.y,h/2),(w*.12,d*.55,h))
            for z in (.32,.68):box((mid.x,mid.y,h*z),(w*.85,d*.35,h*.10))
        elif name=='SM_CandyTree':box((0,0,h*.25),(w*.20,d*.20,h*.50))
        elif name=='SM_Signpost':
            box((mid.x,mid.y,h*.28),(w*.14,d*.38,h*.56))
            box((mid.x,mid.y,h*.76),(w,d,h*.48))
        elif name=='SM_DirectorsChair':
            box((mid.x,mid.y,h*.49),(w*.77,d*.85,h*.06))
            box((mid.x,high.y-d*.12,h*.83),(w*.77,d*.13,h*.32))
            for x in (-1,1):
                for y in (-1,1):box((mid.x+x*w*.33,mid.y+y*d*.34,h*.24),(w*.10,d*.10,h*.48))
        elif name=='SM_LightStand':
            box((0,0,.40),(.08,.08,.40))
            box((mid.x,mid.y,.67),(w,d,.29))
        elif name.startswith('SM_HouseFront'):
            box((mid.x,mid.y,h*.37),(w,d,h*.74))
            # Roof collision is a smaller conservative box, leaving empty top corners.
            box((mid.x,mid.y,h*.85),(w*.55,d,h*.26))
        else:box(mid,dims)
    record={'asset':str(path.relative_to(OUT)),'units':'metres, Blender Z-up','policy':policy,'boxes':boxes,
            'dimensions_m':list(dims),'bounds_min_m':list(low),'bounds_max_m':list(high)}
    if name=='SM_FortGate':
        record['clear_passage_m']={'width':.23,'height':.285}
        record['recommended_uniform_scale_for_220cm_clearance']=2.20/.285
        for b in boxes:
            cx,cy,cz=b['center_m'];sx,sy,sz=b['size_m']
            assert cx+sx/2<=-.11499 or cx-sx/2>=.11499 or cz-sz/2>=.29499
    folder=OUT/'_collision'/path.parent.name;folder.mkdir(parents=True,exist_ok=True)
    if boxes:
        colliders=[]
        for i,b in enumerate(boxes):
            bpy.ops.mesh.primitive_cube_add(size=1,location=b['center_m'])
            obj=bpy.context.object;obj.name='UCX_'+name+'_%02d'%i
            obj.scale=b['size_m'];colliders.append(obj)
            bpy.ops.object.transform_apply(location=False,rotation=False,scale=True)
        # Collision companion includes neutral preview geometry to support FBX import.
        saved=[];neutral=bpy.data.materials.new('CollisionPreview')
        for obj in meshes:
            saved.append((obj,list(obj.data.materials),[p.material_index for p in obj.data.polygons]))
            obj.data.materials.clear();obj.data.materials.append(neutral)
            for p in obj.data.polygons:p.material_index=0
        bpy.ops.object.select_all(action='DESELECT')
        for obj in meshes+colliders:obj.select_set(True)
        destination=folder/(name+'.fbx')
        bpy.ops.export_scene.fbx(filepath=str(destination),use_selection=True,object_types={'MESH'},
            axis_forward='-Y',axis_up='Z',apply_unit_scale=True,apply_scale_options='FBX_SCALE_UNITS',
            bake_anim=False,add_leaf_bones=False,use_custom_props=True)
        for obj,mats,indices in saved:
            obj.data.materials.clear()
            for mat in mats:obj.data.materials.append(mat)
            for face,index in zip(obj.data.polygons,indices):face.material_index=index
        for obj in colliders:bpy.data.objects.remove(obj,do_unlink=True)
        bpy.ops.object.select_all(action='DESELECT')
        record['fbx']=str(destination.relative_to(OUT))
    (folder/(name+'.json')).write_text(json.dumps(record,indent=2),encoding='utf-8')
    return record
