#!/usr/bin/env python3
"""Generate a 3D model with the Tripo API (v3) and save the .glb under ImportSource/Tripo/.

Key: set TRIPO_API_KEY in the environment, or in a git-ignored .env at the project root
(one line: TRIPO_API_KEY=...). The key is never printed or logged.

Examples:
  python Scripts/tripo.py "a rusty vending machine, stylised, game prop" --name vending
  python Scripts/tripo.py "tiny market stall" --name stall --face-limit 20000 --no-pbr
  python Scripts/tripo.py "..." --dry-run          # show the request, send nothing

Each success appends a line to ImportSource/Tripo/manifest.jsonl (prompt, task id, credits)
so Docs/Declared.md can list the generated assets. Import the .glb into UE via Interchange.
"""
import argparse
import json
import os
import re
import sys
import time
import urllib.error
import urllib.request
from pathlib import Path

BASE = "https://openapi.tripo3d.ai/v3"
ROOT = Path(__file__).resolve().parent.parent
OUT = ROOT / "ImportSource" / "Tripo"


def load_key():
    key = os.environ.get("TRIPO_API_KEY")
    env = ROOT / ".env"
    if not key and env.is_file():
        for line in env.read_text(encoding="utf-8").splitlines():
            m = re.match(r"\s*TRIPO_API_KEY\s*=\s*(.+?)\s*$", line)
            if m:
                key = m.group(1).strip("\"'")
    return key


MAX_BYTES = 150 * 1024 * 1024
ALLOWED_HOST_SUFFIXES = ("tripo3d.ai", "tripo3d.com")


def check_url(url, extra_host):
    from urllib.parse import urlparse
    u = urlparse(url)
    suffixes = ALLOWED_HOST_SUFFIXES + ((extra_host,) if extra_host else ())
    host = u.hostname or ""
    if u.scheme != "https" or not any(host == s or host.endswith("." + s) for s in suffixes):
        sys.exit(f"refusing to download from {host!r}; if that host is right, pass --allow-host {host}")


def validate_glb(path):
    """A real binary glTF: right magic/version/length, and no external file references."""
    import struct
    data = path.read_bytes()
    if len(data) < 20 or data[:4] != b"glTF":
        sys.exit("not a binary glTF (bad magic); discarded")
    version, length = struct.unpack_from("<II", data, 4)
    if version != 2 or length != len(data):
        sys.exit("glb header does not match the file; discarded")
    chunk_len, chunk_type = struct.unpack_from("<II", data, 12)
    if chunk_type != 0x4E4F534A or 20 + chunk_len > len(data):
        sys.exit("glb has no valid JSON chunk; discarded")
    doc = json.loads(data[20:20 + chunk_len])
    for item in list(doc.get("buffers", [])) + list(doc.get("images", [])):
        uri = item.get("uri", "")
        if uri and not uri.startswith("data:"):
            sys.exit(f"glb references an external file ({uri[:60]}); discarded")


def call(method, path, key, body=None):
    data = json.dumps(body).encode() if body is not None else None
    req = urllib.request.Request(BASE + path, data=data, method=method)
    req.add_header("Authorization", "Bearer " + key)
    req.add_header("Content-Type", "application/json")
    try:
        with urllib.request.urlopen(req, timeout=60) as r:
            return json.load(r)
    except urllib.error.HTTPError as e:
        # Error bodies carry code/message/suggestion; they never echo the key.
        detail = e.read().decode(errors="replace")[:500]
        sys.exit(f"Tripo HTTP {e.code}: {detail}")


def main():
    p = argparse.ArgumentParser(description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter)
    p.add_argument("prompt", nargs="?")
    p.add_argument("--balance", action="store_true", help="check the key works and show the API credit balance")
    p.add_argument("--name", help="output file stem, letters/digits/_-")
    p.add_argument("--model", default="v3.1-20260211")
    p.add_argument("--negative", default=None)
    p.add_argument("--face-limit", type=int, default=None)
    p.add_argument("--no-pbr", action="store_true")
    p.add_argument("--no-texture", action="store_true")
    p.add_argument("--auto-size", action="store_true")
    p.add_argument("--timeout", type=int, default=600, help="seconds to wait for the task")
    p.add_argument("--allow-host", default=None, help="extra download host to trust (after you've checked it)")
    p.add_argument("--dry-run", action="store_true")
    a = p.parse_args()

    if a.balance:
        key = load_key()
        if not key:
            sys.exit("No TRIPO_API_KEY found (environment, or project-root .env).")
        d = call("GET", "/account/balance", key).get("data", {})
        print("key accepted; balance:", d.get("balance"), "frozen:", d.get("frozen"))
        return
    if not a.prompt or not a.name:
        sys.exit("need a prompt and --name (or use --balance)")
    if not re.fullmatch(r"[A-Za-z0-9_-]+", a.name):
        sys.exit("--name: letters, digits, _ and - only")
    if len(a.prompt) > 1024:
        sys.exit("prompt is over Tripo's 1024 character limit")

    body = {"prompt": a.prompt, "model": a.model}
    if a.negative: body["negative_prompt"] = a.negative
    if a.face_limit: body["face_limit"] = a.face_limit
    if a.no_texture: body["texture"] = False; body["pbr"] = False
    elif a.no_pbr: body["pbr"] = False
    if a.auto_size: body["auto_size"] = True

    if a.dry_run:
        print("POST", BASE + "/generation/text-to-model")
        print(json.dumps(body, indent=2))
        return

    key = load_key()
    if not key:
        sys.exit("No TRIPO_API_KEY (environment or project-root .env).")

    created = call("POST", "/generation/text-to-model", key, body)
    task_id = created["data"]["task_id"]
    print("task", task_id)

    deadline = time.time() + a.timeout
    while True:
        task = call("GET", f"/tasks/{task_id}", key)["data"]
        status = task.get("status")
        print(f"  {status} {task.get('progress', '')}")
        if status == "success":
            break
        if status in ("failed", "cancelled"):
            sys.exit(f"task {status}: {json.dumps(task)[:500]}")
        if time.time() > deadline:
            sys.exit(f"timed out; task {task_id} may still finish (re-check it later)")
        time.sleep(5)

    url = task["output"]["model_url"]
    check_url(url, a.allow_host)
    OUT.mkdir(parents=True, exist_ok=True)
    dest = OUT / f"{a.name}.glb"
    # The CDN url is pre-signed: fetch it without the Authorization header.
    # Download to a quarantine name, validate, then rename. Nothing is ever executed.
    tmp = dest.with_suffix(".glb.part")
    with urllib.request.urlopen(url, timeout=120) as r, open(tmp, "wb") as f:
        n = 0
        while chunk := r.read(1 << 16):
            n += len(chunk)
            if n > MAX_BYTES:
                f.close(); tmp.unlink(); sys.exit("download over the size cap; discarded")
            f.write(chunk)
    try:
        validate_glb(tmp)
    except SystemExit:
        tmp.unlink(missing_ok=True)
        raise
    tmp.replace(dest)
    with open(OUT / "manifest.jsonl", "a", encoding="utf-8") as f:
        f.write(json.dumps({
            "file": dest.name, "prompt": a.prompt, "model": a.model,
            "task_id": task_id, "credits": task.get("credits_consumed"),
            "date": time.strftime("%Y-%m-%d"),
        }) + "\n")
    print("saved", dest, "credits", task.get("credits_consumed"))


if __name__ == "__main__":
    main()
