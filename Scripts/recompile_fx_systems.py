"""Force a clean compile of every generated Niagara attack system, then save it.

Run in a FRESH Unreal Python commandlet, after build_niagara_attacks.py has exited:

    UnrealEditor-Cmd.exe <project> -run=pythonscript -script=<this file>
        -dpcvars=fx.ForceCompileOnLoad=1 -unattended -nullrhi -nosound -nop4

Why: the builder saves each system with compiled scripts that predate its
SetParameters modules, yet Niagara reports them UpToDate, so a later editor
session runs the stale bytecode (particles at origin, scale 1, white) without
recompiling. fx.ForceCompileOnLoad makes PostLoad invalidate the graph compile
ids, which forces a real compile; UNiagaraSystem::PreSave waits for it.
The cvar only acts on load, so no system may be in memory before it is set.
Optional one-time migration: add -fxNormalizeSpriteFacing to set existing sprite
renderers to FaceCamera / Unaligned. New assets already use these modes.
"""
import json
from pathlib import Path
import unreal

ROOT = Path(unreal.Paths.convert_relative_path_to_full(unreal.Paths.project_dir()))
API = unreal.get_default_object(unreal.NiagaraToolset_System)


def main():
    normalize_sprites = "-fxNormalizeSpriteFacing" in unreal.SystemLibrary.get_command_line()
    unreal.SystemLibrary.execute_console_command(None, "fx.ForceCompileOnLoad 1")
    records = json.loads((ROOT / "ImportSource/FX/niagara_assets.json").read_text(encoding="utf-8"))
    filter_file = ROOT / "Saved/FX/build_filter.txt"
    selected = filter_file.read_text(encoding="utf-8-sig").strip() if filter_file.exists() else ""
    done = 0
    for record in records:
        path = record["asset"]
        if selected and path.split(".")[-1] != selected:
            continue
        if unreal.find_object(None, path):
            raise RuntimeError("Already loaded, so the forced compile cannot apply: " + path)
        system = unreal.load_asset(path)
        if not system:
            raise RuntimeError("Missing system: " + path)
        if normalize_sprites:
            summary = API.call_method("GetSystemSummary", (system,))
            for emitter in summary.get_editor_property("Emitters"):
                ref = unreal.NiagaraExt_StackItemReference()
                ref.set_editor_properties({"System": system, "EmitterName": emitter.get_editor_property("EmitterName")})
                topology = API.call_method("GetEmitterTopology", (ref,))
                for renderer in topology.get_editor_property("Renderers"):
                    if renderer.get_editor_property("RendererClass").get_name() == "NiagaraSpriteRendererProperties":
                        ref.set_editor_property("RendererIndex", renderer.get_editor_property("RendererIndex"))
                        data = unreal.NiagaraExt_RendererData()
                        data.set_editor_property("PropertyValues", json.dumps({"FacingMode":"FaceCamera", "Alignment":"Unaligned"}))
                        API.call_method("SetRendererData", (ref, data))
                        unreal.log("FX SPRITE FACING NORMALIZED: " + path)
        # PreSave blocks until the forced compile has finished.
        if not unreal.EditorAssetLibrary.save_loaded_asset(system, only_if_is_dirty=False):
            raise RuntimeError("Save failed: " + path)
        state = API.call_method("GetSystemCompileState", (system,))
        if not state.is_complete or state.error:
            raise RuntimeError(f"Compile state not settled for {path}: {state.error}")
        status = json.loads(state.get_value_as_json_string() or "{}")
        if status.get("bHasErrors") or status.get("aggregateStatus") != "UpToDate":
            raise RuntimeError(f"Compile failed for {path}: {json.dumps(status)}")
        done += 1
        unreal.log("FX SYSTEM RECOMPILED: " + path)
    unreal.log(f"FX RECOMPILE: {done} systems force-compiled and saved")


if __name__ == "__main__":
    main()
