import unreal as u
import json
from pathlib import Path
u.EditorLoadingAndSavingUtils.load_map('/Game/World/L_Sugarborough')
actors=u.get_editor_subsystem(u.EditorActorSubsystem)
plants=sorted([a for a in actors.get_all_level_actors() if a.get_actor_label().startswith('Riverlands_MixedUnderstory')],key=lambda a:a.get_actor_label())
succulent=u.load_asset('/Game/World/Expansion/SM_CandySucculent')
box=succulent.get_bounding_box();s=110/(box.max.z-box.min.z)
for a in plants[1::2]:
    old=a.static_mesh_component.static_mesh.get_bounding_box()
    p=a.get_actor_location();ground=p.z+old.min.z*a.get_actor_scale3d().z
    a.static_mesh_component.set_static_mesh(succulent)
    a.set_actor_scale3d(u.Vector(s,s,s))
    a.set_actor_location(u.Vector(p.x,p.y,ground-box.min.z*s),False,False)
world=u.get_editor_subsystem(u.UnrealEditorSubsystem).get_editor_world()
assert u.EditorLoadingAndSavingUtils.save_map(world,'/Game/World/L_Sugarborough')
counts={}
for a in actors.get_all_level_actors():
    if a.get_actor_label().startswith(('Riverlands_MixedTree','Riverlands_MixedUnderstory')):
        name=a.static_mesh_component.static_mesh.get_name()
        counts[name]=counts.get(name,0)+1
(Path(u.Paths.project_dir())/'ImportSource/Terrain/Expansion/foliage_mix.json').write_text(json.dumps(counts,indent=2))
trees=[a for a in actors.get_all_level_actors() if a.get_actor_label().startswith('Riverlands_MixedTree')]
assert trees
p=trees[len(trees)//2].get_actor_location()
world=u.get_editor_subsystem(u.UnrealEditorSubsystem).get_editor_world()
world.get_world_settings().set_editor_property('default_game_mode',u.GameModeBase)
for a in actors.get_all_level_actors():
    if isinstance(a,u.CameraActor): a.set_editor_property('auto_activate_for_player',u.AutoReceiveInput.DISABLED)
cam=actors.spawn_actor_from_class(u.CameraActor,p+u.Vector(-1600,-1600,1600),u.Rotator(pitch=-32,yaw=45))
cam.set_editor_property('auto_activate_for_player',u.AutoReceiveInput.PLAYER0)
assert u.EditorLoadingAndSavingUtils.save_map(world,'/Game/World/Expansion/L_FoliageMixReview')
