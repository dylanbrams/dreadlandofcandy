#!/usr/bin/env python3
"""Generate one 2D game image with the OpenAI Images API; optionally key out the background and trim it.

Key: OPENAI_API_KEY (environment or git-ignored project-root .env). Never printed.
Model: --model, else OPENAI_IMAGE_MODEL from the environment/.env, else gpt-image-2.5-sunburst.
With --ref images it calls /v1/images/edits (the refs are style references); without, /v1/images/generations.
Output: Assets/Incoming/2D/<name>/<name>.png (an older file is kept as <name>_v1.png, _v2...),
the untouched API image as <name>.raw.png, and a line in ImportSource/OpenAI/manifest.jsonl.

  python -I Scripts/openai_image.py --name T_Skill_Lecture --prompt-file prompt.txt --size 1024x1024
  python -I Scripts/openai_image.py --name T_Loot_Ring --prompt "..." --ref Assets/Incoming/2D/T_Skill_Lecture/T_Skill_Lecture.png
  python -I Scripts/openai_image.py --name T_Loot_Ring --prompt "..." --from-file C:/somewhere/codex_out.png
      (no API call: ingest an image another tool made, e.g. Codex's own image generation, with the same
       versioning, keying, trimming and manifest line)
"""
import argparse
import base64
import json
import os
import sys
from pathlib import Path

sys.path.insert(0, str(Path(__file__).resolve().parent))  # -I drops the script dir
import genlib as g

API = "https://api.openai.com/v1/images"
DEFAULT_MODEL = "gpt-image-2.5-sunburst"


def model_name(arg):
    if arg:
        return arg
    if os.environ.get("OPENAI_IMAGE_MODEL"):
        return os.environ["OPENAI_IMAGE_MODEL"]
    f = g.ROOT / ".env"
    if f.is_file():
        for line in f.read_text(encoding="utf-8").splitlines():
            k, sep, v = line.strip().partition("=")
            if sep and k.strip() == "OPENAI_IMAGE_MODEL" and v.strip():
                return v.strip().strip("\"'")
    return DEFAULT_MODEL


def postprocess(path, key_mode, trim, max_size, pad=8):
    """Key a flat background (if the image has no real transparency), trim to content, cap the size."""
    from PIL import Image
    import numpy as np
    im = Image.open(path).convert("RGBA")
    px = np.asarray(im).copy()
    alpha = px[..., 3]
    has_alpha = (alpha < 250).mean() > 0.02
    if key_mode == "auto" and not has_alpha:
        h, w = alpha.shape
        corners = np.array([px[2, 2, :3], px[2, w - 3, :3], px[h - 3, 2, :3], px[h - 3, w - 3, :3]], dtype=float)
        bg = np.median(corners, axis=0)
        if np.abs(corners - bg).max() < 40:  # corners agree: a flat background worth keying
            d = np.sqrt(((px[..., :3].astype(float) - bg) ** 2).sum(-1))
            a = np.clip((d - 30) / 40, 0, 1)  # soft edge between 30 and 70
            px[..., 3] = (np.minimum(alpha / 255.0, a) * 255).astype(np.uint8)
            if bg[1] > 150 and bg[0] < 120 and bg[2] < 120:  # green key: pull green spill off the edges
                spill = (px[..., 1].astype(int) - np.maximum(px[..., 0], px[..., 2]).astype(int)).clip(0)
                edge = (px[..., 3] > 0) & (px[..., 3] < 255)
                px[..., 1] = np.where(edge, px[..., 1] - spill, px[..., 1]).astype(np.uint8)
            im = Image.fromarray(px, "RGBA")
    if trim:
        bbox = im.getchannel("A").point(lambda v: 255 if v > 8 else 0).getbbox()
        if bbox:
            l, t, r, b = bbox
            im = im.crop((max(l - pad, 0), max(t - pad, 0), min(r + pad, im.width), min(b + pad, im.height)))
    if max_size and max(im.size) > max_size:
        im.thumbnail((max_size, max_size), Image.LANCZOS)
    im.save(path)
    return im.size


def main():
    p = argparse.ArgumentParser(description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter)
    src = p.add_mutually_exclusive_group(required=True)
    src.add_argument("--prompt")
    src.add_argument("--prompt-file")
    p.add_argument("--name", required=True)
    p.add_argument("--ref", action="append", default=[], help="style reference image (repeatable, up to 16)")
    p.add_argument("--model", default=None)
    p.add_argument("--size", default="1024x1024")
    p.add_argument("--quality", default="medium", choices=["low", "medium", "high", "xhigh", "max", "auto"])
    p.add_argument("--background", default="transparent", choices=["transparent", "opaque", "auto"])
    p.add_argument("--key", default="auto", choices=["auto", "none"],
                   help="auto: if the result has no transparency, key out a flat background colour")
    p.add_argument("--no-trim", action="store_true", help="keep the full canvas (frames, paper, posters)")
    p.add_argument("--max-size", type=int, default=1024, help="longest side after trimming (0 = keep)")
    p.add_argument("--out-dir", default="Assets/Incoming/2D")
    p.add_argument("--timeout", type=int, default=400)
    p.add_argument("--from-file", default=None, help="ingest this image instead of calling the API")
    p.add_argument("--source", default="codex", help="manifest label for --from-file images")
    p.add_argument("--dry-run", action="store_true")
    a = p.parse_args()
    g.check_name(a.name)
    prompt = a.prompt if a.prompt is not None else (g.ROOT / a.prompt_file).read_text(encoding="utf-8").strip()
    model = model_name(a.model)
    refs = [Path(r) if os.path.isabs(r) else g.ROOT / r for r in a.ref]
    if len(refs) > 16:
        sys.exit("at most 16 reference images")
    for r in refs:
        if not r.is_file():
            sys.exit(f"reference image not found: {r}")
        g.validate_image(r)

    fields = {"model": model, "prompt": prompt, "size": a.size, "quality": a.quality,
              "background": a.background, "output_format": "png", "n": 1}
    if a.from_file:
        src_img = Path(a.from_file) if os.path.isabs(a.from_file) else g.ROOT / a.from_file
        if not src_img.is_file():
            sys.exit(f"--from-file not found: {src_img}")
        g.validate_image(src_img)
        if src_img.resolve() == (g.ROOT / a.out_dir / a.name / f"{a.name}.png").resolve():
            sys.exit("--from-file is the output path itself; save the new image somewhere else first")
        from PIL import Image
        import io
        buf = io.BytesIO()
        Image.open(src_img).save(buf, "PNG")
        data = base64.b64encode(buf.getvalue()).decode()
        res = {"usage": None}
        model = a.source
    elif a.dry_run:
        print("POST", f"{API}/{'edits' if refs else 'generations'}",
              json.dumps({**fields, "refs": [str(r) for r in refs]}, indent=2))
        return

    if a.from_file:
        pass
    elif refs:
        key = g.env_value("OPENAI_API_KEY")
        files = [("image[]", r.name, r.read_bytes(),
                  "image/png" if r.suffix.lower() == ".png" else
                  "image/webp" if r.suffix.lower() == ".webp" else "image/jpeg") for r in refs]
        body, ctype = g.multipart({k: str(v) for k, v in fields.items()}, files)
        res = g.request("POST", f"{API}/edits", key, raw_body=body, content_type=ctype, timeout=a.timeout)
    else:
        key = g.env_value("OPENAI_API_KEY")
        res = g.request("POST", f"{API}/generations", key, json_body=fields, timeout=a.timeout)
    if not a.from_file:
        data = (res.get("data") or [{}])[0].get("b64_json")
    if not data:
        sys.exit("response had no image: " + json.dumps({k: v for k, v in res.items() if k != "data"})[:300])

    out = g.ROOT / a.out_dir / a.name
    out.mkdir(parents=True, exist_ok=True)
    dest = out / f"{a.name}.png"
    if dest.exists():
        n = 1
        while (out / f"{a.name}_v{n}.png").exists():
            n += 1
        dest.replace(out / f"{a.name}_v{n}.png")
        print("kept the previous image as", f"{a.name}_v{n}.png")
    raw = out / f"{a.name}.raw.png"
    tmp = out / f"{a.name}.png.part"
    tmp.write_bytes(base64.b64decode(data))
    try:
        g.validate_image(tmp)
    except SystemExit:
        tmp.unlink(missing_ok=True)
        raise
    raw.write_bytes(tmp.read_bytes())
    tmp.replace(dest)
    size = postprocess(dest, a.key, not a.no_trim, a.max_size)
    g.log_manifest("OpenAI", {"file": f"{a.name}/{dest.name}", "model": model, "prompt": prompt,
                              "refs": [r.relative_to(g.ROOT).as_posix() if r.is_relative_to(g.ROOT) else r.name
                                       for r in refs],
                              "size": a.size, "quality": a.quality, "final_px": list(size),
                              "usage": res.get("usage")})
    print("saved", dest, "final size", size)


if __name__ == "__main__":
    main()
