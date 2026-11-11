#!/usr/bin/env python3
"""Rig a humanoid with Meshy, then make its animations (library presets and text-to-motion clips).

Key: MESHY_API_KEY (environment or git-ignored project-root .env). Never printed.
Input: a textured Meshy model by name (looked up in ImportSource/Meshy/manifest.jsonl), a Meshy task id,
or any local .glb (sent as a data URI, max 100 MB; this is how a Rodin model would be tried).
Output, all in ImportSource/Meshy/<name>/:
  <name>.fbx / <name>.glb       rigged skeletal mesh (skeleton template ue5_mannequin by default)
  A_<name>_<Label>.fbx          one file per animation
  rig.json                      task ids, so a re-run skips finished pieces and only redoes what failed

  python -I Scripts/meshy_rig.py --from SK_Hero --height 2.0 \\
      --action 0:Idle --action 30:Walk \\
      --motion "SwordRaise=3:raises a sword in the right hand straight up overhead and holds it"
  python -I Scripts/meshy_rig.py --glb ImportSource/Hyper3D/SM_X.glb --name X_Rig ...   # probe a non-Meshy model
  python -I Scripts/meshy_rig.py --from SK_Hero --dry-run

Library action ids: GET /openapi/v1/animations/library (free). Useful ones: 0 Idle, 11/12 Idle_02/03,
30 Casual_Walk, 114 Proud_Strut, 306 Cheer_with_One_Hand_Up, 101 Sword_Shout, 89 Combat_Stance.
"""
import argparse
import base64
import json
import os
import sys
import threading
import time
from concurrent.futures import ThreadPoolExecutor
from pathlib import Path

sys.path.insert(0, str(__import__('pathlib').Path(__file__).resolve().parent))  # -I drops the script dir
import genlib as g

API = "https://api.meshy.ai/openapi/v1"
HOSTS = ("meshy.ai",)
SKELETONS = ["meshyrig", "mixamo", "ue5_mannequin", "max_biped", "unity_humanoid", "maya_human_ik",
             "blender_rigify_basic_human", "cc5_hd_neutral_base", "daz_genesis9"]
_lock = threading.Lock()


def log(*parts):
    with _lock:
        print(*parts, flush=True)


def wait(kind, task_id, key, timeout, label):
    deadline = time.time() + timeout
    last = None
    while True:
        t = g.request("GET", f"{API}/{kind}/{task_id}", key)
        st = (t.get("status"), t.get("progress"))
        if st != last:
            log(f"  [{label}] {st[0]} {st[1] if st[1] is not None else ''}")
            last = st
        if t.get("status") == "SUCCEEDED":
            return t
        if t.get("status") in ("FAILED", "CANCELED"):
            sys.exit(f"[{label}] {t.get('status')}: {json.dumps(t.get('task_error'))[:300]}")
        if time.time() > deadline:
            sys.exit(f"[{label}] timed out; task {task_id} may still finish")
        time.sleep(5)


def parse_actions(items):
    out = []
    for it in items:
        aid, sep, label = it.partition(":")
        if not sep or not aid.strip().isdigit() or not label:
            sys.exit(f"--action wants ID:Label, got {it!r}")
        g.check_name(label)
        out.append((int(aid), label))
    return out


def parse_motions(items):
    out = []
    for it in items:
        label, sep, rest = it.partition("=")
        dur, sep2, prompt = rest.partition(":")
        try:
            d = float(dur)
        except ValueError:
            d = -1
        if not sep or not sep2 or not prompt or not (2 <= d <= 10) or (d * 2) % 1:
            sys.exit(f"--motion wants Label=SECONDS:prompt (2 to 10 s in 0.5 steps), got {it!r}")
        if len(prompt) > 400:
            sys.exit(f"--motion {label}: prompt over 400 characters")
        g.check_name(label)
        out.append((label, d, prompt))
    return out


def main():
    p = argparse.ArgumentParser(description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter)
    src = p.add_mutually_exclusive_group(required=True)
    src.add_argument("--from", dest="from_name", help="name of a textured model made by meshy.py")
    src.add_argument("--task-id", help="a Meshy text-to-3d refine (or image-to-3d) task id")
    src.add_argument("--glb", help="a local .glb to rig (sent as a data URI)")
    p.add_argument("--name", help="output name (default: the --from name)")
    p.add_argument("--height", type=float, default=1.8, help="character height in metres")
    p.add_argument("--skeleton", choices=SKELETONS, default="ue5_mannequin")
    p.add_argument("--action", action="append", default=[], metavar="ID:Label")
    p.add_argument("--motion", action="append", default=[], metavar="Label=SECONDS:prompt")
    p.add_argument("--fps", type=int, choices=[24, 25, 30, 60], default=None)
    p.add_argument("--timeout", type=int, default=1200)
    p.add_argument("--allow-host", default=None)
    p.add_argument("--dry-run", action="store_true")
    a = p.parse_args()
    actions = parse_actions(a.action)
    motions = parse_motions(a.motion)
    name = a.name or a.from_name
    if not name:
        sys.exit("--name is required with --task-id or --glb")
    g.check_name(name)
    out = g.ROOT / "ImportSource" / "Meshy" / name
    state_file = out / "rig.json"
    state = json.loads(state_file.read_text(encoding="utf-8")) if state_file.is_file() else {}

    def save():
        with _lock:
            out.mkdir(parents=True, exist_ok=True)
            state_file.write_text(json.dumps(state, indent=2), encoding="utf-8")

    rig_body = {"height_meters": a.height, "skeleton_template": a.skeleton}
    if a.from_name:
        rec = g.find_manifest("Meshy", a.from_name)
        if not rec or not rec.get("refine_task"):
            sys.exit(f"no textured Meshy model named {a.from_name} with a refine_task in the manifest "
                     "(make it with meshy.py, not --preview-only)")
        rig_body["input_task_id"] = rec["refine_task"]
    elif a.task_id:
        rig_body["input_task_id"] = a.task_id
    else:
        glb = Path(a.glb) if os.path.isabs(a.glb) else g.ROOT / a.glb
        g.validate_glb(glb)
        data = glb.read_bytes()
        if len(data) > 75 * 1024 * 1024:
            sys.exit("glb too large for a data URI (100 MB after base64)")
        rig_body["model_url"] = "data:model/gltf-binary;base64," + base64.b64encode(data).decode()

    if a.dry_run:
        shown = dict(rig_body)
        if "model_url" in shown:
            shown["model_url"] = shown["model_url"][:48] + "..."
        print("POST", f"{API}/rigging", json.dumps(shown, indent=2))
        for aid, label in actions:
            print("POST", f"{API}/animations", json.dumps({"rig_task_id": "<rig>", "action_id": aid}), "->", label)
        for label, d, prompt in motions:
            print("POST", f"{API}/text-to-motion", json.dumps({"prompt": prompt, "duration": d, "mode": "prime"}),
                  "-> then /animations with motion_task_id ->", label)
        print("existing state:", json.dumps(state))
        return

    key = g.env_value("MESHY_API_KEY")
    credits = 0

    # 1. Rig (reused if a previous run finished it).
    rig_fbx = out / f"{name}.fbx"
    if not state.get("rig_task") or not rig_fbx.is_file():
        if not state.get("rig_task"):
            state["rig_task"] = g.request("POST", f"{API}/rigging", key, json_body=rig_body)["result"]
            save()
        log("rig task", state["rig_task"])
        t = wait("rigging", state["rig_task"], key, a.timeout, "rig")
        credits += t.get("consumed_credits") or 0
        res = t.get("result") or {}
        if not res.get("rigged_character_fbx_url"):
            sys.exit("rig finished without an fbx url")
        g.safe_download(res["rigged_character_fbx_url"], rig_fbx, HOSTS, a.allow_host)
        if res.get("rigged_character_glb_url"):
            g.safe_download(res["rigged_character_glb_url"], out / f"{name}.glb", HOSTS, a.allow_host)
        basic = res.get("basic_animations") or {}
        if basic.get("walking_fbx_url"):
            g.safe_download(basic["walking_fbx_url"], out / f"A_{name}_BasicWalk.fbx", HOSTS, a.allow_host)
        log("saved", rig_fbx)
    else:
        log("rig already done:", rig_fbx)

    # 2. Animations, in parallel. Each records its task ids so a retry resumes rather than repays.
    anims = state.setdefault("anims", {})
    post = {"post_process": {"operation_type": "change_fps", "fps": a.fps}} if a.fps else {}

    def animate(label, body_fn):
        dest = out / f"A_{name}_{label}.fbx"
        if dest.is_file():
            log(f"[{label}] already done")
            return 0
        rec = anims.setdefault(label, {})
        used = 0
        if not rec.get("anim_task"):
            body = body_fn(rec)
            if body is None:
                return used
            used += rec.pop("_motion_credits", 0)
            rec["anim_task"] = g.request("POST", f"{API}/animations", key,
                                         json_body={"rig_task_id": state["rig_task"], **body, **post})["result"]
            save()
        t = wait("animations", rec["anim_task"], key, a.timeout, label)
        used += t.get("consumed_credits") or 0
        r = t.get("result") or {}
        url = r.get("processed_animation_fps_fbx_url") or r.get("animation_fbx_url")
        if not url:
            sys.exit(f"[{label}] animation finished without an fbx url")
        g.safe_download(url, dest, HOSTS, a.allow_host)
        log(f"[{label}] saved {dest.name}")
        return used

    def library_body(aid):
        return lambda rec: {"action_id": aid}

    def motion_body(label, d, prompt):
        def fn(rec):
            if not rec.get("motion_task"):
                rec["motion_task"] = g.request("POST", f"{API}/text-to-motion", key,
                                               json_body={"prompt": prompt, "duration": d, "mode": "prime"})["result"]
                rec["prompt"] = prompt
                save()
            t = wait("text-to-motion", rec["motion_task"], key, a.timeout, label + " motion")
            rec["_motion_credits"] = t.get("consumed_credits") or 0
            return {"motion_task_id": rec["motion_task"]}
        return fn

    jobs = [(label, library_body(aid)) for aid, label in actions]
    jobs += [(label, motion_body(label, d, prompt)) for label, d, prompt in motions]
    failures = []

    def run(job):
        label, fn = job
        try:
            return animate(label, fn)
        except SystemExit as e:
            failures.append(f"{label}: {e}")
            # A failed task is dead: forget its ids so the next run starts it again.
            anims.pop(label, None)
            save()
            return 0

    if jobs:
        with ThreadPoolExecutor(max_workers=min(6, len(jobs))) as ex:
            credits += sum(ex.map(run, jobs))
    save()
    g.log_manifest("Meshy", {"file": f"{name}/{rig_fbx.name}", "rig_task": state["rig_task"],
                             "skeleton": a.skeleton, "height": a.height,
                             "animations": sorted(p.name for p in out.glob(f"A_{name}_*.fbx")),
                             "credits": credits})
    if failures:
        sys.exit("some animations failed (re-run to retry only those):\n  " + "\n  ".join(failures))
    print("done", out, "credits", credits)


if __name__ == "__main__":
    main()
