"""Review reel of every CAM (face full-frame) segment in edit.json, numbered, so eye contact can be checked.

    python -I review_cam.py   ->  build/cam_review.mp4 and build/cam_review.txt (the numbered list)

Take segments come straight from the raw take (in/out from edit.json); pickups from build/clean/.
A caption in the black bar at the right shows: number, section, take id, source time range.
"""
import json, subprocess
from pathlib import Path

HERE = Path(__file__).parent
OUT = HERE / "build" / "camreview"
FONT = Path("C:/Windows/Fonts/arial.ttf")

def main():
    OUT.mkdir(parents=True, exist_ok=True)
    import shutil; shutil.copy(FONT, OUT / "font.ttf")   # relative names keep drive-letter colons out of the filter
    edit = json.loads((HERE / "edit.json").read_text(encoding="utf-8"))
    items, parts = [], []
    n = 0
    for sec in edit:
        for g in sec["segments"]:
            if g["frame"] != "C":
                continue
            n += 1
            if g["kind"] == "take":
                src = HERE / g["src"]; ss, to = g["i"], g["o"]
                label = [f"#{n}", f"Section {sec['section']}", f"take {g['take']}", f"{g['i']:.0f}-{g['o']:.0f} s"]
                note = (g.get("note") or "")
            elif g.get("recorded"):
                src = HERE / g["recorded"][0]; ss, to = 0, None
                label = [f"#{n}", f"Section {sec['section']}", f"pickup {g['id']}", "(cleaned)"]
                note = g["text"][:60]
            else:
                n -= 1
                continue
            label.append(note[:34])
            txt = OUT / f"{n:02d}.txt"
            txt.write_text("\n".join(label), encoding="utf-8")
            dest = OUT / f"{n:02d}.mp4"
            vf = (f"scale=1280:720,fps=30,drawtext=fontfile=font.ttf:textfile={txt.name}:"
                  f"x=930:y=40:fontsize=26:fontcolor=white:line_spacing=14")
            cmd = ["ffmpeg", "-v", "error", "-y", "-ss", str(ss)]
            if to is not None:
                cmd += ["-to", str(to)]
            cmd += ["-i", str(src), "-vf", vf, "-c:v", "libx264", "-crf", "20", "-preset", "veryfast",
                    "-pix_fmt", "yuv420p", "-ar", "48000", "-ac", "2", "-c:a", "aac", "-b:a", "160k", str(dest)]
            subprocess.run(cmd, check=True, cwd=OUT)
            parts.append(dest)
            items.append(f"#{n:<2} sec {sec['section']:<4}{label[2]:<24}{label[3]:<14}{note[:50]}")
    lst = OUT / "list.txt"
    lst.write_text("\n".join(f"file '{p.as_posix()}'" for p in parts), encoding="utf-8")
    final = HERE / "build" / "cam_review.mp4"
    subprocess.run(["ffmpeg", "-v", "error", "-y", "-f", "concat", "-safe", "0", "-i", str(lst), "-c", "copy", str(final)], check=True)
    (HERE / "build" / "cam_review.txt").write_text("\n".join(items), encoding="utf-8")
    d = subprocess.run(["ffprobe", "-v", "error", "-show_entries", "format=duration", "-of", "csv=p=0", str(final)],
                       capture_output=True, text=True).stdout.strip()
    print(f"{n} clips, {float(d):.0f}s -> {final}")

main()
