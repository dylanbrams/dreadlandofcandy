"""Post-GO import: the 25 Fenella pickups and Dylan-approved Meshy shield only.
Run via UnrealEditor-Cmd -run=pythonscript -script=<this file>.
"""
from pathlib import Path
import json
import unreal as u

ROOT = Path(u.Paths.convert_relative_path_to_full(u.Paths.project_dir()))
manifest = json.loads((ROOT / "vocals/output/fenella_pickups_2026-11-14/manifest.json").read_text(encoding="utf-8"))
clips = manifest["clips"]
assert len(clips) == 25
tasks = []
for line_id in clips:
    source = ROOT / "vocals/output/fenella" / ("VO_" + line_id + ".wav")
    assert source.is_file(), line_id
    task = u.AssetImportTask()
    task.set_editor_properties(dict(filename=str(source), destination_path="/Game/Audio/VO",
        destination_name="VO_" + line_id, automated=True, replace_existing=True, save=True))
    tasks.append(task)
u.AssetToolsHelpers.get_asset_tools().import_asset_tasks(tasks)
report = {"audio": [], "audio_durations": {}, "shield": {}}
for line_id in clips:
    asset = u.load_asset("/Game/Audio/VO/VO_" + line_id)
    assert isinstance(asset, u.SoundWave), line_id
    duration = float(asset.get_editor_property("duration"))
    assert 0 < duration < 30, (line_id, duration)
    report["audio"].append(str(asset.get_path_name()))
    report["audio_durations"][line_id] = duration
u.log("Fixes2Import: VO PASS count=" + str(len(report["audio"])))

dest = "/Game/Art/Props/Shield"
task = u.AssetImportTask()
task.set_editor_properties(dict(filename=str(ROOT / "ImportSource/Meshy/SM_KnightShield.glb"),
    destination_path=dest, destination_name="SM_KnightShield", automated=True, replace_existing=True, save=True))
u.AssetToolsHelpers.get_asset_tools().import_asset_tasks([task])
registry = u.AssetRegistryHelpers.get_asset_registry()
registry.scan_paths_synchronous([dest], force_rescan=True)
assets = registry.get_assets_by_path(dest, recursive=True)
meshes = [a.get_asset() for a in assets if str(a.asset_class_path.asset_name) == "StaticMesh"]
assert len(meshes) == 1, [str(a.package_name) for a in assets]
mesh = meshes[0]
target = dest + "/SM_KnightShield"
if str(mesh.get_path_name()).split(".")[0] != target:
    assert u.EditorAssetLibrary.rename_asset(str(mesh.get_path_name()), target)
    mesh = u.load_asset(target)
box = mesh.get_bounding_box()
size = box.max - box.min
assert max(size.x, size.y, size.z) > 0
for data in assets:
    asset = data.get_asset()
    if isinstance(asset, u.Texture2D):
        asset.set_editor_property("max_texture_size", 2048)
        u.EditorAssetLibrary.save_loaded_asset(asset)
u.EditorAssetLibrary.save_loaded_asset(mesh)
report["shield"] = {"asset": str(mesh.get_path_name()), "bounds_cm": [size.x, size.y, size.z],
    "min": [box.min.x, box.min.y, box.min.z], "max": [box.max.x, box.max.y, box.max.z],
    "materials": len(mesh.static_materials)}
(ROOT / "ImportSource/Meshy/fixes2_import_report.json").write_text(json.dumps(report, indent=2) + "\n", encoding="utf-8")
u.log("Fixes2Import: SHIELD PASS " + json.dumps(report["shield"]))
