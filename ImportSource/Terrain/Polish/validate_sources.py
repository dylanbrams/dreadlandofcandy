"""Check retained transforms, authored exclusions, source provenance and budgets."""
import hashlib
import importlib.util
import json
import math
from pathlib import Path

HERE=Path(__file__).resolve().parent
ROOT=HERE.parent
spec=importlib.util.spec_from_file_location('ecology_source',ROOT/'Dense/build_ecology.py')
e=importlib.util.module_from_spec(spec)
spec.loader.exec_module(e)
data=json.loads((HERE/'ecology.json').read_text())
old=json.loads((ROOT/'Dense/ecology.json').read_text())
old_points={(g['biome'],g['mesh']):set(tuple(p) for p in g['instances']) for g in old['groups']}
landmarks=json.loads((HERE/'landmarks.json').read_text())['assets']
counts={}
for group in data['groups']:
    layer=group['layer'];mesh=group['mesh']
    original='SM_Rodin_'+mesh[3:].split('_Warp')[0] if layer=='tree' else 'SM_CandyflossThicket' if layer=='bush' else mesh
    original_points=old_points[(group['biome'],original)]
    for point in group['instances']:
        assert tuple(point) in original_points, ('Changed transform',point)
        assert all(math.isfinite(v) for v in point)
        x,y,z,yaw,scale=point
        if layer=='snow':
            continue
        radius=300*scale if layer=='tree' else 160 if layer=='bush' else 90 if layer=='flower' else 32
        assert e.allowed(x,y,max(0,radius-1),layer)
        clearance=300*scale if layer=='tree' else 160*scale if layer=='bush' else 90 if layer=='flower' else 32
        for record in landmarks:
            if record['radius']>0:
                lx,ly=record['location']
                assert math.hypot(x-lx,y-ly)+.01>=record['radius']+clearance
    counts[layer]=counts.get(layer,0)+len(group['instances'])
assert counts==data['counts']
for r in landmarks:
    if r['category'] not in ('fountain','bird'):
        assert e.allowed(*r['location'],r['radius'],'tree')
    elif r['category']=='fountain':
        for row in e.t.LOCATIONS:
            if row['kind'] in ('Giver','Landmark'):
                assert math.hypot(r['location'][0]-row['x'],r['location'][1]-row['y'])>300+r['radius']
records=json.loads((HERE/'assets.json').read_text())['assets']
for r in records:
    source=ROOT.parent.parent/r['source']
    assert hashlib.sha256(source.read_bytes()).hexdigest()==r['source_sha256']
    assert 0<r['triangles']<=18000
    assert all(math.isfinite(c) for pair in r['bounds_cm'] for c in pair)
for r in records+landmarks:
    fbx=HERE/'FBX'/(r['name']+'.fbx')
    assert fbx.read_bytes().startswith(b'Kaydara FBX Binary')
    assert (HERE/'Blend'/(r['name']+'.blend')).exists()
report=dict(status='passed',instances=sum(counts.values()),removed_for_landmarks=sum(old['counts'].values())-sum(counts.values()),
    counts=counts,all_retained_transforms_identical=True,source_hashes_unchanged=True,
    quest_road_and_landmark_exclusions=True,editable_blender_assets=len(records)+len(landmarks))
(HERE/'source_validation.json').write_text(json.dumps(report,indent=2)+'\n')
print(json.dumps(report,indent=2))
