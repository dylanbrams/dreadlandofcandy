"""Read-only asset checks and labelled contact sheets for the batch1 handoff."""
import io
import json
import struct
from pathlib import Path

from PIL import Image, ImageDraw

ROOT = Path(__file__).resolve().parents[3]
OUT = Path(__file__).resolve().parent
DOC = json.loads((ROOT / 'Assets/Generation/jobs.json').read_text(encoding='utf-8'))
STATE = json.loads((ROOT / 'ImportSource/_gen/state.json').read_text(encoding='utf-8'))
DIRS = {'rodin': 'Hyper3D', 'fal': 'Fal', 'meshy': 'Meshy'}
mesh_stats = []
image_stats = []
cards = []

for job in DOC['jobs']:
    if job['stage'] != 'batch1':
        continue
    name = job['id']
    record = STATE['jobs'].get(name, {})
    if job['tool'] == 'image':
        path = ROOT / 'Assets/Incoming/2D' / name / (name + '.png')
        if not path.exists():
            path = ROOT / 'Assets/Incoming/2D/_drop' / (name + '.png')
        if not path.exists():
            continue
        im = Image.open(path).convert('RGBA')
        alpha = im.getchannel('A')
        image_stats.append({'id': name, 'size': list(im.size), 'alpha_extrema': alpha.getextrema(),
                            'visible_bounds': alpha.getbbox(), 'kind': job.get('kind', 'icon')})
        cards.append(('2d', name, im.copy()))
        continue
    tool = record.get('tool') or STATE.get('routes', {}).get(job['group'], job['tool'])
    path = ROOT / 'ImportSource' / DIRS[tool] / (name + '.glb')
    if not path.exists():
        continue
    data = path.read_bytes()
    magic, version, length = struct.unpack_from('<III', data)
    json_len, json_type = struct.unpack_from('<II', data, 12)
    gltf = json.loads(data[20:20 + json_len])
    binary_start = 20 + json_len + 8
    primitives = [p for m in gltf.get('meshes', []) for p in m.get('primitives', [])]
    tris = sum(gltf['accessors'][p['indices']]['count'] // 3 if 'indices' in p else
               gltf['accessors'][p['attributes']['POSITION']]['count'] // 3 for p in primitives)
    mesh_stats.append({'id': name, 'tool': tool, 'valid_header': magic == 0x46546C67 and
                       version == 2 and length == len(data), 'triangles': tris,
                       'materials': len(gltf.get('materials', [])), 'textures': len(gltf.get('textures', [])),
                       'bytes': len(data)})
    preview = next((path.with_suffix(ext) for ext in ('.png', '.webp', '.jpg')
                    if path.with_suffix(ext).exists()), None)
    if preview:
        cards.append(('3d', name, Image.open(preview).convert('RGBA')))
    for material in gltf.get('materials', [])[:1]:
        texture = material.get('pbrMetallicRoughness', {}).get('baseColorTexture', {}).get('index')
        if texture is None:
            continue
        source = gltf['textures'][texture]['source']
        view_id = gltf['images'][source].get('bufferView')
        if view_id is None:
            continue
        view = gltf['bufferViews'][view_id]
        start = binary_start + view.get('byteOffset', 0)
        atlas = Image.open(io.BytesIO(data[start:start + view['byteLength']])).convert('RGBA')
        if tool == 'rodin':
            atlas.save(OUT / (name + '_basecolor.png'))
            cards.append(('atlas', name, atlas))

for kind in ('3d', 'atlas', '2d'):
    selected = [c for c in cards if c[0] == kind]
    for page in range(0, len(selected), 9):
        subset = selected[page:page + 9]
        sheet = Image.new('RGB', (1200, 430 * ((len(subset) + 2) // 3)), '#46505b')
        draw = ImageDraw.Draw(sheet)
        for idx, (_, label, im) in enumerate(subset):
            im.thumbnail((380, 380))
            x, y = (idx % 3) * 400, (idx // 3) * 430
            sheet.paste(im, (x + (400 - im.width) // 2, y + 25), im)
            draw.text((x + 8, y + 7), label, fill='white')
        sheet.save(OUT / f'batch1_{kind}_{page // 9 + 1}.jpg', quality=90)

(OUT / 'batch1_stats.json').write_text(json.dumps({'meshes': mesh_stats, 'images': image_stats}, indent=2) + '\n', encoding='utf-8')
print(json.dumps({'meshes': mesh_stats, 'images_checked': len(image_stats)}, indent=2))
