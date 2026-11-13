"""Keep measured steady capture samples; do not infer gameplay frame rates."""
import csv
import json
from pathlib import Path
import re
import statistics

HERE=Path(__file__).resolve().parent
PROJECT=HERE.parents[2]
csv.field_size_limit(10000000)
results={}
for view in ('before-DenseGrove','DenseGrove','DenseSnow','PolishVillage','CrystalFountain','CrystalFountainSide','LemonadeSpring','StewPond','WhiskeyRun'):
    logfile=PROJECT/'Saved/Logs'/('polish-'+view+'.log')
    if not logfile.exists():
        continue
    log=logfile.read_text(errors='replace')
    match=re.search(r'Writing CSV to file : (.+\.csv)',log)
    if not match:
        results[view]=dict(status='no completed CSV', screenshot_present=False)
        continue
    source=Path(match[1].strip())
    with source.open() as f:
        rows=list(csv.DictReader(f))
    fields=['FrameTime','GPUTime','GameThreadTime','RenderThreadTime','RHIThreadTime']
    samples=[]
    for row in rows:
        try:
            sample={k:float(row[k]) for k in fields}
        except (ValueError,KeyError,TypeError):
            continue
        if sample['FrameTime']>0:
            samples.append(sample)
    total_samples=len(samples)
    assert total_samples>=400,(view,total_samples)
    samples=samples[-400:-10]
    with (HERE/'Review'/('timings-'+view+'.csv')).open('w',newline='') as f:
        w=csv.DictWriter(f,fieldnames=fields)
        w.writeheader()
        w.writerows(samples)
    summary={}
    for field in fields:
        values=sorted(s[field] for s in samples)
        summary[field]=dict(median_ms=round(statistics.median(values),3),p95_ms=round(values[int(len(values)*.95)],3))
    results[view]=dict(status='measured',samples=len(samples),capture_frames=total_samples,timings=summary,
        screenshot_present=(HERE/'Review'/('polish-'+view+'.png')).exists(),
        material_compile_errors=bool(re.search(r'Failed to compile Material|Shader error',log)))
report=dict(hardware='NVIDIA GeForce GTX 1660 Ti',resolution=[1600,1000],rhi='DX12 SM5',
    method='Stationary offscreen Unreal Editor -game, uncapped, no VSync. Last 390 frames before the final 10 of each capture. Normal captures contain 600 frames; extended snow/spring captures contain 1800 frames to exclude startup hitches.',
    limitation='One static camera per view, one local machine. Excludes startup samples; not a packaged benchmark, movement test or crowded combat measurement.',views=results)
(HERE/'performance.json').write_text(json.dumps(report,indent=2)+'\n')
print(json.dumps(report,indent=2))
