"""Validate imported geometry and prepare camera-only copies of the terrain map.

Run via review_entry.py in a full editor session after build_level.py. Never changes gameplay
code or the main map's GameMode. Writes only /Game/World/Review and this folder.
"""
import json
import math
from pathlib import Path
import unreal as u

ROOT = Path(u.Paths.convert_relative_path_to_full(u.Paths.project_dir())) / "ImportSource/Terrain"
DATA = json.loads((ROOT / "world_manifest.json").read_text(encoding="utf-8"))
ACTORS = u.get_default_object(u.EditorActorSubsystem)
world = globals().get("terrain_review_world") or u.EditorLoadingAndSavingUtils.load_map("/Game/World/L_Sugarborough")
if not world:
    raise RuntimeError("Terrain map did not load")

checks = []
for tx in range(4):
    for ty in range(4):
        name = f"SM_Terrain_{tx}_{ty}"
        mesh = u.load_asset("/Game/World/Meshes/" + name)
        if mesh.get_num_sections(0) < 1:
            raise RuntimeError("Terrain render mesh not ready: " + name)
        bounds = mesh.get_bounds()
        checks.append(dict(mesh=name, origin=[bounds.origin.x, bounds.origin.y, bounds.origin.z], extent=[bounds.box_extent.x, bounds.box_extent.y, bounds.box_extent.z]))
        # FBX conversion must preserve exact world XY, orientation and centimetres.
        expected = (-12000 + tx * 8000, -12000 + ty * 8000)
        if abs(bounds.origin.x - expected[0]) > 2 or abs(bounds.origin.y - expected[1]) > 2:
            raise RuntimeError(f"Terrain axis mismatch {name}: {bounds.origin}, expected {expected}")
        if abs(bounds.box_extent.x - 4000) > 2 or abs(bounds.box_extent.y - 4000) > 2:
            raise RuntimeError(f"Terrain scale mismatch {name}: {bounds.box_extent}")

actors = list(ACTORS.get_all_level_actors())
terrains = [a for a in actors if isinstance(a, u.StaticMeshActor) and a.static_mesh_component.static_mesh and a.static_mesh_component.static_mesh.get_name().startswith("SM_Terrain")]
assert len(terrains) == 16
for actor in terrains:
    assert actor.static_mesh_component.get_collision_profile_name() == "BlockAll"
scatter = [a for a in actors if a.get_actor_label().startswith("Scatter_")]
assert len(scatter) == 520
for actor in scatter:
    assert actor.static_mesh_component.get_collision_enabled() == u.CollisionEnabled.NO_COLLISION

ground_hits = []
for row in DATA["locations"]:
    offsets = [(0, 0)]
    if row["kind"] in ("Camp", "FilmSet"):
        r = row["radius"]
        offsets += [(r, 0), (-r, 0), (0, r), (0, -r)]
        if row["kind"] == "FilmSet":
            offsets += [(r, r), (-r, r), (r, -r), (-r, -r)]
    for dx, dy in offsets:
        x, y = row["x"] + dx + 3.14, row["y"] + dy + 2.72
        hit = u.SystemLibrary.line_trace_single(world, u.Vector(x, y, 6000), u.Vector(x, y, -2000), u.TraceTypeQuery.TRACE_TYPE_QUERY1, True, [], u.DrawDebugTrace.NONE)
        if hit is None:
            raise RuntimeError("No terrain collision under " + row["id"])
        parts = hit.to_tuple()
        actor = parts[9]
        normal = parts[7]
        slope = math.degrees(math.acos(max(-1, min(1, normal.z))))
        if not isinstance(actor, u.StaticMeshActor) or not actor.static_mesh_component.static_mesh.get_name().startswith("SM_Terrain"):
            raise RuntimeError("Blocked reserved location: " + row["id"] + " by " + str(actor))
        if slope >= 15:
            raise RuntimeError("Steep reserved location: " + row["id"] + " " + str(slope))
        ground_hits.append(dict(id=row["id"], x=x, y=y, z=parts[5].z, slope=slope))

# A hero-sized capsule can traverse each main approach beyond the village plaza.
# The well and village houses intentionally remain solid objects to walk around.
road_checks = []
for biome, yaw, distance in [("Meadow", 0, 6000), ("Molars", 120, 6500), ("Orchard", -120, 6000)]:
    angle = math.radians(yaw)
    start = u.Vector(1000 * math.cos(angle), 1000 * math.sin(angle), 100)
    end = u.Vector(distance * math.cos(angle), distance * math.sin(angle), 100)
    hit = u.SystemLibrary.capsule_trace_single(world, start, end, 34, 88, u.TraceTypeQuery.TRACE_TYPE_QUERY1, False, [], u.DrawDebugTrace.NONE)
    if hit is not None:
        raise RuntimeError("Hero approach blocked in " + biome + ": " + str(hit.to_tuple()))
    road_checks.append(biome)

# The visible mountain range has a continuous collision backup at every side.
boundary_checks = []
for side in range(4):
    for along in (-9000, 0, 9000):
        start, end = [(u.Vector(11000, along, 1200), u.Vector(12500, along, 1200)),
                      (u.Vector(-11000, along, 1200), u.Vector(-12500, along, 1200)),
                      (u.Vector(along, 11000, 1200), u.Vector(along, 12500, 1200)),
                      (u.Vector(along, -11000, 1200), u.Vector(along, -12500, 1200))][side]
        hit = u.SystemLibrary.line_trace_single(world, start, end, u.TraceTypeQuery.TRACE_TYPE_QUERY1, False, [], u.DrawDebugTrace.NONE)
        if hit is None:
            raise RuntimeError(f"Missing perimeter collision: side {side}, offset {along}")
        boundary_checks.append(dict(side=side, offset=along))
world.get_world_settings().set_editor_property("default_game_mode", u.GameModeBase)
cameras = [a for a in actors if isinstance(a, u.CameraActor)]
reviews = []
for name, location, yaw, pitch, fov in [
    ("Overview", (-21500, -21500, 25000), 45, -39, 65),
    ("Village", (-3500, -3400, 2100), 44, -25, 65),
    ("Meadow", (1700, -150, 250), 0, -4, 80),
    ("Fort", (-1350, 2250, 850), 120, -5, 75),
]:
    for cam in cameras:
        cam.set_editor_property("auto_activate_for_player", u.AutoReceiveInput.DISABLED)
    cam = cameras[0]
    cam.set_actor_location(u.Vector(*location), False, False)
    cam.set_actor_rotation(u.Rotator(pitch=pitch, yaw=yaw), False)
    cam.camera_component.set_field_of_view(fov)
    cam.set_editor_property("auto_activate_for_player", u.AutoReceiveInput.PLAYER0)
    path = "/Game/World/Review/L_Terrain" + name
    if not u.EditorLoadingAndSavingUtils.save_map(world, path):
        raise RuntimeError("Could not save " + path)
    reviews.append(path)

report = dict(terrain_bounds=checks, terrain_count=len(terrains), nonblocking_scatter=len(scatter), ground_collision_samples=ground_hits, traversable_approaches=road_checks, boundary_collision_samples=boundary_checks, review_maps=reviews)
(ROOT / "engine_validation.json").write_text(json.dumps(report, indent=2) + "\n", encoding="utf-8")
u.log("TERRAIN ENGINE GEOMETRY PASS " + json.dumps(report))
