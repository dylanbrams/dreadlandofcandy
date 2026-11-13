"""Independent saved-file validation. Run with normal Python (numpy and Pillow)."""
import hashlib,io,json,struct
from pathlib import Path
import numpy as np
from PIL import Image
OUT=Path(__file__).resolve().parent
SOURCE=OUT.parent


def inspect(path,allow_degenerate=False):
    data=path.read_bytes()
    assert struct.unpack_from('<III',data)==(0x46546C67,2,len(data))
    length=struct.unpack_from('<I',data,12)[0]
    doc=json.loads(data[20:20+length]);binary=data[28+length:]
    def accessor(index):
        a=doc['accessors'][index];view=doc['bufferViews'][a['bufferView']]
        dtype={5126:'<f4',5125:'<u4',5123:'<u2',5121:'u1'}[a['componentType']]
        width={'SCALAR':1,'VEC2':2,'VEC3':3,'VEC4':4}[a['type']];item=np.dtype(dtype).itemsize
        return np.ndarray((a['count'],width),dtype=dtype,buffer=binary,
            offset=view.get('byteOffset',0)+a.get('byteOffset',0),
            strides=(view.get('byteStride',width*item),item))
    triangles=boundary=nonmanifold=collapsed=0
    for mesh in doc.get('meshes',[]):
        positions=[];faces=[];offset=0
        for primitive in mesh['primitives']:
            assert primitive.get('mode',4)==4
            attributes=primitive['attributes'];verts=accessor(attributes['POSITION'])
            for attribute in ('POSITION','NORMAL','TEXCOORD_0'):
                assert attribute in attributes,'Missing '+attribute
                assert np.isfinite(accessor(attributes[attribute])).all(),'Invalid '+attribute
            positions.append(verts)
            indices=accessor(primitive['indices']).reshape(-1,3) if 'indices' in primitive else np.arange(len(verts)).reshape(-1,3)
            faces.append(indices.astype(np.int64)+offset);offset+=len(verts);triangles+=len(indices)
        _,mapping=np.unique(np.concatenate(positions),axis=0,return_inverse=True)
        tri=mapping[np.concatenate(faces)]
        collapsed+=int(np.sum((tri[:,0]==tri[:,1])|(tri[:,1]==tri[:,2])|(tri[:,0]==tri[:,2])))
        if not allow_degenerate:assert collapsed==0,'Collapsed geometric triangle in '+str(path)
        edges=np.sort(np.concatenate((tri[:,[0,1]],tri[:,[1,2]],tri[:,[2,0]])),axis=1)
        _,counts=np.unique(edges,axis=0,return_counts=True)
        boundary+=int(np.sum(counts==1));nonmanifold+=int(np.sum(counts>2))
    hashes=set();images=[]
    for material in doc.get('materials',[]):
        texture=material.get('pbrMetallicRoughness',{}).get('baseColorTexture')
        if not texture:continue
        image=doc['images'][doc['textures'][texture['index']]['source']]
        assert 'bufferView' in image,'External image'
        view=doc['bufferViews'][image['bufferView']];start=view.get('byteOffset',0)
        pixels=Image.open(io.BytesIO(binary[start:start+view['byteLength']])).convert('RGBA')
        digest=hashlib.sha256(pixels.tobytes()).hexdigest();hashes.add(digest)
        images.append({'size':list(pixels.size),'pixels_sha256':digest})
    return {'triangles':triangles,'boundary_edges':boundary,'nonmanifold_edges':nonmanifold,'collapsed_triangles':collapsed,
            'base_color_hashes':sorted(hashes),'base_color_images':images,
            'materials':[m.get('name','') for m in doc.get('materials',[])]}


def main():
    results=json.loads((OUT/'cleanup_results.json').read_text(encoding='utf-8'))
    report=[];failures=[]
    for rel,record in sorted(results.items()):
        try:
            source=SOURCE/rel;destination=OUT/rel
            assert hashlib.sha256(source.read_bytes()).hexdigest()==record['source_sha256'],'Original changed'
            original=inspect(source,allow_degenerate=True);cleaned=inspect(destination)
            assert cleaned['boundary_edges']==0 and cleaned['nonmanifold_edges']==0,'Nonmanifold export'
            assert cleaned['triangles']<=record['target'],'Triangle budget exceeded'
            assert set(original['base_color_hashes'])<=set(cleaned['base_color_hashes']),'Base-colour pixels changed or missing'
            report.append({'file':rel,'status':'PASS','original_unchanged':True,
                'base_color_pixels_preserved':True,'output_sha256':hashlib.sha256(destination.read_bytes()).hexdigest(),**cleaned})
            print('PASS',rel,cleaned['triangles'],'triangles; closed; textures preserved',flush=True)
        except Exception as error:
            failures.append(rel);report.append({'file':rel,'status':'FAIL','error':str(error)})
            print('FAIL',rel,str(error),flush=True)
    (OUT/'validation.json').write_text(json.dumps(report,indent=2),encoding='utf-8')
    print('Passed',len(report)-len(failures),'Failed',len(failures),flush=True)
    assert not failures,failures


if __name__=='__main__':main()
