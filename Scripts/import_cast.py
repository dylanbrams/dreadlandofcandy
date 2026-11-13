"""R6: imports the cast (Meshy rigs, UE5 mannequin skeleton) and their English battle cries.
Run headless:  UnrealEditor-Cmd.exe <project> -run=pythonscript -script=<this file>
  ImportSource/Meshy/SK_<Who>_Rig/SK_<Who>_Rig.fbx      -> /Game/Art/Cast/<Who>/SK_<Who>_Rig (own skeleton)
  ImportSource/Meshy/SK_<Who>_Rig/A_SK_<Who>_Rig_<A>.fbx -> /Game/Art/Cast/<Who>/Anims/A_SK_<Who>_Rig_<A>
  vocals/output/attacks/en/VO_<K01>_en_epic.wav          -> /Game/Audio/VO/Cast/VO_Cry_<K01>
The knight's SwordRaise gets the wrist correction (correct_sword_raise.py), as the old hero's did.
Set JAM_CAST=Knight,Mage to import only some of them; JAM_CAST_NO_AUDIO=1 skips the cries."""
import os
import re
import shutil
import sys
import unreal

PROJECT = unreal.Paths.convert_relative_path_to_full(unreal.Paths.project_dir())
CAST = [c for c in os.environ.get("JAM_CAST", "Knight,Mage,Archer,Barbarian").split(",") if c]
ANIMS = ["Idle", "Walk", "BasicWalk", "SwordRaise", "Scrub", "Sigh", "ProudStrut", "OneHandUp"]


def run(task):
    unreal.AssetToolsHelpers.get_asset_tools().import_asset_tasks([task])
    return list(task.get_editor_property("imported_object_paths"))


def make_task(path, dest, options=None):
    t = unreal.AssetImportTask()
    t.set_editor_property("filename", path)
    t.set_editor_property("destination_path", dest)
    t.set_editor_property("automated", True)
    t.set_editor_property("replace_existing", True)
    t.set_editor_property("save", True)
    if options:
        t.set_editor_property("options", options)
    return t


def import_rig(who):
    src = os.path.join(PROJECT, "ImportSource", "Meshy", f"SK_{who}_Rig")
    dest = f"/Game/Art/Cast/{who}"
    ui = unreal.FbxImportUI()
    ui.set_editor_property("import_mesh", True)
    ui.set_editor_property("import_as_skeletal", True)
    ui.set_editor_property("import_animations", False)
    ui.set_editor_property("import_materials", True)
    ui.set_editor_property("import_textures", True)
    ui.set_editor_property("mesh_type_to_import", unreal.FBXImportType.FBXIT_SKELETAL_MESH)
    unreal.log(f"JamCastImport: {who} mesh -> {run(make_task(os.path.join(src, f'SK_{who}_Rig.fbx'), dest, ui))}")

    reg = unreal.AssetRegistryHelpers.get_asset_registry()
    skeletons = [d for d in reg.get_assets_by_path(dest, recursive=False) if str(d.asset_class_path.asset_name) == "Skeleton"]
    skeleton = unreal.load_asset(str(skeletons[0].package_name)) if skeletons else None
    if not skeleton:
        raise RuntimeError(f"JamCastImport: no skeleton for {who}")

    for anim in ANIMS:
        path = os.path.join(src, f"A_SK_{who}_Rig_{anim}.fbx")
        if not os.path.exists(path):
            continue
        ui = unreal.FbxImportUI()
        ui.set_editor_property("import_mesh", False)
        ui.set_editor_property("import_animations", True)
        ui.set_editor_property("import_materials", False)
        ui.set_editor_property("import_textures", False)
        ui.set_editor_property("skeleton", skeleton)
        ui.set_editor_property("mesh_type_to_import", unreal.FBXImportType.FBXIT_ANIMATION)
        unreal.log(f"JamCastImport: {who} {anim} -> {run(make_task(path, dest + '/Anims', ui))}")

    if who == "Knight":
        sys.path.insert(0, os.path.join(PROJECT, "Scripts"))
        from correct_sword_raise import correct
        correct(f"{dest}/Anims/A_SK_Knight_Rig_SwordRaise", apply=True, refresh_source=True)


def import_cries():
    src = os.path.join(PROJECT, "vocals", "output", "attacks", "en")
    stage = os.path.join(PROJECT, "Saved", "AudioStage", "Cast")
    os.makedirs(stage, exist_ok=True)
    pattern = re.compile(r"^VO_([KMAB]\d\d)_en_epic\.wav$")
    tasks = []
    for name in sorted(os.listdir(src)):
        m = pattern.match(name)
        if not m:
            continue
        staged = os.path.join(stage, f"VO_Cry_{m.group(1)}.wav")
        shutil.copyfile(os.path.join(src, name), staged)
        tasks.append(make_task(staged, "/Game/Audio/VO/Cast"))
    unreal.AssetToolsHelpers.get_asset_tools().import_asset_tasks(tasks)
    done = sum(len(list(t.get_editor_property("imported_object_paths"))) for t in tasks)
    unreal.log(f"JamCastImport: cries {done} of {len(tasks)}")


for who in CAST:
    import_rig(who)
if os.environ.get("JAM_CAST_NO_AUDIO") != "1":
    import_cries()
unreal.log("JamCastImport: done")
