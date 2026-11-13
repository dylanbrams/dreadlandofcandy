"""Sequential resumable renderer for the eight core P0 sound effects."""
import argparse,hashlib,json,subprocess,urllib.error,urllib.request
from datetime import datetime,timezone
from pathlib import Path
from elevenlabs_config import load_credential
from preflight import get
ROOT=Path(__file__).resolve().parent; OUT=ROOT/'output'/'sfx'
PREFIX='Over-the-top epic fantasy SFX: a tiny domestic action treated as world-ending, utterly serious, enormous scale. Isolated, crisp attack, clean ending; no speech, music, background or clipping. '
ROWS=[
('SFX_Hero_Inhale',"A single gigantic intake of breath through a knight's helmet, like a cathedral pipe organ drawing air before passing sentence; rising metal resonance and immense suction, abruptly held. Wordless breath only.",0.8),
('SFX_Lecture_Impact','One thunderous blast of parental disapproval: compressed air cannon, colossal book slammed onto an oak desk, a short metallic shockwave. A physical WHUMP with no spoken voice.',0.5),
('SFX_Bubble_Pop','One comic speech balloon punching into existence with a wildly oversized rubber snap and tiny paper slap; absurdly forceful, crisp, extremely short.',0.2),
('SFX_Pacify','An enormous inflated ego collapses into one tiny embarrassed rubber squeak, descending balloon pressure ending in a meek toy chirp. No creature voice or words.',0.6),
('SFX_Clapper','A movie clapperboard snaps shut with the authority of a castle drawbridge crashing down: explosive wooden crack, gigantic short room resonance, instant finish.',0.4),
('SFX_Cut_Horn',"One outrageously authoritarian film-set air horn, part ocean liner and part royal herald's horn, a sharp blast that stops the universe; abrupt decisive cutoff. Nonmusical warning blast.",0.7),
('SFX_Coin_Trickle','A ludicrously grand cash-register mechanism opens for a pathetically small payout: a few tiny coins trickle down an enormous metal chute, diminishing to one lonely coin. No fanfare.',1.3),
('SFX_UI_Click','A miniature imperial rubber stamp slams down with absurd bureaucratic force: compressed wooden clack, paper punch, tiny metallic latch. One crisp compact event.',0.15)]
def save(m,p):
 t=p.with_suffix('.json.part');t.write_text(json.dumps(m,ensure_ascii=False,indent=2)+'\n',encoding='utf-8');t.replace(p)
def main():
 parser=argparse.ArgumentParser(description=__doc__)
 parser.add_argument('--check',action='store_true',help='Validate all payloads offline without API calls')
 args=parser.parse_args()
 for ident,prompt,duration in ROWS:
  if len(PREFIX+prompt)>450:raise SystemExit(f'{ident}: prompt exceeds 450 characters')
  if not 0.5<=max(0.5,duration)<=30:raise SystemExit(f'{ident}: provider duration must be 0.5 to 30 seconds')
 if args.check:
  for ident,prompt,duration in ROWS:print(f'{ident}: {len(PREFIX+prompt)}/450 characters; target {duration}s; provider {max(0.5,duration)}s')
  return
 key,src=load_credential();sub=get('/v1/user/subscription',key)
 if not isinstance(sub.get('character_limit'),int):raise SystemExit('Subscription check failed; no audio requested')
 OUT.mkdir(parents=True,exist_ok=True);mp=OUT/'manifest.json';m=json.loads(mp.read_text(encoding='utf-8')) if mp.exists() else {'provider':'ElevenLabs','credential_source':src,'model':'eleven_text_to_sound_v2','clips':{}}
 for ident,prompt,duration in ROWS:
  full=PREFIX+prompt;payload={'text':full,'model_id':'eleven_text_to_sound_v2','duration_seconds':max(0.5,duration),'prompt_influence':0.5};finger=hashlib.sha256(json.dumps(payload,sort_keys=True).encode()).hexdigest();mp3=OUT/(ident+'_v01.mp3');wav=OUT/(ident+'_v01.wav');r=m['clips'].get(ident)
  if r and r['hash']!=finger:raise SystemExit(f'{ident}: prompt changed; inspect before rerender')
  if r and r['status']=='complete' and mp3.exists() and wav.exists():continue
  if r and r['status']!='downloaded':raise SystemExit(f'{ident}: unresolved prior request; inspect history')
  r=r or {'id':ident,'prompt':full,'requested_duration':duration,'provider_duration':payload['duration_seconds'],'model_id':'eleven_text_to_sound_v2','hash':finger,'status':'request_started','review':'listening_pending','date':datetime.now(timezone.utc).isoformat()}
  if ident not in m['clips']:m['clips'][ident]=r;save(m,mp)
  req=urllib.request.Request('https://api.elevenlabs.io/v1/sound-generation?output_format=mp3_44100_128',data=json.dumps(payload).encode(),headers={'xi-api-key':key,'Content-Type':'application/json'},method='POST')
  try:
   with urllib.request.urlopen(req,timeout=120) as resp:data=resp.read();r['request_id']=resp.headers.get('request-id');r['character_cost']=resp.headers.get('character-cost')
  except urllib.error.HTTPError as e:
   body=e.read().decode('utf-8',errors='replace').replace(key,'[REDACTED]')
   r.update(status='api_error',http_status=e.code,error_body=body,request_id=e.headers.get('request-id') or e.headers.get('x-request-id'));save(m,mp)
   raise SystemExit(f'{ident}: API HTTP {e.code}: {body}; stopped') from None
  except (urllib.error.URLError,TimeoutError,OSError):r['status']='failed_or_ambiguous';save(m,mp);raise SystemExit(f'{ident}: ambiguous request; inspect history')
  part=mp3.with_suffix('.mp3.part');part.write_bytes(data);part.replace(mp3);r['status']='downloaded';save(m,mp)
  subprocess.run(['ffmpeg','-v','error','-y','-i',str(mp3),'-ac','1','-ar','48000','-c:a','pcm_s16le',str(wav)],check=True)
  import soundfile as sf,numpy as np
  samples,rate=sf.read(wav);peak=float(np.max(np.abs(samples))) if len(samples) else 0
  if not peak:raise SystemExit(f'{ident}: silent/empty WAV')
  r.update(status='complete',wav=str(wav.relative_to(ROOT)),mp3=str(mp3.relative_to(ROOT)),actual_duration=len(samples)/rate,sample_rate=rate,channels=1,peak_dbfs=float(20*np.log10(peak)),clipped_samples=int(np.count_nonzero(np.abs(samples)>=32767/32768)));save(m,mp);print(ident+' complete',flush=True)
 after=get('/v1/user/subscription',key);m['subscription_after']={k:after[k] for k in ('tier','status','character_count','character_limit') if k in after};save(m,mp);print(json.dumps(m['subscription_after']))
if __name__=='__main__':main()
