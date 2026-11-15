"""Assemble the Blender renders into a labeled presentation card (Pillow)."""
from pathlib import Path
from PIL import Image, ImageDraw, ImageFont

here = Path(__file__).resolve().parent / 'assets'
items = [('rat_meshy.png', 'MESHY', 'Original rat candidate'),
         ('rat_rodin.png', 'HYPER3D RODIN', 'Original rat candidate'),
         ('rat_hunyuan.png', 'HUNYUAN3D / FAL', 'Rat; revised prompt'),
         ('tree_tripo.png', 'TRIPO', 'Cleaned tree; different subject')]
canvas = Image.new('RGB', (1600, 550), '#24182a')
draw = ImageDraw.Draw(canvas)
font = ImageFont.truetype('C:/Windows/Fonts/arialbd.ttf', 25)
small = ImageFont.truetype('C:/Windows/Fonts/arial.ttf', 18)
for i, (file, label, detail) in enumerate(items):
    x = i * 400
    image = Image.open(here / file).convert('RGB').resize((380, 380), Image.Resampling.LANCZOS)
    canvas.paste(image, (x + 10, 10))
    draw.text((x + 14, 408), label, font=font, fill='#ffe7c8')
    draw.text((x + 14, 445), detail, font=small, fill='#dcbcae')
draw.text((14, 510), 'BLENDER SOURCE-ASSET RENDERS  /  NOT A CONTROLLED FOUR-WAY TEST', font=small, fill='#e9be92')
canvas.save(here / 'generation-comparison.jpg', quality=92)
