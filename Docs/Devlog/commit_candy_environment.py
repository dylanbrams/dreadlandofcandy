"""Scoped environment checkpoint; run only after build and tests pass."""
from pathlib import Path
import subprocess
ROOT=Path(__file__).resolve().parents[2]
paths=['Content/World/CandyPBR','Content/World/L_Sugarborough.umap',
       'Content/World/Expansion/SM_RiverWater.uasset','Content/World/Expansion/SM_Riverlands.uasset',
       'Content/World/Expansion/M_RiverWater.uasset','Content/World/Expansion/M_Marshmallow.uasset','Content/World/Expansion/M_HotChocolateSteam.uasset',
       'Docs/STATUS.md','Docs/DEVLOG.md','Docs/Declared.md','Docs/ENGINE_HANDOFF.md','Docs/Devlog/release_candy_engine.py','Docs/Devlog/CandyPBR']
paths += [str(p.relative_to(ROOT)).replace('\\','/') for p in (ROOT/'Content/World/Meshes').glob('SM_Terrain_*.uasset')]
paths += [str(p.relative_to(ROOT)).replace('\\','/') for p in (ROOT/'Content/World/Expansion').glob('SM_EastOpening_*.uasset')]
paths += ['Docs/Devlog/'+name for name in ('apply_candy_environment.py','apply_candy_ground.py','candy_landscape_dressing.py','candy_review_maps.py','candy_water.py','generate_candy_pbr.py','import_candy_pbr.py','review_candy_pbr.py','capture_candy.ps1','record_candy_delivery.py','commit_candy_environment.py')]
subprocess.run(['git','add','--',*paths],cwd=ROOT,check=True)
subprocess.run(['git','diff','--cached','--check','--',*paths],cwd=ROOT,check=True)
subprocess.run(['git','commit','--only','-m','Add candy PBR paths and hot chocolate river environment','--',*paths],cwd=ROOT,check=True)
