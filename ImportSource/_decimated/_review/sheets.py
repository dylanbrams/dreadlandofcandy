import json,sys
from pathlib import Path
from PIL import Image, ImageDraw
out=Path(__file__).resolve().parent
if len(sys.argv)>1:out=out/sys.argv[1]
audit=json.loads((out/'audit.json').read_text())
for start in range(0,len(audit),5):
    batch=audit[start:start+5]
    sheet=Image.new('RGB',(1200,len(batch)*330),(45,45,45)); draw=ImageDraw.Draw(sheet)
    for y,row in enumerate(batch):
        draw.text((8,y*330+5),row['file'],fill='white')
        for x,name in enumerate(row['views']):
            im=Image.open(out/name).convert('RGB'); im.thumbnail((300,300))
            sheet.paste(im,(x*300,y*330+25))
    sheet.save(out/('sheet_%02d.jpg'%(start//5+1)),quality=95)
print('Sheet assets:',len(audit))
for name in ('SM_Fenella','SM_Sword'):
    source=out/'source'/('Hyper3D_'+name+'_0.png')
    if not source.exists(): continue
    sheet=Image.new('RGB',(800,430),(45,45,45)); draw=ImageDraw.Draw(sheet)
    draw.text((8,8),name+' source',fill='white')
    draw.text((408,8),name+' first decimation',fill='white')
    sheet.paste(Image.open(source).convert('RGB'),(0,30))
    sheet.paste(Image.open(out/('Hyper3D_'+name+'_0.png')).convert('RGB'),(400,30))
    sheet.save(out/(name+'_comparison.jpg'),quality=95)
