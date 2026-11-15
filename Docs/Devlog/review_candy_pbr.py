"""Build compact contact sheets and a browser review without loading Unreal."""
from pathlib import Path
import json
from PIL import Image, ImageDraw
ROOT=Path(__file__).resolve().parent/'CandyPBR'
rows=[]
for folder in sorted(ROOT.iterdir()):
 if not folder.is_dir() or not (folder/'result.json').exists():continue
 maps=sorted(folder.glob('*.png'))
 preview=next(iter(folder.glob('preview_*.png')),None) or next(iter(folder.glob('basecolor_*.png')),None)
 if not preview:continue
 rows.append((folder,preview))
sheet=Image.new('RGB',(900,((len(rows)+2)//3)*330),(35,35,40))
draw=ImageDraw.Draw(sheet)
for i,(folder,preview) in enumerate(rows):
 im=Image.open(preview).convert('RGB'); im.thumbnail((290,290))
 x=(i%3)*300;y=(i//3)*330
 sheet.paste(im,(x,y));draw.text((x+5,y+295),folder.name,fill='white')
if rows:sheet.save(ROOT/'contact.jpg')
html='<html><body style="background:#222;color:white;font:16px sans-serif"><h1>Candy PBR variants</h1>'
for folder,preview in rows:
 html+=f'<h2>{folder.name}</h2>'
 for path in sorted(folder.glob('*.png')):
  html+=f'<figure style="display:inline-block;margin:8px"><img width="220" src="{folder.name}/{path.name}"><figcaption>{path.stem}</figcaption></figure>'
html+='</body></html>'
(ROOT/'review.html').write_text(html)
print(f'{len(rows)} completed sets in review')
