"""Rebuild polygon-native comic cards: python Expansion/build.py."""
import json, math, random, hashlib
from functools import lru_cache
from pathlib import Path
from PIL import Image, ImageDraw
import importlib.util

HERE = Path(__file__).resolve().parent
spec = importlib.util.spec_from_file_location('original', HERE.parents[4] / 'Scripts/build_angular_comic_cards.py')
base = importlib.util.module_from_spec(spec)
spec.loader.exec_module(base)
base.DEST = HERE
base.range = lambda start, stop, step: range(200, stop, -16)

# Three short cries per attack, in each locale. Deliberate thematic adaptations.
WORDS = {
'en': ['MANNERS!','BE POLITE!','RESPECT!','BRUSH!','FLOSS!','CLEAN TEETH!','VITAMINS!','EAT WELL!','GET STRONG!'],
'zh-CN': ['礼貌！','客气点！','尊重！','刷牙！','用牙线！','清洁牙齿！','维生素！','好好吃饭！','强壮起来！'],
'ru': ['МАНЕРЫ!','БУДЬ ВЕЖЛИВ!','УВАЖЕНИЕ!','ЧИСТИ ЗУБЫ!','ЗУБНАЯ НИТЬ!','ЧИСТЫЕ ЗУБЫ!','ВИТАМИНЫ!','ЕШЬ ПРАВИЛЬНО!','СТАНЬ СИЛЬНЕЕ!'],
'pt-BR': ['MODOS!','SEJA GENTIL!','RESPEITO!','ESCOVE!','FIO DENTAL!','DENTES LIMPOS!','VITAMINAS!','COMA BEM!','FIQUE FORTE!'],
'es-ES': ['¡MODALES!','¡SÉ AMABLE!','¡RESPETO!','¡CEPÍLLATE!','¡HILO DENTAL!','¡DIENTES LIMPIOS!','¡VITAMINAS!','¡COME BIEN!','¡PONTE FUERTE!'],
'de': ['MANIEREN!','SEI HÖFLICH!','RESPEKT!','ZÄHNE PUTZEN!','ZAHNSEIDE!','SAUBERE ZÄHNE!','VITAMINE!','ISS GUT!','WERDE STARK!'],
'ja': ['礼儀！','丁寧に！','敬意！','歯磨き！','フロス！','歯をきれいに！','ビタミン！','しっかり食べて！','強くなれ！'],
'fr': ['LES BONNES MANIÈRES !','SOIS POLI !','RESPECT !','BROSSE TES DENTS !','FIL DENTAIRE !','DENTS PROPRES !','VITAMINES !','MANGE BIEN !','DEVIENS FORT !'],
'pl': ['MANIERY!','BĄDŹ UPRZEJMY!','SZACUNEK!','MYJ ZĘBY!','NIĆ DENTYSTYCZNA!','CZYSTE ZĘBY!','WITAMINY!','JEDZ ZDROWO!','ROŚNIJ W SIŁĘ!'],
'ko': ['예의!','친절하게!','존중!','양치해!','치실!','깨끗한 치아!','비타민!','잘 먹어!','강해져라!'],
'tr': ['TERBİYE!','KİBAR OL!','SAYGI!','DİŞLERİNİ FIRÇALA!','DİŞ İPİ!','TEMİZ DİŞLER!','VİTAMİNLER!','İYİ BESLEN!','GÜÇLEN!'],
'zh-TW': ['禮貌！','客氣點！','尊重！','刷牙！','用牙線！','清潔牙齒！','維生素！','好好吃飯！','強壯起來！'],
'th': ['มารยาท!','สุภาพหน่อย!','เคารพ!','แปรงฟัน!','ไหมขัดฟัน!','ฟันสะอาด!','วิตามิน!','กินดี!','แข็งแรงขึ้น!'],
'uk': ['МАНЕРИ!','БУДЬ ЧЕМНИМ!','ПОВАГА!','ЧИСТЬ ЗУБИ!','ЗУБНА НИТКА!','ЧИСТІ ЗУБИ!','ВІТАМІНИ!','ЇЖ ЗДОРОВО!','СТАВАЙ СИЛЬНІШИМ!'],
'es-419': ['¡MODALES!','¡SÉ AMABLE!','¡RESPETO!','¡CEPÍLLATE!','¡USA HILO DENTAL!','¡DIENTES LIMPIOS!','¡VITAMINAS!','¡COME BIEN!','¡PONTE FUERTE!'],
}

@lru_cache(maxsize=6)
def masks(seed):
    rng = random.Random(seed)
    # Unequal interlocking long blades and deep notches; no regular core panel.
    points=[]
    for i in range(32):
        a=2*math.pi*i/32
        r=rng.uniform(.80,1) if i%2==0 else rng.uniform(.43,.64)
        points.append((512+440*r*math.cos(a),512+400*r*math.sin(a)))
    layers=[]
    for scale in (1,.962,.895,.84):
        mask=Image.new('L',(1024,1024))
        ImageDraw.Draw(mask).polygon([(512+(x-512)*scale,512+(y-512)*scale) for x,y in points],fill=255)
        layers.append(mask)
    return tuple(layers)

def main():
    roster=[]; records=[]
    for lang, words in WORDS.items():
        font={'ja':'YuGothB.ttc','ko':'malgunbd.ttf','zh-CN':'msyhbd.ttc','zh-TW':'msjhbd.ttc','th':'tahomabd.ttf'}.get(lang,'arialbd.ttf' if lang in ('ru','uk') else 'impact.ttf')
        for i,text in enumerate(words):
            meaning=('Manners','Brush','Vitamins')[i//3]
            # Wrap at spaces, keeping short action phrases readable.
            if len(text)>15 and ' ' in text:
                chunks=text.split(); cut=min(range(1,len(chunks)),key=lambda n:abs(len(' '.join(chunks[:n]))-len(' '.join(chunks[n:]))))
                text=' '.join(chunks[:cut])+'\n'+' '.join(chunks[cut:])
            e=dict(id=f'{meaning}_{lang}_{i%3+1}_Shard',meaning=meaning.lower(),locale=lang,text=text,font=font,template=i%6,angle=(-5,3,-2)[i%3],outline=('#3866BB','#168B92','#D54629')[i//3],fill=('#FFF3D8','#E9FFF4','#FFF5CD')[i//3],ink='#17121C')
            roster.append(e)
            records.append(base.render(e))
        print(lang,flush=True)
    (HERE/'roster.json').write_text(json.dumps(roster,ensure_ascii=False,indent=2),encoding='utf-8')
    (HERE/'manifest.json').write_text(json.dumps(dict(cards=records,translation_review='Thematic adaptations; native review pending',geometry='Straight polygon segments at every border; no central panel',status='Source PNGs only; not imported'),ensure_ascii=False,indent=2),encoding='utf-8')
    for n in range(0,len(records),12): base.preview(records[n:n+12],HERE/f'preview-{n//12+1:02}.jpg')
    print(f'Validated {len(records)} RGBA cards')

base.template_masks=masks
if __name__=='__main__': main()
