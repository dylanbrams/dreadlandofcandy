"""Import the pre-generated non-English attack cries without replacing English assets."""
import os
import re
import shutil
import unreal

project = unreal.Paths.convert_relative_path_to_full(unreal.Paths.project_dir())
source = os.path.join(project, "vocals", "output", "attacks")
stage = os.path.join(project, "Saved", "AudioStage", "MultilingualCries")
os.makedirs(stage, exist_ok=True)
languages = ("de", "fr", "ja", "ko", "zh-CN", "pt-BR", "es-ES", "it", "pl", "ru", "tr", "uk", "cs", "th")
tasks = []
for language in languages:
    folder = os.path.join(source, language)
    pattern = re.compile(r"^VO_([KMAB]\d\d)_" + re.escape(language) + r"_epic\.wav$")
    for filename in sorted(os.listdir(folder)):
        match = pattern.match(filename)
        if not match:
            continue
        asset_name = "VO_Cry_" + match.group(1) + "_" + language.replace("-", "_")
        staged = os.path.join(stage, asset_name + ".wav")
        shutil.copyfile(os.path.join(folder, filename), staged)
        task = unreal.AssetImportTask()
        task.set_editor_property("filename", staged)
        task.set_editor_property("destination_path", "/Game/Audio/VO/Cast/International")
        task.set_editor_property("automated", True)
        task.set_editor_property("replace_existing", True)
        task.set_editor_property("save", True)
        tasks.append(task)
if len(tasks) != 154:
    raise RuntimeError("Expected 154 translated cries, found %d" % len(tasks))
unreal.AssetToolsHelpers.get_asset_tools().import_asset_tasks(tasks)
done = sum(bool(task.get_editor_property("imported_object_paths")) for task in tasks)
unreal.log("JamInternationalCries: imported %d/%d" % (done, len(tasks)))
if done != len(tasks):
    raise RuntimeError("Some translated cries failed to import")
