"""Add Riverlands and Dylan's village models; run in exclusive Unreal editor window."""
import unreal as u
import json, math, random
from pathlib import Path
ROOT = Path(u.Paths.project_dir()).resolve()
HERE = ROOT / 'ImportSource/Terrain/Expansion'
BASE = '/Game/World/Expansion'
eal = u.EditorAssetLibrary
actors = u.get_editor_subsystem(u.EditorActorSubsystem)
sm = u.get_default_object(u.StaticMeshEditorSubsystem)
world = u.EditorLoadingAndSavingUtils.load_map('/Game/World/L_Sugarborough')

def import_mesh(file, name):
    dest = BASE + '/' + name
    if eal.does_asset_exist(dest) and name != 'SM_Riverlands':
        mesh=u.load_asset(dest)
        mesh.get_editor_property('body_setup').set_editor_property('collision_trace_flag',u.CollisionTraceFlag.CTF_USE_COMPLEX_AS_SIMPLE)
        for i in range(mesh.get_num_sections(0)): sm.enable_section_collision(mesh,True,0,i)
        sm.remove_collisions(mesh)
        eal.save_loaded_asset(mesh)
        return mesh
    t = u.AssetImportTask()
    t.set_editor_properties(dict(filename=str(file), destination_path=BASE, destination_name=name, automated=True, save=True, replace_existing=True))
    opt = u.FbxImportUI()
    opt.set_editor_properties(dict(import_mesh=True, import_as_skeletal=False, import_materials=True, import_textures=True))
    opt.static_mesh_import_data.set_editor_properties(dict(combine_meshes=True, auto_generate_collision=False))
    t.options = opt
    u.AssetToolsHelpers.get_asset_tools().import_asset_tasks([t])
    meshes = [u.load_asset(p) for p in t.imported_object_paths if isinstance(u.load_asset(p), u.StaticMesh)]
    assert len(meshes) == 1, (name, t.imported_object_paths)
    mesh = meshes[0]
    body = mesh.get_editor_property('body_setup')
    body.set_editor_property('collision_trace_flag', u.CollisionTraceFlag.CTF_USE_COMPLEX_AS_SIMPLE)
    body.set_editor_property('double_sided_geometry', True)
    for i in range(mesh.get_num_sections(0)):
        sm.enable_section_collision(mesh, True, 0, i)
    sm.remove_collisions(mesh)
    eal.save_loaded_asset(mesh)
    return mesh

def place(mesh, name, x, y, z=0, scale=1, yaw=0, blocking=True):
    a = actors.spawn_actor_from_class(u.StaticMeshActor, u.Vector(x,y,z), u.Rotator(yaw=yaw))
    a.set_actor_label('Riverlands_' + name)
    a.set_folder_path('Sugarborough/Riverlands')
    a.static_mesh_component.set_static_mesh(mesh)
    a.set_actor_scale3d(u.Vector(scale,scale,scale))
    c = a.static_mesh_component
    c.set_collision_profile_name('BlockAll' if blocking else 'NoCollision')
    c.set_collision_response_to_channel(u.CollisionChannel.ECC_CAMERA, u.CollisionResponseType.ECR_IGNORE)
    return a

# Idempotent replacement of this pass, preserving the original hill and quest sets.
for a in actors.get_all_level_actors():
    label = a.get_actor_label()
    p = a.get_actor_location()
    if label.startswith('Riverlands_'):
        actors.destroy_actor(a)
    elif label == 'Mountain perimeter collision' and p.x > 12000:
        a.set_actor_location(u.Vector(26050, p.y, p.z), False, False)
    elif isinstance(a, u.StaticMeshActor):
        m = a.static_mesh_component.static_mesh
        if m and m.get_name() == 'SM_SugarMountain' and 12000 < p.x < 16000 and abs(p.y) < 12000:
            a.set_actor_location(u.Vector(p.x+14000,p.y,p.z),False,False)
        elif m and 'CandyHouse' in m.get_name() and math.hypot(p.x,p.y)<2600:
            actors.destroy_actor(a)
        elif m and 'GingerbreadCottage' in m.get_name() and math.hypot(p.x,p.y)<2600:
            actors.destroy_actor(a)

for name in ['SM_Riverlands','SM_RiverWater','SM_WaferBridge']:
    place(import_mesh(HERE/'FBX'/(name+'.fbx'),name), name,4000,0,blocking=name!='SM_RiverWater')

# Generated peppermint-railed bridge; exact deck placement is refined against
# saved-world ground traces, keeping the original sources untouched.
bridge_source=ROOT/'Assets/DylanGenerationFoliageThree/Clean/SM_Bridge126.fbx'
if bridge_source.exists():
    bridge=import_mesh(bridge_source,'SM_GeneratedBridge126')
    b=bridge.get_bounding_box();d=b.max-b.min
    generated=place(bridge,'PeppermintBridge',19100,0,-25)
    generated.set_actor_scale3d(u.Vector(2200/d.x,800/d.y,80/d.z))
    for a in actors.get_all_level_actors():
        if a.get_actor_label()=='Riverlands_SM_WaferBridge':actors.destroy_actor(a)

for a in actors.get_all_level_actors():
    if isinstance(a,u.StaticMeshActor) and a.static_mesh_component.static_mesh:
        name=a.static_mesh_component.static_mesh.get_name()
        if name.startswith('SM_Terrain_3_') or name.startswith('SM_EastOpening_'):
            ty=name.split('_')[-1]
            mesh=import_mesh(HERE/'FBX'/('SM_EastOpening_3_'+ty+'.fbx'),'SM_EastOpening_'+ty)
            mesh.set_material(0,u.load_asset('/Game/World/Materials/M_Terrain'))
            a.static_mesh_component.set_static_mesh(mesh)
            eal.save_loaded_asset(mesh)

buildings = [('TownHall',-1750,0,1000,0),('Bakery',650,-1550,850,0),
             ('Dentist',-1400,-1350,850,30),('Farm',1750,900,1100,180),
             ('ScoutTower',-800,1750,1100,0),('SmallHouse',500,1850,750,180),
             ('SmallHouse',1600,-650,700,90)]
reserves=[]
for index,(name,x,y,size,yaw) in enumerate(buildings):
    mesh=import_mesh(ROOT/'Assets/DylanGeneration'/name/(name+'.fbx'),'SM_'+name)
    box=mesh.get_bounding_box(); dim=box.max-box.min
    scale=size/max(dim.x,dim.y,dim.z)
    place(mesh,name+str(index),x,y,-box.min.z*scale,scale,yaw)
    reserves.append((x,y,750))

foliage={}
for name in ['SM_PeppermintTree','SM_CandyAppleTree','SM_CandyTree','SM_CottonCandyBush','SM_GumdropBush','SM_ToastedMarshmallowMushrooms']:
    foliage[name]=import_mesh(ROOT/'Assets/DylanGenerationFoliageTwo/LowPoly'/(name+'_LowPoly.fbx'),name)
mesh=foliage['SM_PeppermintTree']; box=mesh.get_bounding_box()
place(mesh,'SquarePeppermint',0,550,-box.min.z*900/(box.max.z-box.min.z),900/(box.max.z-box.min.z),blocking=False)
square_trunk=place(u.load_asset('/Engine/BasicShapes/Cylinder'),'SquareTrunk',0,550,90)
square_trunk.set_actor_scale3d(u.Vector(.8,.8,1.8));square_trunk.set_actor_hidden_in_game(True)
square_trunk.static_mesh_component.set_collision_response_to_all_channels(u.CollisionResponseType.ECR_IGNORE)
square_trunk.static_mesh_component.set_collision_response_to_channel(u.CollisionChannel.ECC_PAWN,u.CollisionResponseType.ECR_BLOCK)
reserves.append((0,550,650))

# Remove old foliage from the new village footprints and the riverside approach.
removed=0
for a in actors.get_all_level_actors():
    for c in a.get_components_by_class(u.HierarchicalInstancedStaticMeshComponent):
        for i in range(c.get_instance_count()-1,-1,-1):
            p=c.get_instance_transform(i,True).translation
            if any(math.hypot(p.x-x,p.y-y)<r for x,y,r in reserves) or (p.x>10800 and abs(p.y)<700):
                c.remove_instance(i); removed+=1
        # Narrow existing trunk hulls block the hero, with no spring-arm obstruction.
        mesh=c.static_mesh
        if mesh and any(s in mesh.get_name() for s in ['Birch_Warp','Blossom_Warp','Fir_Warp']):
            c.set_collision_profile_name('BlockAll')
            c.set_collision_enabled(u.CollisionEnabled.QUERY_ONLY)
            c.set_collision_response_to_all_channels(u.CollisionResponseType.ECR_IGNORE)
            c.set_collision_response_to_channel(u.CollisionChannel.ECC_PAWN,u.CollisionResponseType.ECR_BLOCK)

# Spacious new groves frame the crossing and preserve the far quest clearing.
rng=random.Random(14026)
for i in range(240):
    x=rng.uniform(12100,21300); y=rng.uniform(-11200,11200)
    river=15000+500*math.sin(y/3500)
    if abs(x-river)<800 or abs(y)<800 or any(math.hypot(x-cx,y-cy)<1100 for cx,cy in [(18200,0),(13000,-3000),(18500,4200)]): continue
    name=list(foliage)[i%len(foliage)]; mesh=foliage[name]
    b=mesh.get_bounding_box(); height=b.max.z-b.min.z
    size=rng.uniform(450,800) if 'Tree' in name else rng.uniform(100,250)
    # Matches export_extension.py's elevation function.
    z=0 if x<16000 else 220*(math.sin((x-16000)/6000*math.pi)**2)*(0.5+0.5*math.cos(y/5000))
    place(mesh,'Grove'+str(i),x+4000,y,z-b.min.z*size/height,size/height,rng.uniform(0,360),False)
    if 'Tree' in name:
        trunk=place(u.load_asset('/Engine/BasicShapes/Cylinder'),'Trunk'+str(i),x+4000,y,z+90)
        trunk.set_actor_scale3d(u.Vector(.65,.65,1.8));trunk.set_actor_hidden_in_game(True)
        trunk.static_mesh_component.set_collision_response_to_all_channels(u.CollisionResponseType.ECR_IGNORE)
        trunk.static_mesh_component.set_collision_response_to_channel(u.CollisionChannel.ECC_PAWN,u.CollisionResponseType.ECR_BLOCK)

# Extend north/south boundaries over the added area.
cube=u.load_asset('/Engine/BasicShapes/Cube')
for y in [-12050,12050]:
    a=place(cube,'Boundary'+str(y),19000,y,3500)
    a.set_actor_scale3d(u.Vector(140,1,100)); a.set_actor_hidden_in_game(True)
mountain=u.load_asset('/Game/World/Meshes/SM_SugarMountain')
for y in [-13500,13500]:
    for x in [16500,20000,23500,26500]:
        place(mountain,'Rim'+str(x)+'_'+str(y),x,y,100,1.6,rng.uniform(0,360))

for path in eal.list_assets(BASE,recursive=True,include_folder=False):
    asset=u.load_asset(path)
    if isinstance(asset,u.Texture2D):
        asset.set_editor_property('max_texture_size',2048)
        eal.save_loaded_asset(asset)
assert u.EditorLoadingAndSavingUtils.save_map(world,'/Game/World/L_Sugarborough')
(HERE/'report.json').write_text(json.dumps(dict(bounds=[-12000,-12000,26000,12000],area_increase_percent=58.33,removed_foliage=removed,buildings=buildings,far_camp=[22200,0]),indent=2))
u.log('RIVERLANDS BUILD PASS')
world.get_world_settings().set_editor_property('default_game_mode',u.GameModeBase)
for a in actors.get_all_level_actors():
    if isinstance(a,u.CameraActor): a.set_editor_property('auto_activate_for_player',u.AutoReceiveInput.DISABLED)
cam=actors.spawn_actor_from_class(u.CameraActor,u.Vector(-3400,-3800,3600),u.Rotator(pitch=-34,yaw=48))
cam.set_editor_property('auto_activate_for_player',u.AutoReceiveInput.PLAYER0)
if eal.does_asset_exist(BASE+'/L_VillageReview'):eal.delete_asset(BASE+'/L_VillageReview')
assert u.EditorLoadingAndSavingUtils.save_map(world,BASE+'/L_VillageReview')
cam.set_actor_location(u.Vector(15600,-4700,3900),False,False)
cam.set_actor_rotation(u.Rotator(pitch=-30,yaw=48),False)
if eal.does_asset_exist(BASE+'/L_RiverReview'):eal.delete_asset(BASE+'/L_RiverReview')
assert u.EditorLoadingAndSavingUtils.save_map(world,BASE+'/L_RiverReview')
