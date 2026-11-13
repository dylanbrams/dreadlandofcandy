"""Verify current three-slot cast presentation without replaying completed source generation."""
from pathlib import Path
import json
import sys
import time
import re
import subprocess
sys.path.insert(0,str(Path(__file__).resolve().parent))
from verify_cast_delivery import run,python_stage,ROOT,ENGINE,PROJECT,LOGS

stage=sys.argv[1]
if stage=="assets":
    python_stage("import_class_presentations")
elif stage=="review":
    started=time.time()
    log=LOGS/"class-presentation-review.log"
    run("class-presentation-review",[ENGINE/"UnrealEditor-Cmd.exe",PROJECT,"-game","-backlot","-notitle","-scene=2",
        "-classreview","-RenderOffscreen","-unattended","-nosplash","-ResX=1280","-ResY=900","-quitat=50","-abslog="+str(log)])
    text=log.read_text(encoding="utf-8",errors="replace")
    assert "ClassReview: SUCCESS" in text
    assert "BrushTool:" in text and "BrushFoam:" in text
    expected=[(who,mode) for who in ("knight","mage","archer","barbarian") for mode in ("back","0","1","3")]+[("knight","brush")]
    for who,mode in expected:
        path=ROOT/f"Saved/Screenshots/WindowsEditor/classreview_{who}_{mode}.png"
        assert path.exists() and path.stat().st_mtime>=started and path.stat().st_size>10000,path
    for match in re.finditer(r"ClassReview: class=(\w+) mode=back weapon_min_z=([\d.-]+) floor=([\d.-]+)",text):
        assert float(match[2])-float(match[3])>=15,(match[1],"back weapon ground clearance",match[2],match[3])
    print("REVIEW PASS 12 attacks, 4 back carries, oversized brush and foam",flush=True)
elif stage=="play":
    for who in ("knight","mage","archer","barbarian"):
        log=LOGS/("class-play-"+who+".log")
        run("class-play-"+who,[ENGINE/"UnrealEditor-Cmd.exe",PROJECT,"-game","-backlot","-autoplay","-timescale=4",
            "-class="+who,"-RenderOffscreen","-unattended","-nosplash","-quitat=140",
            "-shotat=10","-shotname=classcooldown-"+who,"-abslog="+str(log)])
        text=log.read_text(encoding="utf-8",errors="replace")
        assert "JamAutoplay: PASS" in text,who
        assert "AttackParticles: missing" not in text,who
        for slot in (0,1,3): assert f"skill={slot} " in text,(who,slot)
        assert "JamWeapon:" in text and " back bone=" in text and " hand bone=" in text,who
        assert "BrushTool:" in text and "BrushFoam:" in text,who
elif stage=="tests":
    run("class-automation",["powershell","-NoProfile","-File",ROOT/"Scripts/run-tests.ps1","-NoBuild"])
    run("class-game-build",["powershell","-NoProfile","-File",ROOT/"Scripts/build.ps1","-Game"])
elif stage=="finish":
    for next_stage in ("review","tests","play"):
        subprocess.run([sys.executable,"-I",__file__,next_stage],cwd=ROOT,check=True)
else:
    raise SystemExit("Unknown stage")
