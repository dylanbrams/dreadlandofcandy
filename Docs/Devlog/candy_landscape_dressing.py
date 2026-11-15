"""Add deterministic, nonblocking candy groves and an eastern mountain backdrop.

Run explicitly in the Unreal Editor with the project open. This pass loads and
saves only /Game/World/L_Sugarborough and only owns actors whose labels begin
with ``CandyLandscapeDressing_``. Decorative static mesh actors have collision
disabled so they cannot block gameplay or the camera.
"""

import math
import random
import unreal as u


MAP_PATH = "/Game/World/L_Sugarborough"
ASSET_ROOT = "/Game/World/Expansion"
MOUNTAIN_PATH = "/Game/World/Meshes/SM_SugarMountain"
LABEL_PREFIX = "CandyLandscapeDressing_"
SEED = 20261115
ACTORS = None


def river_local_x(y):
    """Return the extension source-coordinate river center (world minus 4000)."""
    return 15000.0 + 500.0 * math.sin(y / 3500.0)


def terrain_z(world_x, y):
    """Mirror Expansion/export_extension.py terrain_z in world coordinates."""
    x = world_x - 4000.0
    if x < 16000.0:
        hills = 0.0
    else:
        hills = 220.0 * math.sin((x - 16000.0) / 6000.0 * math.pi) ** 2
        hills *= 0.5 + 0.5 * math.cos(y / 5000.0)

    # Flatten the authored far camp pad at local (18200, 0), world (22200, 0).
    camp_distance = math.hypot(x - 18200.0, y)
    camp_t = max(0.0, min(1.0, (camp_distance - 1000.0) / 800.0))
    camp_blend = camp_t * camp_t * (3.0 - 2.0 * camp_t)
    hills *= camp_blend

    river_distance = abs(x - river_local_x(y))
    if river_distance <= 300.0:
        return -180.0
    if river_distance < 600.0:
        return -180.0 * (1.0 - (river_distance - 300.0) / 300.0)
    if river_distance < 900.0:
        t = (river_distance - 600.0) / 300.0
        return hills * t * t * (3.0 - 2.0 * t)
    return hills


def load_mesh(path):
    mesh = u.load_asset(path)
    if not mesh:
        raise RuntimeError("Required candy dressing mesh is missing: " + path)
    bounds = mesh.get_bounding_box()
    size = bounds.max - bounds.min
    if size.x <= 0 or size.y <= 0 or size.z <= 0:
        raise RuntimeError("Invalid mesh bounds: " + path)
    return mesh, bounds


def make_transform(x, y, z, yaw, scale, pitch=0.0, roll=0.0, scale_y=None, scale_z=None):
    if scale_y is None:
        scale_y = scale
    if scale_z is None:
        scale_z = scale
    return (x, y, z, pitch, yaw, roll, scale, scale_y, scale_z)


def clear_previous_pass():
    for actor in list(ACTORS.get_all_level_actors()):
        if actor.get_actor_label().startswith(LABEL_PREFIX):
            if not ACTORS.destroy_actor(actor):
                raise RuntimeError("Could not remove previous dressing actor: " + actor.get_actor_label())


def river_grove_instances(mesh_records):
    rng = random.Random(SEED)
    instances = {name: [] for name in mesh_records}
    tree_names = ["SM_PeppermintTree", "SM_CandyAppleTree", "SM_CandyTree"]
    shrub_names = ["SM_CottonCandyBush", "SM_GumdropBush", "SM_ToastedMarshmallowMushrooms"]
    y_centers = [-9800, -6600, -3400, 3400, 6600, 9800]
    tree_index = 0
    shrub_index = 0

    def is_allowed(x, y):
        river_x = 4000.0 + river_local_x(y)
        return (
            16200.0 <= x <= 25700.0
            and -11200.0 <= y <= 11200.0
            and abs(y) >= 900.0
            and abs(x - river_x) >= 900.0
            and math.hypot(x - 22200.0, y) >= 1800.0
        )

    def add_species(name, x, y, target_height, yaw):
        mesh, bounds = mesh_records[name]
        scale = target_height / (bounds.max.z - bounds.min.z)
        z = terrain_z(x, y) - bounds.min.z * scale
        instances[name].append(make_transform(x, y, z, yaw, scale))

    # Paired groves on both banks. Centers alternate between inner and outer
    # banks; decorations remain outside the river, road, camp, and map margins.
    for side in (-1, 1):
        for row, center_y in enumerate(y_centers):
            river_x = 4000.0 + river_local_x(center_y)
            bank_offset = 2350.0 if row % 2 == 0 else 3850.0
            center_x = river_x + side * bank_offset

            for _ in range(4):
                accepted = None
                for _attempt in range(40):
                    x = center_x + rng.uniform(-430.0, 430.0)
                    y = center_y + rng.uniform(-480.0, 480.0)
                    if is_allowed(x, y):
                        accepted = (x, y)
                        break
                if accepted is None:
                    continue
                x, y = accepted
                name = tree_names[tree_index % len(tree_names)]
                tree_index += 1
                add_species(name, x, y, rng.uniform(560.0, 820.0), rng.uniform(0.0, 360.0))

            for _ in range(9):
                angle = rng.uniform(0.0, math.tau)
                radius = rng.uniform(680.0, 1150.0)
                x = center_x + math.cos(angle) * radius
                y = center_y + math.sin(angle) * radius
                if not is_allowed(x, y):
                    continue
                name = shrub_names[shrub_index % len(shrub_names)]
                shrub_index += 1
                add_species(name, x, y, rng.uniform(150.0, 285.0), rng.uniform(0.0, 360.0))

    if tree_index < 24 or shrub_index < 60:
        raise RuntimeError(
            "River grove placement yielded too few instances: trees="
            + str(tree_index)
            + " shrubs="
            + str(shrub_index)
        )
    return instances


def mountain_instances(mesh, bounds):
    rng = random.Random(SEED + 1)
    instances = []
    height = bounds.max.z - bounds.min.z
    # Two staggered rows form a backdrop beyond the east Pawn collision wall.
    rows = [(27900.0, -14300.0), (30700.0, -13000.0)]
    for row_index, (x, y_start) in enumerate(rows):
        y = y_start
        step = 2650.0
        while y <= 15000.0:
            scale = rng.uniform(4000.0, 5500.0) / height
            ground_z = 0.0
            instances.append(
                make_transform(
                    x + rng.uniform(-220.0, 220.0),
                    y + (row_index * 1150.0),
                    ground_z - bounds.min.z * scale,
                    rng.uniform(0.0, 360.0),
                    scale,
                )
            )
            y += step
    if not instances or min(float(t[0]) for t in instances) <= 26000.0:
        raise RuntimeError("Mountain backdrop must remain east of x=26000")
    return instances


def steam_instances():
    rng = random.Random(SEED + 2)
    instances = []
    for y in range(-10800, 10801, 1500):
        x = 4000.0 + river_local_x(float(y))
        z = rng.uniform(-20.0, 150.0)
        width = rng.uniform(2.8, 3.9)
        height = rng.uniform(2.8, 4.2)
        # Crossed vertical cards keep the translucent wisp visible from either bank.
        instances.append(make_transform(x, y, z, 0.0, height, pitch=90.0, scale_y=width))
        instances.append(make_transform(x, y, z, 0.0, width, roll=90.0, scale_y=height))
    return instances


def marshmallow_instances(mesh, bounds):
    rng = random.Random(SEED + 3)
    instances = []
    dimensions = bounds.max - bounds.min
    for y in range(-9600, 9601, 1600):
        x = 4000.0 + river_local_x(float(y)) + rng.uniform(-100.0, 100.0)
        radius = rng.uniform(25.0, 40.0)
        height = rng.uniform(45.0, 60.0)
        scale_x = radius * 2.0 / dimensions.x
        scale_y = radius * 2.0 / dimensions.y
        scale_z = height / dimensions.z
        center_local_z = (bounds.min.z + bounds.max.z) * 0.5
        center_z = -65.0 + height / 3.0 - center_local_z * scale_z
        instances.append(
            make_transform(
                x,
                float(y) + rng.uniform(-180.0, 180.0),
                center_z,
                rng.uniform(0.0, 360.0),
                scale_x,
                scale_y=scale_y,
                scale_z=scale_z,
            )
        )
    return instances


def spawn_mesh_instances(label, mesh, transforms, material=None):
    """Spawn ordinary static mesh actors; avoid relying on runtime HISM creation APIs."""
    spawned = []
    for index, transform in enumerate(transforms):
        x, y, z, pitch, yaw, roll, sx, sy, sz = transform
        actor = ACTORS.spawn_actor_from_class(
            u.StaticMeshActor,
            u.Vector(x, y, z),
            u.Rotator(pitch=pitch, yaw=yaw, roll=roll),
        )
        if not actor:
            raise RuntimeError("Could not spawn dressing actor " + label + "_" + str(index))
        actor.set_actor_label(LABEL_PREFIX + label + "_" + str(index))
        actor.set_folder_path("Sugarborough/Candy landscape dressing")
        component = actor.static_mesh_component
        component.set_static_mesh(mesh)
        if material:
            component.set_material(0, material)
        component.set_collision_profile_name("NoCollision")
        component.set_collision_enabled(u.CollisionEnabled.NO_COLLISION)
        if label == 'River_Marshmallows':
            component.set_editor_property('bounds_scale',1000.)
        actor.set_actor_scale3d(u.Vector(sx, sy, sz))
        spawned.append(actor)
    return spawned


def main():
    global ACTORS
    world = u.EditorLoadingAndSavingUtils.load_map(MAP_PATH)
    if not world:
        raise RuntimeError("Could not load target map " + MAP_PATH)
    ACTORS = u.get_editor_subsystem(u.EditorActorSubsystem)
    # Dense terrain installed per-actor material overrides; mesh edits alone
    # therefore do not update the original village ground.
    ground_material = u.load_asset('/Game/World/CandyPBR/M_CandyTerrain')
    assert ground_material
    for actor in ACTORS.get_all_level_actors():
        if isinstance(actor,u.StaticMeshActor):
            mesh=actor.static_mesh_component.static_mesh
            if mesh and mesh.get_name().startswith(('SM_Terrain_','SM_EastOpening_')):
                actor.static_mesh_component.set_material(0,ground_material)

    tree_and_shrub_names = [
        "SM_PeppermintTree",
        "SM_CandyAppleTree",
        "SM_CandyTree",
        "SM_CottonCandyBush",
        "SM_GumdropBush",
        "SM_ToastedMarshmallowMushrooms",
    ]
    records = {
        name: load_mesh(ASSET_ROOT + "/" + name)
        for name in tree_and_shrub_names
    }
    mountain_mesh, mountain_bounds = load_mesh(MOUNTAIN_PATH)
    plane_mesh, _ = load_mesh("/Engine/BasicShapes/Plane")
    marshmallow_mesh, marshmallow_bounds = load_mesh("/Engine/BasicShapes/Cylinder")
    steam_material = u.load_asset(ASSET_ROOT + "/M_HotChocolateSteam")
    marshmallow_material = u.load_asset(ASSET_ROOT + "/M_Marshmallow")
    if not steam_material or not marshmallow_material:
        raise RuntimeError("Run the hot chocolate material builder before this dressing pass")

    grove = river_grove_instances(records)
    mountains = mountain_instances(mountain_mesh, mountain_bounds)
    steam = steam_instances()
    marshmallows = marshmallow_instances(marshmallow_mesh, marshmallow_bounds)

    clear_previous_pass()
    spawned = []
    for name in tree_and_shrub_names:
        spawned.extend(spawn_mesh_instances("Grove_" + name, records[name][0], grove[name]))
    spawned.extend(spawn_mesh_instances("East_Mountain_Backdrop", mountain_mesh, mountains))
    spawned.extend(spawn_mesh_instances("River_Steam", plane_mesh, steam, steam_material))
    spawned.extend(
        spawn_mesh_instances("River_Marshmallows", marshmallow_mesh, marshmallows, marshmallow_material)
    )
    # Preserve the generated marshmallow upgrade when rebuilding all scenery.
    variants=[u.load_asset('/Game/World/CandyPBR/Marshmallows/SM_Marshmallow_%02d'%i) for i in range(1,4)]
    if all(variants):
        mallows=[a for a in spawned if '_River_Marshmallows_' in a.get_actor_label()]
        for index,actor in enumerate(mallows):
            mesh=variants[index%len(variants)]
            b=mesh.get_bounding_box();d=b.max-b.min
            height=48+(index%4)*4;width=65+(index%3)*8
            scale=u.Vector(width/d.x,width/d.y,height/d.z)
            actor.static_mesh_component.set_static_mesh(mesh)
            actor.set_actor_scale3d(scale)
            p=actor.get_actor_location();p.z=-65+height*.2-(b.min.z+b.max.z)*.5*scale.z
            actor.set_actor_location(p,False,False)

    expected = sum(len(items) for items in grove.values()) + len(mountains) + len(steam) + len(marshmallows)
    if len(spawned) != expected:
        raise RuntimeError("Dressing actor count mismatch: expected " + str(expected) + " got " + str(len(spawned)))
    if not u.EditorLoadingAndSavingUtils.save_map(world, MAP_PATH):
        raise RuntimeError("Could not save dressed map " + MAP_PATH)
    u.log(
        "CANDY LANDSCAPE DRESSING PASS trees="
        + str(sum(len(grove[name]) for name in tree_and_shrub_names[:3]))
        + " shrubs="
        + str(sum(len(grove[name]) for name in tree_and_shrub_names[3:]))
        + " mountains="
        + str(len(mountains))
        + " steam_cards="
        + str(len(steam))
        + " marshmallows="
        + str(len(marshmallows))
    )


if __name__ == "__main__":
    main()
