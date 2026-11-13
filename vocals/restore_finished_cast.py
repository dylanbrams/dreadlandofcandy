"""Restore completed knight/mage assets and manifest rows from prior-cast archive."""
import json, shutil
from pathlib import Path
root=Path(__file__).resolve().parent
archive=root/'output/archive/prior-cast'
active=root/'output/attacks'
old=json.loads((archive/'manifest.json').read_text(encoding='utf-8'))
cur=json.loads((active/'manifest.json').read_text(encoding='utf-8'))
for ident,row in old['clips'].items():
    if ident.startswith(('VO_K','VO_M')):
        for key in ('mp3','wav'):
            relative=Path(row[key]).relative_to('output/attacks')
            source=archive/relative
            target=active/relative
            target.parent.mkdir(parents=True,exist_ok=True)
            shutil.copy2(source,target)
        cur['clips'][ident]=row
(active/'manifest.json').write_text(json.dumps(cur,ensure_ascii=False,indent=2)+'\n',encoding='utf-8')
print('restored finished knight and mage clips')
