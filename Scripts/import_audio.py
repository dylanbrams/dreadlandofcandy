"""Imports the sound effects and Fenella's English lines (R1).
Run headless:  UnrealEditor-Cmd.exe <project> -run=pythonscript -script=<this file>
  vocals/output/sfx/SFX_<Name>_v01.wav  ->  /Game/Audio/SFX/SFX_<Name>   (the _v01 is dropped; the table names the cue)
  vocals/output/fenella/VO_<LineId>.wav ->  /Game/Audio/VO/VO_<LineId>   (found by convention, BUILD_SPEC 5A)
  music/theme-song.mp3, village_take1/2  ->  /Game/Audio/Music/M_theme_song_take1, M_village_take1/2 (R2, via ffmpeg)
Set JAM_IMPORT_MUSIC_ONLY=1 to import only the music.
Files are staged under Saved/AudioStage with their final names first, because the asset takes the file's name."""
import os
import re
import shutil
import unreal

PROJECT = unreal.Paths.convert_relative_path_to_full(unreal.Paths.project_dir())
STAGE = os.path.join(PROJECT, "Saved", "AudioStage")
SETS = [
    (os.path.join(PROJECT, "vocals", "output", "sfx"), "/Game/Audio/SFX", re.compile(r"^(SFX_.+?)_v\d+\.wav$", re.I)),
    (os.path.join(PROJECT, "vocals", "output", "fenella"), "/Game/Audio/VO", re.compile(r"^(VO_.+)\.wav$", re.I)),
]

import subprocess
import sys

# R2: music added after T18. MP3s go through ffmpeg to WAV (the importer takes WAV). `-music` imports only these.
MUSIC = {"theme-song.mp3": "M_theme_song_take1", "village_take1.mp3": "M_village_take1", "village_take2.mp3": "M_village_take2"}

def music_tasks():
    stage = os.path.join(STAGE, "Music")
    os.makedirs(stage, exist_ok=True)
    out = []
    for src, name in MUSIC.items():
        path = os.path.join(PROJECT, "music", src)
        if not os.path.exists(path):
            unreal.log_warning(f"JamAudioImport: no {path}")
            continue
        wav = os.path.join(stage, name + ".wav")
        subprocess.run(["ffmpeg", "-y", "-loglevel", "error", "-i", path, "-ar", "44100", "-ac", "2", "-sample_fmt", "s16", wav], check=True)
        out.append((wav, "/Game/Audio/Music"))
    return out

ONLY_MUSIC = "-music" in sys.argv or os.environ.get("JAM_IMPORT_MUSIC_ONLY") == "1"
if ONLY_MUSIC:
    SETS = []

tasks = []
for wav, dest in music_tasks():
    t = unreal.AssetImportTask()
    t.set_editor_property("filename", wav)
    t.set_editor_property("destination_path", dest)
    t.set_editor_property("automated", True)
    t.set_editor_property("replace_existing", True)
    t.set_editor_property("save", True)
    tasks.append(t)

for src, dest, pattern in SETS:
    stage = os.path.join(STAGE, dest.rsplit("/", 1)[-1])
    os.makedirs(stage, exist_ok=True)
    for name in sorted(os.listdir(src)):
        m = pattern.match(name)
        if not m:
            continue
        staged = os.path.join(stage, m.group(1) + ".wav")
        shutil.copyfile(os.path.join(src, name), staged)
        t = unreal.AssetImportTask()
        t.set_editor_property("filename", staged)
        t.set_editor_property("destination_path", dest)
        t.set_editor_property("automated", True)
        t.set_editor_property("replace_existing", True)
        t.set_editor_property("save", True)
        tasks.append(t)

unreal.AssetToolsHelpers.get_asset_tools().import_asset_tasks(tasks)
done = sum(len(list(t.get_editor_property("imported_object_paths"))) for t in tasks)
unreal.log(f"JamAudioImport: {done} of {len(tasks)} imported")
