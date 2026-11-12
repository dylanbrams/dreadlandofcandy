"""Label the rendered foliage candidates for a single human review."""
import json
from pathlib import Path
from PIL import Image, ImageDraw, ImageFont

HERE = Path(__file__).resolve().parent
assets = json.loads((HERE / "prepared_manifest.json").read_text(encoding="utf-8"))["assets"]
canvas = Image.new("RGB", (1600, 940), "#edf0f2")
draw = ImageDraw.Draw(canvas)
title_font = ImageFont.truetype("C:/Windows/Fonts/arial.ttf", 27)
font = ImageFont.truetype("C:/Windows/Fonts/arial.ttf", 19)
small = ImageFont.truetype("C:/Windows/Fonts/arial.ttf", 17)
draw.text((22, 14), "Hyper3D candy foliage — eight generated candidates", fill="#172833", font=title_font)
for i, asset in enumerate(assets):
    x, y = (i % 4) * 400, 62 + (i // 4) * 435
    image = Image.open(HERE / "Review" / (asset["name"] + ".png")).convert("RGB")
    image.thumbnail((390, 370))
    canvas.paste(image, (x + (400 - image.width) // 2, y))
    name = asset["name"].replace("SM_Rodin_", "")
    draw.text((x + 15, y + 373), name, fill="#172833", font=font)
    draw.text((x + 15, y + 401), f"{asset['triangles']:,} triangles | {asset['height_cm']} cm tall", fill="#3d525e", font=small)
canvas.save(HERE / "Review/contact-sheet.png")
print(HERE / "Review/contact-sheet.png")
