"""Include the terrain maps in normal packaging through an owned cook label."""
from pathlib import Path
import json
import unreal as u

label_path = "/Game/World/PAL_Sugarborough"
tools = u.AssetToolsHelpers.get_asset_tools()
if u.EditorAssetLibrary.does_asset_exist(label_path):
    label = u.load_asset(label_path)
else:
    factory = u.DataAssetFactory()
    factory.set_editor_property("data_asset_class", u.PrimaryAssetLabel)
    label = tools.create_asset("PAL_Sugarborough", "/Game/World", u.PrimaryAssetLabel, factory)
if not label:
    raise RuntimeError("Could not create terrain cook label")
rules = label.get_editor_property("rules")
rules.set_editor_property("cook_rule", u.PrimaryAssetCookRule.ALWAYS_COOK)
rules.set_editor_property("priority", 1)
rules.set_editor_property("apply_recursively", True)
label.set_editor_property("rules", rules)
label.set_editor_property("label_assets_in_my_directory", True)
label.set_editor_property("is_runtime_label", True)
maps = ["/Game/World/L_Sugarborough"] + ["/Game/World/Review/L_Terrain" + s for s in ("Overview", "Village", "Meadow", "Fort")]
maps += ["/Game/World/Review/L_" + s for s in ("DenseMeadow", "DenseSnow", "CandyWander") if u.EditorAssetLibrary.does_asset_exist("/Game/World/Review/L_" + s)]
project_root = Path(u.Paths.convert_relative_path_to_full(u.Paths.project_dir()))
review_index = project_root / "ImportSource/Terrain/Dense/review_maps.json"
if review_index.exists():
    maps += list(json.loads(review_index.read_text()).values())
worlds = [u.load_asset(path) for path in maps]
if not all(worlds):
    raise RuntimeError("Terrain cook label references a missing map")
label.set_editor_property("explicit_assets", worlds)
u.EditorAssetLibrary.save_loaded_asset(label)
root = Path(u.Paths.convert_relative_path_to_full(u.Paths.project_dir())) / "ImportSource/Terrain"
(root / "cook_label.json").write_text(json.dumps({"asset": label_path, "rule": "AlwaysCook", "explicit_maps": maps}, indent=2) + "\n", encoding="utf-8")
u.log("TERRAIN COOK LABEL PASS " + label_path)
