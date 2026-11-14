"""Build the second comic-card batch without adding a central panel.

python Scripts/build_angular_comic_cards.py
Requires Pillow, fontTools and the installed Windows fonts named in roster.json.
"""
import hashlib
import json
from functools import lru_cache
from pathlib import Path

from PIL import Image, ImageChops, ImageDraw, ImageFilter, ImageFont
from fontTools.ttLib import TTFont

ROOT = Path(__file__).resolve().parents[1]
DEST = ROOT / "Assets/Incoming/2D/ComicCards/Angular"
SIZE = 1024


@lru_cache(maxsize=4)
def template_masks(name):
    im = Image.open(DEST / "Templates" / f"{name}.png").convert("RGBA")
    im = im.resize((896, 896), Image.Resampling.LANCZOS)
    padded = Image.new("RGBA", (SIZE, SIZE))
    padded.paste(im, (64, 64))
    im = padded
    alpha = im.getchannel("A")
    assert alpha.getextrema() == (0, 255), f"Missing alpha: {name}"
    # Preserve the generated silhouette exactly: never union a rectangle into it.
    silhouette = alpha.point(lambda p: 255 if p > 128 else 0)
    rim = silhouette.filter(ImageFilter.MinFilter(15))
    paper = rim.filter(ImageFilter.MinFilter(27))
    safe = paper.filter(ImageFilter.MinFilter(15))
    return alpha, rim, paper, safe


def lettering(entry, safe):
    font_path = Path("C:/Windows/Fonts") / entry["font"]
    ft = TTFont(font_path, fontNumber=0)
    cmap = ft.getBestCmap()
    assert all(c.isspace() or ord(c) in cmap for c in entry["text"]), entry["id"]
    ft.close()
    bounds = safe.getbbox()
    cx, cy = (bounds[0]+bounds[2])/2, (bounds[1]+bounds[3])/2
    unsafe = ImageChops.invert(safe)
    for size in range(264, 35, -4):
        font = ImageFont.truetype(str(font_path), size)
        mask = Image.new("L", (SIZE, SIZE))
        draw = ImageDraw.Draw(mask)
        box = draw.multiline_textbbox((0, 0), entry["text"], font=font,
            spacing=4, align="center", stroke_width=1)
        xy = (cx-(box[2]-box[0])/2-box[0], cy-(box[3]-box[1])/2-box[1])
        draw.multiline_text(xy, entry["text"], font=font, spacing=4,
            align="center", fill=255, stroke_width=1, stroke_fill=255)
        mask = mask.rotate(entry["angle"], Image.Resampling.BICUBIC)
        if ImageChops.multiply(mask, unsafe).getbbox() is None:
            break
    else:
        raise ValueError(f"Cannot fit {entry['id']}")
    layer = Image.new("RGBA", (SIZE, SIZE))
    ImageDraw.Draw(layer).multiline_text(xy, entry["text"], font=font,
        spacing=4, align="center", fill=entry["ink"], stroke_width=1,
        stroke_fill="#17121C")
    return layer.rotate(entry["angle"], Image.Resampling.BICUBIC), size


def render(entry):
    alpha, rim, paper, safe = template_masks(entry["template"])
    im = Image.new("RGBA", (SIZE, SIZE), "#17121C")
    im.paste(entry["outline"], (0, 0), rim)
    im.paste(entry["fill"], (0, 0), paper)
    im.putalpha(alpha)
    letters, font_size = lettering(entry, safe)
    im = Image.alpha_composite(im, letters)
    # All visible text remains in the existing silhouette, including rotated corners.
    outside = alpha.point(lambda p: 255 if p == 0 else 0)
    assert ImageChops.multiply(letters.getchannel("A"), outside).getbbox() is None
    im.putalpha(alpha)
    bounds = alpha.getbbox()
    assert bounds[0] > 0 and bounds[1] > 0 and bounds[2] < SIZE and bounds[3] < SIZE
    path = DEST / f"T_Comic_{entry['id']}.png"
    im.save(path)
    return {**entry, "file": path.name, "font_size": font_size,
        "pixels": [SIZE, SIZE], "alpha_bounds": bounds,
        "text_bounds": letters.getbbox(), "sha256": hashlib.sha256(path.read_bytes()).hexdigest(),
        "suggested_unreal_path": f"/Game/FX/Attacks/Textures/{path.stem}"}


def preview(records, path):
    sheet = Image.new("RGB", (1200, 960), "#303742")
    draw = ImageDraw.Draw(sheet)
    font = ImageFont.truetype("C:/Windows/Fonts/NotoSans-Regular.ttf", 16)
    for i, entry in enumerate(records):
        x, y = (i % 4)*300, (i // 4)*320
        im = Image.open(DEST / entry["file"]).convert("RGBA")
        im.thumbnail((300, 280), Image.Resampling.LANCZOS)
        sheet.paste(im, (x+(300-im.width)//2, y), im)
        draw.text((x+12, y+282), entry["id"], font=font, fill="white")
    sheet.save(path, quality=95)


def main():
    roster = json.loads((DEST / "roster.json").read_text(encoding="utf-8"))
    records = []
    for entry in roster:
        records.append(render(entry))
        print(f"Built {entry['id']}", flush=True)
    manifest = {"cards": records, "tool": "Built-in image_gen blank templates; Pillow lettering and colors",
        "silhouette_policy": "Generated contours with transparent canvas padding; no central rectangle or body polygon",
        "alpha": "straight RGBA", "color_space": "sRGB",
        "translation_review": "Inherited thematic translations; native-speaker review not performed",
        "status": "PNG sources only; Unreal import and effects integration pending"}
    (DEST / "manifest.json").write_text(json.dumps(manifest, indent=2, ensure_ascii=False)+"\n", encoding="utf-8")
    preview(records[:12], DEST / "preview-a.jpg")
    preview(records[12:], DEST / "preview-b.jpg")
    print(f"Validated {len(records)} cards: transparency, padding, glyphs and rotated lettering containment.")


if __name__ == "__main__":
    main()
