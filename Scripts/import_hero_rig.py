"""Imports the rigged hero (Meshy, UE5 mannequin skeleton) and its animations into /Game/Art/Hero.
Run headless:  UnrealEditor-Cmd.exe <project> -run=pythonscript -script=<this file>
The mesh comes in once; every animation FBX also carries the mesh, so animations import onto the existing skeleton."""
import os
import sys
import unreal

PROJECT = unreal.Paths.convert_relative_path_to_full(unreal.Paths.project_dir())
SRC = os.path.join(PROJECT, "ImportSource", "Meshy", "SK_Hero_Rig")
DEST = "/Game/Art/Hero"
ANIMS = ["Idle", "Walk", "BasicWalk", "SwordRaise", "Scrub", "Sigh", "ProudStrut", "OneHandUp"]

# Interchange imports FBX in 5.8; it converts FbxImportUI options (mesh vs animation, target skeleton) itself.
# No destination_name: forcing one name makes the mesh, skeleton and materials of one file overwrite each other.

def run(task):
    unreal.AssetToolsHelpers.get_asset_tools().import_asset_tasks([task])
    return list(task.get_editor_property("imported_object_paths"))

def make_task(path, dest, options):
    t = unreal.AssetImportTask()
    t.set_editor_property("filename", path)
    t.set_editor_property("destination_path", dest)
    t.set_editor_property("automated", True)
    t.set_editor_property("replace_existing", True)
    t.set_editor_property("save", True)
    t.set_editor_property("options", options)
    return t

# 1. The skeletal mesh (and its skeleton, materials, textures).
mesh_ui = unreal.FbxImportUI()
mesh_ui.set_editor_property("import_mesh", True)
mesh_ui.set_editor_property("import_as_skeletal", True)
mesh_ui.set_editor_property("import_animations", False)
mesh_ui.set_editor_property("import_materials", True)
mesh_ui.set_editor_property("import_textures", True)
mesh_ui.set_editor_property("mesh_type_to_import", unreal.FBXImportType.FBXIT_SKELETAL_MESH)
paths = run(make_task(os.path.join(SRC, "SK_Hero_Rig.fbx"), DEST, mesh_ui))
unreal.log(f"JamHeroImport: mesh -> {paths}")

reg = unreal.AssetRegistryHelpers.get_asset_registry()
skeletons = [d for d in reg.get_assets_by_path(DEST, recursive=True) if str(d.asset_class_path.asset_name) == "Skeleton"]
skeleton = unreal.load_asset(str(skeletons[0].package_name)) if skeletons else None
if not skeleton:
    raise RuntimeError("JamHeroImport: no skeleton after the mesh import")
unreal.log(f"JamHeroImport: skeleton {skeleton.get_path_name()}")

# 2. Each animation onto that skeleton, no mesh.
for anim in ANIMS:
    src = os.path.join(SRC, f"A_SK_Hero_Rig_{anim}.fbx")
    if not os.path.exists(src):
        unreal.log_warning(f"JamHeroImport: missing {src}")
        continue
    ui = unreal.FbxImportUI()
    ui.set_editor_property("import_mesh", False)
    ui.set_editor_property("import_animations", True)
    ui.set_editor_property("import_materials", False)
    ui.set_editor_property("import_textures", False)
    ui.set_editor_property("skeleton", skeleton)
    ui.set_editor_property("mesh_type_to_import", unreal.FBXImportType.FBXIT_ANIMATION)
    paths = run(make_task(src, f"{DEST}/Anims", ui))
    unreal.log(f"JamHeroImport: {anim} -> {paths}")

# The generated raise has a waving wrist. Bake its correction after import so
# reimporting the source FBX does not restore the hand/sword mismatch.
sys.path.insert(0, os.path.join(PROJECT, "Scripts"))
from correct_sword_raise import correct
correct(f"{DEST}/Anims/A_SK_Hero_Rig_SwordRaise", apply=True, refresh_source=True)

unreal.log("JamHeroImport: done")
