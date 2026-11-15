"""Generate edit.json from the segment list below (source: NARRATIVE.md), and report CAM streaks.

    python -I build_edit.py

Segment: (take, in, out, frame, visual, note)   frame: C=CAM  P=PIP  B=B-ROLL (voice only)
Pickup:  ("PICKUP", id, text, frame, est_seconds, visual)
Times are seconds in the source take; a pickup is a take Dylan still has to record (raw/pickups/<id>.mp4).
Within a segment, pauses over 0.35 s between words are tightened and um/uh spans (transcripts/fillers.json) cut at render time.
"""
import json, subprocess
from pathlib import Path

HERE = Path(__file__).parent
TOASTS = json.loads((HERE.parent / "GameFootage" / "toasts.json").read_text(encoding="utf-8"))
BED = "music/{}.mp3".format

S = {}
ORDER = ["00", "01", "02", "01b", "03", "03b", "04", "04b", "05", "06", "07", "08", "08b", "09", "10"]
S["00"] = dict(title="Cold open", bed=None, segs=[
    ("raw/pickups/BuildMeAnMMOPlease.wav", 0.0, 4.43, "B", "coldopen beat 1", "hopeful; Dylan re-recorded (Nov 15 evening)"),
    ("12-16-09", 12.4, 15.4, "B", "coldopen beat 2", "more tired"),
    ("12-16-38", 1.9, 6.2, "B", "coldopen beat 4", "confident"),
    ("12-16-38", 10.7, 12.1, "B", "coldopen beat 5 card", ""),
])
S["01"] = dict(title="The plain version", bed=BED("title-theme_take1"), segs=[
    ("12-17-35", 0.9, 11.5, "C", "", "opener"),
    ("PICKUP", "01-plain", "Four days later, here's what I ended up with: an open-world game with dynamic quests.", "B", 8, "world pan over the candy world, then village from above"),
])
S["01b"] = dict(title="How we got here", bed=BED("setting-out_take1"), segs=[
    ("12-18-01", 0.9, 7.4, "B", "molar-hills pan + caption 'I am older than them thar hills'", ""),
    ("12-18-01", 9.2, 20.0, "B", "molar-hills pan", ""),
    ("12-18-57", 6.9, 22.3, "B", "slide: MMO list over molar-hills", ""),
    ("12-20-17", 29.2, 40.3, "C", "", "scope punchline"),
    ("12-29-51", 0.3, 7.6, "B", "slide: idea", "thesis"),
    ("12-29-51", 22.5, 37.2, "C", "", "cut to molar-hills mid-way (15s)"),
])
S["02"] = dict(title="Hook montage", bed=BED("standard-combat_take1"), segs=[
    ("13-09-00", 0.3, 2.5, "C", "", "what did I end up with"),
    ("13-09-07", 8.3, 23.9, "B", "BROLL attack word cards, then non-English battle cry", "check 'talk back'"),
    ("13-09-43", 0.3, 12.1, "B", "BROLL knight placards, Resolve bar, pacified rat", "check 'have health'"),
    ("13-10-07", 0.5, 21.2, "B", "BROLL improvise screen, two new routes", ""),
    ("13-10-34", 0.6, 16.5, "B", "BROLL typed proposal, route, refusal", ""),
    ("13-11-24", 0.9, 10.4, "B", "architecture diagram", "AI on a leash: Director service diagram (graphics/architecture.png)"),
    ("PICKUP", "02-encores", "And the villagers keep coming back. Two to four encore jobs each, with twists: a time limit, a crowd, rats that fight back. Then a goodbye.", "B", 12, "BROLL encore offer, Rush timer, Crowd swarm"),
    ("13-12-35", 4.5, 5.7, "B", "BROLL film shoot", ""),
    ("PICKUP", "02-filmshoot", "Quests are scenes. Fenella yells CUT! and pitches rewrites. You can type director's notes mid-scene, and the scene changes. And when the budget runs out, the rats are replaced by cardboard boxes.", "B", 14, "BROLL CUT pitch, typed note, budget boxes"),
    ("PICKUP", "02-usabletake", "You're not saving the day. You're trying to get a usable take.", "C", 4, ""),
])
S["03"] = dict(title="The setup", bed=BED("setting-out_take1"), segs=[
    ("13-13-39", 0.9, 13.6, "B", "checkpoint slides", ""),
    ("13-13-39", 34.8, 39.9, "B", "checkpoint slides", ""),
    ("PICKUP", "03-declared", "Some setup and design work existed before the jam, and it's declared.", "B", 5, "Declared.md slide"),
    ("12-30-44", 23.8, 26.0, "C", "", "recording while an AI works"),
])
S["03b"] = dict(title="Reading the chart", bed=BED("setting-out_take1"), segs=[
    ("13-54-05", 4.4, 8.4, "P", "gantt full", ""),
    ("13-54-05", 20.2, 36.4, "P", "gantt: colour key, one bar each", "highlights"),
    ("PICKUP", "03b-warning", "A warning before anyone reads it like a leaderboard.", "C", 4, ""),
    ("13-54-05", 68.2, 80.4, "P", "gantt: approximate bars", "trim trailing 'and the'"),
    ("13-54-05", 118.8, 137.0, "P", "gantt: dim all but Saturday 18:00 overlap", ""),
    ("13-58-12", 2.0, 15.8, "C", "", "'what's not on the chart is me'; 14s: cut to PIP over full gantt after 8s"),
    ("13-58-41", 49.1, 57.2, "P", "gantt full", ""),
])
S["04"] = dict(title="Day one", bed=BED("quest-accepted_take1"), segs=[
    ("12-34-50", 2.2, 12.9, "B", "first-hero and autoplay captures", ""),
    ("12-35-30", 26.3, 31.9, "B", "early arena screenshot", "trim stutter"),
    ("12-36-41", 0.5, 13.8, "B", "HUD, Fenella captures", ""),
    ("12-36-41", 28.8, 40.7, "B", "early problem screenshots", ""),
])
S["04b"] = dict(title="Asset audition", bed=BED("mystery_take1"), segs=[
    ("13-16-35", 1.1, 15.7, "B", "rat comparison card", ""),
    ("12-38-12", 14.3, 18.0, "C", "", "'I picked the Rodan rat'; trim 'and then'"),
    ("13-16-35", 25.5, 35.3, "B", "rat in game, foliage", "check Rodin NPCs claim"),
    ("13-16-35", 37.4, 42.6, "B", "game camera view", ""),
    ("12-39-11", 0.5, 14.2, "B", "BROLL Blender cleanup / mesh view", "needs capture"),
])
S["05"] = dict(title="The world gets bigger", bed=BED("open-world-exploration_take1"), segs=[
    ("12-40-31", 0.5, 12.7, "B", "world pan, compass", "trim after 'arena game'?"),
    ("13-17-38", 9.3, 15.6, "B", "village", ""),
    ("13-17-38", 18.1, 48.3, "P", "quest givers footage", ""),
    ("13-17-38", 50.0, 56.9, "B", "release plan slide", ""),
])
S["06"] = dict(title="Playtest", bed=BED("standard-combat_take2"), segs=[
    ("PICKUP", "06-playtest", "This is where a list of finished tasks starts being a game. A green test can't tell me whether an attack feels funny, or whether walking across the river is any fun.", "B", 12, "BROLL compass, HUD, controls (optional section)"),
])
S["07"] = dict(title="The long night", bed=BED("hard-combat_take1"), segs=[
    ("13-18-57", 0.5, 6.5, "B", "village from above", ""),
    ("13-18-57", 57.8, 60.0, "B", "village", ""),
    ("13-20-06", 0.4, 18.9, "B", "attack effects in motion", ""),
    ("13-20-55", 0.5, 12.7, "B", "word cards, attacks", ""),
    ("13-20-55", 15.9, 18.8, "C", "", "'eat your vegetables!'"),
    ("13-20-55", 31.2, 51.8, "B", "celery arrows, flour cloud, card storm", "check '15 languages'"),
])
S["08"] = dict(title="Sunday morning", bed=BED("village_take2"), segs=[
    ("13-22-00", 0.5, 15.6, "C", "", "second release candidate; confirm itch upload count; cut to itch page after 8s"),
    ("PICKUP", "08-three", "That's the point where a sensible person stops adding things. Instead, three things happened at once.", "C", 6, ""),
    ("13-22-27", 8.4, 32.2, "B", "river, bridge, village from above", "check river timing"),
    ("PICKUP", "08-pbr", "ChatGPT generated fifteen candy PBR materials, and I picked mine from a review page.", "B", 6, "PBR contact sheet"),
    ("13-22-27", 37.8, 50.5, "B", "marshmallow contact sheet", "trim after 'time frame'"),
    ("PICKUP", "08-marshmallows", "Thirteen cylinders had been standing in for marshmallows. Now there are three generated ones, drifting downstream. Nobody needed this.", "C", 9, "marshmallows drifting under bridge (feeding-candy bed)"),
    ("13-22-27", 118.9, 120.6, "B", "UI concepts", ""),
    ("13-24-31", 7.4, 23.8, "B", "UI concept sheet", ""),
    ("PICKUP", "08-blacksugar", "Four UI directions were mocked up. I picked Black Sugar: pewter frames, peppermint enamel, proper serif fonts. I cut the full review short to save usage. The collaborators have a budget too.", "B", 16, "old title -> Black Sugar title"),
])
S["08b"] = dict(title="The river is terrible", bed=BED("boss-battle_take1"), segs=[
    ("13-25-32", 0.5, 3.2, "B", "fixes4.txt on screen", ""),
    ("13-25-32", 6.0, 8.0, "C", "", ""),
    ("13-25-32", 10.9, 14.3, "P", "fixes4 text", ""),
    ("13-25-53", 1.1, 23.2, "P", "fixes4 quotes text on screen", ""),
    ("PICKUP", "08b-fixed", "So: Controls is called Controls now. The flashes are about five times dimmer. The river got carved into a real channel with banks, the bridge stopped being flat, and fourteen hundred and thirty-two things growing in the middle of roads were removed.", "B", 16, "river before/after"),
    ("13-29-20", 0.6, 31.1, "B", "camera lifting over a canopy", ""),
    ("13-29-20", 32.6, 35.8, "C", "", "'Four days. One dude.'"),
])
S["09"] = dict(title="Why do this", bed=BED("emotional-revelation_take1"), segs=[
    ("13-30-15", 7.7, 27.8, "C", "", "20s on CAM: cut to B-roll every other sentence"),
    ("PICKUP", "09-lookatwhat", "Look at what's in here.", "B", 3, ""),
    ("13-30-15", 132.4, 165.3, "B", "one shot per draft-list item", "check list items and 154"),
    ("PICKUP", "09-green", "Every test was green when I wrote 'the river is terrible.'", "C", 4, "river-before on 'the river is terrible'"),
])
S["10"] = dict(title="Ending", bed=BED("victory_take1"), segs=[
    ("PICKUP", "10-ending", "That's the Dread Land of Candy. It's on itch.io, link below. It isn't an MMO. But you can lecture a rat, and your hero shouts in a hundred and fifty-four translations, and I'd call that a start.", "B", 20, "title screen, itch page; then credits_take1"),
])

# Gameplay B-roll picks from ../GameFootage/INDEX.md (30 fps, 1600x900 re-capture of 18:29): (substring of the
# segment's visual, [(file, in, out)], status). Index times can be 1-2 s early: scrub edges at render.
# status: ok | unconfirmed | missing (use a still/slide, capture later)
GF = "../GameFootage/"
FLY, PLAY = "LandOfCandyFlythrough.mp4", "DylanPlaythrough.mp4"   # Dylan's own; never scanned for toasts -> mask always
BROLL = [
    ("molar-hills pan", [("tour-high.mp4", 26, 34)], "ok"),
    ("over molar-hills", [("tour-high.mp4", 27, 33)], "ok"),
    ("attack word cards, then non-english", [("knight-run.mp4", 10, 20), ("village-improvise.mp4", 83, 92)], "ok (Turkish card, village-improvise 1:23)"),
    ("knight placards", [("knight-run.mp4", 10, 20)], "ok (pacify counter 1/3-3/3; no Resolve bar visible)"),
    ("improvise", [("village-improvise.mp4", 120, 130)], "ok"),
    ("typed proposal", [("village-propose.mp4", 40, 56)], "ok (silly refusal missing; outcome of proposal unclear)"),
    ("encore offer", [("village-third.mp4", 35, 45), ("village-third.mp4", 105, 115), (PLAY, 210, 229)], "ok (Crowd, Visit; Rush = clock objective in playthrough)"),
    ("film shoot", [("knight-run.mp4", 0, 8)], "ok"),
    ("cut pitch", [("knight-run.mp4", 43, 49), ("typed-note.mp4", 20, 35), ("budget-out.mp4", 10, 20)], "ok (CUT!, typed note, budget boxes)"),
    ("first-hero", [("hero-close.mp4", 0, 8)], "ok (late build, not day one)"),
    ("early arena", [], "missing"),
    ("hud, fenella", [("knight-run.mp4", 60, 64)], "ok"),
    ("early problem", [], "missing"),
    ("rat in game", [("knight-run.mp4", 20, 24), ("hero-close.mp4", 19, 24)], "ok"),
    ("world pan", [("tour-high.mp4", 11, 19)], "ok"),
    ("quest givers", [("village-improvise.mp4", 35, 45), ("village-improvise.mp4", 100, 115)], "ok (Mayor Bonbon, Baker Crumb)"),
    ("village from above", [("village-aerial.mp4", 0, 12), ("tour-high.mp4", 95, 105)], "ok"),
    ("village", [("tour-low.mp4", 97, 105), ("village-aerial.mp4", 0, 8)], "ok"),
    ("attack effects", [("knight-run.mp4", 10, 20), ("archer-run.mp4", 10, 20), ("barbarian-run.mp4", 45, 55), ("mage-run.mp4", 45, 50)], "ok"),
    ("word cards, attacks", [("knight-run.mp4", 75, 80), ("mage-run.mp4", 45, 50)], "ok"),
    ("celery arrows", [("archer-run.mp4", 10, 20), ("barbarian-run.mp4", 45, 55)], "ok"),
    ("river, bridge", [("tour-high.mp4", 25, 35), ("tour-high.mp4", 40, 44)], "ok"),
    ("marshmallows drifting", [("tour-high.mp4", 25, 35), ("tour-low.mp4", 40, 45)], "ok"),
    ("old title", [("title-idle.mp4", 0, 14)], "ok (OLD title screen missing)"),
    ("title screen", [("title-idle.mp4", 0, 14)], "ok"),
    ("river before", [(FLY, 58, 61), ("tour-high.mp4", 40, 44)], "partial: pre-fix bridge (flythrough) vs fixed bridge; no before/after river"),
    ("camera lifting", [("tour-low.mp4", 90, 97)], "ok: camera inside foliage (the bug); the 18:29 build fades foliage instead of lifting the camera"),
    ("compass", [("knight-run.mp4", 60, 64), ("controls-screen.mp4", 0, 8)], "ok (HUD, controls screen; compass not clearly visible)"),
    ("controls", [("controls-screen.mp4", 0, 10)], "ok"),
]

def toast_mask(name, i, o):
    """'always' for Dylan's unscanned recordings, else the toast ranges (+-2 s) that overlap [i, o]."""
    if name in (FLY, PLAY):
        return "always"
    hits = [[a, b] for a, b in TOASTS.get(name, []) if a - 2.0 <= o and b + 2.0 >= i]
    return hits or None

def broll_for(visual):
    v = visual.lower()
    for key, clips, status in BROLL:
        if key in v:
            out = []
            for f, a, b in clips:
                c = dict(src=GF + f, i=a, o=b)
                if f not in ("title-idle.mp4", "controls-screen.mp4"):
                    c["interp"] = True   # gameplay captures repeat 30-70% of their frames: interpolate real motion at render
                m = toast_mask(f, a, b)
                if m:
                    c["mask_bottom_right"] = m   # blur bottom-right 11.5% x 9.5% of the frame (see Trailer/build_demo.py mask_toast)
                out.append(c)
            return dict(clips=out, status=status)
    return None

def clean_clips(pid):
    """build/clean/<pid>*.mp4 -> [(relative path, seconds)], so recorded pickups replace the placeholder."""
    res = []
    for f in sorted((HERE / "build" / "clean").glob(pid + "*.mp4")):
        d = float(subprocess.run(["ffprobe", "-v", "error", "-show_entries", "format=duration", "-of", "csv=p=0", str(f)],
                                 capture_output=True, text=True).stdout.strip())
        res.append((f"build/clean/{f.name}", round(d, 1)))
    return res

def main():
    out, tot, camrun_flags = [], 0.0, []
    for sid in ORDER:
        sec = S[sid]
        segs, dur = [], 0.0
        for s in sec["segs"]:
            if s[0] in ("PICKUP", "BLOOPER"):
                d = s[4]
                g = dict(kind=s[0].lower(), id=s[1], text=s[2], frame=s[3], est=d, visual=s[5])
                clips = clean_clips(s[1]) if s[0] == "PICKUP" else []
                if clips:
                    g["recorded"] = [c[0] for c in clips]
                    d = g["dur"] = round(sum(c[1] for c in clips), 1)
                    g["partial"] = "partial" in clips[0][0]
                segs.append(g)
            else:
                d = round(s[2] - s[1], 1)
                segs.append(dict(kind="take", take=s[0], src=(s[0] if s[0].startswith("raw/") else f"raw/original/2026-11-15 {s[0]}.mp4"), i=s[1], o=s[2], dur=d,
                                 frame=s[3], visual=s[4], note=s[5]))
            dur += d
        for g in segs:
            if g["frame"] in "BP" and g.get("visual"):
                b = broll_for(g["visual"])
                if b:
                    g["broll"] = b
        for k, g in enumerate(segs):
            if g["frame"] == "C":
                d = g.get("dur", g.get("est", 0))
                if d > 8:
                    camrun_flags.append(f"{sid} {g.get('take', g.get('id'))} {d:.0f}s")
        out.append(dict(section=sid, title=sec["title"], bed=sec["bed"], est_s=round(dur, 1), segments=segs))
        cam = sum(g.get("dur", g.get("est", 0)) for g in segs if g["frame"] == "C")
        print(f"{sid:>3} {sec['title']:<24} {dur:6.1f}s  CAM {cam:5.1f}s  pickups {sum(1 for g in segs if g['kind']=='pickup')}")
        tot += dur
    (HERE / "edit.json").write_text(json.dumps(out, indent=1), encoding="utf-8")
    cam = sum(g.get("dur", g.get("est", 0)) for s in out for g in s["segments"] if g["frame"] == "C")
    print(f"TOTAL {tot/60:.1f} min ({tot:.0f}s), CAM {cam:.0f}s = {100*cam/tot:.0f}%")
    miss = sorted({g["visual"][:48] for s in out for g in s["segments"] if g.get("broll", {}).get("status", "").startswith(("missing", "partial", "unconf"))})
    print("B-roll missing/unconfirmed:", "; ".join(miss))
    print("CAM >8s:", "; ".join(camrun_flags))

if __name__ == "__main__":
    main()
