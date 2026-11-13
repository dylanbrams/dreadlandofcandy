"""Save fresh player-height review copies without overwriting loaded maps."""
import importlib.util
import json
from pathlib import Path
import unreal as u

ROOT = Path(u.Paths.convert_relative_path_to_full(u.Paths.project_dir())) / "ImportSource/Terrain"
HERE = ROOT / "Dense"
ACTORS = u.get_default_object(u.EditorActorSubsystem)
TOOLS = u.AssetToolsHelpers.get_asset_tools()
world = u.EditorLoadingAndSavingUtils.load_map("/Game/World/L_Sugarborough")
world.get_world_settings().set_editor_property("default_game_mode", u.GameModeBase)
for actor in ACTORS.get_all_level_actors():
    if isinstance(actor, u.CameraActor):
        actor.set_editor_property("auto_activate_for_player", u.AutoReceiveInput.DISABLED)
camera = ACTORS.spawn_actor_from_class(u.CameraActor, u.Vector(3100, -120, 180))
camera.camera_component.set_field_of_view(80)
camera.camera_component.set_editor_property("constrain_aspect_ratio", False)
camera.set_editor_property("auto_activate_for_player", u.AutoReceiveInput.PLAYER0)
spec = importlib.util.spec_from_file_location("terrain_source", ROOT / "build_sources.py")
t = importlib.util.module_from_spec(spec)
spec.loader.exec_module(t)
maps = {}
for key, label, location, yaw in [
    ("DenseMeadow", "SweetMeadow", (3100, -120, 180), 0),
    ("DenseGrove", "CandyGrove", (4800, -1600, t.height(4800, -1600) + 175), -20),
    ("DenseSnow", "SugarSnow", (-2000, 3464, 180), 120),
]:
    camera.set_actor_location(u.Vector(*location), False, False)
    camera.set_actor_rotation(u.Rotator(pitch=-3, yaw=yaw), False)
    path, name = TOOLS.create_unique_asset_name("/Game/World/Review/L_" + label, "")
    if not u.EditorLoadingAndSavingUtils.save_map(world, path):
        raise RuntimeError("Review copy failed: " + path)
    maps[key] = path
camera.set_editor_property("auto_activate_for_player", u.AutoReceiveInput.DISABLED)
for actor in ACTORS.get_all_level_actors():
    if isinstance(actor, u.PlayerStart):
        actor.set_actor_location(u.Vector(3100, 0, 180), False, False)
        actor.set_actor_rotation(u.Rotator(), False)
path, name = TOOLS.create_unique_asset_name("/Game/World/Review/L_CandyExplorer", "")
if not u.EditorLoadingAndSavingUtils.save_map(world, path):
    raise RuntimeError("Explorer map save failed")
maps["Explore"] = path
(HERE / "review_maps.json").write_text(json.dumps(maps, indent=2) + "\n", encoding="utf-8")
report_path = HERE / "engine_report.json"
if report_path.exists():
    report = json.loads(report_path.read_text())
    report["review_maps"] = list(maps.values())
    report["review_maps_status"] = "saved and current"
    report_path.write_text(json.dumps(report, indent=2) + "\n", encoding="utf-8")
u.log("DENSE CAMERA MAPS PASS " + json.dumps(maps))
