"""Generate isolated marshmallow source meshes with the project's fal queue helpers."""
import json
import sys
import time
from pathlib import Path

ROOT = Path(__file__).resolve().parents[3]
OUT = Path(__file__).resolve().parent
sys.path.insert(0, str(ROOT / "Scripts"))
import genlib as g
from fal3d import DEFAULT_MODEL, HOSTS, find_url

PROMPTS = {
    "MM_CreamClassic": "A single small realistic plain cream-white cylindrical marshmallow, squat pillow shape with softly rounded corners and a slightly domed top, gently squashed under its own weight, subtle natural asymmetry and shallow wrinkles, fine matte powdered sugar surface, soft studio diffuse lighting, isolated 3D product asset, full object visible, no props, no plate, no ground plane, no text, no face, no cartoon styling",
    "MM_TwistedCream": "A single small realistic cream-white marshmallow with a gentle uneven twist along its short axis, soft pillow-like cylindrical body with rounded ends, subtly pinched waist and organic asymmetry, fine matte powdered sugar dusting, believable soft confection texture, isolated 3D product asset, full object visible, no props, no plate, no ground plane, no text, no face, no cartoon styling",
    "MM_SquashedCream": "A single small realistic cream-white marshmallow, noticeably squat and softly flattened, plump oval pillow cylinder with naturally lopsided sides, pillowy bulges and shallow compression dents, rounded edges, very subtle matte powdered sugar surface, isolated 3D product asset, full object visible, no props, no plate, no ground plane, no text, no face, no cartoon styling",
}

def main():
    key = g.env_value("FAL3D_API_KEY", "FAL_KEY", "FAL_API_KEY")
    for name, prompt in PROMPTS.items():
        body = {"prompt": prompt, "enable_pbr": True}
        print("submitting", name, flush=True)
        sub = g.request("POST", f"{g.QUEUE if hasattr(g, 'QUEUE') else 'https://queue.fal.run'}/{DEFAULT_MODEL}", key, json_body=body, scheme="Key")
        rid = sub.get("request_id")
        status_url, response_url = sub.get("status_url"), sub.get("response_url")
        if not (rid and status_url and response_url):
            raise RuntimeError("unexpected fal submit response")
        for u in (status_url, response_url):
            g.check_url(u, HOSTS)
        print("request id", rid, flush=True)
        deadline = time.time() + 900
        while True:
            st = g.request("GET", status_url, key, scheme="Key")
            status = st.get("status")
            print(name, status, flush=True)
            if status == "COMPLETED":
                break
            if status not in ("IN_QUEUE", "IN_PROGRESS"):
                raise RuntimeError(f"fal status: {status}")
            if time.time() > deadline:
                raise TimeoutError(name)
            time.sleep(5)
        res = g.request("GET", response_url, key, scheme="Key")
        glb = find_url(res, (".glb",))
        if not glb:
            raise RuntimeError("fal response had no GLB")
        dest = OUT / f"{name}.glb"
        g.safe_download(glb, dest, HOSTS)
        thumb = find_url(res.get("thumbnail"), (".png", ".jpg", ".webp"))
        if thumb:
            suffix = thumb.split("?")[0].rsplit(".", 1)[-1].lower()
            g.safe_download(thumb, dest.with_suffix("." + suffix), HOSTS)
        print("saved", dest.name, "size", dest.stat().st_size, flush=True)
        (OUT / f"{name}.metadata.json").write_text(json.dumps({"name": name, "model": DEFAULT_MODEL, "request_id": rid, "seed": res.get("seed"), "prompt": prompt, "pbr": True}, indent=2), encoding="utf-8")

if __name__ == "__main__":
    main()
