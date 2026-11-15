import bpy, bmesh, json, math
from pathlib import Path
from mathutils import Vector

BASE=Path(__file__).resolve().parent
import os
NAMES=tuple(os.environ.get("MM_ONLY","MM_CreamClassic,MM_TwistedCream,MM_SquashedCream").split(","))
TARGET_TRIANGLES=4800
TARGET_LENGTH=0.22
REPORT=[]

def connected_components(bm):
    unseen=set(bm.verts); comps=[]
    while unseen:
        start=unseen.pop(); stack=[start]; comp={start}
        while stack:
            v=stack.pop()
            for e in v.link_edges:
                n=e.other_vert(v)
                if n in unseen: unseen.remove(n);comp.add(n);stack.append(n)
        comps.append(comp)
    return comps

def set_camera_and_render(obj, name):
    bpy.ops.object.select_all(action='DESELECT');obj.select_set(True);bpy.context.view_layer.objects.active=obj
    center=Vector((0,0,0)); bpy.ops.object.origin_set(type='ORIGIN_GEOMETRY',center='BOUNDS')
    # center in X/Y while keeping the bottom on the floor
    b=[obj.matrix_world @ Vector(corner) for corner in obj.bound_box]
    mid=Vector(((min(p.x for p in b)+max(p.x for p in b))/2,(min(p.y for p in b)+max(p.y for p in b))/2,min(p.z for p in b)))
    obj.location-=mid
    bpy.context.view_layer.update()
    # Soft studio camera and lights; previews are rendered against a neutral transparent world.
    bpy.ops.object.camera_add(location=(0.34,-0.48,0.31)); cam=bpy.context.object;cam.name='PreviewCamera'
    direction=Vector((0,0,0.095))-cam.location;cam.rotation_euler=direction.to_track_quat('-Z','Y').to_euler();cam.data.type='ORTHO';cam.data.ortho_scale=0.39;bpy.context.scene.camera=cam
    lights=[]
    for loc,power,size in [((-.3,-.3,.45),420,.35),((.28,.1,.32),260,.3),((0,.3,.4),180,.25)]:
        bpy.ops.object.light_add(type='AREA',location=loc);light=bpy.context.object;light.data.energy=power;light.data.shape='DISK';light.data.size=size
        light.rotation_euler=(Vector((0,0,.08))-light.location).to_track_quat('-Z','Y').to_euler();lights.append(light)
    scene=bpy.context.scene;scene.render.engine='BLENDER_EEVEE';scene.eevee.use_gtao=True;scene.eevee.gtao_distance=.2;scene.eevee.gtao_factor=1.25
    scene.render.resolution_x=512;scene.render.resolution_y=512;scene.render.resolution_percentage=100
    scene.render.image_settings.file_format='PNG';scene.render.film_transparent=True
    scene.view_settings.view_transform='Standard';scene.view_settings.look='Medium High Contrast';scene.view_settings.exposure=0;scene.view_settings.gamma=1
    scene.render.filepath=str(BASE/(name+'_clean.png'));bpy.ops.render.render(write_still=True)
    bpy.data.objects.remove(cam,do_unlink=True)
    for light in lights:bpy.data.objects.remove(light,do_unlink=True)

for name in NAMES:
    bpy.ops.object.select_all(action='SELECT');bpy.ops.object.delete(use_global=False)
    # MM_SquashedCream's generated GLB is a broken shell (mixed inside-out faces, ~10% of the expected signed volume):
    # with the one-sided marshmallow material it renders as a hole in the underside. Build it from the watertight
    # MM_CreamClassic GLB instead and squash it in the squash step below.
    bpy.ops.import_scene.gltf(filepath=str(BASE/(('MM_CreamClassic' if name=='MM_SquashedCream' else name)+'.glb')))
    meshes=[o for o in bpy.context.scene.objects if o.type=='MESH']
    if not meshes: raise RuntimeError(name+' has no mesh objects')
    # Join generated submeshes, then remove unreferenced mesh fragments and duplicate vertices.
    bpy.ops.object.select_all(action='DESELECT')
    source_triangles=sum(max(0,len(p.vertices)-2) for o in meshes for p in o.data.polygons)
    for o in meshes:o.select_set(True)
    bpy.context.view_layer.objects.active=meshes[0];bpy.ops.object.join();obj=bpy.context.object;obj.name=name+'_clean'
    bm=bmesh.new();bm.from_mesh(obj.data);bmesh.ops.remove_doubles(bm,verts=bm.verts,dist=0.00001)
    comps=connected_components(bm);counts=[sum(len(f.verts)==3 for v in c for f in v.link_faces)//3 for c in comps]
    keep=max(comps,key=len);removed_components=max(0,len(comps)-1)
    bmesh.ops.delete(bm,geom=[v for c in comps if c is not keep for v in c],context='VERTS')
    loose=[v for v in bm.verts if not v.link_faces]
    removed_loose=len(loose)
    if loose:bmesh.ops.delete(bm,geom=loose,context='VERTS')
    bmesh.ops.recalc_face_normals(bm,faces=bm.faces);bm.to_mesh(obj.data);bm.free();obj.data.update()
    if name=='MM_SquashedCream':
        from mathutils import Matrix
        me=obj.data;zs=[v.co.z for v in me.vertices];z0,z1=min(zs),max(zs)
        for v in me.vertices:
            t=(v.co.z-z0)/(z1-z0)            # 0 bottom .. 1 top
            k=1.18-0.30*t                    # fat at the base, narrower on top
            v.co.x*=k*1.08;v.co.y*=k*0.88;v.co.z=z0+(v.co.z-z0)*0.80
        me.transform(Matrix.Rotation(0.12,4,'X'));me.update()
    smooth=obj.modifiers.new('SoftConfection','SMOOTH');smooth.factor=0.22 if name!='MM_SquashedCream' else 0.1;smooth.iterations=2
    bpy.context.view_layer.objects.active=obj;bpy.ops.object.modifier_apply(modifier=smooth.name)
    tri_count=sum(max(0,len(p.vertices)-2) for p in obj.data.polygons)
    if tri_count>TARGET_TRIANGLES:
        dec=obj.modifiers.new('TriangleBudget','DECIMATE');dec.decimate_type='COLLAPSE';dec.ratio=TARGET_TRIANGLES/tri_count
        bpy.context.view_layer.objects.active=obj;bpy.ops.object.modifier_apply(modifier=dec.name)
    for p in obj.data.polygons:p.use_smooth=True
    # Normalize dimensions to a small confection prop and apply transforms.
    dims=obj.dimensions;factor=TARGET_LENGTH/max(dims)
    obj.scale=(factor,factor,factor);bpy.ops.object.transform_apply(location=False,rotation=False,scale=True)
    bpy.context.view_layer.update()
    dims=tuple(round(v,5) for v in obj.dimensions)
    set_camera_and_render(obj,name)
    # Export a single cleaned static mesh per file, with embedded materials/textures.
    bpy.ops.object.select_all(action='DESELECT');obj.select_set(True);bpy.context.view_layer.objects.active=obj
    target=BASE/(name+'_clean.fbx')
    bpy.ops.export_scene.fbx(filepath=str(target),use_selection=True,apply_scale_options='FBX_SCALE_ALL',object_types={'MESH'},mesh_smooth_type='FACE',use_mesh_modifiers=True,path_mode='COPY',embed_textures=True,add_leaf_bones=False)
    tri_count=sum(max(0,len(p.vertices)-2) for p in obj.data.polygons)
    REPORT.append({'name':name,'fbx':target.name,'triangles':tri_count,'dimensions_m':dims,'disconnected_components_removed':removed_components,'loose_vertices_removed':removed_loose,'source_objects_joined':len(meshes),'source_triangle_count':source_triangles})

(BASE/'cleanup_report.json').write_text(json.dumps({'target_triangle_budget':TARGET_TRIANGLES,'target_longest_dimension_m':TARGET_LENGTH,'assets':REPORT},indent=2),encoding='utf-8')
