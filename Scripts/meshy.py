#!/usr/bin/env python3
"""Generate a 3D model with Meshy (text-to-3D, preview then textured refine); save the .glb.

Key: MESHY_API_KEY (environment or git-ignored project-root .env). Never printed.
Output: ImportSource/Meshy/<name>.glb plus a line in ImportSource/Meshy/manifest.jsonl.

  python -I Scripts/meshy.py "a rusty vending machine, stylised game prop" --name vending
  python -I Scripts/meshy.py "..." --name rock --preview-only      # untextured, cheaper
  python -I Scripts/meshy.py "..." --name x --dry-run              # show requests, send nothing
"""
import argparse
import json
import sys
import time

sys.path.insert(0, str(__import__('pathlib').Path(__file__).resolve().parent))  # -I drops the script dir
import genlib as g

BASE = "https://api.meshy.ai/openapi/v2/text-to-3d"
HOSTS = ("meshy.ai",)


def wait(task_id, key, timeout):
    deadline = time.time() + timeout
    while True:
        t = g.request("GET", f"{BASE}/{task_id}", key)
        print(f"  {t.get('status')} {t.get('progress', '')}%")
        if t.get("status") == "SUCCEEDED":
            return t
        if t.get("status") in ("FAILED", "CANCELED"):
            sys.exit(f"task {t.get('status')}: {json.dumps(t.get('task_error'))[:300]}")
        if time.time() > deadline:
            sys.exit(f"timed out; task {task_id} may still finish")
        time.sleep(5)


def main():
    p = argparse.ArgumentParser(description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter)
    p.add_argument("prompt")
    p.add_argument("--name", required=True)
    p.add_argument("--type", choices=["character", "npc", "equipment", "terrain_piece", "particle_mesh"],
                   help="wrap the prompt in the locked style for this type (Docs/styles.json)")
    p.add_argument("--ai-model", default="latest")
    p.add_argument("--polycount", type=int, default=None, help="target faces (turns on remeshing)")
    p.add_argument("--preview-only", action="store_true")
    p.add_argument("--no-pbr", action="store_true")
    p.add_argument("--timeout", type=int, default=900)
    p.add_argument("--allow-host", default=None)
    p.add_argument("--dry-run", action="store_true")
    a = p.parse_args()
    g.check_name(a.name)
    a.prompt = g.apply_style(a.prompt, a.type)
    if len(a.prompt) > 800:
        sys.exit("prompt is over Meshy's 800 character limit")

    preview = {"mode": "preview", "prompt": a.prompt, "ai_model": a.ai_model}
    if a.polycount:
        preview.update(should_remesh=True, target_polycount=a.polycount)
    refine = {"mode": "refine", "preview_task_id": "<preview id>", "enable_pbr": not a.no_pbr}
    if a.dry_run:
        print("POST", BASE, json.dumps(preview, indent=2))
        if not a.preview_only:
            print("POST", BASE, json.dumps(refine, indent=2))
        return

    key = g.env_value("MESHY_API_KEY")
    pid = g.request("POST", BASE, key, json_body=preview)["result"]
    print("preview task", pid)
    task = wait(pid, key, a.timeout)
    credits = task.get("consumed_credits") or 0
    if not a.preview_only:
        refine["preview_task_id"] = pid
        rid = g.request("POST", BASE, key, json_body=refine)["result"]
        print("refine task", rid)
        task = wait(rid, key, a.timeout)
        credits += task.get("consumed_credits") or 0

    url = (task.get("model_urls") or {}).get("glb")
    if not url:
        sys.exit("task finished but has no glb url")
    dest = g.ROOT / "ImportSource" / "Meshy" / f"{a.name}.glb"
    g.safe_download(url, dest, HOSTS, a.allow_host)
    g.log_manifest("Meshy", {"file": dest.name, "prompt": a.prompt, "ai_model": a.ai_model,
                             "preview_task": pid, "credits": credits, "textured": not a.preview_only})
    print("saved", dest, "credits", credits)


if __name__ == "__main__":
    main()
