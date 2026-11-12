#!/usr/bin/env python3
"""Run the asset-generation jobs in Assets/Generation/jobs.json, in parallel, with an approval gate.

State: ImportSource/_gen/state.json. Logs: ImportSource/_gen/logs/<id>.log. Review page: ImportSource/_gen/review.html.
One runner at a time (lock file ImportSource/_gen/run.lock).

  python -I Scripts/gen_batch.py status                      # what's done, pending, failed, approved
  python -I Scripts/gen_batch.py run --stage test            # 3D jobs of a stage, in parallel (dry first: --dry-run)
  python -I Scripts/gen_batch.py images --stage test         # image jobs ready for Codex: prompt, refs, where to save
  python -I Scripts/gen_batch.py ingest T_Skill_Lecture      # take Codex's image from the drop folder, key/trim, mark done
  python -I Scripts/gen_batch.py review                      # write the review page (thumbnails of everything made)
  python -I Scripts/gen_batch.py approve npc terrain_piece   # Dylan approved the test asset: unlock that group's batches
  python -I Scripts/gen_batch.py redo SM_Rat                 # make it again (old files kept as _v1, _v2 ...)
  python -I Scripts/gen_batch.py declare                     # add a Docs/Declared.md line for each finished asset
  python -I Scripts/gen_batch.py route terrain_piece rodin   # run that group's batches on rodin / tripo / meshy / default
  python -I Scripts/gen_batch.py run --stage batch1 --fallback rodin   # if a mesh tool runs dry mid-batch, finish on Rodin

Images: by default they are made by the agent's own image tool (Codex/ChatGPT, on the subscription) through
`images` + `ingest`. `run --images api` makes them with the OpenAI Images API instead (needs OPENAI_API_KEY).
Exit code of `run`: 0 if every job it attempted finished, 1 otherwise.
"""
import argparse
import html
import json
import os
import shutil
import subprocess
import sys
import threading
import time
from concurrent.futures import FIRST_COMPLETED, ThreadPoolExecutor, wait
from pathlib import Path

ROOT = Path(__file__).resolve().parent.parent
JOBS = ROOT / "Assets" / "Generation" / "jobs.json"
GEN = ROOT / "ImportSource" / "_gen"
STATE = GEN / "state.json"
LOGS = GEN / "logs"
LOCK = GEN / "run.lock"
DROP = ROOT / "Assets" / "Incoming" / "2D" / "_drop"
STAGES = ["test", "batch1", "batch2"]
LIMITS = {"meshy": 8, "rodin": 3, "tripo": 4, "meshy_rig": 1, "image": 4}
SCRIPTS = {"meshy": "meshy.py", "rodin": "rodin.py", "tripo": "tripo.py", "meshy_rig": "meshy_rig.py",
           "image": "openai_image.py"}
MESH_TOOLS = ("meshy", "rodin", "tripo")
OUT_DIRS = {"meshy": "Meshy", "rodin": "Hyper3D", "tripo": "Tripo"}
DEFAULT_RODIN_TIER = "Gen-2.5-Medium"
ROUTES = {}  # group -> mesh tool, from state["routes"]; set in main()
NO_RETRY = ("HTTP 400", "HTTP 401", "HTTP 403", "HTTP 404", "HTTP 422", "refusing to download", "No key found",
            "over Meshy's 800", "not found")
OUT_OF_CREDITS = ("HTTP 402", "insufficient", "Insufficient", "quota", "credit balance", "not enough credits")
TOOL_NAMES = {"meshy": "Meshy text-to-3D", "rodin": "Hyper3D Rodin Gen-2.5", "tripo": "Tripo text-to-3D",
              "meshy_rig": "Meshy rigging + animation (library presets and text-to-motion)",
              "image": "ChatGPT image generation"}

_lock = threading.Lock()


def say(*parts):
    with _lock:
        print(*parts, flush=True)


# ---------- data ----------

def load_jobs():
    doc = json.loads(JOBS.read_text(encoding="utf-8"))
    jobs = {}
    for j in doc["jobs"]:
        if j["id"] in jobs:
            sys.exit(f"duplicate job id {j['id']}")
        if j["tool"] not in SCRIPTS or j["stage"] not in STAGES:
            sys.exit(f"job {j['id']}: bad tool or stage")
        jobs[j["id"]] = j
    for j in jobs.values():
        for n in j.get("needs", []):
            if n not in jobs:
                sys.exit(f"job {j['id']} needs unknown job {n}")
    return doc, jobs


def load_state():
    if STATE.is_file():
        return json.loads(STATE.read_text(encoding="utf-8"))
    return {"jobs": {}, "approved": [], "declared": []}


def save_state(state):
    with _lock:
        GEN.mkdir(parents=True, exist_ok=True)
        tmp = STATE.with_suffix(".json.part")
        tmp.write_text(json.dumps(state, indent=2), encoding="utf-8")
        tmp.replace(STATE)


def arg_after(args, flag):
    return args[args.index(flag) + 1] if flag in args else None


def tool_of(j, override=None):
    """The mesh tool a job actually runs on: an explicit override (credit fallback), else the group's route
    (never for test jobs, which are the comparison), else the job's own tool."""
    if j["tool"] not in MESH_TOOLS:
        return j["tool"]
    if override:
        return override
    if j["stage"] != "test" and not j.get("pin_tool") and ROUTES.get(j["group"]):
        return ROUTES[j["group"]]
    return j["tool"]


def primary_output(j, tool=None):
    t = tool or tool_of(j)
    if t in MESH_TOOLS:
        return ROOT / "ImportSource" / OUT_DIRS[t] / f"{j['id']}.glb"
    if j["tool"] == "meshy_rig":
        name = arg_after(j.get("args", []), "--name") or arg_after(j.get("args", []), "--from")
        return ROOT / "ImportSource" / "Meshy" / name / f"{name}.fbx"
    return ROOT / "Assets" / "Incoming" / "2D" / j["id"] / f"{j['id']}.png"


def preview(j):
    out = primary_output(j)
    if j["tool"] == "image":
        return out if out.is_file() else None
    for ext in (".png", ".jpg", ".webp"):
        if out.with_suffix(ext).is_file() and j["tool"] != "meshy_rig":
            return out.with_suffix(ext)
    return None


def status_of(j, state):
    st = state["jobs"].get(j["id"], {}).get("status")
    if st in (None, "running"):  # unknown, or a runner died mid-job: trust the files
        return "done" if primary_output(j).is_file() else "pending"
    return st


def anchor_of(group, jobs):
    return next((j for j in jobs.values() if j["group"] == group and j["stage"] == "test"
                 and j["tool"] != "meshy_rig"), None)


def gate_ok(j, state, no_gate=False):
    return no_gate or j["stage"] == "test" or j["group"] in state["approved"]


def image_prompt(j, doc):
    kind = j.get("kind", "icon")
    return " ".join([doc["styles_2d"][j["group"]], j["subject"], doc["rules_2d"][kind]])


def image_refs(j, jobs, state):
    """Reference images for a 2D job: its group's anchor once that is done (never for the anchor itself)."""
    if j["stage"] == "test":
        return []
    a = anchor_of(j["group"], jobs)
    if a and status_of(a, state) == "done" and primary_output(a).is_file():
        return [primary_output(a)]
    return None  # anchor not ready


def image_flags(j):
    kind = j.get("kind", "icon")
    flags = ["--size", j.get("size", "1024x1024"), "--quality", j.get("quality", "medium")]
    if kind == "full":
        flags += ["--background", "opaque", "--key", "none", "--no-trim"]
    if j.get("max_size"):
        flags += ["--max-size", str(j["max_size"])]
    return flags


def mesh_args(j, tool):
    """Translate a job's args to the tool it runs on. Jobs carry --polycount (Meshy) and/or --tier (Rodin)."""
    args = list(j.get("args", []))
    poly = arg_after(args, "--polycount")
    tier = arg_after(args, "--tier")
    if tool == j["tool"]:
        return args
    if tool == "meshy":
        return ["--polycount", poly] if poly else []
    if tool == "rodin":
        return ["--tier", tier or DEFAULT_RODIN_TIER]  # face count is reduced in UE/Blender afterwards
    if tool == "tripo":
        return ["--face-limit", poly] if poly else []
    return args


def command(j, doc, jobs, state, tool=None):
    t = tool_of(j, tool)
    py = [sys.executable, "-I", str(ROOT / "Scripts" / SCRIPTS[t])]
    if t in ("meshy", "rodin"):
        return py + [j["subject"], "--name", j["id"], "--type", j["group"]] + mesh_args(j, t)
    if t == "tripo":  # tripo.py has no --type: apply the locked style words here
        sys.path.insert(0, str(ROOT / "Scripts"))
        import genlib
        return py + [genlib.apply_style(j["subject"], j["group"]), "--name", j["id"]] + mesh_args(j, t)
    if j["tool"] == "meshy_rig":
        return py + j.get("args", [])
    refs = image_refs(j, jobs, state) or []
    cmd = py + ["--name", j["id"], "--prompt", image_prompt(j, doc)] + image_flags(j)
    for r in refs:
        cmd += ["--ref", str(r)]
    return cmd


def keep_old(j):
    """Before a redo, move the previous result aside as _v1, _v2 ... (images version themselves)."""
    out = primary_output(j)
    if j["tool"] == "image":
        return
    if j["tool"] == "meshy_rig":
        d = out.parent
        if d.is_dir():
            n = 1
            while d.with_name(f"{d.name}_v{n}").exists():
                n += 1
            d.replace(d.with_name(f"{d.name}_v{n}"))
        return
    if out.is_file():
        n = 1
        while out.with_name(f"{out.stem}_v{n}{out.suffix}").exists():
            n += 1
        for ext in (".glb", ".png", ".jpg", ".webp"):
            f = out.with_suffix(ext)
            if f.is_file():
                f.replace(f.with_name(f"{out.stem}_v{n}{ext}"))


# ---------- commands ----------

def cmd_status(a, doc, jobs, state):
    rows = []
    for j in jobs.values():
        if a.stage and j["stage"] != a.stage:
            continue
        st = status_of(j, state)
        rec = state["jobs"].get(j["id"], {})
        rows.append({"id": j["id"], "tool": tool_of(j), "group": j["group"], "stage": j["stage"], "pri": j.get("pri"),
                     "status": st, "gate": "open" if gate_ok(j, state) else "locked",
                     "error": rec.get("error", "") if st == "failed" else ""})
    if a.json:
        print(json.dumps({"approved": state["approved"], "jobs": rows}, indent=2))
        return
    print("approved groups:", ", ".join(state["approved"]) or "(none)")
    print("routes:", ", ".join(f"{k} -> {v}" for k, v in sorted(ROUTES.items())) or "(each job's own tool)")
    for stage in STAGES:
        rs = [r for r in rows if r["stage"] == stage]
        if not rs:
            continue
        counts = {}
        for r in rs:
            counts[r["status"]] = counts.get(r["status"], 0) + 1
        print(f"\n== {stage}: " + ", ".join(f"{v} {k}" for k, v in sorted(counts.items())))
        for r in rs:
            extra = f"  <- {r['error'][:110]}" if r["error"] else ""
            print(f"  {r['status']:8} {r['gate']:6} {r['tool']:9} {r['group']:13} {r['pri'] or '':7} {r['id']}{extra}")


def cmd_approve(a, doc, jobs, state, on=True):
    groups = {j["group"] for j in jobs.values()}
    for gname in a.groups:
        if gname not in groups:
            sys.exit(f"unknown group {gname}; groups: {', '.join(sorted(groups))}")
        if on and gname not in state["approved"]:
            state["approved"].append(gname)
        if not on and gname in state["approved"]:
            state["approved"].remove(gname)
    save_state(state)
    print("approved groups:", ", ".join(state["approved"]) or "(none)")


def cmd_redo(a, doc, jobs, state):
    for i in a.ids:
        if i not in jobs:
            sys.exit(f"unknown job {i}")
        state["jobs"][i] = {"status": "redo"}
        if i in state["declared"]:
            state["declared"].remove(i)
    save_state(state)
    print("marked for redo:", ", ".join(a.ids), "(edit the subject in jobs.json first if the prompt was the problem)")


def cmd_route(a, doc, jobs, state):
    groups = {j["group"] for j in jobs.values() if j["tool"] in MESH_TOOLS}
    if a.group not in groups:
        sys.exit(f"unknown mesh group {a.group}; groups: {', '.join(sorted(groups))}")
    routes = state.setdefault("routes", {})
    if a.tool == "default":
        routes.pop(a.group, None)
    else:
        routes[a.group] = a.tool
    save_state(state)
    ROUTES.clear()
    ROUTES.update(routes)
    n = sum(1 for j in jobs.values() if j["group"] == a.group and j["stage"] != "test" and j["tool"] in MESH_TOOLS)
    print(f"{a.group}: {n} non-test job(s) now run on {a.tool if a.tool != 'default' else 'their own tool'}")


def selected(a, jobs):
    only = set(a.only.split(",")) if a.only else None
    if only and (only - set(jobs)):
        sys.exit("unknown ids in --only: " + ", ".join(sorted(only - set(jobs))))
    out = []
    for j in jobs.values():
        if only is not None:
            if j["id"] in only:
                out.append(j)
        elif j["stage"] == a.stage or (a.stage == "all"):
            out.append(j)
    return out


def classify(log_text):
    if any(s in log_text for s in OUT_OF_CREDITS):
        return "credits"
    if any(s in log_text for s in NO_RETRY):
        return "fatal"
    return "retry"


def cmd_run(a, doc, jobs, state):
    sel = selected(a, jobs)
    todo, skipped = [], []
    for j in sel:
        st = status_of(j, state)
        if st == "done":
            continue
        if j["tool"] == "image" and a.images == "codex":
            skipped.append((j["id"], "image: made by Codex (run `gen_batch.py images`)"))
            continue
        if not gate_ok(j, state, a.no_gate):
            skipped.append((j["id"], f"group {j['group']} not approved yet"))
            continue
        todo.append(j)
    if a.dry_run:
        for j in todo:
            print(j["id"], "->", subprocess.list2cmdline(command(j, doc, jobs, state)[2:])[:400])
        for i, why in skipped:
            print("skip", i, "-", why)
        return 0
    if not todo:
        print("nothing to run.")
        for i, why in skipped:
            print("  skip", i, "-", why)
        return 0

    GEN.mkdir(parents=True, exist_ok=True)
    LOGS.mkdir(parents=True, exist_ok=True)
    try:
        fd = os.open(LOCK, os.O_CREAT | os.O_EXCL | os.O_WRONLY)
        os.write(fd, f"started {time.ctime()}\n".encode())
        os.close(fd)
    except FileExistsError:
        sys.exit(f"another run holds {LOCK}; if none is running, delete that file")

    exhausted = set()
    running = {}
    active = {t: 0 for t in LIMITS}
    pending = {j["id"]: j for j in todo}
    results = {}
    t0 = time.time()

    def run_one(j):
        rec = state["jobs"].setdefault(j["id"], {})
        was_redo = rec.get("status") == "redo"
        if was_redo:
            keep_old(j)
        log = LOGS / f"{j['id']}.log"
        use = None
        for attempt in range(a.retries + 1):
            t = tool_of(j, use)
            if t in exhausted:
                fb = a.fallback if t in MESH_TOOLS else None
                if not fb or fb in exhausted or fb == t:
                    return "failed", f"{t} is out of credits"
                use = fb
                say(f"  {j['id']}: {t} is out of credits, using {fb}")
            cmd = command(j, doc, jobs, state, use)
            with open(log, "a", encoding="utf-8") as f:
                f.write(f"\n===== {time.ctime()} attempt {attempt + 1}\n")
                f.flush()
                try:
                    p = subprocess.run(cmd, cwd=ROOT, stdout=f, stderr=subprocess.STDOUT, timeout=a.job_timeout)
                    code = p.returncode
                except subprocess.TimeoutExpired:
                    code = -1
                    f.write(f"\nkilled after {a.job_timeout}s\n")
            text = log.read_text(encoding="utf-8", errors="replace")
            tail = text[text.rfind("===== "):]
            if code == 0:
                if use:
                    j["_used"] = use
                return "done", ""
            kind = classify(tail)
            last = [ln for ln in tail.strip().splitlines() if ln.strip()][-1:] or ["(no output)"]
            if kind == "credits":
                exhausted.add(tool_of(j, use))
                fb = a.fallback
                if fb and fb not in exhausted and tool_of(j, use) in MESH_TOOLS:
                    use = fb
                    say(f"  {j['id']}: out of credits, retrying on {fb}")
                    cmd = command(j, doc, jobs, state, use)
                    with open(log, "a", encoding="utf-8") as f:
                        f.write(f"\n===== {time.ctime()} fallback to {fb}\n")
                        f.flush()
                        p2 = subprocess.run(cmd, cwd=ROOT, stdout=f, stderr=subprocess.STDOUT, timeout=a.job_timeout)
                    if p2.returncode == 0:
                        j["_used"] = fb
                        return "done", ""
                    text = log.read_text(encoding="utf-8", errors="replace")
                    last = [ln for ln in text[text.rfind("===== "):].strip().splitlines() if ln.strip()][-1:]
                    return "failed", f"fallback {fb} failed too: {(last or ['?'])[0]}"
                return "failed", f"OUT OF CREDITS: {last[0]}"
            if kind == "fatal" or attempt == a.retries:
                return "failed", last[0]
            say(f"  retry {j['id']} in 20s ({last[0][:100]})")
            time.sleep(20)
        return "failed", "unreachable"

    try:
        with ThreadPoolExecutor(max_workers=sum(LIMITS.values())) as ex:
            while pending or running:
                for jid, j in list(pending.items()):
                    needs = j.get("needs", [])
                    in_flight = {r["id"] for r in running.values()}
                    bad = [n for n in needs if results.get(n) == "failed" or
                           (n not in pending and n not in in_flight and status_of(jobs[n], state) != "done"
                            and results.get(n) != "done")]
                    if bad:
                        del pending[jid]
                        results[jid] = "failed"
                        state["jobs"][jid] = {"status": "blocked", "error": f"needs {', '.join(bad)}"}
                        save_state(state)
                        say(f"BLOCKED {jid}: needs {', '.join(bad)}")
                        continue
                    if any(n in pending or n in in_flight for n in needs):
                        continue
                    if j["tool"] == "image" and image_refs(j, jobs, state) is None:
                        del pending[jid]
                        results[jid] = "failed"
                        say(f"BLOCKED {jid}: its group's anchor image isn't done")
                        continue
                    if active[tool_of(j)] >= LIMITS[tool_of(j)]:
                        continue
                    del pending[jid]
                    j["_slot"] = tool_of(j)
                    active[j["_slot"]] += 1
                    prev = state["jobs"].get(jid, {}).get("status")
                    state["jobs"][jid] = {"status": "redo" if prev == "redo" else "running", "started": time.ctime()}
                    save_state(state)
                    say(f"START {jid} ({j['_slot']})")
                    running[ex.submit(run_one, j)] = j
                if not running:
                    break
                done, _ = wait(list(running), return_when=FIRST_COMPLETED)
                for fut in done:
                    j = running.pop(fut)
                    active[j["_slot"]] -= 1
                    try:
                        st, err = fut.result()
                    except Exception as e:  # a bug in this runner, not in the job
                        st, err = "failed", f"runner error: {e}"
                    results[j["id"]] = st
                    state["jobs"][j["id"]] = {"status": st, "error": err, "finished": time.ctime(),
                                              "via": "api", "tool": j.get("_used") or j["_slot"]}
                    save_state(state)
                    say(f"{'DONE' if st == 'done' else 'FAIL'} {j['id']}" + (f": {err[:160]}" if err else ""))
    finally:
        LOCK.unlink(missing_ok=True)

    ok = [i for i, s in results.items() if s == "done"]
    bad = [i for i, s in results.items() if s != "done"]
    mins = (time.time() - t0) / 60
    lines = [f"run {a.stage if not a.only else a.only} finished in {mins:.1f} min: {len(ok)} done, {len(bad)} failed"]
    for i in bad:
        lines.append(f"  FAILED {i}: {state['jobs'].get(i, {}).get('error', '')[:200]}  (log: ImportSource/_gen/logs/{i}.log)")
    if exhausted:
        lines.append("  OUT OF CREDITS: " + ", ".join(sorted(exhausted)) + " - top up or switch tools before re-running")
    for i, why in skipped:
        lines.append(f"  skipped {i}: {why}")
    summary = "\n".join(lines)
    (GEN / "last_run.txt").write_text(summary + "\n", encoding="utf-8")
    print("\n" + summary)
    write_review(doc, jobs, state)
    print("review page:", GEN / "review.html")
    return 0 if not bad else 1


def ready_images(a, doc, jobs, state):
    out = []
    for j in jobs.values():
        if j["tool"] != "image" or status_of(j, state) == "done":
            continue
        if a.stage and j["stage"] != a.stage:
            continue
        if not gate_ok(j, state, a.no_gate):
            continue
        refs = image_refs(j, jobs, state)
        if refs is None:
            continue
        w, _, h = j.get("size", "1024x1024").partition("x")
        out.append({"id": j["id"], "stage": j["stage"], "pri": j.get("pri"), "group": j["group"],
                    "aspect": f"{w}x{h}",
                    "prompt": image_prompt(j, doc),
                    "style_reference": [r.relative_to(ROOT).as_posix() for r in refs],
                    "save_to": (DROP / f"{j['id']}.png").relative_to(ROOT).as_posix(),
                    "then_run": f"python -I Scripts/gen_batch.py ingest {j['id']}"})
    return out


def cmd_images(a, doc, jobs, state):
    items = ready_images(a, doc, jobs, state)
    if a.json:
        print(json.dumps(items, indent=2))
        return
    if not items:
        print("no image jobs are ready (all done, or their group isn't approved yet / anchor not made).")
        return
    print(f"{len(items)} image(s) ready. For each: generate it at the given aspect with the prompt as written,"
          " attach the style reference if one is listed, save the PNG to save_to, then run then_run.\n")
    for it in items:
        print(f"--- {it['id']}  ({it['pri']}, {it['aspect']})")
        print(f"prompt: {it['prompt']}")
        if it["style_reference"]:
            print("style reference: " + ", ".join(it["style_reference"]))
        print(f"save to: {it['save_to']}")
        print(f"then: {it['then_run']}\n")


def cmd_ingest(a, doc, jobs, state):
    j = jobs.get(a.id)
    if not j or j["tool"] != "image":
        sys.exit(f"{a.id} is not an image job")
    src = Path(a.file) if a.file else DROP / f"{a.id}.png"
    if not src.is_absolute():
        src = ROOT / src
    if not src.is_file():
        sys.exit(f"no image at {src}; save the generated PNG there first")
    refs = image_refs(j, jobs, state) or []
    cmd = [sys.executable, "-I", str(ROOT / "Scripts" / "openai_image.py"), "--name", j["id"],
           "--prompt", image_prompt(j, doc), "--from-file", str(src), "--source", a.source] + image_flags(j)
    for r in refs:
        cmd += ["--ref", str(r)]
    p = subprocess.run(cmd, cwd=ROOT)
    if p.returncode != 0:
        sys.exit("ingest failed")
    done_dir = DROP / "ingested"
    done_dir.mkdir(parents=True, exist_ok=True)
    if src.parent == DROP:
        shutil.move(str(src), str(done_dir / f"{a.id}_{time.strftime('%H%M%S')}.png"))
    state["jobs"][a.id] = {"status": "done", "finished": time.ctime(), "via": a.source}
    save_state(state)
    print("done:", a.id)


def write_review(doc, jobs, state):
    GEN.mkdir(parents=True, exist_ok=True)
    parts = ["<!doctype html><meta charset=utf-8><title>Asset review</title><style>"
             "body{font:14px system-ui;background:#1b1b1f;color:#eee;margin:16px}"
             "h2{margin:24px 0 8px}.g{display:grid;grid-template-columns:repeat(auto-fill,minmax(220px,1fr));gap:12px}"
             ".c{background:#2a2a30;border-radius:8px;padding:8px}.c img{width:100%;height:200px;object-fit:contain;"
             "background:repeating-conic-gradient(#3a3a40 0 25%,#2f2f35 0 50%) 0 0/20px 20px;border-radius:4px}"
             ".none{height:200px;display:flex;align-items:center;justify-content:center;color:#888;background:#222;"
             "border-radius:4px}.s{font-size:12px;color:#aaa}.done{color:#7c7}.failed,.blocked{color:#f77}"
             ".t{font-weight:600;margin:6px 0 2px}</style>",
             f"<h1>Asset review</h1><p class=s>Written {html.escape(time.ctime())}. Approved groups: "
             f"{html.escape(', '.join(state['approved']) or 'none')}. 3D: thumbnails are the generator's own render; "
             "judge them for real in the game camera.</p>"]
    for stage in STAGES:
        parts.append(f"<h2>{stage}</h2><div class=g>")
        for j in jobs.values():
            if j["stage"] != stage:
                continue
            st = status_of(j, state)
            pv = preview(j)
            img = (f"<img src='{html.escape(os.path.relpath(pv, GEN).replace(os.sep, '/'))}?{int(pv.stat().st_mtime)}'>"
                   if pv else f"<div class=none>{'no preview' if st == 'done' else st}</div>")
            err = state["jobs"].get(j["id"], {}).get("error", "")
            parts.append(f"<div class=c>{img}<div class=t>{html.escape(j['id'])}</div>"
                         f"<div class='s {st}'>{st} · {j['tool']} · {j['group']} · {j.get('pri', '')}</div>"
                         f"<div class=s>{html.escape(j['subject'][:160])}</div>"
                         + (f"<div class='s failed'>{html.escape(err[:200])}</div>" if err else "") + "</div>")
        parts.append("</div>")
    (GEN / "review.html").write_text("".join(parts), encoding="utf-8")


def cmd_review(a, doc, jobs, state):
    write_review(doc, jobs, state)
    print(GEN / "review.html")


def cmd_declare(a, doc, jobs, state):
    f = ROOT / "Docs" / "Declared.md"
    text = f.read_text(encoding="utf-8")
    header = "## Generated during the jam"
    rows = []
    for j in jobs.values():
        if j["id"] in state["declared"] or status_of(j, state) != "done":
            continue
        via = state["jobs"].get(j["id"], {}).get("via", "")
        tool = TOOL_NAMES[state["jobs"].get(j["id"], {}).get("tool") or tool_of(j)]
        if j["tool"] == "image":
            tool = "OpenAI Images API" if via == "api" else "ChatGPT image generation (Codex)"
        where = primary_output(j).relative_to(ROOT).as_posix()
        rows.append(f"| `{j['id']}` | {tool} | {time.strftime('%Y-%m-%d')} | `{where}`; prompt in "
                    f"`Assets/Generation/jobs.json` |")
        state["declared"].append(j["id"])
    if not rows:
        print("nothing new to declare.")
        return
    if header not in text:
        text = text.rstrip() + (f"\n\n{header}\n\nAI-generated assets, added by `Scripts/gen_batch.py declare`. Full prompts "
                                "(with the locked style words) are in the `manifest.jsonl` beside each output.\n\n"
                                "| Asset | Tool | Date | File; prompt |\n| --- | --- | --- | --- |\n")
    text = text.rstrip() + "\n" + "\n".join(rows) + "\n"
    f.write_text(text, encoding="utf-8")
    save_state(state)
    print(f"declared {len(rows)} asset(s) in Docs/Declared.md")


def main():
    p = argparse.ArgumentParser(description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter)
    sub = p.add_subparsers(dest="cmd", required=True)
    s = sub.add_parser("status"); s.add_argument("--stage", choices=STAGES); s.add_argument("--json", action="store_true")
    r = sub.add_parser("run")
    r.add_argument("--stage", choices=STAGES + ["all"], default="test")
    r.add_argument("--only", help="comma-separated job ids (ignores --stage)")
    r.add_argument("--no-gate", action="store_true", help="run even if the group isn't approved (Dylan said so)")
    r.add_argument("--images", choices=["codex", "api"], default="codex")
    r.add_argument("--retries", type=int, default=1)
    r.add_argument("--job-timeout", type=int, default=3600)
    r.add_argument("--fallback", choices=list(MESH_TOOLS), default=None,
                   help="when a mesh tool runs out of credits, run its remaining jobs on this one")
    r.add_argument("--dry-run", action="store_true")
    im = sub.add_parser("images"); im.add_argument("--stage", choices=STAGES); im.add_argument("--json", action="store_true")
    im.add_argument("--no-gate", action="store_true")
    ing = sub.add_parser("ingest"); ing.add_argument("id"); ing.add_argument("file", nargs="?")
    ing.add_argument("--source", default="codex")
    sub.add_parser("review")
    ap = sub.add_parser("approve"); ap.add_argument("groups", nargs="+")
    un = sub.add_parser("unapprove"); un.add_argument("groups", nargs="+")
    rd = sub.add_parser("redo"); rd.add_argument("ids", nargs="+")
    sub.add_parser("declare")
    ro = sub.add_parser("route", help="run a group's non-test mesh jobs on another tool")
    ro.add_argument("group")
    ro.add_argument("tool", choices=list(MESH_TOOLS) + ["default"])
    a = p.parse_args()
    doc, jobs = load_jobs()
    state = load_state()
    ROUTES.update(state.get("routes", {}))
    if a.cmd == "run":
        sys.exit(cmd_run(a, doc, jobs, state))
    {"status": cmd_status, "images": cmd_images, "ingest": cmd_ingest, "review": cmd_review,
     "approve": cmd_approve, "unapprove": lambda *x: cmd_approve(*x, on=False), "redo": cmd_redo,
     "declare": cmd_declare, "route": cmd_route}[a.cmd](a, doc, jobs, state)


if __name__ == "__main__":
    main()
