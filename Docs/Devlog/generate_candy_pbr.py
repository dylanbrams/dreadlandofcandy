"""Resumable fal PATINA batch. Secrets handled only by existing genlib."""
from pathlib import Path
import sys, json, time, concurrent.futures
ROOT=Path(__file__).resolve().parents[2]
sys.path.insert(0,str(ROOT/'Scripts'))
import genlib as g
OUT=Path(__file__).resolve().parent/'CandyPBR'
OUT.mkdir(exist_ok=True)
TYPES={
 'rock_candy':'tiny angular translucent rock candy gravel, turquoise rose lilac and honey crystals, densely packed walkable gravel with sugar dust in crevices',
 'peanut_brittle':'peanut butter brittle paving bricks, golden caramel rectangular staggered bricks embedded with crushed peanuts, narrow dark caramel joints',
 'peppermint':'rounded peppermint candy cobblestones, ivory stones with raspberry red spiral stripes, narrow cocoa mortar joints',
 'chocolate_mint':'rounded dark chocolate cobblestones with mint green marbled swirls and pale mint chips, narrow cocoa mortar joints',
 'candy_grass':'dense short mint green sugar grass, tiny edible mint blades and sparse pastel sugar sprinkles, lush continuous ground cover, mostly green, no large objects'
}
VARIANTS=['chunky storybook shapes, gently worn edges','smaller irregular pieces, subtle sugar crystal sparkle','handcrafted varied sizes, restrained pastel palette']
KEY=g.env_value('FAL3D_API_KEY','FAL_KEY','FAL_API_KEY')
def run(kind,index):
 folder=OUT/f'{kind}_{index+1}';folder.mkdir(exist_ok=True)
 if (folder/'result.json').exists():return
 state=folder/'request.json'
 if state.exists(): sub=json.loads(state.read_text())
 else:
  prompt=f'Seamless game environment PBR ground material, top down orthographic flat surface: {TYPES[kind]}, {VARIANTS[index]}. Cohesive whimsical candy fantasy, readable at gameplay distance, no perspective, no cast shadows, no text, no objects above surface.'
  body=dict(prompt=prompt,image_size='square_hd',seed=261115+index,tiling_mode='both',maps=['basecolor','normal','roughness','height','metalness'],output_format='png')
  (folder/'input.json').write_text(json.dumps(body,indent=2))
  sub=g.request('POST','https://queue.fal.run/fal-ai/patina/material',KEY,json_body=body,scheme='Key')
  state.write_text(json.dumps(sub,indent=2))
 for url in (sub['status_url'],sub['response_url']):g.check_url(url,('fal.run','fal.ai'),None)
 print(kind,index+1,'queued',flush=True)
 deadline=time.time()+3600
 while time.time()<deadline:
  status=g.request('GET',sub['status_url'],KEY,scheme='Key')
  if status['status']=='COMPLETED':break
  if status['status'] not in ('IN_QUEUE','IN_PROGRESS'):raise RuntimeError('Unexpected status')
  time.sleep(8)
 else:raise RuntimeError('Timed out; request preserved')
 result=g.request('GET',sub['response_url'],KEY,scheme='Key')
 for i,item in enumerate(result['images']):
  label=item.get('map_type') or 'preview'
  g.safe_download(item['url'],folder/f'{label}_{i}.png',('fal.media','fal.run','fal.ai'),None)
 (folder/'result.json').write_text(json.dumps(result,indent=2))
 print(kind,index+1,'downloaded',flush=True)
if __name__=='__main__':
 with concurrent.futures.ThreadPoolExecutor(max_workers=3) as pool:
  jobs={pool.submit(run,k,i):(k,i) for k in TYPES for i in range(3)}
  for future in concurrent.futures.as_completed(jobs):
   try:future.result()
   except BaseException as error:print('FAILED',jobs[future],type(error).__name__,str(error)[:180],flush=True)
