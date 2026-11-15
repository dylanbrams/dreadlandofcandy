"""Run through Unreal's Python script runner after the engine handoff. Imports only Black Sugar UI art."""
from pathlib import Path
import unreal

root = Path(unreal.Paths.project_dir()).resolve()
destination = "/Game/Art/UI/BlackSugar"
names = ("T_UI_BlackSugarSkillFrame", "T_UI_BlackSugarCrest", "T_UI_BlackSugarMenuFrame")
sources = [root / f"Assets/Incoming/2D/BlackSugar/{name}.png" for name in names]
assert all(source.is_file() for source in sources), "Black Sugar source artwork is incomplete"
for source in sources:
    name = source.stem
    task = unreal.AssetImportTask()
    for key, value in {
        "filename": str(source), "destination_path": destination,
        "destination_name": name, "automated": True,
        "replace_existing": True, "save": True,
    }.items():
        task.set_editor_property(key, value)
    unreal.AssetToolsHelpers.get_asset_tools().import_asset_tasks([task])
    texture = unreal.load_asset(f"{destination}/{name}")
    assert isinstance(texture, unreal.Texture2D), task.get_editor_property("imported_object_paths")
    texture.set_editor_property("compression_settings", unreal.TextureCompressionSettings.TC_EDITOR_ICON)
    texture.set_editor_property("mip_gen_settings", unreal.TextureMipGenSettings.TMGS_NO_MIPMAPS)
    texture.set_editor_property("srgb", True)
    unreal.EditorAssetLibrary.save_loaded_asset(texture)
    unreal.log("BlackSugarUI: imported " + texture.get_path_name())
unreal.log("BlackSugarUI: SUCCESS 3 textures")
