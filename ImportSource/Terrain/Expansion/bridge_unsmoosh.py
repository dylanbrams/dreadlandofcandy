"""Un-smoosh the Riverlands bridge (Nov 15). Commandlet use; JAM_BRIDGE_APPLY=1 to apply and save.

SM_GeneratedBridge126 was generated with its span ~35 degrees off the mesh X axis, and fixes4_terrain.py scaled it
per axis (20 m x 9 m x 3.2 m) to fill an axis-aligned box: a non-uniform scale on a diagonal mesh shears it. This
measures the span axis from the mesh's own vertices, yaws the bridge so the span crosses the river along world X,
scales it uniformly, rests its ends on the bank lips, and rebuilds the hidden walk deck as ramps along the arch.
"""
import unreal as u
import math, os

APPLY = os.environ.get('JAM_BRIDGE_APPLY') == '1'
SPAN = 1650.0        # world span (the river channel is ~1440 between its edge blockers)
BANK = 75.0          # bank lip height the deck ends rest on (fixes4_terrain DECK)
SEGMENTS = 10

def log(*a): u.log('BRIDGE_FIX ' + ' '.join(str(x) for x in a))

u.EditorLoadingAndSavingUtils.load_map('/Game/World/L_Sugarborough')
actors = u.get_editor_subsystem(u.EditorActorSubsystem)
level = actors.get_all_level_actors()
bridge = [a for a in level if a.get_actor_label() == 'Riverlands_PeppermintBridge'][0]
mesh = bridge.static_mesh_component.static_mesh
desc = mesh.get_static_mesh_description(0)
pts = []
for i in range(desc.get_vertex_count()):
    vid = u.VertexID(i)
    if desc.is_vertex_valid(vid): pts.append(desc.get_vertex_position(vid))
zmin = min(p.z for p in pts); zmax = max(p.z for p in pts); H0 = zmax - zmin
top = [p for p in pts if p.z > zmin + 0.85 * H0]   # post tops line the two rails, along the span
mx = sum(p.x for p in top) / len(top); my = sum(p.y for p in top) / len(top)
sxx = sum((p.x - mx) ** 2 for p in top); syy = sum((p.y - my) ** 2 for p in top); sxy = sum((p.x - mx) * (p.y - my) for p in top)
ang = 0.5 * math.atan2(2 * sxy, sxx - syy)
ca, sa = math.cos(ang), math.sin(ang)
U = [(p.x * ca + p.y * sa, -p.x * sa + p.y * ca, p.z) for p in pts]
lu = min(q[0] for q in U); hu = max(q[0] for q in U); lv = min(q[1] for q in U); hv = max(q[1] for q in U)
cu, cv = (lu + hu) / 2, (lv + hv) / 2
S0 = hu - lu
s = SPAN / S0
log('vertices', len(pts), 'span angle deg', round(math.degrees(ang), 2), 'mesh span/width/height', round(S0, 2), round(hv - lv, 2), round(H0, 2), 'uniform scale', round(s, 3))
prof = []
for k in range(SEGMENTS + 1):
    a = lu + S0 * k / SEGMENTS
    near = [q[2] for q in U if abs(q[0] - a) <= S0 * 0.03 and abs(q[1] - cv) <= (hv - lv) * 0.15]
    prof.append(max(near) if near else None)
for k in range(len(prof)):   # fill gaps from neighbours
    if prof[k] is None: prof[k] = next(p for p in prof[k:] + prof[::-1] if p is not None)
rail = min(abs(min(q[1] for q in U if abs(q[0] - cu) <= S0 * 0.05 and q[2] > zmin + 0.7 * H0) - cv),
           abs(max(q[1] for q in U if abs(q[0] - cu) <= S0 * 0.05 and q[2] > zmin + 0.7 * H0) - cv))
log('deck profile (mesh z)', [round(p, 1) for p in prof], 'rail inner half-width', round(rail, 2))

old = bridge.get_actor_location()
BX, BY = old.x, old.y
yaw = -math.degrees(ang)   # mesh span axis -> world X
end_z = min(prof[0], prof[-1])
base_z = BANK - (end_z - zmin) * s - zmin * s
# The mesh pivot is not the span centre: offset so the span centre lands on (BX, BY).
off = u.Vector(cu * s, cv * s, 0)
ry = math.radians(yaw)   # rotate (cu, cv) from the aligned frame back into the mesh frame, then by the actor yaw = identity
loc = u.Vector(BX - (cu * s), BY - (cv * s), base_z)
log('old loc', old, 'old scale', bridge.get_actor_scale3d(), 'old rot', bridge.get_actor_rotation())
log('new yaw', round(yaw, 2), 'scale', round(s, 3), 'loc', loc, 'deck ends z', round(BANK, 1), 'deck top z', round(base_z + (max(prof) - zmin) * s + zmin * s, 1))

if APPLY:
    bridge.set_actor_rotation(u.Rotator(roll=0, pitch=0, yaw=yaw), False)
    bridge.set_actor_scale3d(u.Vector(s, s, s))
    bridge.set_actor_location(loc, False, False)
    cube = u.load_asset('/Engine/BasicShapes/Cube')
    def hidden_block(name, x, y, z, sx, sy, sz, pitch=0.0):
        a = actors.spawn_actor_from_class(u.StaticMeshActor, u.Vector(x, y, z), u.Rotator(roll=0, pitch=pitch, yaw=0))
        a.set_actor_label('Riverlands_Fixes4_' + name); a.set_folder_path('Sugarborough/Riverlands/Fixes4')
        c = a.static_mesh_component; c.set_static_mesh(cube)
        a.set_actor_scale3d(u.Vector(sx / 100, sy / 100, sz / 100)); a.set_actor_hidden_in_game(True)
        c.set_collision_enabled(u.CollisionEnabled.QUERY_ONLY)
        c.set_collision_response_to_all_channels(u.CollisionResponseType.ECR_IGNORE)
        c.set_collision_response_to_channel(u.CollisionChannel.ECC_PAWN, u.CollisionResponseType.ECR_BLOCK)
        return a
    for a in actors.get_all_level_actors():
        if a.get_actor_label().startswith('Riverlands_Fixes4_Bridge'): actors.destroy_actor(a)
    def deck_z(k): return base_z + prof[k] * s
    width = 2 * rail * s - 40
    x0 = BX - SPAN / 2
    for k in range(SEGMENTS):
        xa = x0 + SPAN * k / SEGMENTS; xb = x0 + SPAN * (k + 1) / SEGMENTS
        za, zb = deck_z(k), deck_z(k + 1)
        length = math.hypot(xb - xa, zb - za) + 12
        pitch = math.degrees(math.atan2(zb - za, xb - xa))
        hidden_block('BridgeDeck%d' % k, (xa + xb) / 2, BY, (za + zb) / 2 - 20, length, width, 40, pitch)
    top_z = base_z + max(prof) * s
    for side in (-1, 1):
        hidden_block('BridgeRail' + str(side), BX, BY + side * (rail * s + 15), (BANK + top_z) / 2 + 100, SPAN, 30, top_z - BANK + 250)
    u.EditorLoadingAndSavingUtils.save_dirty_packages(True, False)
    log('applied and saved')
