"""Import cleaned fal variants and replace only river marshmallow actors."""
from pathlib import Path
import unreal as u
HERE=Path(__file__).resolve().parent
A=u.EditorAssetLibrary
tools=u.AssetToolsHelpers.get_asset_tools()
files=sorted((HERE/'Marshmallows').rglob('*_clean.fbx'))
assert len(files)>=3, 'Need three cleaned fal variants before touching map'
meshes=[]
for index,file in enumerate(files):
 name='SM_Marshmallow_%02d'%(index+1)
 task=u.AssetImportTask()
 options=u.FbxImportUI()
 options.set_editor_properties(dict(import_mesh=True,import_as_skeletal=False,import_materials=False,import_textures=False))
 options.static_mesh_import_data.set_editor_properties(dict(combine_meshes=True,auto_generate_collision=False))
 task.set_editor_properties(dict(filename=str(file),destination_path='/Game/World/CandyPBR/Marshmallows',destination_name=name,automated=True,save=True,replace_existing=True,options=options))
 tools.import_asset_tasks([task])
 found=[u.load_asset(p) for p in task.imported_object_paths if isinstance(u.load_asset(p),u.StaticMesh)]
 assert len(found)==1
 mesh=found[0]
 meshes.append(mesh)
exec(compile((HERE/'candy_water.py').read_text(),str(HERE/'candy_water.py'),'exec'),{'__file__':str(HERE/'candy_water.py'),'__name__':'__main__'})
material=u.load_asset('/Game/World/Expansion/M_Marshmallow')
world=u.EditorLoadingAndSavingUtils.load_map('/Game/World/L_Sugarborough')
actors=u.get_editor_subsystem(u.EditorActorSubsystem)
prefix='CandyLandscapeDressing_River_Marshmallows_'
targets=sorted([a for a in actors.get_all_level_actors() if a.get_actor_label().startswith(prefix)],key=lambda a:int(a.get_actor_label()[len(prefix):]))
assert len(targets)==52,len(targets)
for index,actor in enumerate(targets):
 mesh=meshes[index%len(meshes)]
 box=mesh.get_bounding_box();dim=box.max-box.min
 width=(65+(index%3)*8)*2.0/3.0
 height=(48+(index%4)*4)*2.0/3.0
 scale=u.Vector(width/dim.x,width/dim.y,height/dim.z)
 comp=actor.static_mesh_component
 comp.set_static_mesh(mesh)
 comp.set_material(0,material)
 comp.set_editor_property('bounds_scale',1000.)
 actor.set_actor_scale3d(scale)
 pos=actor.get_actor_location()
 pos.z=-65+height*.20-(box.min.z+box.max.z)*.5*scale.z
 actor.set_actor_location(pos,False,False)
 A.save_loaded_asset(mesh)
assert u.EditorLoadingAndSavingUtils.save_map(world,'/Game/World/L_Sugarborough')
exec(compile((HERE/'candy_review_maps.py').read_text(),str(HERE/'candy_review_maps.py'),'exec'),{'__file__':str(HERE/'candy_review_maps.py')})
u.log('MARSHMALLOW UPDATE PASS variants=%d actors=%d speed=45cm/s; expanded render bounds and rigid object wrapping'%(len(meshes),len(targets)))
