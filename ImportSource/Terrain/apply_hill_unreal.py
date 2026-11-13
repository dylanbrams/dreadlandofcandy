"""Apply the Molar Hills uplift to the existing polished map without rebaking art.

Run in a full editor with -ExecutePythonScript. A map tag makes this repeatable.
Only upgrade the pre-hill baked baseline. Fully regenerated placements already
contain the elevation and must not receive this incremental lift.
"""
import hashlib
import importlib.util
import json
import math
import time
import traceback
from pathlib import Path
import unreal as u

ROOT = Path(u.Paths.convert_relative_path_to_full(u.Paths.project_dir()))/'ImportSource/Terrain'
icon_script = ROOT/'import_skill_icons.py'
exec(compile(icon_script.read_text(encoding='utf-8'),str(icon_script),'exec'),{'__file__':str(icon_script),'__name__':'__main__'})
spec = importlib.util.spec_from_file_location('hill_profile', ROOT/'hill_profile.py')
h = importlib.util.module_from_spec(spec)
spec.loader.exec_module(h)
data = json.loads((ROOT/'world_manifest.json').read_text())
source = json.loads((ROOT/'hill_sources.json').read_text())
actors = u.get_editor_subsystem(u.EditorActorSubsystem)
sm = u.get_editor_subsystem(u.StaticMeshEditorSubsystem)
eal = u.EditorAssetLibrary
world = u.EditorLoadingAndSavingUtils.load_map('/Game/World/L_Sugarborough')
assert world
marker = 'MolarHillElevationV1'
already = any(marker in [str(t) for t in a.tags] for a in actors.get_all_level_actors())
for name in source['tiles']:
    path = '/Game/World/Meshes/'+name
    mesh = u.load_asset(path)
    source_hash = hashlib.sha256((ROOT/'FBX'/(name+'.fbx')).read_bytes()).hexdigest()
    if already and eal.get_metadata_tag(mesh,'Terrain.FBXHash') == source_hash:
        continue
    materials = [s.material_interface for s in mesh.static_materials]
    task = u.AssetImportTask()
    task.set_editor_properties(dict(filename=str(ROOT/'FBX'/(name+'.fbx')),
        destination_path='/Game/World/Meshes', destination_name=name,
        automated=True, save=False, replace_existing=True))
    options = u.FbxImportUI()
    options.set_editor_properties(dict(import_mesh=True, import_as_skeletal=False,
        import_materials=False, import_textures=False))
    options.static_mesh_import_data.set_editor_properties(dict(combine_meshes=True,
        auto_generate_collision=False, import_uniform_scale=1.))
    task.options = options
    u.AssetToolsHelpers.get_asset_tools().import_asset_tasks([task])
    mesh = u.load_asset(path)
    for i, mat in enumerate(materials):
        mesh.set_material(i, mat)
    mesh.get_editor_property('body_setup').set_editor_properties(dict(collision_trace_flag=u.CollisionTraceFlag.CTF_USE_COMPLEX_AS_SIMPLE,
        double_sided_geometry=True))
    for section in range(mesh.get_num_sections(0)):
        sm.enable_section_collision(mesh, True, 0, section)
    sm.remove_collisions(mesh)
    eal.set_metadata_tag(mesh, 'Terrain.FBXHash', hashlib.sha256((ROOT/'FBX'/(name+'.fbx')).read_bytes()).hexdigest())
    eal.save_loaded_asset(mesh)

moved_actors = moved_instances = total_instances = 0
for actor in actors.get_all_level_actors():
    components = actor.get_components_by_class(u.InstancedStaticMeshComponent)
    for comp in components:
        total_instances += comp.get_instance_count()
        if already:
            continue
        for i in range(comp.get_instance_count()):
            transform = comp.get_instance_transform(i, world_space=True)
            p = transform.translation
            dz = h.mesh_elevation(p.x, p.y, data['locations'])
            if abs(dz) < .001:
                continue
            transform.translation = u.Vector(p.x, p.y, p.z+dz)
            assert comp.update_instance_transform(i, transform, world_space=True,
                mark_render_state_dirty=False, teleport=True)
            moved_instances += 1
        # Update render bounds once per batch.
        if comp.get_instance_count():
            last = comp.get_instance_count()-1
            comp.update_instance_transform(last, comp.get_instance_transform(last, world_space=True),
                world_space=True, mark_render_state_dirty=True, teleport=True)
    if already or components:
        continue
    if isinstance(actor, u.StaticMeshActor):
        mesh = actor.static_mesh_component.static_mesh
        if not mesh or mesh.get_name().startswith('SM_Terrain') or 'perimeter' in actor.get_actor_label():
            continue
    elif not isinstance(actor, (u.TextRenderActor, u.TargetPoint, u.PlayerStart)):
        continue
    p = actor.get_actor_location()
    dz = h.mesh_elevation(p.x, p.y, data['locations'])
    if abs(dz) > .001:
        actor.set_actor_location(u.Vector(p.x, p.y, p.z+dz), False, True)
        moved_actors += 1
assert total_instances == 85091, total_instances
terrain = next(a for a in actors.get_all_level_actors() if isinstance(a,u.StaticMeshActor)
    and a.static_mesh_component.static_mesh and a.static_mesh_component.static_mesh.get_name().startswith('SM_Terrain'))
if not already:
    terrain.tags = list(terrain.tags)+[marker]
assert u.EditorLoadingAndSavingUtils.save_map(world,'/Game/World/L_Sugarborough')

u.EditorPythonScripting.set_keep_python_script_alive(True)
started = time.monotonic()

def finish(dt):
    if time.monotonic()-started < 3:
        return
    u.unregister_slate_post_tick_callback(callback)
    try:
        samples = []
        ignore = [a for a in actors.get_all_level_actors() if not (
            isinstance(a,u.StaticMeshActor) and a.static_mesh_component.static_mesh
            and a.static_mesh_component.static_mesh.get_name().startswith('SM_Terrain'))]
        for distance in range(2500, 6750, 125):
            a = math.radians(120)
            x, y = distance*math.cos(a), distance*math.sin(a)
            hit = u.SystemLibrary.line_trace_single(world,u.Vector(x,y,6000),u.Vector(x,y,-2000),
                u.TraceTypeQuery.TRACE_TYPE_QUERY1,True,ignore,u.DrawDebugTrace.NONE)
            assert hit is not None, distance
            parts = hit.to_tuple()
            assert parts[9].static_mesh_component.static_mesh.get_name().startswith('SM_Terrain')
            slope = math.degrees(math.acos(max(-1,min(1,parts[7].z))))
            assert slope < 44, (distance,slope)
            samples.append(dict(distance=distance,z=parts[5].z,slope=slope))
        assert samples[-1]['z'] > 890
        assert samples[0]['z'] < 15
        report = dict(status='passed',hill_height_cm=900,instances=total_instances,
            moved_instances=moved_instances,moved_actors=moved_actors,already_applied=already,
            approach=samples)
        (ROOT/'hill_validation.json').write_text(json.dumps(report,indent=2)+'\n')
        u.log('MOLAR HILL PASS '+json.dumps(report))
        # Camera-only review copies; the gameplay map was saved before this.
        world.get_world_settings().set_editor_property('default_game_mode',u.GameModeBase)
        for actor in actors.get_all_level_actors():
            if isinstance(actor,u.CameraActor):
                actor.set_editor_property('auto_activate_for_player',u.AutoReceiveInput.DISABLED)
        cam = actors.spawn_actor_from_class(u.CameraActor,u.Vector())
        cam.camera_component.set_field_of_view(75)
        cam.camera_component.set_editor_property('constrain_aspect_ratio',False)
        cam.set_editor_property('auto_activate_for_player',u.AutoReceiveInput.PLAYER0)
        for name, eye, yaw, pitch in [
            ('MolarHillApproach',(-1650,2858,440),120,0),
            ('MolarHillOverview',(-7500,1000,3400),48,-18)]:
            cam.set_actor_location(u.Vector(*eye),False,False)
            cam.set_actor_rotation(u.Rotator(pitch=pitch,yaw=yaw),False)
            assert u.EditorLoadingAndSavingUtils.save_map(world,'/Game/World/Review/L_'+name)
    except Exception:
        (ROOT/'hill_validation.json').write_text(json.dumps(dict(status='failed',error=traceback.format_exc()),indent=2))
        u.log_error(traceback.format_exc())
    u.SystemLibrary.quit_editor()

callback = u.register_slate_post_tick_callback(finish)
