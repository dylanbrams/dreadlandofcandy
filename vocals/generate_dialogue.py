"""Sequential resumable English renderer for Fenella and non-hero candidates."""
import hashlib,json,subprocess,urllib.error,urllib.request
from datetime import datetime,timezone
from pathlib import Path
from elevenlabs_config import load_credential
from preflight import get

ROOT=Path(__file__).resolve().parent; OUT=ROOT/'output'; MODEL='eleven_multilingual_v2'
CAST={
 'fenella':('hpp4J3VqNfWAUOO0d1Us','Bella'), 'vil':('bIHbv24MWmeRgasZH58o','Will'),
 'farm':('iP95p4xoKVk53GoZ742B','Chris'), 'marsh':('onwK4e9ZLuTAKqWW03F9','Daniel'),
 'herd':('XrExE9yKIg1WjnnlVkGX','Matilda'), 'aud':('nPczCjzI2devNBz1zQrb','Brian'),
 'rat':('N2lVS1w4EtoT3dr4eOWO','Callum'), 'gob':('CwhRBWXzGAHq8TQ4Fs17','Roger'),
 'bug':('cgSgspJ2msm6clMCkdW9','Jessica'), 'pac':('SAz9YHcvj6GT2YYXdXww','River')}
SET={'stability':0.4,'similarity_boost':0.75,'style':0.45,'use_speaker_boost':True}
def save(m,path):
 p=path.with_suffix('.json.part');p.write_text(json.dumps(m,ensure_ascii=False,indent=2)+'\n',encoding='utf-8');p.replace(path)
def main():
 key,src=load_credential(); sub=get('/v1/user/subscription',key)
 if not isinstance(sub.get('character_limit'),int):raise SystemExit('Subscription check failed; no audio requested')
 rows=json.loads((ROOT/'line_inventory.json').read_text(encoding='utf-8')); jobs=[r for r in rows if r.get('speaker') in CAST]
 base=OUT/'dialogue';base.mkdir(parents=True,exist_ok=True); mp=base/'manifest.json'
 m=json.loads(mp.read_text(encoding='utf-8')) if mp.exists() else {'provider':'ElevenLabs','credential_source':src,'model':MODEL,'settings':SET,'clips':{}}
 for r in jobs:
  sp=r['speaker'];vid,vname=CAST[sp]; ident=r['id']; text=r['text']; payload={'text':text,'model_id':MODEL,'voice_settings':SET}; endpoint=f'/v1/text-to-speech/{vid}'
  fingerprint=hashlib.sha256(json.dumps({'text':text,'voice':vid,'payload':payload},sort_keys=True,ensure_ascii=False).encode()).hexdigest()
  folder=OUT/'fenella' if sp=='fenella' else OUT/'cast'/sp;folder.mkdir(parents=True,exist_ok=True);mp3=folder/f'VO_{ident}.mp3';wav=folder/f'VO_{ident}.wav';old=m['clips'].get(ident)
  if old and old['hash']!=fingerprint:raise SystemExit(f'{ident}: changed mapping; inspect before re-render')
  if old and old['status']=='complete' and mp3.exists() and wav.exists():continue
  if old and old['status']!='downloaded':raise SystemExit(f'{ident}: unresolved previous request; inspect provider history')
  rec=old or {'id':ident,'speaker':sp,'text':text,'voice_id':vid,'voice_name':vname,'model_id':MODEL,'settings':SET,'source':r.get('source'),'status':'request_started','review':'listening_pending','date':datetime.now(timezone.utc).isoformat(),'hash':fingerprint}
  if old is None:m['clips'][ident]=rec;save(m,mp)
  req=urllib.request.Request('https://api.elevenlabs.io'+endpoint+'?output_format=mp3_44100_128',data=json.dumps(payload,ensure_ascii=False).encode(),headers={'xi-api-key':key,'Content-Type':'application/json'},method='POST')
  try:
   with urllib.request.urlopen(req,timeout=120) as resp:data=resp.read();rec['request_id']=resp.headers.get('request-id');rec['character_cost']=resp.headers.get('character-cost')
  except urllib.error.HTTPError as e:rec['status']='api_error';rec['http_status']=e.code;save(m,mp);raise SystemExit(f'{ident}: API HTTP {e.code}; stopped')
  except (urllib.error.URLError,TimeoutError,OSError):rec['status']='failed_or_ambiguous';save(m,mp);raise SystemExit(f'{ident}: ambiguous request; inspect history')
  if not data:raise SystemExit(f'{ident}: empty result')
  part=mp3.with_suffix('.mp3.part');part.write_bytes(data);part.replace(mp3);rec['status']='downloaded';save(m,mp)
  subprocess.run(['ffmpeg','-v','error','-y','-i',str(mp3),'-ac','1','-ar','48000','-c:a','pcm_s16le',str(wav)],check=True)
  import soundfile as sf,numpy as np
  samples,rate=sf.read(wav);peak=float(np.max(np.abs(samples))) if len(samples) else 0
  if not peak:raise SystemExit(f'{ident}: silent/empty WAV')
  rec.update(status='complete',wav=str(wav.relative_to(ROOT)),mp3=str(mp3.relative_to(ROOT)),duration_seconds=len(samples)/rate,sample_rate=rate,channels=1,peak_dbfs=float(20*np.log10(peak)),clipped_samples=int(np.count_nonzero(np.abs(samples)>=32767/32768)));save(m,mp)
  print(f'{ident} complete',flush=True)
 m['subscription_after']=get('/v1/user/subscription',key);m['subscription_after']={k:m['subscription_after'][k] for k in ('tier','status','character_count','character_limit') if k in m['subscription_after']};save(m,mp)
 print(json.dumps({'lines':len(jobs),'complete':sum(x['status']=='complete' for x in m['clips'].values()),'subscription_after':m['subscription_after']}))
if __name__=='__main__':main()
