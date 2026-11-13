"""Wearable placement previews and portal ray tests, without Unreal or GLB edits."""
import bpy,json,math,sys
from pathlib import Path
from mathutils import Vector
from mathutils.bvhtree import BVHTree
OUT=Path(__file__).resolve().parent/'fit';OUT.mkdir(exist_ok=True)
ASSETS=OUT.parents[1]
results=[]
for name,position in [('SM_Hat_Straw',(0,-.03,1.898)),('SM_HeadMirror',(0,-.03,1.65))]:
    bpy.ops.wm.read_factory_settings(use_empty=True)
    bpy.ops.import_scene.gltf(filepath=str(ASSETS/'Hyper3D/SM_Villager.glb'))
    bpy.ops.import_scene.gltf(filepath=str(ASSETS/'Fal'/(name+'.glb')))
    accessory=next(o for o in bpy.context.scene.objects if o.type=='MESH' and o.name.startswith(name))
    accessory.location=position
    accessory.data.calc_loop_triangles()
    tree=BVHTree.FromPolygons([v.co for v in accessory.data.vertices],
        [list(t.vertices) for t in accessory.data.loop_triangles],all_triangles=True)
    if name=='SM_Hat_Straw':
        hit=tree.ray_cast(Vector((0,0,-2)),Vector((0,0,1)),5)[0]
        hollow=hit is not None and hit.z>.25
    else:
        hit=tree.ray_cast(Vector((0,-2,.10)),Vector((0,1,0)),4)[0]
        hollow=hit is None
    assert hollow,'Accessory opening obstructed: '+name
    results.append({'asset':name,'villager_local_mount_m':position,'hollow_opening_check':hollow,
        'preview_front':name+'_0.png','preview_back':name+'_1.png'})
    if '--checks-only' in sys.argv:continue
    scene=bpy.context.scene;scene.render.engine='BLENDER_EEVEE';scene.eevee.taa_render_samples=16
    scene.eevee.use_gtao=True;scene.eevee.gtao_distance=.15
    scene.world=bpy.data.worlds.new('Fit world');scene.world.color=(.3,.3,.3)
    scene.render.resolution_x=600;scene.render.resolution_y=600;scene.render.resolution_percentage=100
    scene.view_settings.view_transform='Standard';scene.view_settings.exposure=-.8
    for location,energy in [((3,-4,5),1600),((-3,-2,3),800),((1,3,4),1200)]:
        lightdata=bpy.data.lights.new('Fit light','AREA');lightdata.energy=energy;lightdata.size=4
        light=bpy.data.objects.new('Fit light',lightdata);scene.collection.objects.link(light);light.location=location
        light.rotation_euler=(Vector((0,0,1.5))-light.location).to_track_quat('-Z','Y').to_euler()
    camera_data=bpy.data.cameras.new('Fit camera');camera=bpy.data.objects.new('Fit camera',camera_data)
    scene.collection.objects.link(camera);scene.camera=camera;camera_data.type='ORTHO';camera_data.ortho_scale=3.1
    scene.render.image_settings.file_format='PNG'
    for i,location in enumerate(((2,-4,2.8),(-2,4,2.8))):
        camera.location=location;camera.rotation_euler=(Vector((0,0,1.35))-camera.location).to_track_quat('-Z','Y').to_euler()
        scene.render.filepath=str(OUT/(name+'_'+str(i)+'.png'));bpy.ops.render.render(write_still=True)
bpy.ops.wm.read_factory_settings(use_empty=True)
bpy.ops.import_scene.gltf(filepath=str(ASSETS/'Fal/SM_FortGate.glb'))
obj=next(o for o in bpy.context.scene.objects if o.type=='MESH');obj.data.calc_loop_triangles()
bvh=BVHTree.FromPolygons([obj.matrix_world@v.co for v in obj.data.vertices],[list(t.vertices) for t in obj.data.loop_triangles],all_triangles=True)
tests=[]
for x,z in [(0,.05),(0,.15),(0,.25),(-.075,.10),(.075,.10)]:
    hit=bvh.ray_cast(Vector((x,-2,z)),Vector((0,1,0)),4)[0]
    tests.append({'x_m':x,'z_m':z,'clear':hit is None})
assert all(test['clear'] for test in tests),'Gate still obstructed'
results.append({'asset':'SM_FortGate','portal_ray_tests':tests})
(OUT/'fit_results.json').write_text(json.dumps(results,indent=2),encoding='utf-8')
print('FIT AND PORTAL CHECKS COMPLETE',flush=True)
