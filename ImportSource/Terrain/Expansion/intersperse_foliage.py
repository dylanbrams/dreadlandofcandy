"""Mix generated candy trees into existing broadleaf stands, preserving snow."""
from pathlib import Path
import json
import unreal as u

source = Path(u.Paths.project_dir()) / 'ImportSource/Terrain/Expansion/build_expansion.py'
exec(source.read_text().split('# Idempotent replacement')[0])
assert not any(a.get_actor_label().startswith('Riverlands_MixedTree') for a in actors.get_all_level_actors()), 'Mix already applied; restore original map before rebuilding this pass'
names = ['SM_PeppermintTree', 'SM_CandyAppleTree', 'SM_CandyTree',
         'SM_CandyBerryBush', 'SM_CandySucculent']
meshes = {n: import_mesh(ROOT/'Assets/DylanGenerationFoliageTwo/LowPoly'/(n+'_LowPoly.fbx'), n) for n in names}
counts = {n: 0 for n in names}
# Replace a sparse, deterministic subset at existing cleared tree positions.
# Fir groups (including every snow tree) are deliberately excluded.
for actor in list(actors.get_all_level_actors()):
    for comp in actor.get_components_by_class(u.HierarchicalInstancedStaticMeshComponent):
        old = comp.static_mesh
        if not old or not any(n in old.get_name() for n in ['Birch_Warp', 'Blossom_Warp']):
            continue
        for i in range(comp.get_instance_count()-1, -1, -1):
            if i % 5 != 2:
                continue
            t = comp.get_instance_transform(i, True)
            p = t.translation
            name = names[sum(counts.values()) % 3]
            mesh = meshes[name]
            b = mesh.get_bounding_box()
            old_height = (old.get_bounding_box().max.z-old.get_bounding_box().min.z)*t.scale3d.z
            height = min(750, max(450, old_height))
            scale = height/(b.max.z-b.min.z)
            idx = sum(counts.values())
            place(mesh, 'MixedTree'+str(idx), p.x, p.y, p.z-b.min.z*scale, scale, i*137.5, False)
            trunk = place(u.load_asset('/Engine/BasicShapes/Cylinder'), 'MixedTrunk'+str(idx), p.x,p.y,p.z+90)
            trunk.set_actor_scale3d(u.Vector(.65,.65,1.8))
            trunk.set_actor_hidden_in_game(True)
            trunk.static_mesh_component.set_collision_response_to_all_channels(u.CollisionResponseType.ECR_IGNORE)
            trunk.static_mesh_component.set_collision_response_to_channel(u.CollisionChannel.ECC_PAWN,u.CollisionResponseType.ECR_BLOCK)
            counts[name] += 1
            comp.remove_instance(i)
            # Small omitted plants share this already occupied footprint.
            plant_name = names[3 + (idx//2)%2]
            plant = meshes[plant_name]; box = plant.get_bounding_box()
            s = 110/(box.max.z-box.min.z)
            place(plant,'MixedUnderstory'+str(idx),p.x+100,p.y,p.z-box.min.z*s,s,i*97,False)
            counts[plant_name] += 1
assert sum(counts.values()) > 0, 'No broadleaf positions found'
assert u.EditorLoadingAndSavingUtils.save_map(world,'/Game/World/L_Sugarborough')
(HERE/'foliage_mix.json').write_text(json.dumps(counts,indent=2))
u.log('FOLIAGE MIX PASS '+str(counts))
