from pathlib import Path
ROOT=Path(__file__).resolve().parents[2]
note='''

### Nov 15: generated drifting marshmallows

Replaced the river's 13 cylinder placeholders with three fal-generated,
Blender-cleaned marshmallow variants. Material flow remains +Y at 45 cm/s,
following the sinuous river centerline, with slight bobbing and endpoint wrap.
Whole-object offsets avoid splitting a mesh at the wrap; expanded bounds keep
the animated meshes visible away from their original actor positions. Finish
is matte cream. Sources and cleanup report: Docs/Devlog/Marshmallows.
Timed runtime screenshots: marshmallow-flow-8.png and marshmallow-flow-18.png.
This task changes no C++ and does no packaging. Other worker source edits made
after the previous green build/82 tests are outside this asset verification.
'''
for rel in ('Docs/STATUS.md','Docs/DEVLOG.md'):
 p=ROOT/rel;t=p.read_text(encoding='utf-8')
 if '### Nov 15: generated drifting marshmallows' not in t:p.write_text(t+note,encoding='utf-8')
p=ROOT/'Docs/Declared.md';t=p.read_text(encoding='utf-8')
if 'three fal-generated marshmallow mesh variants' not in t:p.write_text(t+'\n- Nov 15: three fal-generated marshmallow mesh variants, cleaned in Blender for the hot chocolate river; provenance in Docs/Devlog/Marshmallows.\n',encoding='utf-8')
