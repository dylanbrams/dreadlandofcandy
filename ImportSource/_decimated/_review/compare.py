"""Compare source/export connectivity and embedded texture pixels, without modifying GLBs."""
import io, json, struct, hashlib
from pathlib import Path
import numpy as np
from PIL import Image, ImageDraw

OUT=Path(__file__).resolve().parent
ROOT=OUT.parents[2]

def load(path):
    data=path.read_bytes(); length=struct.unpack_from('<I',data,12)[0]
    doc=json.loads(data[20:20+length]); binary=data[28+length:]
    def accessor(index):
        a=doc['accessors'][index]; v=doc['bufferViews'][a['bufferView']]
        dtype={5126:'<f4',5125:'<u4',5123:'<u2',5121:'u1'}[a['componentType']]
        width={'SCALAR':1,'VEC2':2,'VEC3':3,'VEC4':4}[a['type']]
        item=np.dtype(dtype).itemsize
        return np.ndarray((a['count'],width),dtype=dtype,buffer=binary,
            offset=v.get('byteOffset',0)+a.get('byteOffset',0),strides=(v.get('byteStride',width*item),item))
    stats=[]
    for mesh in doc.get('meshes',[]):
        for p in mesh['primitives']:
            verts=accessor(p['attributes']['POSITION'])
            indices=accessor(p['indices']).reshape(-1,3) if 'indices' in p else np.arange(len(verts)).reshape(-1,3)
            # Exact coordinates identify glTF attribute splits without merging nearby surfaces.
            _,mapping=np.unique(verts,axis=0,return_inverse=True)
            tri=mapping[indices]
            edges=np.sort(np.concatenate((tri[:,[0,1]],tri[:,[1,2]],tri[:,[2,0]])),axis=1)
            _,counts=np.unique(edges,axis=0,return_counts=True)
            stats.append({'triangles':len(tri),'exact_weld_boundary_edges':int(np.sum(counts==1)),
                          'exact_weld_nonmanifold_edges':int(np.sum(counts>2))})
    images=[]
    for m in doc.get('materials',[]):
        t=m.get('pbrMetallicRoughness',{}).get('baseColorTexture')
        if not t: continue
        im=doc['images'][doc['textures'][t['index']]['source']]
        view=doc['bufferViews'][im['bufferView']]; start=view.get('byteOffset',0)
        raw=binary[start:start+view['byteLength']]
        decoded=Image.open(io.BytesIO(raw)).convert('RGBA')
        images.append({'size':list(decoded.size),'pixels_sha256':hashlib.sha256(decoded.tobytes()).hexdigest()})
    return {'primitives':stats,'base_color_images':images,'skins':len(doc.get('skins',[])),
            'animations':len(doc.get('animations',[]))}

rows=[]
for path in sorted(OUT.parent.glob('*/*.glb')):
    rel=path.relative_to(OUT.parent)
    source=load(ROOT/'ImportSource'/rel); exported=load(path)
    row={'file':str(rel),'source':source,'decimated':exported,
         'base_color_pixels_identical':source['base_color_images']==exported['base_color_images']}
    rows.append(row)
    print(str(rel), 'boundaries',sum(p['exact_weld_boundary_edges'] for p in source['primitives']),
          '->',sum(p['exact_weld_boundary_edges'] for p in exported['primitives']),
          'same texture',row['base_color_pixels_identical'],flush=True)
(OUT/'comparison.json').write_text(json.dumps(rows,indent=2),encoding='utf-8')

if (OUT/'audit.json').exists():
    audit=json.loads((OUT/'audit.json').read_text())
    for start in range(0,len(audit),5):
        batch=audit[start:start+5]
        sheet=Image.new('RGB',(1200,len(batch)*330),(45,45,45)); draw=ImageDraw.Draw(sheet)
        for y,row in enumerate(batch):
            draw.text((8,y*330+5),row['file'],fill='white')
            for x,name in enumerate(row['views']):
                im=Image.open(OUT/name).convert('RGB'); im.thumbnail((300,300))
                sheet.paste(im,(x*300,y*330+25))
        sheet.save(OUT/('sheet_%02d.jpg'%(start//5+1)),quality=95)
