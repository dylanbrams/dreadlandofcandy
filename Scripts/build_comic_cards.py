"""Bake editable multilingual action-card lettering onto AI-created blank templates.

Run: python Scripts/build_comic_cards.py
Requires Pillow and fontTools. Fonts are read from Windows, never redistributed.
"""
import hashlib
import json
from pathlib import Path

from PIL import Image, ImageChops, ImageDraw, ImageFilter, ImageFont
from fontTools.ttLib import TTFont

ROOT = Path(__file__).resolve().parents[1]
DEST = ROOT / "Assets/Incoming/2D/ComicCards"
SIZE = 1024


def render(entry):
    template = DEST / "Templates" / (entry["template"] + ".png")
    image = Image.open(template).convert("RGBA").resize((SIZE, SIZE), Image.Resampling.LANCZOS)
    alpha = image.getchannel("A")
    assert alpha.getextrema() == (0, 255), f"Missing transparency: {template}"
    # Broaden the burst's central body so long words stay readable in flight.
    # Original generated rays remain; this editable central polygon has no tail.
    silhouette = alpha.point(lambda p: 255 if p > 128 else 0)
    ImageDraw.Draw(silhouette).polygon([(225, 385), (300, 335), (725, 335),
        (800, 385), (800, 650), (725, 700), (300, 700), (225, 650)], fill=255)
    alpha = ImageChops.lighter(alpha, silhouette)
    image = Image.new("RGBA", (SIZE, SIZE), "#17121C")
    image.putalpha(alpha)
    interior = silhouette.filter(ImageFilter.MinFilter(15))
    # Tint only the ivory region; keep the original black outside contour and alpha.
    inner = interior.filter(ImageFilter.MinFilter(25))
    paper = inner.filter(ImageFilter.MinFilter(13))
    image.paste(entry["outline"], (0, 0), interior)
    image.paste(entry["accent"], (0, 0), inner)
    image.paste("#FFF5DA", (0, 0), paper)
    image.putalpha(alpha)
    safe = paper.filter(ImageFilter.MinFilter(25))
    bounds = interior.getbbox()
    cx, cy = (bounds[0] + bounds[2]) / 2, (bounds[1] + bounds[3]) / 2
    font_path = Path("C:/Windows/Fonts") / entry["font"]
    font_data = TTFont(font_path, fontNumber=0)
    cmap = font_data.getBestCmap()
    missing = [c for c in entry["text"] if not c.isspace() and ord(c) not in cmap]
    assert not missing, f"Unsupported glyphs in {entry['id']}: {missing}"
    font_data.close()
    # Fit the entire text bounding rectangle within the blank interior, including accents.
    for size in range(240, 39, -2):
        font = ImageFont.truetype(str(font_path), size)
        probe = ImageDraw.Draw(image)
        box = probe.multiline_textbbox((0, 0), entry["text"], font=font, spacing=8, align="center", stroke_width=1)
        w, h = box[2] - box[0], box[3] - box[1]
        rect = (round(cx-w/2-9), round(cy-h/2-9), round(cx+w/2+9), round(cy+h/2+9))
        if safe.crop(rect).getextrema() == (255, 255):
            break
    else:
        raise ValueError(f"Cannot fit {entry['id']}")
    ImageDraw.Draw(image).multiline_text((cx-w/2-box[0], cy-h/2-box[1]), entry["text"],
        font=font, fill="#17121C", spacing=8, align="center", stroke_width=1, stroke_fill="#17121C")
    path = DEST / f"T_Comic_{entry['id']}.png"
    image.save(path)
    assert image.getchannel("A").getextrema() == (0, 255)
    assert image.getchannel("A").getbbox()[0] > 0
    return {**entry, "file": path.name, "pixels": [SIZE, SIZE], "font_size": size,
        "alpha_bounds": image.getchannel("A").getbbox(), "text_bounds": rect,
        "sha256": hashlib.sha256(path.read_bytes()).hexdigest(),
        "suggested_unreal_path": f"/Game/FX/Attacks/Textures/T_Comic_{entry['id']}"}


def main():
    roster = json.loads((DEST / "roster.json").read_text(encoding="utf-8"))
    records = [render(entry) for entry in roster]
    (DEST / "manifest.json").write_text(json.dumps({"cards": records,
        "status": "Source PNG delivery; Unreal import and gameplay integration pending",
        "tool": "Built-in image_gen blank templates; deterministic Pillow lettering and colors",
        "translation_review": "Thematic translations authored for this set; native-speaker review not performed",
        "color_space": "sRGB", "alpha": "straight RGBA", "origin": "top-left",
        "fonts": "Windows installed fonts rasterized only; font binaries are not included"}, ensure_ascii=False, indent=2)+"\n", encoding="utf-8")
    # Contact sheet is a review artifact, never a particle texture.
    sheet = Image.new("RGB", (1200, 1050), "#303742")
    draw = ImageDraw.Draw(sheet)
    label = ImageFont.truetype("C:/Windows/Fonts/NotoSans-Regular.ttf", 16)
    for index, record in enumerate(records):
        x, y = (index % 4)*300, (index // 4)*350
        card = Image.open(DEST / record["file"]).convert("RGBA")
        card.thumbnail((300, 300), Image.Resampling.LANCZOS)
        sheet.paste(card, (x, y), card)
        draw.text((x+12, y+300), record["id"], font=label, fill="white")
    sheet.save(DEST / "preview.jpg", quality=94)
    print(f"Built {len(records)} RGBA cards; glyph coverage and text fit verified.")


if __name__ == "__main__":
    main()
