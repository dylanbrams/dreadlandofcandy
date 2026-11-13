"""Strengthen glass-candy readability after the first transparency capture."""
import importlib.util
import json
from pathlib import Path
import unreal as u

here=Path(u.Paths.convert_relative_path_to_full(u.Paths.project_dir()))/'ImportSource/Terrain/Polish'
spec=importlib.util.spec_from_file_location('polish_materials',here/'build_unreal.py')
p=importlib.util.module_from_spec(spec)
spec.loader.exec_module(p)
color=json.loads((here/'landmarks.json').read_text())['palette']['Crystal'][:3]
mat=p.material('M_Song_Crystal','crystal',color=color)
assert mat.get_editor_property('blend_mode')==u.BlendMode.BLEND_TRANSLUCENT
(here/'crystal_refinement.json').write_text(json.dumps(dict(status='saved',opacity_head_on=.48,
    opacity_grazing=.8,refraction=1.04,lit_translucency=True),indent=2)+'\n')
u.log('CRYSTAL READABILITY REFINEMENT PASS')
