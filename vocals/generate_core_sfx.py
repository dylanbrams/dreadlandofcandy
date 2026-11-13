"""Sequential resumable renderer for the complete planned SFX set."""
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
('SFX_UI_Click','A miniature imperial rubber stamp slams down with absurd bureaucratic force: compressed wooden clack, paper punch, tiny metallic latch. One crisp compact event.',0.15),
('SFX_Sword_Raise','An impossibly heavy ceremonial sword rises: tectonic steel scrape, armor groan and rising air pressure, building toward apocalypse, then cutting dead before any strike. No hit.',0.8),
('SFX_FormLine_Contact','A gigantic ruler strikes a desk and steel filing drawers snap into military alignment, one rapid straight chain of precise percussive clacks.',0.6),
('SFX_DisappointedSigh','One wordless exhausted exhalation magnified into an ancient bellows emptying a cathedral; deep air pressure rolls outward and dwindles in crushing disappointment.',1.0),
('SFX_LastJudgment_Contact','A courthouse-sized book slams shut, colossal air shockwave radiates, metal fittings tremble, one tiny pencil drops at the end. No blade strike.',1.0),
('SFX_Flustered','A proud spring mechanism loses its place: explosive tiny clockwork stumble, two panicked wooden clacks and a rising rubber wobble. Wordless.',0.35),
('SFX_Interact_Complete','A monumental bureaucratic stamp certifies a tiny task: paper slap, vault-sized lock engaging, one immaculate crystalline ping.',0.4),
('SFX_Quest_Accept','A parchment unfurls like a siege banner, enormous wax seal crashes down, mechanical ratchet locks with ludicrous finality.',0.8),
('SFX_Stage_Complete','A huge theatrical stage mechanism locks into place with three escalating wooden clunks and one brilliant glass chime, pompous and triumphant without music.',0.7),
('SFX_LevelUp','A massive ascending clockwork elevator rockets upward, opens an enormous celestial latch, then resolves into one tiny immaculate bell. No melody.',1.0),
('SFX_Ledger_Deduction',"A merciless accountant's heavy stamp punches paper while a cash drawer snaps the wrong way: one sharp mechanical confiscation.",0.25),
('SFX_Budget_Warning',"A giant studio light's electrical ballast sputters dangerously and a huge spring-loaded meter knocks against its stop twice. No horn.",0.7),
('SFX_Budget_Empty','The entire movie studio powers down: giant breaker clack, descending mechanical whine, stage-light fizz, one pathetic final relay tick.',1.3),
('SFX_Retake_Rewind','A gigantic film reel spins backward at impossible speed, sprockets chatter, then brakes with a rubbery squeal and decisive latch.',0.8),
('SFX_UI_Back','A ridiculously heavy official stamp is sucked backward off paperwork, short vacuum pop followed by one tiny latch snapping shut.',0.2),
('SFX_UI_Unavailable','A monumental locked filing cabinet refuses to open: one metal thunk, strained spring wobble, offended little mechanical rattle.',0.3),
('SFX_Step_Stone','One armored boot plants on village cobbles like a miniature siege engine, heavy iron clonk, stone tick and weighty armor rattle, very short.',0.25),
('SFX_Step_CandySoil','One heavy armored boot compresses sugary dirt: exaggerated biscuit crunch, sticky sugar pull and weighty armor clonk, crisp short finish.',0.25),
('SFX_Fenella_Fly_Loop','Continuous industrious tiny fairy wings, like a miniature studio helicopter made of silk and clockwork, light flutter with rapid mechanical detail. Steady texture, no passes.',4.0),
('SFX_Backlot_Ambience_Loop','Quiet continuous outdoor film-backlot air, breeze through flimsy wooden scenery, faint canvas creaks and distant loose rigging rattles with oversized wood resonance. No voices or foreground events.',10.0),
('SFX_Shield_Bash','A shield gives one politely devastating shove: huge bronze door knocker, compact pressure blast and short armor rattle. Firm impact, no cutting flesh.',0.4),
('SFX_Manners_Dome_Open','A castle-sized transparent ceremonial dome unfolds from a shield, gigantic resonant bronze panels locking together around an airy crystalline shell. No choir.',0.8),
('SFX_Manners_Dome_Rebuke','An invisible royal boundary repels one intruder: taut glass boing, massive doorknocker snap, one tiny offended porcelain tick.',0.3),
('SFX_Manners_Dome_Close','A monumental protective glass dome folds into a tiny mannered shield latch, descending panel clicks ending in a delicate single tick.',0.6),
('SFX_Knead_Impact','Two gigantic fists knead impossibly heavy dough in one double-fisted strike: thunderous wet dough thwump, sticky compression and explosive flour puff. No gore.',0.35),
('SFX_RollingPin_Cyclone_Loop','Continuous storm of colossal wooden rolling pins orbiting at hurricane speed, deep rushing air, dense controlled wood clatter and flour hiss. Steady middle only, no intro or finale.',3.0),
('SFX_RollingPin_Finale','One absurd outward explosion of heavy wooden rolling pins and flour, rapid wooden collisions, ending cleanly. No lingering individual roll.',1.0),
('SFX_RollingPin_Stray','After catastrophe, one lonely wooden rolling pin falls, bounces twice, rolls a short distance and stops with a pathetic little tap. Otherwise silent.',0.8),
('SFX_Encouragement_Blast','A catastrophic compressed-air blast launches stiff paper placards: enormous pressure wave, sharp cardboard slaps, short crumpling finish. No voices.',0.7),
('SFX_Sugar_Volley','A gigantic silver spoon flick launches a tiny spray of sugar cubes: grand polished-metal snap, brilliant crystalline ticks and short air zip. No impact.',0.3),
('SFX_Sugar_Impact','One sugar cube lands with the force of a crystal monument: sharp tink into massive compressed crunch, sparkling fragments, very short clean finish.',0.3),
('SFX_Peppermint_Fall','One enormous peppermint meteor descends, spiraling whistle and dense rushing air building to impending candy catastrophe, no impact.',0.7),
('SFX_Peppermint_Impact','One giant hard peppermint hits like siege artillery: deep compact thud, brittle candy fracture and sparkling sugar debris. No avalanche or long tail.',0.7),
('SFX_Soap_Torrent_Loop','Continuous apocalyptic pressure washer pumps thick magical soap, roaring viscous spray, ferocious foam churn and tiny bright bubbles. Steady energy, no start or stop.',3.0),
('SFX_Soap_Finish','A colossal soap cannon shuts off with a pressure-valve gulp and collapsing foam, then one tiny sanitizer-dispenser plip and immaculate glass squeak.',0.7),
('SFX_Celery_Launch','An epic war bow releases a celery stalk like a ballista bolt: monumental taut-string snap, leafy whip and extremely fast air zip. No impact.',0.3),
('SFX_Celery_Impact','One celery stalk strikes with siege-weapon force: gigantic wet fibrous crunch, snapping vegetable ribs and tiny leaf flutter. No gore, one compact hit.',0.3),
('SFX_Floss_Launch','A siege-bow tension snap pulls absurdly taut dental thread from a tiny dispenser, huge string resonance with a sharp plastic click. No impact.',0.3),
('SFX_Floss_Snare','An explosive dental-floss spool unravels into gigantic elastic strand twangs, binding a target with ludicrous tensile force, ending taut.',0.5),
('SFX_Floss_Retract','An outrageously fast dental-floss dispenser retracts its thread, violent elastic zip ending in one tiny plastic click.',0.5)]
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
