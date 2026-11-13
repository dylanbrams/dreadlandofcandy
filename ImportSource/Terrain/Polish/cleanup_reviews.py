"""Remove only this pass's unused scratch camera copies; protect tracked maps."""
import json
from pathlib import Path
import re
import unreal as u

here=Path(u.Paths.convert_relative_path_to_full(u.Paths.project_dir()))/'ImportSource/Terrain/Polish'
protected=set(json.loads((here/'tracked_review_maps.json').read_text()))
protected.update(json.loads((here/'review_maps.json').read_text()).values())
actors=u.get_default_object(u.EditorActorSubsystem)
edited=[]
perches_corrected=[]
# The first art captures used floating destination labels. Keep the scenery
# readable through the lemon, ingredients, bottle and fountain silhouettes.
for package in ['/Game/World/L_Sugarborough']+list(json.loads((here/'review_maps.json').read_text()).values()):
    world=u.EditorLoadingAndSavingUtils.load_map(package)
    labels=[a for a in actors.get_all_level_actors() if a.get_actor_label().startswith('SongLandmark_Label_')]
    for actor in labels:
        actors.destroy_actor(actor)
    # The upright lemon wheel was removed in the floating-slice pass. Move its
    # bluebird onto the tallest spring crystal instead of leaving it in midair.
    changed=False
    if package.rsplit('/',1)[-1].startswith('L_PineVarieties'):
        if not any(isinstance(a,u.PlayerStart) for a in actors.get_all_level_actors()):
            actors.spawn_actor_from_class(u.PlayerStart,u.Vector(-2500,0,100))
            changed=True
        if not any(isinstance(a,u.ExponentialHeightFog) for a in actors.get_all_level_actors()):
            fog=actors.spawn_actor_from_class(u.ExponentialHeightFog,u.Vector(0,0,-500))
            fog.component.set_editor_properties(dict(fog_density=.01,fog_height_falloff=.15))
            changed=True
    if u.EditorAssetLibrary.does_asset_exist('/Game/World/Polish/Meshes/SM_CandyCaneFir_Warp5'):
        report=json.loads((here/'landmark_report.json').read_text())
        spring=report['locations']['SM_LemonadeSpring']
        target=u.Vector(spring[0]-24,spring[1]+175,spring[2]+239)
        for actor in actors.get_all_level_actors():
            current=actor.get_actor_location()
            if actor.get_actor_label()=='SongLandmark_Bluebird_1' and abs(current.x-target.x)+abs(current.y-target.y)+abs(current.z-target.z)>1:
                actor.set_actor_location(target,False,False)
                changed=True
                perches_corrected.append(package)
    if labels or changed:
        assert u.EditorLoadingAndSavingUtils.save_map(world,package)
        edited.append(package)
pattern=re.compile(r'L_(?:(?:SweetMeadow|CandyGrove|SugarSnow|CandyExplorer)\d+|Polish(?:Village|Grass)\d*|(?:CrystalFountain(?:Side)?|LemonadeSpring|StewPond|WhiskeyRun|PineVarieties)\d*)$')
removed=[]
for path in u.EditorAssetLibrary.list_assets('/Game/World/Review',recursive=False):
    package=path.split('.')[0]
    if package in protected or not pattern.fullmatch(package.rsplit('/',1)[-1]):
        continue
    assert package.startswith('/Game/World/Review/')
    if not u.EditorAssetLibrary.delete_asset(package):
        raise RuntimeError('Cannot remove scratch preview '+package)
    removed.append(package)
(here/'review_cleanup.json').write_text(json.dumps(dict(removed=removed,protected=sorted(protected),floating_labels_removed_from=edited,perches_corrected=perches_corrected),indent=2)+'\n')
u.log('POLISH SCRATCH PREVIEW CLEANUP '+str(len(removed)))
