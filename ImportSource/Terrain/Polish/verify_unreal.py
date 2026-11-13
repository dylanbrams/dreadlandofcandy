"""Reload actual saved map, check physics/LODs and save current review cameras."""
import json
import time
import traceback
from pathlib import Path
import unreal as u

ROOT = Path(u.Paths.convert_relative_path_to_full(u.Paths.project_dir())) / 'ImportSource/Terrain'
HERE = ROOT / 'Polish'
actors = u.get_default_object(u.EditorActorSubsystem)
world = u.EditorLoadingAndSavingUtils.load_map('/Game/World/L_Sugarborough')
u.EditorPythonScripting.set_keep_python_script_alive(True)
started = time.monotonic()
(HERE/'verification.json').write_text(json.dumps(dict(status='running')))

def tick(dt):
    global world
    if time.monotonic()-started < 2:
        return
    u.unregister_slate_post_tick_callback(callback)
    try:
        script = ROOT/'review_level.py'
        exec(compile(script.read_text(),str(script),'exec'),dict(__file__=str(script),__name__='__main__',terrain_review_world=world))
        records = json.loads((HERE/'assets.json').read_text())['assets']
        if (HERE/'landmarks.json').exists():
            records += json.loads((HERE/'landmarks.json').read_text())['assets']
        lods = {}
        for r in records:
            mesh = u.load_asset('/Game/World/Polish/Meshes/'+r['name'])
            assert mesh.get_num_lods()==3, r['name']
            lods[r['name']] = mesh.get_num_lods()
        song_actors=[a for a in actors.get_all_level_actors() if a.get_actor_label().startswith('SongLandmark_')]
        song_meshes=[a for a in song_actors if isinstance(a,u.StaticMeshActor)]
        if (HERE/'landmarks.json').exists():
            assert len(song_meshes)==9, len(song_meshes)
            assert all(a.static_mesh_component.get_collision_enabled()==u.CollisionEnabled.NO_COLLISION for a in song_meshes)
        # Start from the freshly loaded main world; never change its GameMode.
        script = ROOT/'Dense/capture_maps.py'
        exec(compile(script.read_text(),str(script),'exec'),dict(__file__=str(script),__name__='__main__'))
        # Capture village at walking height in addition to woodland and snow.
        world = u.get_editor_subsystem(u.UnrealEditorSubsystem).get_editor_world()
        for actor in actors.get_all_level_actors():
            if isinstance(actor,u.CameraActor):
                actor.set_editor_property('auto_activate_for_player',u.AutoReceiveInput.DISABLED)
        cam = actors.spawn_actor_from_class(u.CameraActor,u.Vector(-650,-850,220))
        cam.set_actor_rotation(u.Rotator(pitch=2,yaw=35),False)
        cam.camera_component.set_field_of_view(78)
        cam.camera_component.set_editor_property('constrain_aspect_ratio',False)
        cam.set_editor_property('auto_activate_for_player',u.AutoReceiveInput.PLAYER0)
        maps = json.loads((ROOT/'Dense/review_maps.json').read_text())
        views=[('PolishVillage',(-650,-850,220),35,2),('PolishGrass',(3650,-650,100),0,-18)]
        if (HERE/'landmark_report.json').exists():
            landmarks=json.loads((HERE/'landmark_report.json').read_text())
            for name,key,offset,yaw,pitch in [
                ('SM_CrystalFountain','CrystalFountain',(-430,-480,300),48,-15),
                ('SM_CrystalFountain','CrystalFountainSide',(440,-450,260),135,-13),
                ('SM_LemonadeSpring','LemonadeSpring',(-300,-900,400),70,-22),
                ('SM_StewPond','StewPond',(-550,-650,450),50,-28),
                ('SM_WhiskeyRun','WhiskeyRun',(-450,-650,420),55,-25)]:
                center=landmarks['locations'][name]
                views.append((key,tuple(a+b for a,b in zip(center,offset)),yaw,pitch))
        for key, location, yaw, pitch in views:
            cam.set_actor_location(u.Vector(*location),False,False)
            cam.set_actor_rotation(u.Rotator(pitch=pitch,yaw=yaw),False)
            path,_ = u.AssetToolsHelpers.get_asset_tools().create_unique_asset_name('/Game/World/Review/L_'+key,'')
            assert u.EditorLoadingAndSavingUtils.save_map(world,path)
            maps[key]=path
        # A separate two-tree asset lineup makes the new silhouettes easy to
        # compare. It never adds presentation props to the gameplay world.
        if u.EditorAssetLibrary.does_asset_exist('/Game/World/Polish/Meshes/SM_CandyCaneFir_Warp5'):
            stage=u.EditorLoadingAndSavingUtils.new_blank_map(False)
            stage.get_world_settings().set_editor_property('default_game_mode',u.GameModeBase)
            stage.get_world_settings().set_editor_property('force_no_precomputed_lighting',True)
            floor=actors.spawn_actor_from_class(u.StaticMeshActor,u.Vector(0,0,-25))
            floor.static_mesh_component.set_static_mesh(u.load_asset('/Engine/BasicShapes/Cube'))
            floor.static_mesh_component.set_material(0,u.load_asset('/Game/World/Polish/Materials/M_Song_Sugar'))
            floor.set_actor_scale3d(u.Vector(100,100,.5))
            for i,y in [(4,-340),(5,340)]:
                tree=actors.spawn_actor_from_class(u.StaticMeshActor,u.Vector(0,y,0))
                tree.static_mesh_component.set_static_mesh(u.load_asset('/Game/World/Polish/Meshes/SM_CandyCaneFir_Warp'+str(i)))
            sun=actors.spawn_actor_from_class(u.DirectionalLight,u.Vector(0,0,2000),u.Rotator(pitch=-45,yaw=-35))
            sun.light_component.set_mobility(u.ComponentMobility.MOVABLE)
            sun.light_component.set_editor_property('intensity',3.5)
            sky=actors.spawn_actor_from_class(u.SkyLight,u.Vector(0,0,1000))
            sky.light_component.set_mobility(u.ComponentMobility.MOVABLE)
            sky.light_component.set_editor_properties(dict(source_type=u.SkyLightSourceType.SLS_SPECIFIED_CUBEMAP,
                cubemap=u.load_asset('/Engine/MapTemplates/Sky/DaylightAmbientCubemap'),intensity=.9))
            actors.spawn_actor_from_class(u.SkyAtmosphere,u.Vector())
            fog=actors.spawn_actor_from_class(u.ExponentialHeightFog,u.Vector(0,0,-500))
            fog.component.set_editor_properties(dict(fog_density=.01,fog_height_falloff=.15))
            actors.spawn_actor_from_class(u.PlayerStart,u.Vector(-2500,0,100))
            cam=actors.spawn_actor_from_class(u.CameraActor,u.Vector(-1450,0,420),u.Rotator(pitch=0,yaw=0))
            cam.camera_component.set_field_of_view(65)
            cam.camera_component.set_editor_property('constrain_aspect_ratio',False)
            cam.set_editor_property('auto_activate_for_player',u.AutoReceiveInput.PLAYER0)
            path,_=u.AssetToolsHelpers.get_asset_tools().create_unique_asset_name('/Game/World/Review/L_PineVarieties','')
            assert u.EditorLoadingAndSavingUtils.save_map(stage,path)
            maps['PineVarieties']=path
        (ROOT/'Dense/review_maps.json').write_text(json.dumps(maps,indent=2)+'\n')
        (HERE/'review_maps.json').write_text(json.dumps(maps,indent=2)+'\n')
        current=json.loads((HERE/'engine_report.json').read_text())
        current.update(pcg_graph='/Game/World/Polish/PCG/PCG_CandyEcology',review_maps=list(maps.values()),review_maps_status='saved and current',source_pass='Polish')
        for path in (HERE/'engine_report.json',ROOT/'Dense/engine_report.json'):
            path.write_text(json.dumps(current,indent=2)+'\n')
        script=ROOT/'label_maps.py'
        exec(compile(script.read_text(),str(script),'exec'),dict(__file__=str(script),__name__='__main__'))
        geometry=json.loads((ROOT/'engine_validation.json').read_text())
        report=dict(status='passed',lods=lods,instances=geometry['dense_nonblocking_instances'],
            components=geometry['dense_instanced_components'],ground_samples=len(geometry['ground_collision_samples']),
            routes=len(geometry['traversable_approaches']),boundary_samples=len(geometry['boundary_collision_samples']),song_meshes=len(song_meshes),review_maps=maps)
        (HERE/'verification.json').write_text(json.dumps(report,indent=2)+'\n')
        u.log('CANDY POLISH VERIFICATION PASS '+json.dumps(report))
    except Exception:
        (HERE/'verification.json').write_text(json.dumps(dict(status='failed',error=traceback.format_exc()),indent=2)+'\n')
        u.log_error(traceback.format_exc())
    finally:
        u.SystemLibrary.quit_editor()

callback = u.register_slate_post_tick_callback(tick)
