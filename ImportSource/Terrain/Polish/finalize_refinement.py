"""Record the actual final saved checks and targeted render comparisons."""
import json
from pathlib import Path

HERE=Path(__file__).resolve().parent
PROJECT=HERE.parents[2]
def read(name):
    return json.loads((HERE/name).read_text(encoding='utf-8-sig'))

verification=read('verification.json')
source=read('source_validation.json')
slices=read('slice_validation.json')
crystal=read('crystal_refinement.json')
assert verification['status']==source['status']==slices['status']=='passed'
assert len(verification['lods'])==20 and verification['instances']==85091
assert crystal['status']=='saved'
assets=read('assets.json')['assets']
pines=[a for a in assets if a.get('species')=='CandyCaneFir']
assert len(pines)==5 and {a['variant'] for a in pines}==set(range(5))
assert 'bAllowAsynchronousShaderCompiling' not in (HERE/'run_review.ps1').read_text()
performance=read('performance.json')
baseline=read('refinement_baseline_performance.json')
comparisons={}
for view in ('DenseSnow','CrystalFountain','CrystalFountainSide','LemonadeSpring'):
    log=PROJECT/'Saved/Logs'/('polish-'+view+'.log')
    assert log.stat().st_mtime>(HERE/'crystal_refinement.json').stat().st_mtime
    current=performance['views'][view]
    assert current['screenshot_present'] and not current['material_compile_errors']
    before=baseline['views'].get(view)
    comparisons[view]=dict(current=current['timings']['FrameTime'],
        previous=before['timings']['FrameTime'] if before else None)
report=dict(status='passed',snow_pine_varieties=5,tree_varieties=11,meshes_with_three_lods=20,
    foliage_instances=85091,instanced_components=33,floating_slices=slices,
    crystal_material=crystal,lemonade_material='Lit single-surface translucency with ripple normals and an opaque basin floor',
    performance_comparison=comparisons,performance_limit=performance['limitation'],
    performance_status='Static medians have headroom; intermittent spring frame-time spikes remain in the extended capture. Gameplay performance is not signed off.',
    previews={key:read('review_maps.json')[key] for key in ('PineVarieties','DenseSnow','CrystalFountain','CrystalFountainSide','LemonadeSpring')},
    packaging='not rerun',paid_generation=False)
(HERE/'refinement_report.json').write_text(json.dumps(report,indent=2)+'\n')
diagnostic=read('interruption_diagnostic.json')
diagnostic.update(source_crystal_tint_applied_to_runtime=True,remaining='none for this art refinement',final_render_status='passed')
(HERE/'interruption_diagnostic.json').write_text(json.dumps(diagnostic,indent=2)+'\n')
print(json.dumps(report,indent=2))
