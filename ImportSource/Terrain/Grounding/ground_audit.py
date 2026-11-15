"""Audit (and optionally fix) props floating above the terrain in L_Sugarborough.

Run headless:  UnrealEditor MegaJam.uproject -unattended -nullrhi -ExecutePythonScript=<this>
Add -groundfix to apply the snap and saves the map; otherwise read-only report.
"""
import json, math, os
import unreal as u

MAP = '/Game/World/L_Sugarborough'
OUT = os.path.join(os.path.dirname(os.path.abspath(__file__)), 'ground_report.json')
TERRAIN = ('SM_Terrain_', 'SM_EastOpening_', 'SM_Riverlands')
FIX = '-groundfix' in u.SystemLibrary.get_command_line().lower()
THRESHOLD = 6.0          # cm of air under the lowest point before we snap
SKIP_MESH = ('SM_SugarSnowflake', 'SM_Bluebird', 'Bridge', 'Cube', 'Water', 'Steam', 'Cloud', 'MatineeCam')
SKIP_LABEL = ('CandyLandscapeDressing_River_Marshmallows_',)  # drift on the river surface

world = u.EditorLoadingAndSavingUtils.load_map(MAP)
sub = u.get_editor_subsystem(u.EditorActorSubsystem)
actors = sub.get_all_level_actors()

def mesh_name(c):
    m = c.get_editor_property('static_mesh')
    return m.get_name() if m else ''

def is_terrain_actor(a):
    if isinstance(a, u.Landscape) or isinstance(a, u.LandscapeProxy):
        return True
    if isinstance(a, u.StaticMeshActor):
        return mesh_name(a.static_mesh_component).startswith(TERRAIN)
    return False

terrain = [a for a in actors if is_terrain_actor(a)]
u.log('GROUND terrain actors=%d' % len(terrain))

CELL = 100.0
cells = {}
def add_terrain(a):
    c = a.static_mesh_component
    m = c.get_editor_property('static_mesh')
    xf = c.get_world_transform()
    ntri = 0
    for sec in range(m.get_num_sections(0)):
        verts, tris, _n, _uv, _t = u.ProceduralMeshLibrary.get_section_from_static_mesh(m, 0, sec)
        w = [xf.transform_location(v) for v in verts]
        w = [(p.x, p.y, p.z) for p in w]
        for i in range(0, len(tris), 3):
            t = (w[tris[i]], w[tris[i + 1]], w[tris[i + 2]])
            x0 = int(math.floor(min(p[0] for p in t) / CELL)); x1 = int(math.floor(max(p[0] for p in t) / CELL))
            y0 = int(math.floor(min(p[1] for p in t) / CELL)); y1 = int(math.floor(max(p[1] for p in t) / CELL))
            for cx in range(x0, x1 + 1):
                for cy in range(y0, y1 + 1):
                    cells.setdefault((cx, cy), []).append(t)
            ntri += 1
    u.log('GROUND terrain %s tris=%d' % (a.get_actor_label(), ntri))

for a in terrain:
    if isinstance(a, u.StaticMeshActor):
        add_terrain(a)

def ground(x, y):
    best = None
    for (ax, ay, az), (bx, by, bz), (cx, cy, cz) in cells.get((int(math.floor(x / CELL)), int(math.floor(y / CELL))), ()):
        d = (by - cy) * (ax - cx) + (cx - bx) * (ay - cy)
        if abs(d) < 1e-9:
            continue
        l1 = ((by - cy) * (x - cx) + (cx - bx) * (y - cy)) / d
        l2 = ((cy - ay) * (x - cx) + (ax - cx) * (y - cy)) / d
        l3 = 1 - l1 - l2
        if l1 >= -1e-6 and l2 >= -1e-6 and l3 >= -1e-6:
            z = l1 * az + l2 * bz + l3 * cz
            best = z if best is None else max(best, z)
    return best

# Props standing on these meshes are supported by them, not by the terrain.
zones = []
for a in actors:
    if isinstance(a, u.StaticMeshActor) and any(k in mesh_name(a.static_mesh_component) for k in ('CaveMouth', 'Bridge')):
        o, e = a.get_actor_bounds(False)
        zones.append((o.x, o.y, e.x, e.y))

def in_zone(x, y):
    return any(abs(x - zx) < ex and abs(y - zy) < ey for zx, zy, ex, ey in zones)

rows = []
fixes = []
_base = {}
def base_points(mesh):
    """Local-space points on the part of the mesh that should touch the ground: the
    lowest band of vertices (trunk ring, stem, foot), reduced to its 8-direction extremes."""
    n = mesh.get_name()
    if n not in _base:
        verts = []
        for sec in range(mesh.get_num_sections(0)):
            verts += u.ProceduralMeshLibrary.get_section_from_static_mesh(mesh, 0, sec)[0]
        if not verts:
            bb = mesh.get_bounding_box()
            _base[n] = [u.Vector((bb.min.x + bb.max.x) / 2, (bb.min.y + bb.max.y) / 2, bb.min.z)]
            return _base[n]
        zmin = min(v.z for v in verts); zmax = max(v.z for v in verts)
        band = [v for v in verts if v.z <= zmin + max(8.0, 0.04 * (zmax - zmin))]
        pts = {}
        for dx, dy in ((1, 0), (-1, 0), (0, 1), (0, -1), (.7, .7), (.7, -.7), (-.7, .7), (-.7, -.7)):
            v = max(band, key=lambda q: q.x * dx + q.y * dy)
            pts[(round(v.x, 1), round(v.y, 1), round(v.z, 1))] = v
        cx = sum(v.x for v in band) / len(band); cy = sum(v.y for v in band) / len(band)
        pts['c'] = u.Vector(cx, cy, zmin)
        _base[n] = list(pts.values())
    return _base[n]

def audit(owner, comp, idx, xf, mesh):
    b = mesh.get_bounding_box()
    loc, s = xf.translation, xf.scale3d
    rot = xf.rotation.rotator()
    tilted = abs(rot.pitch) > 20 or abs(rot.roll) > 20
    gap = None; gc = None
    if not tilted:
        for p in base_points(mesh):
            w = xf.transform_location(p)
            g = ground(w.x, w.y)
            if g is not None:
                gap = w.z - g if gap is None else max(gap, w.z - g)
        gc = ground(loc.x, loc.y)
    rows.append(dict(actor=owner.get_actor_label(), folder=str(owner.get_folder_path()), comp=comp.get_name(),
        idx=idx, mesh=mesh.get_name(), x=round(loc.x), y=round(loc.y), z=round(loc.z, 1),
        gap=None if gap is None else round(gap, 1), ground_c=None if gc is None else round(gc, 1),
        height=round((b.max.z - b.min.z) * s.z), tilted=tilted,
        hidden=bool(owner.is_hidden_ed()) or not comp.is_visible()))
    r = rows[-1]
    if (gap is None or r['hidden'] or any(k in mesh.get_name() for k in SKIP_MESH)
            or owner.get_actor_label().startswith(SKIP_LABEL) or in_zone(loc.x, loc.y)):
        return
    if gap > THRESHOLD:
        sink = 3.0 if r['height'] < 150 else 6.0
        fixes.append((owner, comp, idx, xf, -(gap + sink), r))

for a in actors:
    if a in terrain:
        continue
    for c in a.get_components_by_class(u.StaticMeshComponent):
        m = c.get_editor_property('static_mesh')
        if not m:
            continue
        if isinstance(c, u.InstancedStaticMeshComponent):
            for i in range(c.get_instance_count()):
                audit(a, c, i, c.get_instance_transform(i, True), m)
        else:
            audit(a, c, -1, c.get_world_transform(), m)

moved = []
if FIX:
    dirty = set()
    for owner, comp, idx, xf, dz, r in fixes:
        if idx < 0:
            p = owner.get_actor_location()
            owner.set_actor_location(u.Vector(p.x, p.y, p.z + dz), False, False)
        else:
            t = xf.translation
            nxf = u.Transform(u.Vector(t.x, t.y, t.z + dz), xf.rotation.rotator(), xf.scale3d)
            comp.update_instance_transform(idx, nxf, True, True, True)
            dirty.add(comp)
        moved.append(dict(actor=r['actor'], mesh=r['mesh'], idx=idx, x=r['x'], y=r['y'], dz=round(dz, 1)))
    for comp in dirty:
        if hasattr(comp, 'build_tree_if_outdated'):
            comp.build_tree_if_outdated(False, True)
    assert u.EditorLoadingAndSavingUtils.save_map(world, MAP), 'save failed'
    with open(os.path.join(os.path.dirname(OUT), 'ground_fix_moved.json'), 'w') as f:
        json.dump(moved, f, indent=0)
u.log('GROUND candidates=%d moved=%d fix=%s' % (len(fixes), len(moved), FIX))
with open(OUT, 'w') as f:
    json.dump(rows, f)
u.log('GROUND audit rows=%d -> %s' % (len(rows), OUT))
