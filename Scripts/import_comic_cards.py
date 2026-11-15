"""Fixes5 #1: import the multilingual comic action cards (12 + 24 angular + 135 expansion shards) as world effect
textures (with mips). Expansion names swap '-' for '_' (es-419 -> es_419) to keep asset names plain.
Run through Unreal's Python commandlet:
  UnrealEditor-Cmd.exe <uproject> -run=pythonscript -script=Scripts/import_comic_cards.py -unattended -nullrhi
"""
from pathlib import Path
import unreal

root = Path(unreal.Paths.project_dir()).resolve()
destination = "/Game/Art/UI/ComicCards"
sources = sorted((root / "Assets/Incoming/2D/ComicCards").glob("T_Comic_*.png"))
sources += sorted((root / "Assets/Incoming/2D/ComicCards/Angular").glob("T_Comic_*.png"))
sources += sorted((root / "Assets/Incoming/2D/ComicCards/Expansion").glob("T_Comic_*.png"))
assert len(sources) == 171, f"expected 171 comic cards, found {len(sources)}"
tools = unreal.AssetToolsHelpers.get_asset_tools()
for source in sources:
    name = source.stem.replace("-", "_")
    task = unreal.AssetImportTask()
    for key, value in {
        "filename": str(source), "destination_path": destination,
        "destination_name": name, "automated": True,
        "replace_existing": True, "save": True,
    }.items():
        task.set_editor_property(key, value)
    tools.import_asset_tasks([task])
    texture = unreal.load_asset(f"{destination}/{name}")
    assert isinstance(texture, unreal.Texture2D), task.get_editor_property("imported_object_paths")
    texture.set_editor_property("lod_group", unreal.TextureGroup.TEXTUREGROUP_EFFECTS)
    texture.set_editor_property("mip_gen_settings", unreal.TextureMipGenSettings.TMGS_FROM_TEXTURE_GROUP)
    texture.set_editor_property("max_texture_size", 512)
    texture.set_editor_property("srgb", True)
    texture.set_editor_property("address_x", unreal.TextureAddress.TA_CLAMP)
    texture.set_editor_property("address_y", unreal.TextureAddress.TA_CLAMP)
    unreal.EditorAssetLibrary.save_loaded_asset(texture)
    unreal.log("ComicCards: imported " + texture.get_path_name())
unreal.log(f"ComicCards: SUCCESS {len(sources)} textures")
