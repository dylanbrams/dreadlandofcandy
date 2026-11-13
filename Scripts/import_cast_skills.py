"""Import and validate skill clips onto the four existing cast skeletons. No mesh reimport."""
import json
from pathlib import Path
import sys
import unreal

ROOT = Path(unreal.Paths.project_dir()).resolve()
sys.path.insert(0, str(ROOT / "Scripts"))
from correct_sword_raise import correct

report = []
for who in ("Knight", "Mage", "Archer", "Barbarian"):
    dest = f"/Game/Art/Cast/{who}"
    mesh = unreal.load_asset(f"{dest}/SK_{who}_Rig")
    assert mesh, who
    skeleton = mesh.get_editor_property("skeleton")
    for label in ("SwordRaise", "FormALine", "Sigh", "LastJudgment", "Scrub"):
        name = f"A_SK_{who}_Rig_{label}"
        path = ROOT / f"ImportSource/Meshy/SK_{who}_Rig/{name}.fbx"
        assert path.is_file(), path
        ui = unreal.FbxImportUI()
        for prop, value in {"import_mesh": False, "import_animations": True,
                            "import_materials": False, "import_textures": False,
                            "skeleton": skeleton,
                            "mesh_type_to_import": unreal.FBXImportType.FBXIT_ANIMATION}.items():
            ui.set_editor_property(prop, value)
        task = unreal.AssetImportTask()
        for prop, value in {"filename": str(path), "destination_path": dest + "/Anims",
                            "automated": True, "replace_existing": True,
                            "save": True, "options": ui}.items():
            task.set_editor_property(prop, value)
        unreal.AssetToolsHelpers.get_asset_tools().import_asset_tasks([task])
        assert task.get_editor_property("imported_object_paths"), name
        asset_path = f"{dest}/Anims/{name}"
        sequence = unreal.load_asset(asset_path)
        assert isinstance(sequence, unreal.AnimSequence), name
        assert sequence.get_editor_property("skeleton") == skeleton, name
        assert sequence.get_play_length() > 0.5, name
        model = sequence.get_editor_property("data_model_interface")
        assert len(model.get_bone_track_names()) >= 20, name
        row = {"character": who, "clip": label, "seconds": sequence.get_play_length(),
               "tracks": len(model.get_bone_track_names()), "skeleton": skeleton.get_path_name()}
        if label == "SwordRaise":
            row["wrist"] = correct(asset_path, apply=True, refresh_source=True)
        report.append(row)
        unreal.log("CastSkills: validated " + json.dumps(row))
out = ROOT / "ImportSource/RigCorrections/cast_skills_validation.json"
out.write_text(json.dumps(report, indent=2), encoding="utf-8")
unreal.log("CastSkills: SUCCESS 20 clips, four skeletons")
