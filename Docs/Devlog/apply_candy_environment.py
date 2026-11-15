"""Exclusive-engine entry point; applies selected materials and map dressing.

Use UnrealEditor -ExecutePythonScript=<this file> -unattended -nosplash.
Do not run until the active game worker has handed over the environment.
"""
from pathlib import Path
import unreal as u
HERE=Path(__file__).resolve().parent
steps=['apply_candy_ground.py','candy_water.py','candy_landscape_dressing.py','candy_review_maps.py']
if not u.EditorAssetLibrary.does_asset_exist('/Game/World/CandyPBR/Textures/T_candy_grass_normal'):
 steps.insert(0,'import_candy_pbr.py')
for filename in steps:
 path=HERE/filename
 assert path.exists(),str(path)
 u.log('CANDY ENVIRONMENT step: '+filename)
 exec(compile(path.read_text(encoding='utf-8'),str(path),'exec'),{'__file__':str(path),'__name__':'__main__'})
u.log('CANDY ENVIRONMENT APPLY COMPLETE; rendering and traversal review still required')
