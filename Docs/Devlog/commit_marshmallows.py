from pathlib import Path
import subprocess
root=Path(__file__).resolve().parents[2]
paths=['Content/World/CandyPBR/Marshmallows','Content/World/CandyPBR/L_ChocolateReview.umap','Content/World/CandyPBR/L_VillageReview.umap','Content/World/L_Sugarborough.umap','Content/World/Expansion/M_Marshmallow.uasset','Content/World/Expansion/M_RiverWater.uasset','Content/World/Expansion/M_HotChocolateSteam.uasset','Docs/Devlog/Marshmallows','Docs/Devlog/candy_water.py','Docs/Devlog/candy_landscape_dressing.py','Docs/Devlog/update_marshmallows.py','Docs/Devlog/capture_marshmallow_flow.ps1','Docs/Devlog/record_marshmallow_delivery.py','Docs/Devlog/commit_marshmallows.py','Docs/STATUS.md','Docs/DEVLOG.md','Docs/Declared.md','Docs/ENGINE_HANDOFF.md']
subprocess.run(['git','add','--',*paths],cwd=root,check=True)
subprocess.run(['git','diff','--cached','--check','--',*paths],cwd=root,check=True)
subprocess.run(['git','commit','--only','-m','Replace river cylinders with generated drifting marshmallows','--',*paths],cwd=root,check=True)
