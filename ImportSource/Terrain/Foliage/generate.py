"""Generate/resume the bounded Hyper3D foliage batch; credentials never printed.

python -I ImportSource/Terrain/Foliage/generate.py
Raw originals and sanitized provenance stay in this folder. The private queue
state contains per-job subscription keys and is git-ignored before generation.
No resubmission of accepted jobs, and no automatic regeneration after a failure.
"""
import concurrent.futures
import json
from pathlib import Path
import sys
import threading
import time

HERE = Path(__file__).resolve().parent
ROOT = HERE.parents[2]
sys.path.insert(0, str(ROOT / "Scripts"))
import genlib as g

BASE = "https://api.hyper3d.com/api/v2"
HOSTS = ("hyper3d.com", "hyper3d.ai")
LOCK = threading.Lock()
JOBS = json.loads((HERE / "jobs.json").read_text(encoding="utf-8"))
PRIVATE = HERE / "queue_state.json"
STATE = json.loads(PRIVATE.read_text(encoding="utf-8")) if PRIVATE.exists() else {}


def persist():
    tmp = PRIVATE.with_suffix(".json.part")
    tmp.write_text(json.dumps(STATE, indent=2) + "\n", encoding="utf-8")
    tmp.replace(PRIVATE)
    public = {"provider": "Hyper3D Rodin", "expected_max_credits": JOBS["expected_credits"], "jobs": {}}
    for name, entry in STATE.items():
        public["jobs"][name] = {k: v for k, v in entry.items() if k != "subscription_key"}
    (HERE / "generation_manifest.json").write_text(json.dumps(public, indent=2) + "\n", encoding="utf-8")


def update(name, **values):
    with LOCK:
        STATE.setdefault(name, {}).update(values)
        persist()


def run(job, seed, key):
    name = job["name"]
    dest = HERE / "Raw" / (name + ".glb")
    previous = STATE.get(name, {})
    if previous.get("status") == "done" and dest.exists():
        print(name + ": already downloaded", flush=True)
        return True
    if previous.get("status") in ("failed", "submit_failed", "submit_started") and not previous.get("task_uuid"):
        print(name + ": held for inspection; no automatic resubmission", flush=True)
        return False
    try:
        prompt = g.apply_style(job["subject"], "terrain_piece")
        if len(prompt) > 1024:
            raise ValueError("Prompt exceeds provider limit")
        fields = dict(prompt=prompt, tier=JOBS["tier"], geometry_file_format="glb", mesh_mode="Raw", quality_override=str(JOBS["quality_override"]), material="PBR", texture_mode="low", seed=str(seed))
        if not previous.get("task_uuid"):
            update(name, status="submit_started", request=fields, category=job["category"], height_cm=job["height_cm"], biomes=job["biomes"])
            data, content_type = g.multipart(fields)
            response = g.request("POST", BASE + "/rodin", key, raw_body=data, content_type=content_type)
            if response.get("error") or not response.get("uuid"):
                raise ValueError("Provider did not accept the generation")
            update(name, task_uuid=response["uuid"], subscription_key=response["jobs"]["subscription_key"], credits=response.get("consumed"), status="submitted")
            print(f"{name}: accepted, consumed {response.get('consumed')} credits", flush=True)
        entry = STATE[name]
        deadline = time.monotonic() + 1200
        last = None
        while True:
            status = g.request("POST", BASE + "/status", key, json_body={"subscription_key": entry["subscription_key"]})
            states = [j.get("status") for j in status.get("jobs", [])]
            if states != last:
                print(name + ": " + ", ".join(map(str, states)), flush=True)
                last = states
            if any(s == "Failed" for s in states):
                update(name, status="failed", error="Provider reported a failed job")
                return False
            if states and all(s == "Done" for s in states):
                break
            if time.monotonic() > deadline:
                update(name, status="timeout")
                return False
            time.sleep(12)
        files = g.request("POST", BASE + "/download", key, json_body={"task_uuid": entry["task_uuid"]})
        items = files.get("list") or files.get("files") or []
        matches = [f for f in items if str(f.get("name", f.get("url", ""))).split("?")[0].lower().endswith(".glb")]
        if not matches:
            raise ValueError("No GLB returned")
        g.safe_download(matches[0]["url"], dest, HOSTS)
        update(name, status="done", raw_file=dest.relative_to(HERE).as_posix(), completed_utc=time.strftime("%Y-%m-%dT%H:%M:%SZ", time.gmtime()))
        print(name + ": downloaded", flush=True)
        return True
    except (Exception, SystemExit) as exc:
        update(name, status="download_or_poll_failed" if STATE.get(name, {}).get("task_uuid") else "submit_failed", error=str(exc)[:300])
        print(name + ": " + type(exc).__name__ + "; details recorded without credentials", flush=True)
        return False


def main():
    assert len(JOBS["jobs"]) <= JOBS["max_models"] == 8
    key = g.env_value("HYPER3D_API_KEY", "RODIN_API_KEY", "HYPER3D_KEY")
    with concurrent.futures.ThreadPoolExecutor(max_workers=3) as pool:
        futures = [pool.submit(run, job, 4200 + i, key) for i, job in enumerate(JOBS["jobs"])]
        results = [f.result() for f in futures]
    print(f"Foliage batch: {sum(results)}/{len(results)} downloaded", flush=True)
    if not all(results):
        raise SystemExit(1)


if __name__ == "__main__":
    main()
