"""Refresh fiber materials after in-engine inspection without rebaking ecology."""
import importlib.util
from pathlib import Path
import unreal as u
here=Path(u.Paths.convert_relative_path_to_full(u.Paths.project_dir()))/'ImportSource/Terrain/Polish'
spec=importlib.util.spec_from_file_location('polish',here/'build_unreal.py')
p=importlib.util.module_from_spec(spec)
spec.loader.exec_module(p)
p.import_assets()
u.log('FIBER MATERIAL REFINEMENT PASS')
u.SystemLibrary.quit_editor()
