"""Sequential, resumable ElevenLabs renderer for the authored attack cry table."""
import hashlib, json, subprocess, urllib.error, urllib.request
from datetime import datetime, timezone
from pathlib import Path
from elevenlabs_config import load_credential
from preflight import get

ROOT=Path(__file__).resolve().parent
OUT=ROOT/'output'/'attacks'
LANGS=['en','de','fr','ja','ko','zh-CN','pt-BR','es-ES','it','pl','ru','tr','uk','cs','th']
VOICE={'K':'SOYHLrjzK2X1ezoPC6cr','B':'nPczCjzI2devNBz1zQrb','M':'JBFqnCBsd6RMkjVDRZzb','A':'TX3LPaxmHKxFdv7VOQHJ'}
VOICE_NAME={'K':'Harry','B':'Brian','M':'George','A':'Liam'}
MODEL='eleven_multilingual_v2'
SETTINGS={'stability':0.35,'similarity_boost':0.75,'style':0.65,'use_speaker_boost':True}

def save(m):
 p=OUT/'manifest.json.part'; p.write_text(json.dumps(m,ensure_ascii=False,indent=2)+'\n',encoding='utf-8'); p.replace(OUT/'manifest.json')

def main():
 key,source=load_credential(); sub=get('/v1/user/subscription',key)
 if not isinstance(sub.get('character_limit'),int): raise SystemExit('Subscription check failed; no audio requested')
 remain=sub['character_limit']-sub['character_count']
 if remain < 3000: raise SystemExit('Fewer than 3000 included characters remain; batch stopped')
 rows=json.loads((ROOT/'attack_translations.json').read_text(encoding='utf-8'))
 mp=OUT/'manifest.json'; OUT.mkdir(parents=True,exist_ok=True)
 m=json.loads(mp.read_text(encoding='utf-8')) if mp.exists() else {'provider':'ElevenLabs','credential_source':source,'model':MODEL,'voice_settings':SETTINGS,'translation_review':'fluent_review_pending','clips':{}}
 for row in rows:
  for locale in LANGS:
   text=row[locale]; ident=f"VO_{row['id']}_{locale}_{'parent' if row['id']=='B03_parent' else 'epic'}"
   attack=row['id'].split('_')[0]
   if attack[0] not in {'A','B'}: continue  # Knight (Harry) and mage are already approved/complete.
   vid=VOICE[attack[0]]
   payload={'text':text,'model_id':MODEL,'voice_settings':SETTINGS}
   endpoint=f'/v1/text-to-speech/{vid}'
   fingerprint=hashlib.sha256(json.dumps({'id':ident,'payload':payload},sort_keys=True,ensure_ascii=False).encode()).hexdigest()
   folder=OUT/locale; folder.mkdir(parents=True,exist_ok=True)
   mp3=folder/(ident+'.mp3'); wav=folder/(ident+'.wav')
   old=m['clips'].get(ident)
   if old and old.get('hash')!=fingerprint: raise SystemExit(f'{ident}: existing request differs; inspect before retry')
   if old and old.get('status')=='complete' and mp3.exists() and wav.exists(): continue
   if old and old.get('status') not in ('downloaded',): raise SystemExit(f'{ident}: unresolved prior request; inspect provider history')
   rec=old or {'id':ident,'attack_id':row['id'],'locale':locale,'text':text,'english_source':row['en'],'voice_id':vid,'voice_name':VOICE_NAME[attack[0]],'model_id':MODEL,'settings':SETTINGS,'delivery':'quiet, emotionally spent' if row['id']=='B03_parent' else 'serious theatrical battlecry','review':'fluent_review_pending' if locale!='en' else 'listening_pending','hash':fingerprint,'status':'request_started','date':datetime.now(timezone.utc).isoformat(),'endpoint':endpoint}
   if old is None: m['clips'][ident]=rec; save(m)
   req=urllib.request.Request('https://api.elevenlabs.io'+endpoint+'?output_format=mp3_44100_128',data=json.dumps(payload,ensure_ascii=False).encode(),headers={'xi-api-key':key,'Content-Type':'application/json'},method='POST')
   try:
    with urllib.request.urlopen(req,timeout=120) as resp:
     data=resp.read(); rec['request_id']=resp.headers.get('request-id'); rec['character_cost']=resp.headers.get('character-cost')
   except urllib.error.HTTPError as e:
    rec['status']='api_error'; rec['http_status']=e.code; save(m)
    if e.code in (401,402,403,429): raise SystemExit(f'{ident}: API stopped batch with HTTP {e.code}')
    raise SystemExit(f'{ident}: API error HTTP {e.code}; stopped')
   except (urllib.error.URLError,TimeoutError,OSError):
    rec['status']='failed_or_ambiguous'; save(m); raise SystemExit(f'{ident}: ambiguous network result; inspect request history before retry')
   if not data: rec['status']='failed_or_ambiguous'; save(m); raise SystemExit(f'{ident}: empty response; stopped')
   part=mp3.with_suffix('.mp3.part'); part.write_bytes(data); part.replace(mp3); rec['status']='downloaded'; save(m)
   subprocess.run(['ffmpeg','-v','error','-y','-i',str(mp3),'-ac','1','-ar','48000','-c:a','pcm_s16le',str(wav)],check=True)
   import soundfile as sf, numpy as np
   samples,rate=sf.read(wav); peak=float(np.max(np.abs(samples))) if len(samples) else 0
   if not peak: raise SystemExit(f'{ident}: decoded audio is empty or silent')
   rec.update(status='complete',mp3=str(mp3.relative_to(ROOT)),wav=str(wav.relative_to(ROOT)),duration_seconds=len(samples)/rate,sample_rate=rate,channels=1,peak_dbfs=float(20*np.log10(peak)),clipped_samples=int(np.count_nonzero(np.abs(samples)>=32767/32768)))
   save(m); print(f"{ident} complete ({rec['duration_seconds']:.2f}s)",flush=True)
   if get('/v1/user/subscription',key).get('status') not in ('active',None): raise SystemExit('Subscription status changed; batch stopped')
 after=get('/v1/user/subscription',key); m['subscription_after']={k:after[k] for k in ('tier','status','character_count','character_limit') if k in after}; save(m)
 print(json.dumps({'clips_complete':sum(x.get('status')=='complete' for x in m['clips'].values()),'subscription_after':m['subscription_after']}))

if __name__=='__main__': main()
