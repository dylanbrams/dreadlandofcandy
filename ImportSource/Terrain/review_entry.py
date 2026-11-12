"""Run review_level in a full editor session so physics queries are available."""
from pathlib import Path
from datetime import datetime, timezone
import json
import time
import traceback
import unreal

root = Path(unreal.Paths.convert_relative_path_to_full(unreal.Paths.project_dir())) / "ImportSource/Terrain"
(root / "engine_validation.json").unlink(missing_ok=True)
(root / "review_run.json").write_text(json.dumps({"status": "running", "started_utc": datetime.now(timezone.utc).isoformat()}, indent=2) + "\n", encoding="utf-8")
world = unreal.EditorLoadingAndSavingUtils.load_map("/Game/World/L_Sugarborough")
unreal.EditorPythonScripting.set_keep_python_script_alive(True)
started = time.monotonic()


def check_after_load(delta_seconds):
    # Loaded static components can defer physics creation until the next ticks.
    # Keep the editor ticking instead of querying during the map-load callback.
    if time.monotonic() - started < 2:
        return
    unreal.unregister_slate_post_tick_callback(callback)
    try:
        script = root / "review_level.py"
        exec(compile(script.read_text(encoding="utf-8"), str(script), "exec"), {
            "__file__": str(script), "__name__": "__main__", "terrain_review_world": world})
        (root / "review_failure.txt").unlink(missing_ok=True)
        (root / "review_run.json").write_text(json.dumps({"status": "passed", "completed_utc": datetime.now(timezone.utc).isoformat()}, indent=2) + "\n", encoding="utf-8")
    except Exception:
        (root / "review_failure.txt").write_text(traceback.format_exc(), encoding="utf-8")
        unreal.log_error(traceback.format_exc())
        (root / "review_run.json").write_text(json.dumps({"status": "failed", "completed_utc": datetime.now(timezone.utc).isoformat(), "error": traceback.format_exc()}, indent=2) + "\n", encoding="utf-8")
    finally:
        unreal.SystemLibrary.quit_editor()


callback = unreal.register_slate_post_tick_callback(check_after_load)
