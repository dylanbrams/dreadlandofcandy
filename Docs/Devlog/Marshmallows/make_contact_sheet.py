from pathlib import Path
from PIL import Image, ImageDraw, ImageFont

root=Path(__file__).resolve().parent
names=("MM_CreamClassic","MM_TwistedCream","MM_SquashedCream")
thumb=384; margin=18; label_h=42
canvas=Image.new("RGB",(3*thumb+4*margin,thumb+label_h+2*margin),(42,44,48))
draw=ImageDraw.Draw(canvas)
try:font=ImageFont.truetype("arial.ttf",18)
except OSError:font=ImageFont.load_default()
for i,name in enumerate(names):
    img=Image.open(root/(name+".png")).convert("RGB").resize((thumb,thumb),Image.Resampling.LANCZOS)
    x=margin+i*(thumb+margin);y=margin
    canvas.paste(img,(x,y));draw.text((x,y+thumb+8),name,fill=(240,240,240),font=font)
canvas.save(root/"Marshmallow_contact_sheet.jpg",quality=92)
