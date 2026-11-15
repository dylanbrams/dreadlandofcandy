"""Update only the river marshmallow actors in the existing, saved world."""
from pathlib import Path
import runpy
import unreal as u

HERE = Path(__file__).resolve().parent
dressing = runpy.run_path(str(HERE / 'candy_landscape_dressing.py'))
water = runpy.run_path(str(HERE / 'candy_water.py'))
material = u.load_asset('/Game/World/Expansion/M_Marshmallow')
meshes = [u.load_asset('/Game/World/CandyPBR/Marshmallows/SM_Marshmallow_%02d' % i) for i in range(1, 4)]
assert material and all(meshes)
world = u.EditorLoadingAndSavingUtils.load_map('/Game/World/L_Sugarborough')
actors = u.get_editor_subsystem(u.EditorActorSubsystem)
prefix = 'CandyLandscapeDressing_River_Marshmallows_'
existing = [a for a in actors.get_all_level_actors() if a.get_actor_label().startswith(prefix)]
assert len(existing) in (13, 52), 'Unexpected marshmallow count: %d' % len(existing)
existing.sort(key=lambda a: int(a.get_actor_label()[len(prefix):]))
basic = u.load_asset('/Engine/BasicShapes/Cylinder')
transforms = dressing['marshmallow_instances'](basic, basic.get_bounding_box())
assert len(transforms) == 52
water['build_marshmallow_material'](material)
for index, transform in enumerate(transforms):
    x, y, _, pitch, yaw, roll, _, _, _ = transform
    actor = existing[index] if index < len(existing) else actors.spawn_actor_from_class(
        u.StaticMeshActor, u.Vector(x, y, 0), u.Rotator(pitch=pitch, yaw=yaw, roll=roll))
    assert actor
    actor.set_actor_label(prefix + str(index))
    actor.set_folder_path('Sugarborough/Candy landscape dressing')
    mesh = meshes[index % len(meshes)]
    box = mesh.get_bounding_box()
    dim = box.max - box.min
    width = (65 + (index % 3) * 8) * 2.0 / 3.0
    height = (48 + (index % 4) * 4) * 2.0 / 3.0
    scale = u.Vector(width / dim.x, width / dim.y, height / dim.z)
    component = actor.static_mesh_component
    component.set_static_mesh(mesh)
    component.set_material(0, material)
    component.set_collision_profile_name('NoCollision')
    component.set_collision_enabled(u.CollisionEnabled.NO_COLLISION)
    component.set_editor_property('bounds_scale', 1000.0)
    actor.set_actor_scale3d(scale)
    actor.set_actor_rotation(u.Rotator(pitch=pitch, yaw=yaw, roll=roll), False)
    actor.set_actor_location(u.Vector(x, y, -65 + height * .2 - (box.min.z + box.max.z) * .5 * scale.z), False, False)
assert u.EditorLoadingAndSavingUtils.save_map(world, '/Game/World/L_Sugarborough')
u.log('MARSHMALLOW DENSITY PASS count=52 scale=2/3 lanes=curved seed=20261118')
