"""Generate and rig the three expansion heroes; resume saved Meshy task IDs.

python -I Assets/Generation/expansion_characters.py --dry-run
python -I Assets/Generation/expansion_characters.py
Keys are read by genlib and never printed. No Unreal editor or project changes.
"""
import argparse
import json
from pathlib import Path
import subprocess
import sys
from concurrent.futures import ThreadPoolExecutor
import threading

ROOT = Path(__file__).resolve().parents[2]
sys.path.insert(0, str(ROOT / "Scripts"))
import genlib as g
import meshy

CONFIG = Path(__file__).with_suffix(".json")
OUT = ROOT / "ImportSource/Meshy"
STATE = ROOT / "ImportSource/_gen/expansion_characters"
RIG_LOCK = threading.Lock()


def run_character(character, dry_run):
    name = character["name"]
    prompt = g.apply_style(character["subject"], "character")
    assert len(prompt) <= 800, (name, len(prompt))
    preview_body = {"mode": "preview", "prompt": prompt, "ai_model": "latest",
                    "should_remesh": True, "target_polycount": 40000, "pose_mode": "a-pose"}
    rig_command = [sys.executable, "-I", str(ROOT / "Scripts/meshy_rig.py"),
                   "--from", name, "--name", name + "_Rig", "--height", str(character["height"]),
                   "--skeleton", "ue5_mannequin", "--action", "0:Idle", "--action", "30:Walk"]
    if dry_run:
        print(name, len(prompt), "characters", json.dumps(preview_body), flush=True)
        print("Rig:", rig_command, flush=True)
        return
    STATE.mkdir(parents=True, exist_ok=True)
    state_path = STATE / (name + ".json")
    state = json.loads(state_path.read_text()) if state_path.exists() else {}

    def save():
        state_path.write_text(json.dumps(state, indent=2), encoding="utf-8")

    key = g.env_value("MESHY_API_KEY")
    if not state.get("downloaded"):
        for stage in ("preview", "refine"):
            if not state.get(stage + "_task"):
                body = preview_body if stage == "preview" else {
                    "mode": "refine", "preview_task_id": state["preview_task"], "enable_pbr": True}
                state[stage + "_task"] = g.request("POST", meshy.BASE, key, json_body=body)["result"]
                save()
            print(name, stage, state[stage + "_task"], flush=True)
            task = meshy.wait(state[stage + "_task"], key, 1800)
            state[stage + "_credits"] = task.get("consumed_credits", 0)
            save()
        g.safe_download(task["model_urls"]["glb"], OUT / (name + ".glb"), meshy.HOSTS)
        if task.get("thumbnail_url"):
            g.safe_download(task["thumbnail_url"], OUT / (name + ".png"), meshy.HOSTS)
        g.log_manifest("Meshy", {"file": name + ".glb", "prompt": prompt, "ai_model": "latest",
                               "preview_task": state["preview_task"], "refine_task": state["refine_task"],
                               "pose": "a-pose", "textured": True,
                               "credits": (state.get("preview_credits") or 0) + (state.get("refine_credits") or 0)})
        state["downloaded"] = True
        save()
    # Keep rig jobs serial, as in the project's generation runbook.
    with RIG_LOCK:
        print(name, "rigging", flush=True)
        with (STATE / (name + "_rig.log")).open("a", encoding="utf-8") as log:
            subprocess.run(rig_command, cwd=ROOT, stdout=log, stderr=subprocess.STDOUT, check=True)
        state["rigged"] = True
        save()
        print(name, "rig and idle/walk delivered", flush=True)


if __name__ == "__main__":
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--dry-run", action="store_true")
    parser.add_argument("--only", choices=["SK_Mage", "SK_Archer", "SK_Barbarian"])
    args = parser.parse_args()
    characters = json.loads(CONFIG.read_text(encoding="utf-8"))["characters"]
    if args.only:
        characters = [character for character in characters if character["name"] == args.only]
    failures = []
    def run(character):
        try:
            run_character(character, args.dry_run)
        except (Exception, SystemExit) as error:
            failures.append(character["name"])
            print(character["name"], "FAILED:", str(error), flush=True)
    with ThreadPoolExecutor(max_workers=3) as executor:
        list(executor.map(run, characters))
    if failures:
        sys.exit("Incomplete: " + ", ".join(failures))
