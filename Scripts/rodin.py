#!/usr/bin/env python3
"""Generate a 3D model with Hyper3D Rodin (text-to-3D); save the .glb.

Key: HYPER3D_API_KEY (environment or git-ignored project-root .env). Never printed.
Output: ImportSource/Hyper3D/<name>.glb plus a line in ImportSource/Hyper3D/manifest.jsonl.

  python -I Scripts/rodin.py "a rusty vending machine, stylised game prop" --name vending
  python -I Scripts/rodin.py "..." --name x --dry-run

Basic API access only allows tiers Gen-2.5-Low and Gen-2.5-Extreme-Low; the default tier
here is Gen-2.5-Low. The download response shape is not fully documented, so the first
real run may need small fixes (the script prints what it received if it can't find a glb).
"""
import argparse
import json
import sys
import time

sys.path.insert(0, str(__import__('pathlib').Path(__file__).resolve().parent))  # -I drops the script dir
import genlib as g

BASE = "https://api.hyper3d.com/api/v2"
HOSTS = ("hyper3d.com", "hyper3d.ai")


def job_states(status):
    jobs = status.get("jobs", [])
    return [j.get("status") for j in jobs if isinstance(j, dict)]


def main():
    p = argparse.ArgumentParser(description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter)
    p.add_argument("prompt")
    p.add_argument("--name", required=True)
    p.add_argument("--type", choices=["character", "npc", "equipment", "terrain_piece", "particle_mesh"],
                   help="wrap the prompt in the locked style for this type (Docs/styles.json)")
    p.add_argument("--tier", default="Gen-2.5-Low")
    p.add_argument("--timeout", type=int, default=900)
    p.add_argument("--allow-host", default=None, help="extra download host to trust after you've checked it")
    p.add_argument("--dry-run", action="store_true")
    a = p.parse_args()
    g.check_name(a.name)
    a.prompt = g.apply_style(a.prompt, a.type)

    fields = {"prompt": a.prompt, "tier": a.tier, "geometry_file_format": "glb"}
    if a.dry_run:
        print("POST", BASE + "/rodin (multipart)", json.dumps(fields, indent=2))
        return

    key = g.env_value("HYPER3D_API_KEY", "RODIN_API_KEY", "HYPER3D_KEY")
    body, ctype = g.multipart(fields)
    sub = g.request("POST", BASE + "/rodin", key, raw_body=body, content_type=ctype)
    if sub.get("error"):
        sys.exit(f"submit failed: {json.dumps(sub)[:400]}")
    task_uuid = sub["uuid"]
    sub_key = sub["jobs"]["subscription_key"]
    print("task", task_uuid, "consumed", sub.get("consumed"))

    time.sleep(6)
    deadline = time.time() + a.timeout
    delay = 6
    while True:
        st = g.request("POST", BASE + "/status", key, json_body={"subscription_key": sub_key})
        states = job_states(st)
        print("  ", states)
        if any(s == "Failed" for s in states):
            sys.exit(f"job failed: {json.dumps(st)[:300]}")
        if states and all(s == "Done" for s in states):
            break
        if time.time() > deadline:
            sys.exit(f"timed out; task {task_uuid} may still finish")
        time.sleep(delay)
        delay = min(delay + 3, 20)

    dl = g.request("POST", BASE + "/download", key, json_body={"task_uuid": task_uuid})
    items = dl.get("list") or dl.get("files") or []
    glb = next((i for i in items if isinstance(i, dict) and str(i.get("name", i.get("url", ""))).split("?")[0].lower().endswith(".glb")), None)
    if not glb:
        sys.exit("no .glb in the download response; keys seen: " + ", ".join(dl.keys()) +
                 "; names: " + ", ".join(str(i.get("name")) for i in items if isinstance(i, dict)))
    dest = g.ROOT / "ImportSource" / "Hyper3D" / f"{a.name}.glb"
    g.safe_download(glb["url"], dest, HOSTS, a.allow_host)
    g.log_manifest("Hyper3D", {"file": dest.name, "prompt": a.prompt, "tier": a.tier,
                               "task_uuid": task_uuid, "credits": sub.get("consumed")})
    print("saved", dest)


if __name__ == "__main__":
    main()
