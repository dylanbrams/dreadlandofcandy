import json,sys,urllib.request,urllib.error
from pathlib import Path
sys.path.insert(0,str(Path(__file__).resolve().parents[3]/"Scripts"))
import genlib as g
records=json.loads((Path(__file__).resolve().parent/"queue.json").read_text())
key=g.env_value("FAL3D_API_KEY","FAL_KEY","FAL_API_KEY")
for rec in records:
 for url in (rec["status_url"],rec["response_url"]):
  req=urllib.request.Request(url,headers={"Authorization":"Key "+key})
  try:
   with urllib.request.urlopen(req,timeout=30) as r: print(rec["name"],"ok",r.status,r.read(200).decode(errors="replace"))
  except urllib.error.HTTPError as e: print(rec["name"],"http",e.code,e.read(200).decode(errors="replace"))
