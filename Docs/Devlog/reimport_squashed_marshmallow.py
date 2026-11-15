"""Reimport only the fixed MM_SquashedCream mesh over SM_Marshmallow_02 (same path, so level actors keep it)."""
from pathlib import Path
import unreal as u
HERE=Path(__file__).resolve().parent
tools=u.AssetToolsHelpers.get_asset_tools()
task=u.AssetImportTask();o=u.FbxImportUI()
o.set_editor_properties(dict(import_mesh=True,import_as_skeletal=False,import_materials=False,import_textures=False))
o.static_mesh_import_data.set_editor_properties(dict(combine_meshes=True,auto_generate_collision=False))
task.set_editor_properties(dict(filename=str(HERE/'Marshmallows'/'MM_SquashedCream_clean.fbx'),destination_path='/Game/World/CandyPBR/Marshmallows',destination_name='SM_Marshmallow_02',automated=True,save=True,replace_existing=True,options=o))
tools.import_asset_tasks([task])
m=u.load_asset('/Game/World/CandyPBR/Marshmallows/SM_Marshmallow_02')
b=m.get_bounding_box();u.log('SQUASHED REIMPORT OK bounds %s %s'%(b.min,b.max))
# Mesh bounds changed, so redo the per-actor scale/height exactly as update_marshmallows.py does (squashed = index%3==1).
world=u.EditorLoadingAndSavingUtils.load_map('/Game/World/L_Sugarborough')
actors=u.get_editor_subsystem(u.EditorActorSubsystem)
prefix='CandyLandscapeDressing_River_Marshmallows_'
targets=sorted([a for a in actors.get_all_level_actors() if a.get_actor_label().startswith(prefix)],key=lambda a:int(a.get_actor_label()[len(prefix):]))
assert len(targets)==52,len(targets)
box=m.get_bounding_box();dim=box.max-box.min
for index,actor in enumerate(targets):
 if index%3!=1: continue
 width=(65+(index%3)*8)*2.0/3.0;height=(48+(index%4)*4)*2.0/3.0
 scale=u.Vector(width/dim.x,width/dim.y,height/dim.z)
 actor.set_actor_scale3d(scale)
 pos=actor.get_actor_location();pos.z=-65+height*.20-(box.min.z+box.max.z)*.5*scale.z
 actor.set_actor_location(pos,False,False)
assert u.EditorLoadingAndSavingUtils.save_map(world,'/Game/World/L_Sugarborough')
u.log('SQUASHED REIMPORT DONE')
