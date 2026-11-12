#!/usr/bin/env python3
"""Generate a 3D model through fal.ai's queue API (default: Hunyuan3D v3.1 Pro text-to-3D); save the .glb.

Key: FAL3D_API_KEY (or FAL_KEY / FAL_API_KEY) in the environment or git-ignored project-root .env. Never printed.
Output: ImportSource/Fal/<name>.glb, a preview <name>.png when fal returns one, and a line in
ImportSource/Fal/manifest.jsonl.

  python -I Scripts/fal3d.py "a chunky candy fence segment" --name SM_Fence --type terrain_piece
  python -I Scripts/fal3d.py "..." --name X --model fal-ai/hunyuan-3d/v3.1/rapid/text-to-3d --field enable_geometry=true
  python -I Scripts/fal3d.py "..." --name X --dry-run

Prices (fal's page, Nov 2026): v3.1 Pro $0.375 a model (+$0.15 each for PBR or a custom face_count);
v3.1 Rapid $0.225 (its textured output is OBJ, not GLB, so Pro is the default here).
"""
import argparse
import json
import sys
import time

sys.path.insert(0, str(__import__('pathlib').Path(__file__).resolve().parent))  # -I drops the script dir
import genlib as g

QUEUE = "https://queue.fal.run"
HOSTS = ("fal.media", "fal.run", "fal.ai")
DEFAULT_MODEL = "fal-ai/hunyuan-3d/v3.1/pro/text-to-3d"


def parse_value(v):
    low = v.lower()
    if low in ("true", "false"):
        return low == "true"
    try:
        return int(v)
    except ValueError:
        try:
            return float(v)
        except ValueError:
            return v


def find_url(obj, suffixes):
    """Depth-first search of a fal result for the first file URL ending in one of suffixes."""
    if isinstance(obj, dict):
        url = obj.get("url")
        if isinstance(url, str) and url.split("?")[0].lower().endswith(suffixes):
            return url
        for k in ("model_glb", "model_urls", "model_mesh", "glb", "thumbnail"):
            if k in obj:
                hit = find_url(obj[k], suffixes)
                if hit:
                    return hit
        for v in obj.values():
            hit = find_url(v, suffixes)
            if hit:
                return hit
    elif isinstance(obj, list):
        for v in obj:
            hit = find_url(v, suffixes)
            if hit:
                return hit
    return None


def main():
    p = argparse.ArgumentParser(description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter)
    p.add_argument("prompt")
    p.add_argument("--name", required=True)
    p.add_argument("--type", choices=["character", "npc", "equipment", "terrain_piece", "particle_mesh"],
                   help="wrap the prompt in the locked style for this type (Docs/styles.json)")
    p.add_argument("--model", default=DEFAULT_MODEL, help="fal endpoint id")
    p.add_argument("--pbr", action="store_true", help="enable_pbr (+$0.15)")
    p.add_argument("--field", action="append", default=[], metavar="KEY=VALUE", help="extra input field")
    p.add_argument("--timeout", type=int, default=900)
    p.add_argument("--allow-host", default=None)
    p.add_argument("--dry-run", action="store_true")
    a = p.parse_args()
    g.check_name(a.name)
    prompt = g.apply_style(a.prompt, a.type)
    if len(prompt.encode("utf-8")) > 1024:
        sys.exit("prompt is over 1024 bytes")
    body = {"prompt": prompt}
    if a.pbr:
        body["enable_pbr"] = True
    for kv in a.field:
        k, sep, v = kv.partition("=")
        if not sep or not k:
            sys.exit(f"--field wants KEY=VALUE, got {kv!r}")
        body[k] = parse_value(v)
    if a.dry_run:
        print("POST", f"{QUEUE}/{a.model}", json.dumps(body, indent=2))
        return

    key = g.env_value("FAL3D_API_KEY", "FAL_KEY", "FAL_API_KEY")
    sub = g.request("POST", f"{QUEUE}/{a.model}", key, json_body=body, scheme="Key")
    rid = sub.get("request_id")
    status_url, response_url = sub.get("status_url"), sub.get("response_url")
    if not (rid and status_url and response_url):
        sys.exit("unexpected submit response: " + json.dumps(sub)[:300])
    for u in (status_url, response_url):
        g.check_url(u, HOSTS, a.allow_host)
    print("request", rid)

    deadline = time.time() + a.timeout
    last = None
    while True:
        st = g.request("GET", status_url, key, scheme="Key")
        s = st.get("status")
        if s != last:
            pos = st.get("queue_position")
            print(f"  {s}" + (f" (queue position {pos})" if pos is not None else ""), flush=True)
            last = s
        if s == "COMPLETED":
            if st.get("error"):
                sys.exit(f"fal reported an error: {str(st.get('error'))[:300]}")
            break
        if s not in ("IN_QUEUE", "IN_PROGRESS"):
            sys.exit(f"unexpected status: {json.dumps(st)[:300]}")
        if time.time() > deadline:
            sys.exit(f"timed out; request {rid} may still finish")
        time.sleep(5)

    res = g.request("GET", response_url, key, scheme="Key")
    glb = find_url(res, (".glb",))
    if not glb:
        sys.exit("no .glb in the result; keys: " + ", ".join(res.keys() if isinstance(res, dict) else []))
    dest = g.ROOT / "ImportSource" / "Fal" / f"{a.name}.glb"
    g.safe_download(glb, dest, HOSTS, a.allow_host)
    thumb = find_url(res.get("thumbnail") if isinstance(res, dict) else None, (".png", ".jpg", ".webp"))
    if thumb:
        try:
            g.safe_download(thumb, dest.with_suffix("." + thumb.split("?")[0].rsplit(".", 1)[1].lower()),
                            HOSTS, a.allow_host)
        except SystemExit as e:
            print("thumbnail skipped:", e)
    g.log_manifest("Fal", {"file": dest.name, "prompt": prompt, "model": a.model, "request_id": rid,
                           "input": {k: v for k, v in body.items() if k != "prompt"}, "seed": res.get("seed")})
    print("saved", dest)


if __name__ == "__main__":
    main()
