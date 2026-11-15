"""Fixes4 #9-10 terrain pass. Exclusive Unreal editor (commandlet) use.

Run export_extension.py in Blender first. Reimports the deeper river channel
and bank-to-bank water, re-runs the candy ground materials (road gap closed),
shifts riverside props onto the new banks, un-flattens the bridge with a
walkable hidden deck and river-edge blockers, clears roads of trees/buildings,
adds brush along the east road, saves L_Sugarborough and two review maps.
"""
import unreal as u
import math, random
from pathlib import Path

ROOT = Path(u.Paths.project_dir()).resolve()
HERE = ROOT / 'ImportSource/Terrain/Expansion'
BASE = '/Game/World/Expansion'
eal = u.EditorAssetLibrary
actors = u.get_editor_subsystem(u.EditorActorSubsystem)
sm = u.get_default_object(u.StaticMeshEditorSubsystem)
world = u.EditorLoadingAndSavingUtils.load_map('/Game/World/L_Sugarborough')
LOG = []
def log(*a):
    s = 'FIXES4 ' + ' '.join(str(x) for x in a); u.log(s); LOG.append(s)

def ss(t):
    t = max(0.0, min(1.0, t)); return t * t * (3 - 2 * t)
def river(y): return 15000 + 500 * math.sin(y / 3500)
def hills(x, y):
    h = 0.0 if x < 16000 else 220 * math.sin((x - 16000) / 6000 * math.pi) ** 2 * (0.5 + 0.5 * math.cos(y / 5000))
    return h * ss((math.hypot(x - 18200, y) - 1000) / 800)
def old_z(x, y):  # local coords, previous profile
    d = abs(x - river(y)); h = hills(x, y)
    if d <= 300: return -180
    if d < 600: return -180 * (1 - (d - 300) / 300)
    if d < 900: return h * ss((d - 600) / 300)
    return h
def new_z(x, y):  # local coords, export_extension.py
    d = abs(x - river(y)); h = hills(x, y)
    if d <= 380: return -260
    if d < 660: return -260 + 320 * ss((d - 380) / 280)
    if d < 900: return 60
    if d < 1300: return 60 + (h - 60) * ss((d - 900) / 400)
    return h

# 1. Reimport terrain + water, keeping material assignments.
def reimport(name):
    dest = BASE + '/' + name
    old = u.load_asset(dest)
    mats = [s.get_editor_property('material_interface') for s in old.get_editor_property('static_materials')] if old else []
    t = u.AssetImportTask()
    t.set_editor_properties(dict(filename=str(HERE / 'FBX' / (name + '.fbx')), destination_path=BASE, destination_name=name,
                                 automated=True, save=True, replace_existing=True))
    opt = u.FbxImportUI()
    opt.set_editor_properties(dict(import_mesh=True, import_as_skeletal=False, import_materials=False, import_textures=False))
    opt.static_mesh_import_data.set_editor_properties(dict(combine_meshes=True, auto_generate_collision=False))
    t.options = opt
    u.AssetToolsHelpers.get_asset_tools().import_asset_tasks([t])
    mesh = u.load_asset(dest)
    body = mesh.get_editor_property('body_setup')
    body.set_editor_property('collision_trace_flag', u.CollisionTraceFlag.CTF_USE_COMPLEX_AS_SIMPLE)
    body.set_editor_property('double_sided_geometry', True)
    for i in range(mesh.get_num_sections(0)): sm.enable_section_collision(mesh, True, 0, i)
    sm.remove_collisions(mesh)
    for i, m in enumerate(mats):
        if m and i < len(mesh.get_editor_property('static_materials')): mesh.set_material(i, m)
    eal.save_loaded_asset(mesh)
    b = mesh.get_bounding_box(); log('reimported', name, b.min, b.max)
reimport('SM_Riverlands'); reimport('SM_RiverWater')
ground_script = ROOT / 'Docs/Devlog/apply_candy_ground.py'
exec(compile(ground_script.read_text(encoding='utf-8'), str(ground_script), 'exec'), {'__name__': '__main__'})

def label(a): return a.get_actor_label()
def mesh_name(c):
    m = c.static_mesh; return m.get_name() if m else ''

FIRST_RUN = not any(label(a).startswith('Riverlands_Fixes4_') for a in actors.get_all_level_actors())
log('first run', FIRST_RUN)
# 2. Riverside props follow the new bank heights (water props stay put).
shifted = 0
for a in (actors.get_all_level_actors() if FIRST_RUN else []):
    lb = label(a)
    if 'River' in lb and ('Steam' in lb or 'Marshmallow' in lb or 'RiverWater' in lb or 'SM_Riverlands' in lb): continue
    if 'Bridge' in lb or 'Boundary' in lb: continue
    if isinstance(a, u.StaticMeshActor):
        p = a.get_actor_location()
        if 16000 < p.x < 26000 and abs(p.y) < 12000:
            dz = new_z(p.x - 4000, p.y) - old_z(p.x - 4000, p.y)
            if abs(dz) > .5:
                a.set_actor_location(u.Vector(p.x, p.y, p.z + dz), False, True); shifted += 1
    for c in a.get_components_by_class(u.InstancedStaticMeshComponent):
        for i in range(c.get_instance_count()):
            tr = c.get_instance_transform(i, True); p = tr.translation
            if 16000 < p.x < 26000:
                dz = new_z(p.x - 4000, p.y) - old_z(p.x - 4000, p.y)
                if abs(dz) > .5:
                    tr.translation = u.Vector(p.x, p.y, p.z + dz)
                    c.update_instance_transform(i, tr, True, True, True); shifted += 1
log('props shifted to new banks', shifted)

# 3. Bridge: real proportions, deck level with the bank lips, hidden walk deck.
cube = u.load_asset('/Engine/BasicShapes/Cube')
def hidden_block(name, x, y, z, sx, sy, sz, yaw=0):
    a = actors.spawn_actor_from_class(u.StaticMeshActor, u.Vector(x, y, z), u.Rotator(yaw=yaw))
    a.set_actor_label('Riverlands_Fixes4_' + name); a.set_folder_path('Sugarborough/Riverlands/Fixes4')
    c = a.static_mesh_component; c.set_static_mesh(cube)
    a.set_actor_scale3d(u.Vector(sx / 100, sy / 100, sz / 100)); a.set_actor_hidden_in_game(True)
    c.set_collision_enabled(u.CollisionEnabled.QUERY_ONLY)
    c.set_collision_response_to_all_channels(u.CollisionResponseType.ECR_IGNORE)
    c.set_collision_response_to_channel(u.CollisionChannel.ECC_PAWN, u.CollisionResponseType.ECR_BLOCK)
    return a
for a in actors.get_all_level_actors():
    if label(a).startswith('Riverlands_Fixes4_'): actors.destroy_actor(a)
bridge = [a for a in actors.get_all_level_actors() if label(a) == 'Riverlands_PeppermintBridge'][0]
b = bridge.static_mesh_component.static_mesh.get_bounding_box(); d = b.max - b.min
BX = 4000 + river(0); DECK = 75.0; L, W, H = 2000.0, 900.0, 320.0
sz = H / d.z
bridge.set_actor_scale3d(u.Vector(L / d.x, W / d.y, sz))
# Deck surface sits at ~49% of the mesh height (measured in Blender).
bridge.set_actor_location(u.Vector(BX, 0, DECK - 0.49 * H - b.min.z * sz), False, False)
bridge.static_mesh_component.set_collision_profile_name('NoCollision')
hidden_block('BridgeDeck', BX, 0, DECK - 20, 1750, 760, 40)
for side in (-1, 1):
    hidden_block('BridgeRail' + str(side), BX, side * 395, DECK + 100, 1750, 30, 200)
# River-edge blockers: the new banks are too steep to climb out of.
n = 0
for side in (-1, 1):
    y = -12000.0
    while y < 12000:
        y1 = min(y + 400, 12000)
        if not (abs(y) < 400 or abs(y1) < 400 or (y < 0 < y1)):
            x0 = 4000 + river(y) + side * 720; x1 = 4000 + river(y1) + side * 720
            yaw = math.degrees(math.atan2(y1 - y, x1 - x0))
            hidden_block('RiverEdge%d' % n, (x0 + x1) / 2, (y + y1) / 2, 150,
                         math.hypot(x1 - x0, y1 - y) + 20, 30, 400, yaw); n += 1
        y = y1
log('bridge', bridge.get_actor_location(), 'edge blockers', n)

# 4. Roads: plaza, ring, three spokes, east road (matches apply_candy_ground.py).
def road_d(x, y):
    r = math.hypot(x, y)
    if r < 1050: return 0.0
    dd = abs(r - 3600) if r > 2700 else 1e9
    if max(abs(x), abs(y)) < 10700:
        for k in range(3):
            a = k * 2.0943951
            if 0 <= x * math.cos(a) + y * math.sin(a) <= 10500:
                dd = min(dd, abs(-x * math.sin(a) + y * math.cos(a)))
    if x > 10000: dd = min(dd, abs(y))
    return dd
TREES = ('Tree', 'Fir', 'Birch', 'Blossom', 'Pine', 'Taffy', 'Umbrella')
BUILDINGS = ('House', 'Cottage', 'Hall', 'Bakery', 'Dentist', 'Farm', 'Tower', 'Shop', 'Inn', 'Mill')
removed = 0
for a in actors.get_all_level_actors():
    for c in a.get_components_by_class(u.InstancedStaticMeshComponent):
        nm = mesh_name(c); clear = 430 if any(t in nm for t in TREES) else 320
        for i in range(c.get_instance_count() - 1, -1, -1):
            p = c.get_instance_transform(i, True).translation
            if math.hypot(p.x, p.y) > 1100 and road_d(p.x, p.y) < clear:
                c.remove_instance(i); removed += 1
log('road instances removed', removed)
gone = []
for a in actors.get_all_level_actors():
    if not isinstance(a, u.StaticMeshActor): continue
    nm = mesh_name(a.static_mesh_component); p = a.get_actor_location()
    if math.hypot(p.x, p.y) < 1100 or nm.startswith(('SM_Terrain_', 'SM_EastOpening_', 'SM_Riverlands', 'SM_River')): continue
    if any(t in nm for t in TREES) and road_d(p.x, p.y) < 450:
        log('tree removed from road', label(a), nm, p); gone.append(p); actors.destroy_actor(a)
    elif any(t in nm for t in BUILDINGS):
        org, ext = a.get_actor_bounds(False); rad = 0.75 * max(ext.x, ext.y) + 330
        x, y = p.x, p.y; steps = 0
        while road_d(x, y) < rad and steps < 60:
            e = 10.0; gx = road_d(x + e, y) - road_d(x - e, y); gy = road_d(x, y + e) - road_d(x, y - e)
            g = math.hypot(gx, gy) or 1.0; x += 40 * gx / g; y += 40 * gy / g; steps += 1
        if steps:
            a.set_actor_location(u.Vector(x, y, p.z), False, False)
            log('building moved off road', label(a), nm, 'from', round(p.x), round(p.y), 'to', round(x), round(y), 'road_d', round(road_d(x, y)), 'needed', round(rad))
for a in actors.get_all_level_actors():  # hidden trunk colliders of removed trees
    if isinstance(a, u.StaticMeshActor) and mesh_name(a.static_mesh_component) == 'Cylinder':
        p = a.get_actor_location()
        if any(math.hypot(p.x - q.x, p.y - q.y) < 150 for q in gone): actors.destroy_actor(a)

# 5. Brush and grass along the east road to the river and on to the camp.
hism = {}
for a in actors.get_all_level_actors():
    for c in a.get_components_by_class(u.HierarchicalInstancedStaticMeshComponent):
        if c.get_instance_count(): hism.setdefault(mesh_name(c), c)
def scale_of(c):
    vals = sorted(c.get_instance_transform(i, True).scale3d.z for i in range(0, c.get_instance_count(), max(1, c.get_instance_count() // 40)))
    return vals[len(vals) // 2]
def ground(x, y):
    hits = u.SystemLibrary.line_trace_multi_for_objects(world, u.Vector(x, y, 3000), u.Vector(x, y, -1000),
        [u.ObjectTypeQuery.ECC_WORLD_STATIC], True, [], u.DrawDebugTrace.NONE, True)
    hits = hits[-1] if isinstance(hits, tuple) else hits
    for h in hits:
        t = h.to_tuple(); actor = t[9]
        if isinstance(actor, u.StaticMeshActor) and mesh_name(actor.static_mesh_component).startswith(('SM_Terrain_', 'SM_EastOpening_', 'SM_Riverlands')):
            return t[4].z
    return None
kinds = [('Grass', 0.62, 470, 1500), ('Bush', 0.28, 600, 1700), ('Fern', 0.10, 520, 1400)]
pools = {'Grass': [k for k in hism if 'CandyGrass' in k],
         'Bush': [k for k in hism if any(s in k for s in ('JellybeanBush', 'CottonCandyBush', 'GumdropBush', 'Candyfloss'))],
         'Fern': [k for k in hism if any(s in k for s in ('SugarFern', 'CandyFlowerClump', 'GummySucculent', 'Marshmallow'))]}
log('brush pools', pools)
rng = random.Random(1504)
added = {}
for _ in range(2600 if FIRST_RUN else 0):
    kind = rng.choices([k[0] for k in kinds], [k[1] for k in kinds])[0]
    _, _, lo, hi = [k for k in kinds if k[0] == kind][0]
    if not pools[kind]: continue
    x = rng.uniform(3900, 25800); y = rng.choice((-1, 1)) * rng.uniform(lo, hi)
    if abs(x - 4000 - river(y)) < 1150 or math.hypot(x - 22200, y) < 1900 or road_d(x, y) < lo - 20: continue
    z = ground(x, y)
    if z is None: continue
    nm = rng.choice(pools[kind]); c = hism[nm]; s = scale_of(c) * rng.uniform(.8, 1.25)
    c.add_instance(u.Transform(u.Vector(x, y, z - 4), u.Rotator(yaw=rng.uniform(0, 360)), u.Vector(s, s, s)), True)
    added[nm] = added.get(nm, 0) + 1
log('brush added', added, 'total', sum(added.values()))

assert u.EditorLoadingAndSavingUtils.save_map(world, '/Game/World/L_Sugarborough')
log('MAP SAVED')

# Review maps (camera-only copies, GameModeBase).
world.get_world_settings().set_editor_property('default_game_mode', u.GameModeBase)
for a in actors.get_all_level_actors():
    if isinstance(a, u.CameraActor): a.set_editor_property('auto_activate_for_player', u.AutoReceiveInput.DISABLED)
cam = actors.spawn_actor_from_class(u.CameraActor, u.Vector(BX - 2300, -2100, 420), u.Rotator(pitch=-9, yaw=40))
cam.set_editor_property('auto_activate_for_player', u.AutoReceiveInput.PLAYER0)
for name, loc, rot in [('L_RiverReview', None, None),
                       ('L_RoadReview', u.Vector(8200, -1700, 650), u.Rotator(pitch=-12, yaw=20))]:
    if loc: cam.set_actor_location(loc, False, False); cam.set_actor_rotation(rot, False)
    if eal.does_asset_exist(BASE + '/' + name): eal.delete_asset(BASE + '/' + name)
    assert u.EditorLoadingAndSavingUtils.save_map(world, BASE + '/' + name)
(HERE / 'fixes4_report.txt').write_text('\n'.join(LOG), encoding='utf-8')
u.log('FIXES4 TERRAIN PASS')
