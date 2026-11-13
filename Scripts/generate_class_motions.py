"""Class-appropriate gestures for the existing four skill mechanics; no Unreal launches."""
import argparse
import json
from pathlib import Path
import subprocess
import sys

ROOT=Path(__file__).resolve().parents[1]
sys.path.insert(0,str(ROOT/"Scripts"))
import genlib as g

MOTIONS={
    "Knight": {
        "KnightShield": "A heavily armoured knight braces his feet and raises his LEFT forearm across his chest as a shield, then pushes that forearm firmly forward to repel someone politely. RIGHT hand keeps an imaginary sword upright beside his right hip without swinging it. Strong restrained weight shift, action in first second then return to guard. No walking, no jump, no generated props.",
        "KnightWard": "A heavily armoured knight stands firm, RIGHT hand holding an imaginary sword upright in front of his chest. His empty LEFT palm rises and opens forward in a solemn protective ward, shoulders square and knees braced, then returns to ready. Clear defensive silhouette, complete in first second. Feet planted. No sword swing, no walking, no generated shield or props.",
    },
    "Mage": {
        "MageLecture": "A stern battle mage stands with his right hand holding an imaginary vertical staff beside his right hip. His LEFT palm thrusts forward to release a magic burst, fingers spread, chest turning into the spell. Gesture finishes in the first second, then recovers. Right staff hand stays steady and low. Feet planted. No sword raise, no walking, no generated props.",
        "MageLine": "A battle mage holds an imaginary upright staff steady in his RIGHT hand beside his hip. His LEFT arm draws back with an open palm then pushes straight forward decisively, channeling a narrow beam. Strong torso twist and recoil, gesture in first second then recover. Feet planted. Right arm never lifts overhead. No props, no sword movement, no walking.",
        "MageSigh": "A stern battle mage holds an imaginary upright staff steady in his RIGHT hand beside his hip. He takes a deep exasperated breath, spreads his LEFT hand outward in a circular dismissive gesture, shoulders fall with a forceful sigh, then recovers. Action in first second. Feet planted, no walking, no sword raise, no generated props.",
        "MageJudgment": "A commanding battle mage holds an imaginary upright staff steady in his RIGHT hand beside his hip. His LEFT arm reaches high overhead, palm open toward the sky, then pulls down forcefully to summon a magical rain. Chest lifted, knees brace on release. Finish gesture in first second and recover. Feet stay planted. No right arm sword raise, no props, no walking.",
    },
    "Archer": {
        "ArcherShot": "A skilled female archer fires an imaginary longbow held in her LEFT hand. Left arm extends forward at shoulder height, LEFT fist keeps a vertical bow grip. RIGHT hand draws the invisible string beside her cheek, releases crisply, then returns to ready. Real archery stance with left foot forward, feet stay planted. Complete shot in first second. No sword, no walking, no generated prop.",
        "ArcherLine": "A female archer aims a powerful precision shot with an imaginary longbow in her LEFT hand. LEFT arm extended forward with vertical grip, RIGHT hand draws to cheek with a clear shoulder and elbow pull, holds briefly then releases and recovers. Strong focused torso twist, feet firmly planted. Complete in first second. No sword raise, no walking, no generated prop.",
        "ArcherSigh": "A weary female archer holds an imaginary longbow lowered in her LEFT hand beside her left thigh. She shakes her head, exhales sharply, and sweeps her free RIGHT palm outward in a dismissive circular gesture, then returns to ready. Controlled athletic posture, feet planted, action in first second. No sword gesture, no walking, no generated bow or props.",
        "ArcherJudgment": "A female archer fires a skyward volley with an imaginary longbow in her LEFT hand. LEFT bow arm rises to aim forty-five degrees upward, RIGHT hand draws beside her cheek and releases twice in quick succession, then lowers to ready. Clear archery silhouette, left foot forward, no stepping. Complete volley within 1.3 seconds. No sword raise, no generated props, no walking.",
    },
    "Barbarian": {
        "BarbarianKnead": "A massive barbarian grips an imaginary heavy rolling pin in his RIGHT hand at waist height, barrel held forward horizontally. He drives the right forearm forward in two short forceful kneading pushes while his empty LEFT palm presses downward as if flattening invisible dough. Broad squat stance, torso works with the pushes, then recover. In place, no sword raise, no generated props.",
        "BarbarianCyclone": "A massive barbarian holds an imaginary heavy club in his RIGHT hand extended out at waist height. He crouches, coils his torso then makes one powerful full-body spinning turn in place, LEFT arm extended for balance, knees bent, and returns to a wide ready stance. Clear whirlwind silhouette. Complete turn within 1.3 seconds, no travel, no sword raise, no generated props.",
        "BarbarianBellow": "A huge barbarian braces in a wide stance, RIGHT hand holds an imaginary heavy wooden club upright beside his right shoulder. He thrusts his empty LEFT fist forward and bellows with chest out and head forward, then recovers. Powerful shoulders and torso recoil. Gesture in first second. Feet planted. No sword raise, no club swing, no walking, no generated props.",
        "BarbarianLine": "A huge barbarian holds an imaginary heavy club upright in his RIGHT hand beside his shoulder. His empty LEFT arm points sharply forward then sweeps sideways to order everyone into a line; chest twists and knees brace, then returns to ready. Broad commanding body language. Finish in first second. Feet planted. No sword raise, no club swing, no generated props.",
        "BarbarianSigh": "A massive barbarian keeps an imaginary heavy club upright in his RIGHT hand beside his shoulder. He throws his head back, clenches his empty LEFT fist against his chest, then exhales a tremendous frustrated bellow with shoulders dropping and torso leaning forward. Recover to ready. Action in first second. No sword raise, no club swing, no walking, no generated props.",
        "BarbarianJudgment": "A massive barbarian braces wide, imaginary heavy club upright in RIGHT hand beside his shoulder. He lifts his LEFT knee and stamps that foot down hard while pumping his empty LEFT fist overhead and bellowing, chest and shoulders shaking on impact, then recovers. Complete stomp in first second. Remain in place, no travel, no sword raise, no club swing, no generated props.",
    },
}

if __name__=="__main__":
    parser=argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--dry-run",action="store_true")
    args=parser.parse_args()
    if not args.dry_run:
        balance=g.request("GET","https://api.meshy.ai/openapi/v1/balance",g.env_value("MESHY_API_KEY"))
        print("Meshy balance:",balance.get("balance",balance.get("credit_balance","available")),flush=True)
    for who,motions in MOTIONS.items():
        rig=f"SK_{who}_Rig"
        folder=ROOT/"ImportSource/Meshy"/rig
        assert (folder/(rig+".fbx")).exists()
        assert json.loads((folder/"rig.json").read_text())["rig_task"]
        command=[sys.executable,"-I",str(ROOT/"Scripts/meshy_rig.py"),"--task-id","existing-rig","--name",rig]
        for label,prompt in motions.items():
            assert len(prompt)<=400,(label,len(prompt))
            command += ["--motion",f"{label}=2:{prompt}"]
        if args.dry_run:
            print(json.dumps({"character":who,"motions":motions},indent=2))
        else:
            print("Generating class gestures:",who,flush=True)
            subprocess.run(command,cwd=ROOT,check=True)
