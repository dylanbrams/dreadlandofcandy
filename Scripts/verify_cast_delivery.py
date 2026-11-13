"""Run the scoped build/import/FX/gameplay validation sequence; logs stay in Saved."""
from pathlib import Path
import subprocess
import sys
import json
import time
import shutil

ROOT=Path(__file__).resolve().parents[1]
ENGINE=Path("D:/Program Files/Epic/UE_5.8/Engine/Binaries/Win64")
PROJECT=ROOT/"MegaJam.uproject"
LOGS=ROOT/"Saved/CastDelivery"
LOGS.mkdir(parents=True,exist_ok=True)

def run(label,command):
    print("START",label,flush=True)
    with (LOGS/(label+".stdout.log")).open("w",encoding="utf-8") as log:
        subprocess.run([str(c) for c in command],cwd=ROOT,stdout=log,stderr=subprocess.STDOUT,check=True)
    print("PASS",label,flush=True)

def python_stage(name,extra=()):
    run(name,[ENGINE/"UnrealEditor-Cmd.exe",PROJECT,"-run=pythonscript",
        "-script="+str(ROOT/"Scripts"/(name+".py")),"-unattended","-nullrhi","-nosound","-nop4",
        "-abslog="+str(LOGS/(name+".log")),*extra])

if __name__=="__main__":
    stage=sys.argv[1] if len(sys.argv)>1 else "imports"
    if stage=="imports":
        run("build",["powershell","-NoProfile","-File",ROOT/"Scripts/build.ps1"])
        python_stage("import_cast_skills")
        python_stage("import_cast_weapons",["-dpcvars=Interchange.FeatureFlags.Import.FBX=0"])
    elif stage=="weapons":
        python_stage("import_cast_weapons",["-dpcvars=Interchange.FeatureFlags.Import.FBX=0"])
    elif stage=="fx":
        run("fx-validation",[sys.executable,ROOT/"Scripts/validate_attack_fx.py"])
        started=time.time()
        run("test_cast_particles",[ENGINE/"UnrealEditor-Cmd.exe",PROJECT,
            "-ExecutePythonScript="+str(ROOT/"Scripts/test_cast_particles.py"),"-RenderOffscreen","-unattended","-nosound","-nop4",
            "-abslog="+str(LOGS/"test_cast_particles.log")])
        report=ROOT/"ImportSource/FX/live_parameter_validation.json"
        assert report.exists() and report.stat().st_mtime>=started
        assert len(json.loads(report.read_text()))==33
        run("fx-review",["powershell","-NoProfile","-File",ROOT/"Scripts/review_attack_fx.ps1"])
        run("fx-validation-fresh",[sys.executable,ROOT/"Scripts/validate_attack_fx.py"])
    elif stage=="tests":
        run("final-editor-build",["powershell","-NoProfile","-File",ROOT/"Scripts/build.ps1"])
        python_stage("finish_cast_materials")
        run("automation",["powershell","-NoProfile","-File",ROOT/"Scripts/run-tests.ps1","-NoBuild"])
        run("game-build",["powershell","-NoProfile","-File",ROOT/"Scripts/build.ps1","-Game"])
    elif stage=="play":
        python_stage("fix_arc_visibility")
        run("fx-review-readable-arc",["powershell","-NoProfile","-File",ROOT/"Scripts/review_attack_fx.ps1"])
        for who in ("knight","mage","archer","barbarian"):
            log=LOGS/("play-"+who+".log")
            run("play-"+who,[ENGINE/"UnrealEditor-Cmd.exe",PROJECT,"-game","-backlot","-autoplay","-timescale=4",
                "-class="+who,"-RenderOffscreen","-unattended","-nosplash","-quitat=180",
                "-shotat=22","-shotname=cast-"+who,"-abslog="+str(log)])
            text=log.read_text(encoding="utf-8",errors="replace")
            assert "JamAutoplay: PASS" in text,who
            assert "AttackParticles: missing" not in text,who
            assert "AttackParticles:" in text,who
            for skill in range(4):
                assert f"skill={skill} " in text,(who,skill)
            print("GAMEPLAY PASS",who,flush=True)
    elif stage=="package":
        assert (LOGS/"arc_review_passed.txt").exists(),"Shield arc visual review must pass before packaging"
        run("package",["powershell","-NoProfile","-File",ROOT/"Scripts/package.ps1"])
        run("packaged-backlot",["powershell","-NoProfile","-File",ROOT/"Scripts/smoke-packaged.ps1",
            "-RunSeconds","75","-Extra","-autoplay -timescale=4 -backlot -class=knight"])
        text=(ROOT/"Saved/Automation/Packaged-Smoke.log").read_text(encoding="utf-8",errors="replace")
        assert "JamAutoplay: PASS" in text
        assert "AttackParticles: missing" not in text
        shutil.copy2(ROOT/"Saved/Automation/Packaged-Smoke.log",LOGS/"packaged-backlot.log")
        run("packaged-open",["powershell","-NoProfile","-File",ROOT/"Scripts/smoke-packaged.ps1",
            "-Map","/Game/World/L_Sugarborough","-RunSeconds","90","-Extra","-autoplay -timescale=4 -open -class=barbarian"])
        text=(ROOT/"Saved/Automation/Packaged-Smoke.log").read_text(encoding="utf-8",errors="replace")
        assert "JamAutoplay: PASS mode=open main=3" in text
        assert "AttackParticles: missing" not in text
        shutil.copy2(ROOT/"Saved/Automation/Packaged-Smoke.log",LOGS/"packaged-open.log")
    elif stage=="accept_arc":
        # Called after inspecting the fresh K01 image: .8 s lifetime makes the arc readable.
        started=time.time()
        run("test_cast_particles-final",[ENGINE/"UnrealEditor-Cmd.exe",PROJECT,
            "-ExecutePythonScript="+str(ROOT/"Scripts/test_cast_particles.py"),"-RenderOffscreen","-unattended","-nosound","-nop4",
            "-abslog="+str(LOGS/"test_cast_particles-final.log")])
        report=ROOT/"ImportSource/FX/live_parameter_validation.json"
        assert report.exists() and report.stat().st_mtime>=started
        assert len(json.loads(report.read_text()))==33
        (LOGS/"arc_review_passed.txt").write_text("Fresh K01 capture inspected: arc and impact visible after lifetime/opacity repair.\n")
        subprocess.run([sys.executable,"-I",__file__,"package"],cwd=ROOT,check=True)
    elif stage=="finish":
        for next_stage in ("fx","tests","play","package"):
            subprocess.run([sys.executable,"-I",__file__,next_stage],cwd=ROOT,check=True)
    else:
        raise SystemExit("Unknown stage")
