from pathlib import Path
from datetime import datetime
root=Path(__file__).resolve().parents[2]
(root/'Docs/ENGINE_HANDOFF.md').write_text('''# Unreal engine coordination

Status: **FREE**
Updated: '''+datetime.now().isoformat(timespec='minutes')+''', ChatGPT GAME.
Owner: none. Candy environment pass saved and visually reviewed.
Village and hot chocolate captures: Docs/Devlog/CandyPBR/*-final.png.
No package or upload. Preserve unrelated Claude source edits and Android token.
See Docs/STATUS.md and Docs/Devlog/CandyPBR/README.md for remaining checks.
''',encoding='utf-8')
