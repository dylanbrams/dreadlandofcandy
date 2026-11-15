"""Import generated NPC skeletal meshes and their locomotion clips into Unreal.

Run headless with UnrealEditor-Cmd.exe <project> -run=pythonscript -script=<this file>.
Sources are expected at ImportSource/Meshy/SK_<Who>_Rig/:
  SK_<Who>_Rig.fbx
  A_SK_<Who>_Rig_{Idle,Walk,BasicWalk}.fbx
Meshes and their own skeletons go to /Game/Art/NPC/<Who>; clips go to
/Game/Art/NPC/<Who>/Anims. Missing rigs are skipped. Any present rig must have
both Idle and Walk clips so imported rigs are immediately usable.
"""
import os

import unreal


PROJECT = unreal.Paths.convert_relative_path_to_full(unreal.Paths.project_dir())
NPCS = ("Villager", "Goblin", "Rat")
CLIPS = ("Idle", "Walk", "BasicWalk")
REQUIRED_CLIPS = ("Idle", "Walk")


def make_task(filename, destination, options):
    task = unreal.AssetImportTask()
    task.set_editor_property("filename", filename)
    task.set_editor_property("destination_path", destination)
    task.set_editor_property("automated", True)
    task.set_editor_property("replace_existing", True)
    task.set_editor_property("save", True)
    task.set_editor_property("options", options)
    return task


def import_task(task):
    unreal.AssetToolsHelpers.get_asset_tools().import_asset_tasks([task])
    return list(task.get_editor_property("imported_object_paths"))


def import_npc(who):
    source_dir = os.path.join(PROJECT, "ImportSource", "Meshy", f"SK_{who}_Rig")
    mesh_file = os.path.join(source_dir, f"SK_{who}_Rig.fbx")
    if not os.path.isfile(mesh_file):
        unreal.log(f"JamNpcImport: SK_{who}_Rig.fbx not found; skipped {who}")
        return "skipped"

    clip_files = {
        clip: os.path.join(source_dir, f"A_SK_{who}_Rig_{clip}.fbx")
        for clip in CLIPS
    }
    missing = [clip for clip in REQUIRED_CLIPS if not os.path.isfile(clip_files[clip])]
    if missing:
        raise RuntimeError(
            f"JamNpcImport: {who} mesh exists, but required animation FBX file(s) "
            f"are missing: {', '.join(missing)}"
        )

    destination = f"/Game/Art/NPC/{who}"
    mesh_options = unreal.FbxImportUI()
    mesh_options.set_editor_property("import_mesh", True)
    mesh_options.set_editor_property("import_as_skeletal", True)
    mesh_options.set_editor_property("import_animations", False)
    mesh_options.set_editor_property("import_materials", True)
    mesh_options.set_editor_property("import_textures", True)
    mesh_options.set_editor_property("mesh_type_to_import", unreal.FBXImportType.FBXIT_SKELETAL_MESH)
    mesh_paths = import_task(make_task(mesh_file, destination, mesh_options))
    unreal.log(f"JamNpcImport: {who} mesh -> {mesh_paths}")

    mesh = unreal.load_asset(f"{destination}/SK_{who}_Rig")
    if not mesh or not isinstance(mesh, unreal.SkeletalMesh):
        raise RuntimeError(f"JamNpcImport: imported skeletal mesh missing for {who} at {destination}")
    skeleton = mesh.get_editor_property("skeleton")
    if not skeleton:
        raise RuntimeError(f"JamNpcImport: imported mesh has no own skeleton for {who}")

    imported_clips = []
    for clip in CLIPS:
        filename = clip_files[clip]
        if not os.path.isfile(filename):
            continue
        animation_options = unreal.FbxImportUI()
        animation_options.set_editor_property("import_mesh", False)
        animation_options.set_editor_property("import_animations", True)
        animation_options.set_editor_property("import_materials", False)
        animation_options.set_editor_property("import_textures", False)
        animation_options.set_editor_property("skeleton", skeleton)
        animation_options.set_editor_property("mesh_type_to_import", unreal.FBXImportType.FBXIT_ANIMATION)
        paths = import_task(make_task(filename, destination + "/Anims", animation_options))
        if not paths:
            raise RuntimeError(f"JamNpcImport: {who} {clip} animation import produced no assets")
        animation = unreal.load_asset(f"{destination}/Anims/A_SK_{who}_Rig_{clip}")
        if not isinstance(animation, unreal.AnimSequence) or animation.get_editor_property("skeleton") != skeleton:
            raise RuntimeError(f"JamNpcImport: {who} {clip} is missing or has a different skeleton")
        # Gameplay steers the capsule. Keep the animation's root from walking away from it.
        animation.set_editor_property("force_root_lock", True)
        animation.set_editor_property("enable_root_motion", False)
        unreal.EditorAssetLibrary.save_loaded_asset(animation)
        imported_clips.append(clip)
        unreal.log(f"JamNpcImport: {who} {clip} -> {paths}")

    # Importing with this mesh's skeleton enforces compatibility for both required clips.
    if not all(clip in imported_clips for clip in REQUIRED_CLIPS):
        raise RuntimeError(f"JamNpcImport: {who} did not import both required Idle and Walk clips")
    for asset in unreal.AssetRegistryHelpers.get_asset_registry().get_assets_by_path(destination, recursive=True):
        if str(asset.asset_class_path.asset_name) == "Texture2D":
            texture = asset.get_asset()
            texture.set_editor_property("max_texture_size", 1024)
            unreal.EditorAssetLibrary.save_loaded_asset(texture)
    return "imported"


def main():
    imported = []
    skipped = []
    failures = []
    for who in NPCS:
        try:
            result = import_npc(who)
            (imported if result == "imported" else skipped).append(who)
        except Exception as error:
            failures.append((who, str(error)))
            unreal.log_error(f"JamNpcImport: {who} FAILED: {error}")

    unreal.log(
        f"JamNpcImport: summary imported={imported} skipped={skipped} "
        f"failed={[who for who, _ in failures]}"
    )
    if failures:
        raise RuntimeError("JamNpcImport: one or more NPC rigs failed; see per-rig errors above")


main()
