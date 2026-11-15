"""Resume the first fal request and run the remaining requests together."""
import json, sys, time
from pathlib import Path
ROOT=Path(__file__).resolve().parents[3]; OUT=Path(__file__).resolve().parent
sys.path.insert(0,str(OUT))
sys.path.insert(0,str(ROOT/"Scripts"))
import genlib as g
from fal3d import DEFAULT_MODEL, HOSTS, find_url
from generate import PROMPTS

records_path=OUT/"queue.json"
if records_path.exists():
    records=json.loads(records_path.read_text(encoding="utf-8"))
else:
    records=[{"name":"MM_CreamClassic","request_id":"01a1de01-fa90-7ad0-8296-54bd519fd189",
              "status_url":"https://queue.fal.run/fal-ai/hunyuan-3d/requests/01a1de01-fa90-7ad0-8296-54bd519fd189/status",
              "response_url":"https://queue.fal.run/fal-ai/hunyuan-3d/requests/01a1de01-fa90-7ad0-8296-54bd519fd189"}]
key=g.env_value("FAL3D_API_KEY","FAL_KEY","FAL_API_KEY")
submitted_names={r["name"] for r in records}
for name,prompt in PROMPTS.items():
    if name in submitted_names: continue
    sub=g.request("POST",f"https://queue.fal.run/{DEFAULT_MODEL}",key,json_body={"prompt":prompt,"enable_pbr":True},scheme="Key")
    rec={"name":name,"request_id":sub.get("request_id"),"status_url":sub.get("status_url"),"response_url":sub.get("response_url")}
    if not all((rec["request_id"],rec["status_url"],rec["response_url"])): raise RuntimeError("unexpected fal submit response")
    for u in (rec["status_url"],rec["response_url"]): g.check_url(u,HOSTS)
    records.append(rec)
    records_path.write_text(json.dumps(records,indent=2),encoding="utf-8")
    print("submitted",name,"request",rec["request_id"],flush=True)
# Save the resumed first request along with both new IDs before polling.
records_path.write_text(json.dumps(records,indent=2),encoding="utf-8")
deadline=time.time()+900; pending={r["name"] for r in records}
while pending:
    for rec in records:
        name=rec["name"]
        if name not in pending: continue
        st=g.request("GET",rec["status_url"],key,scheme="Key")
        state=st.get("status"); print(name,state,flush=True)
        if state=="COMPLETED":
            res=g.request("GET",rec["response_url"],key,scheme="Key")
            url=find_url(res,(".glb",))
            if not url: raise RuntimeError(name+" returned no GLB")
            dest=OUT/(name+".glb");g.safe_download(url,dest,HOSTS)
            thumb=find_url(res.get("thumbnail"),(".png",".jpg",".webp"))
            if thumb:
                ext=thumb.split("?")[0].rsplit(".",1)[-1].lower();g.safe_download(thumb,dest.with_suffix("."+ext),HOSTS)
            rec["seed"]=res.get("seed");rec["prompt"]=PROMPTS[name];rec["pbr"]=True;rec["result_size_bytes"]=dest.stat().st_size
            (OUT/(name+".metadata.json")).write_text(json.dumps(rec,indent=2),encoding="utf-8")
            pending.remove(name);records_path.write_text(json.dumps(records,indent=2),encoding="utf-8")
            print("saved",dest.name,"bytes",dest.stat().st_size,flush=True)
        elif state not in ("IN_QUEUE","IN_PROGRESS"):
            raise RuntimeError(f"{name} unexpected status {state}")
    if time.time()>deadline: raise TimeoutError(",".join(sorted(pending)))
    if pending: time.sleep(5)
