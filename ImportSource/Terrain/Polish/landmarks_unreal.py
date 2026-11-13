"""Place scenic song landmarks. No new quest, swimming or alcohol mechanics."""
import importlib.util
import json
import math
import unreal as u

def build(d,material):
    source=json.loads((d.HERE/'landmarks.json').read_text())
    spec=importlib.util.spec_from_file_location('terrain_height',d.ROOT/'build_sources.py')
    t=importlib.util.module_from_spec(spec)
    spec.loader.exec_module(t)
    actors=d.ACTORS
    for actor in list(actors.get_all_level_actors()):
        if actor.get_actor_label().startswith('SongLandmark_'):
            actors.destroy_actor(actor)
    materials={}
    for name,color in source['palette'].items():
        kind='clearwater' if name=='Lemonade' else 'water' if name in ('Broth','Whiskey') else 'crystal' if name=='Crystal' else 'landmark'
        materials[name]=material('M_Song_'+name,kind,color=color[:3])
    meshes={}
    sm=u.get_default_object(u.StaticMeshEditorSubsystem)
    for record in source['assets']:
        mesh=d.import_file(d.HERE/'FBX'/(record['name']+'.fbx'),'Meshes')
        for i,slot in enumerate(mesh.get_editor_property('static_materials')):
            name=str(slot.get_editor_property('imported_material_slot_name'))
            mesh.set_material(i,materials[name])
        reduction=u.StaticMeshReductionOptions()
        reduction.set_editor_property('auto_compute_lod_screen_size',False)
        reduction.set_editor_property('reduction_settings',[u.StaticMeshReductionSettings(percent_triangles=p,screen_size=s) for p,s in [(1.,1.),(.45,.3),(.18,.1)]])
        assert sm.set_lods(mesh,reduction)==3
        d.EAL.save_loaded_asset(mesh)
        meshes[record['name']]=mesh
    positions={}
    for record in source['assets']:
        if record['category']=='bird':
            continue
        x,y=record['location']
        # A shallow sugar basin sits just above this local terrain. Existing
        # collision ground supports the player; no landscape excavation.
        radius=record['radius']*.8
        z=max(t.height(x+radius*math.cos(a),y+radius*math.sin(a)) for a in [i*math.tau/24 for i in range(24)])+12
        actor=actors.spawn_actor_from_class(u.StaticMeshActor,u.Vector(x,y,z))
        actor.set_actor_label('SongLandmark_'+record['name'])
        actor.set_folder_path('Sugarborough/Song landmarks')
        actor.static_mesh_component.set_static_mesh(meshes[record['name']])
        actor.static_mesh_component.set_collision_enabled(u.CollisionEnabled.NO_COLLISION)
        actor.static_mesh_component.set_collision_profile_name('NoCollision')
        positions[record['name']]=[x,y,z]
    spring=positions['SM_LemonadeSpring'];fountain=positions['SM_CrystalFountain']
    perches=[(spring[0]+410,spring[1],spring[2]+48),(spring[0]-24,spring[1]+175,spring[2]+239),
        (spring[0]-330,spring[1]-180,spring[2]+45),(fountain[0]+170,fountain[1],fountain[2]+48),
        (fountain[0]-90,fountain[1]+90,fountain[2]+50)]
    for i,pos in enumerate(perches):
        actor=actors.spawn_actor_from_class(u.StaticMeshActor,u.Vector(*pos),u.Rotator(yaw=i*71))
        actor.set_actor_label('SongLandmark_Bluebird_'+str(i))
        actor.set_folder_path('Sugarborough/Song landmarks')
        actor.static_mesh_component.set_static_mesh(meshes['SM_Bluebird'])
        actor.set_actor_scale3d(u.Vector(1.4,1.4,1.4))
        actor.static_mesh_component.set_collision_profile_name('NoCollision')
        actor.static_mesh_component.set_collision_enabled(u.CollisionEnabled.NO_COLLISION)
    (d.HERE/'landmark_report.json').write_text(json.dumps(dict(status='placed',locations=positions,bluebirds=len(perches),scenic_only=True),indent=2)+'\n')
