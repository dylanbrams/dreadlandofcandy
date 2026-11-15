"""Assemble the rough cut from edit.json: build/devlog_rough_v1.mp4.

    python -I render_rough.py              # everything (cached; only changed segments re-render)
    python -I render_rough.py --fresh      # ignore the cache
    python -I render_rough.py --only 12 13 # render just these segment numbers (no final mix)

Per segment (build/seg/NNN.mp4 = silent picture, NNN.wav = voice, exact lengths):
  voice   = the take's words in [in, out], um/uh removed, long pauses cut (clean_takes.spans), loudnorm -16 LUFS,
            plus a short tail.
  picture = B  : game footage (blurred bottom-right where a Discord toast was seen), stills, or a placeholder card
            C  : Dylan keyed (key_still.key) at full height over a blurred copy of the section's footage
            P  : the B picture with Dylan keyed small, bottom-right
Section 0 (cold open) is the coldopen.html render with the voice lines and a click track laid on it.
Music beds are mixed last, ducked under the voice (sidechaincompress). No theme-song audio except the blooper.
"""
import hashlib, json, math, re, subprocess, sys, textwrap, wave
from concurrent.futures import ProcessPoolExecutor
from pathlib import Path

import cv2
import numpy as np
from PIL import Image, ImageDraw, ImageFont

HERE = Path(__file__).parent
DEVLOG = HERE.parent
ROOT = HERE.parents[2]
sys.path.insert(0, str(HERE))
import clean_takes as ct      # noqa: E402
import key_still as ks        # noqa: E402

BUILD = HERE / "build"
SEG = BUILD / "seg"
TMP = BUILD / "tmp"
W, H, FPS = 1920, 1080, 30
TAIL = 0.25
FONT = "C:/Windows/Fonts/arial.ttf"
FONT_B = "C:/Windows/Fonts/arialbd.ttf"
VERSION = "v2"
FOOTAGE_REV = "stall1"   # bump to re-render every piece that shows game footage   # bump to invalidate every cached segment

STILLS = [  # (substring of visual, [images])
    ("slide: sound", [HERE / "graphics/slide_sound.png"]),
    ("slide: bot", [HERE / "graphics/slide_bot.png"]),
    ("architecture stage 1", [HERE / "graphics/architecture_s1.png"]),
    ("architecture stage 2", [HERE / "graphics/architecture_s2.png"]),
    ("architecture stage 3", [HERE / "graphics/architecture_s3.png"]),
    ("architecture stage 4", [HERE / "graphics/architecture_s4.png"]),
    ("architecture stage 5", [HERE / "graphics/architecture.png"]),
    ("slide: idea", [HERE / "graphics/slide_idea.png"]),
    ("checkpoint slides", [HERE / "graphics/slide_team.png"]),
    ("declared.md slide", [HERE / "graphics/slide_declared.png"]),
    ("release plan slide", [HERE / "graphics/slide_release.png"]),
    ("blender cleanup", [HERE / "graphics/slide_cleanup.png"]),
    ("one shot per draft-list item", [HERE / "graphics/slide_inside.png"]),
    ("architecture diagram", [HERE / "graphics/architecture.png"]),
    ("gantt", [DEVLOG / "Checkpoint1/gantt/gantt_slide.png"]),
    ("rat comparison", [DEVLOG / "Tomorrow/assets/generation-comparison.jpg"]),
    ("pbr contact", [DEVLOG / "CandyPBR/contact.jpg"]),
    ("marshmallow contact", [DEVLOG / "Marshmallows/Marshmallow_contact_sheet.jpg"]),
    ("ui concept", [ROOT / "Docs/Design/CandyUI" / n for n in
                    ("candy-arcade.png", "frosted-stickerbook.png", "saccharine-reliquary.png", "black-sugar-frostbound.png")]),
]
FIXES4 = [  # verbatim, from SCRIPT.md section 8b
    'Controls are hidden under "Notes" on the title menu. These are the ONE THING THAT SHOULD NOT HAVE A JOKE LABEL.',
    "The archer's flashes are the right idea, but a little too big and bright. It hurts.",
    "The river is terrible. It's a single ribbon with no depth above the ground. The bridge is flattened, for some reason, and looks like crap.",
    "There's a house actually sitting on the path, and a tree in the middle of where a road should be.",
]
BED_VOL = {"credits_take1": -8}   # dB before ducking; default below
BED_DEFAULT_VOL = -12


# ---------------------------------------------------------------- helpers
def ff(*args, cwd=None):
    subprocess.run(["ffmpeg", "-v", "error", "-y", *map(str, args)], check=True, cwd=cwd)


def dur(path):
    r = subprocess.run(["ffprobe", "-v", "error", "-show_entries", "format=duration", "-of", "csv=p=0", str(path)],
                       capture_output=True, text=True)
    return float(r.stdout.strip())


def font(size, bold=False):
    return ImageFont.truetype(FONT_B if bold else FONT, size)


def make_card(path, title, body_lines, small=None, body_size=54):
    im = Image.new("RGB", (W, H), (38, 26, 42))
    d = ImageDraw.Draw(im)
    d.text((80, 60), title, font=font(34, True), fill=(255, 190, 90))
    y = 200
    for ln in body_lines:
        for w in textwrap.wrap(ln, 44 if body_size >= 50 else 62) or [""]:
            d.text((120, y), w, font=font(body_size), fill=(240, 235, 245))
            y += int(body_size * 1.35)
        y += 30
    if small:
        d.text((80, H - 90), small, font=font(28), fill=(170, 160, 175))
    im.save(path)
    return path


def fit_canvas(src, path, scale=2):
    """Contain-fit an image on a dark 2x canvas (for the slow push)."""
    im = Image.open(src).convert("RGB")
    cw, ch = W * scale, H * scale
    s = min(cw / im.width, ch / im.height)
    im = im.resize((int(im.width * s), int(im.height * s)), Image.LANCZOS)
    c = Image.new("RGB", (cw, ch), (24, 20, 28))
    c.paste(im, ((cw - im.width) // 2, (ch - im.height) // 2))
    c.save(path)
    return path


# ---------------------------------------------------------------- voice
def voice_parts(g):
    if g["kind"] == "take":
        src = HERE / g["src"]
        words = json.loads(src.with_suffix(".words.json").read_text(encoding="utf-8"))["words"]
        return [(src, ct.spans(words, [(g["i"], g["o"])], [], complete=not g.get("exact")))]
    if g["kind"] == "pickup":
        return [(HERE / p, None) for p in g["recorded"]]
    return []


def build_voice(g, out_mp4, video):
    """Concat the kept spans (um/uh and long pauses cut). Writes out_mp4 (video+audio) or an audio-only wav."""
    parts = voice_parts(g)
    inputs, fc, labels, n = [], [], "", 0
    for k, (src, sp) in enumerate(parts):
        inputs += ["-i", src]
        if sp is None:
            sp = [(0.0, dur(src))]
        for s, e in sp:
            d = e - s
            if video:
                fc.append(f"[{k}:v]trim=start={s:.3f}:end={e:.3f},setpts=PTS-STARTPTS,fps={FPS}[v{n}]")
            fc.append(f"[{k}:a]atrim=start={s:.3f}:end={e:.3f},asetpts=PTS-STARTPTS,aresample=48000,"
                      f"aformat=channel_layouts=stereo,afade=t=in:d=0.03,afade=t=out:st={max(d - 0.03, 0):.3f}:d=0.03[a{n}]")
            labels += f"[v{n}][a{n}]" if video else f"[a{n}]"
            n += 1
    fc.append(f"{labels}concat=n={n}:v={1 if video else 0}:a=1" + ("[v][a]" if video else "[a]"))
    scr = out_mp4.with_suffix(".filter.txt")
    scr.write_text(";".join(fc), encoding="utf-8")
    if video:
        ff(*inputs, "-/filter_complex", scr, "-map", "[v]", "-map", "[a]", "-c:v", "libx264", "-crf", "16",
           "-preset", "veryfast", "-pix_fmt", "yuv420p", "-c:a", "pcm_s16le", out_mp4.with_suffix(".mkv"))
        scr.unlink()
        return out_mp4.with_suffix(".mkv")
    ff(*inputs, "-/filter_complex", scr, "-map", "[a]", "-c:a", "pcm_s16le", out_mp4.with_suffix(".wav"))
    scr.unlink()
    return out_mp4.with_suffix(".wav")


def finish_voice(src, wav_out, D):
    """loudnorm to about -16 LUFS, pad/trim to exactly D seconds."""
    ff("-i", src, "-vn", "-af", f"loudnorm=I=-16:TP=-1.5:LRA=11,apad=whole_dur={D:.5f},atrim=0:{D:.5f}",
       "-ar", 48000, "-ac", 2, "-c:a", "pcm_s16le", wav_out)


def lufs(path):
    """Integrated loudness (ebur128) of a short clip."""
    r = subprocess.run(["ffmpeg", "-hide_banner", "-nostats", "-i", str(path), "-af", "ebur128", "-f", "null", "-"],
                       capture_output=True, text=True)
    m = re.findall(r"I:\s+(-?[\d.]+) LUFS", r.stderr)
    return float(m[-1]) if m else None


def relevel(path, target=-16.0, tol=0.3):
    """Measure, apply the exact gain, re-measure; up to 4 corrections. loudnorm alone can miss by 1 dB+ on 3-4 s clips."""
    cur = lufs(path)
    if cur is None or cur < -69:        # silence
        return cur
    if abs(cur - target) <= tol:
        return cur
    src = path.with_suffix(".src.wav")
    path.replace(src)
    total = 0.0
    for _ in range(4):
        total += target - cur
        ff("-i", src, "-af", f"volume={total:.2f}dB,alimiter=limit=0.95:level=disabled", "-c:a", "pcm_s16le", path)
        cur = lufs(path)
        if abs(cur - target) <= tol:
            break
    src.unlink()
    return cur


# ---------------------------------------------------------------- picture
def mask_filter(clip_mask):
    if not clip_mask:
        return ""
    x0, y0, w, h = int(W * 0.885), int(H * 0.905), W - int(W * 0.885), H - int(H * 0.905)
    return (f",split[a][b];[b]crop={w}:{h}:{x0}:{y0},scale=14:6,scale={w}:{h}:flags=bicubic,boxblur=6[bb];"
            f"[a][bb]overlay={x0}:{y0}")


TOASTS = json.loads((DEVLOG / "GameFootage" / "toasts.json").read_text(encoding="utf-8"))
UNSCANNED = ("LandOfCandyFlythrough.mp4", "DylanPlaythrough.mp4")


def window_mask(name, start, n):
    """Discord-toast blur for the window ACTUALLY used (start..start+n): 'always' for Dylan's unscanned recordings,
    else the toast ranges (+-2 s) overlapping it."""
    if name in UNSCANNED:
        return "always"
    return [[a, b] for a, b in TOASTS.get(name, []) if a - 2.0 <= start + n and b + 2.0 >= start] or None


STALLS_FILE = HERE / "build" / "stalls.json"
STALLS = json.loads(STALLS_FILE.read_text(encoding="utf-8")) if STALLS_FILE.exists() else {}
STATIC_UI = ("title-idle.mp4", "controls-screen.mp4")          # meant to be still: give them a slow push instead
KEEP_STALLS = ("improvise", "typed proposal", "encore offer")    # offer screens that wait for the player: holds are real


def cut_stalls(name, start, n, sd):
    """Split [start, start+n] around game stalls (>= 0.5 s of frozen frames): keep 0.08 s of each, drop the rest, and run
    on further into the source to make up the length. Returns [(start, length), ...]."""
    stalls = STALLS.get(name, [])
    parts, cursor, need = [], start, n
    while need > 0.05 and cursor < sd - 0.1:
        st = next((s for s in stalls if s[1] > cursor and s[0] < cursor + need), None)
        if not st:
            parts.append((cursor, min(need, sd - cursor - 0.05)))
            break
        if st[0] <= cursor:                       # we are inside a stall: skip to its end
            cursor = st[1]
            continue
        take = st[0] + 0.08 - cursor
        if take > 0.05:
            parts.append((cursor, take))
            need -= take
        cursor = st[1]
    parts = [pt for pt in parts if pt[1] >= 0.35]            # slivers make empty files downstream
    return parts or [(start, n)]


def plan_windows(clips, D, keep_stalls=False):
    """Sub-clips share D in proportion to their length; a window is extended into the source if D is longer."""
    clips = list(clips)
    while len(clips) > 1 and D / len(clips) < 1.5:
        clips.pop()
    tot = sum(c["o"] - c["i"] for c in clips)
    out = []
    for c in clips:
        src = DEVLOG / c["src"].replace("../", "")
        n = D * (c["o"] - c["i"]) / tot
        avail = c["o"] - c["i"]
        start = c["i"] if n <= avail else max(0.0, c["i"] - (n - avail) / 2)
        sd = dur(src)
        n = min(n, sd - start - 0.05)
        if keep_stalls or src.name in STATIC_UI or src.name in UNSCANNED:
            out.append((c, src, start, n))
        else:
            out.extend((c, src, s, l) for s, l in cut_stalls(src.name, start, n, sd))
    return out


def footage_video(clips, D, out, tag, blur=False, keep_stalls=False):
    files = []
    for k, (c, src, start, n) in enumerate(plan_windows(clips, D, keep_stalls)):
        mask = window_mask(src.name, start, n)
        if c.get("interp"):
            # drop the repeated frames, then synthesise real in-betweens (motion-compensated) at 30 fps
            vf = (f"scale=1280:720,mpdecimate,minterpolate=fps={FPS}:mi_mode=mci:mc_mode=aobmc:me_mode=bidir:vsbmc=1,"
                  f"scale={W}:{H}:flags=lanczos" + mask_filter(mask))
        else:
            vf = f"scale={W}:{H}:flags=lanczos,fps={FPS}" + mask_filter(mask)
        if src.name in STATIC_UI:
            vf = (f"scale={W}:{H}:flags=lanczos,zoompan=z='min(1+0.0005*on,1.12)':x='iw/2-(iw/zoom/2)':y='ih/2-(ih/zoom/2)'"
                  f":d=1:s={W}x{H}:fps={FPS}")
        if blur:
            vf += ",gblur=sigma=28,eq=brightness=-0.14"
        f = TMP / f"{tag}_{k}.mp4"
        ff("-ss", f"{start:.3f}", "-t", f"{n:.3f}", "-i", src, "-vf", vf, "-an", "-c:v", "libx264", "-crf", "17",
           "-preset", "veryfast", "-pix_fmt", "yuv420p", f)
        if f.stat().st_size > 2000:                          # an empty result (interpolation had nothing to work with)
            files.append(f)
    if not files:                                            # last resort: the plain window, no stall cutting, no interpolation
        c, src, start, n = plan_windows(clips, D, True)[0]
        f = TMP / f"{tag}_plain.mp4"
        ff("-ss", f"{start:.3f}", "-t", f"{n:.3f}", "-i", src, "-vf", f"scale={W}:{H}:flags=lanczos,fps={FPS}", "-an",
           "-c:v", "libx264", "-crf", "17", "-preset", "veryfast", "-pix_fmt", "yuv420p", f)
        files.append(f)
    return concat_video(files, D, out, tag)


def concat_video(files, D, out, tag):
    N = round(D * FPS)
    lst = TMP / f"{tag}_list.txt"
    lst.write_text("\n".join(f"file '{f.as_posix()}'" for f in files), encoding="utf-8")
    # hold the last frame if the footage ran short
    ff("-f", "concat", "-safe", 0, "-i", lst, "-vf", "tpad=stop_mode=clone:stop_duration=3", "-frames:v", N, "-an",
       "-c:v", "libx264", "-crf", "17", "-preset", "veryfast", "-pix_fmt", "yuv420p", out)
    return out


def still_video(imgs, D, out, tag, blur=False):
    files = []
    per = D / len(imgs)
    for k, im in enumerate(imgs):
        big = fit_canvas(im, TMP / f"{tag}_c{k}.png")
        f = TMP / f"{tag}_s{k}.mp4"
        frames = max(2, round(per * FPS))
        vf = (f"zoompan=z='1+0.0003*on':x='iw/2-(iw/zoom/2)':y='ih/2-(ih/zoom/2)':d=1:s={W}x{H}:fps={FPS}"
              + (",gblur=sigma=28,eq=brightness=-0.14" if blur else ""))
        ff("-loop", 1, "-framerate", FPS, "-i", big, "-vf", vf, "-frames:v", frames, "-an", "-c:v", "libx264",
           "-crf", "17", "-preset", "veryfast", "-pix_fmt", "yuv420p", f)
        files.append(f)
    return concat_video(files, D, out, tag)


def still_image_video(img, D, out, tag, blur=False):
    f = TMP / f"{tag}_still.mp4"
    N = round(D * FPS) + 3
    vf = "fps=30" + (",gblur=sigma=28,eq=brightness=-0.14" if blur else "")
    ff("-loop", 1, "-framerate", FPS, "-i", img, "-vf", vf, "-frames:v", N, "-an", "-c:v", "libx264", "-crf", "17",
       "-preset", "veryfast", "-pix_fmt", "yuv420p", f)
    return concat_video([f], D, out, tag)


def add_caption(video, text, tag):
    """Burn a bottom caption onto a finished picture (the 'older than them thar hills' joke)."""
    png = TMP / f"{tag}_cap.png"
    im = Image.new("RGBA", (W, H), (0, 0, 0, 0))
    d = ImageDraw.Draw(im)
    f = font(58, True)
    w = d.textlength(text, font=f)
    d.rectangle([(W - w) / 2 - 30, H - 190, (W + w) / 2 + 30, H - 100], fill=(0, 0, 0, 150))
    d.text(((W - w) / 2, H - 182), text, font=f, fill=(255, 245, 220, 255))
    im.save(png)
    tmp = video.with_name(video.stem + "_cap.mp4")
    ff("-i", video, "-i", png, "-filter_complex", "overlay", "-an", "-c:v", "libx264", "-crf", "17", "-preset", "veryfast",
       "-pix_fmt", "yuv420p", tmp)
    tmp.replace(video)


def early_stills():
    pics = sorted((DEVLOG / "Screenshots").glob("*.png"), key=lambda p: p.stat().st_mtime)
    return pics[:3]


def picture(g, D, out, tag, blur=False):
    """Silent 1920x1080 picture of exactly round(D*30) frames for a B/P segment's visual."""
    v = (g.get("visual") or "").lower()
    b = g.get("broll") or {}
    clips = b.get("clips") or []
    if clips:
        return footage_video(clips, D, out, tag, blur, keep_stalls=any(k in v for k in KEEP_STALLS))
    if "fixes4" in v:
        return still_image_video(make_card(TMP / f"{tag}_card.png", "FIXES4.TXT, VERBATIM", FIXES4, body_size=40), D, out, tag, blur)
    if "early" in v and early_stills():
        return still_video(early_stills(), D, out, tag, blur)
    for key, imgs in STILLS:
        if key in v:
            return still_video([i for i in imgs if i.exists()], D, out, tag, blur)
    if v == "black" or v.startswith("black"):
        ff("-f", "lavfi", "-i", f"color=black:s={W}x{H}:r={FPS}", "-frames:v", round(D * FPS), "-c:v", "libx264", "-pix_fmt", "yuv420p", out)
        return out
    status = b.get("status", "")
    return still_image_video(make_card(TMP / f"{tag}_card.png", "B-ROLL NEEDED (placeholder)",
                                       [g.get("visual") or "(no visual specified)"], small=status or None, body_size=54),
                             D, out, tag, blur)


# ---------------------------------------------------------------- composite (CAM / PIP)
def cam_backdrop():
    """Soft plum radial gradient for CAM shots (cached PNG)."""
    out = TMP / "cam_backdrop.png"
    if not out.exists():
        yy, xx = np.mgrid[0:H, 0:W].astype(np.float32)
        r = np.sqrt(((xx - W / 2) / (W * 0.62)) ** 2 + ((yy - H * 0.42) / (H * 0.75)) ** 2)
        k = np.clip(1 - r, 0, 1)[..., None] ** 1.3
        c0, c1 = np.array([30, 22, 38], np.float32), np.array([88, 62, 96], np.float32)      # RGB: edge, centre
        img = (c0 + (c1 - c0) * k).astype(np.uint8)[..., ::-1]
        cv2.imwrite(str(out), np.ascontiguousarray(img))
    return out


def wall_x_for(g):
    return 820 if g.get("kind") == "take" and g["take"].startswith("12-") else None


def composite(mode, person, bg, wav, D, out, wall):
    N = round(D * FPS)
    cp = cv2.VideoCapture(str(person))
    cb = None if str(bg).endswith(".png") else cv2.VideoCapture(str(bg))
    enc = subprocess.Popen(["ffmpeg", "-v", "error", "-y", "-f", "rawvideo", "-pix_fmt", "bgr24", "-s", f"{W}x{H}",
                            "-r", str(FPS), "-i", "-", "-frames:v", str(N), "-an", "-c:v", "libx264", "-crf", "17",
                            "-preset", "veryfast", "-pix_fmt", "yuv420p", str(out)], stdin=subprocess.PIPE)
    last = None
    prev_a = None
    bg_last = cv2.imread(str(bg)) if cb is None else np.zeros((H, W, 3), np.uint8)
    for _ in range(N):
        if cb is None:
            bgf = bg_last
        else:
            okb, bgf = cb.read()
            if okb:
                bg_last = bgf
            else:
                bgf = bg_last
        okp, pf = cp.read()
        if okp:
            fg, a = ks.key(pf, wall)
            if prev_a is not None and prev_a.shape == a.shape:
                a = 0.65 * a + 0.35 * prev_a
            prev_a = a
            last = (fg, a)
        elif last is None:
            last = (np.zeros((720, ks.CROP_W, 3), np.float32), np.zeros((720, ks.CROP_W), np.float32))
        fg, a = last
        frame = bgf.astype(np.float32)
        if mode == "C":
            s = 0.9 * H / fg.shape[0]                     # about 90% of the frame height, sat on the bottom edge
            f = cv2.resize(fg, None, fx=s, fy=s, interpolation=cv2.INTER_AREA)
            m = cv2.resize(a, None, fx=s, fy=s, interpolation=cv2.INTER_AREA)
            x0, y0 = (W - f.shape[1]) // 2, H - f.shape[0]
        else:
            ph = 400
            pw = int(ks.CROP_W * ph / fg.shape[0])
            f = cv2.resize(fg, (pw, ph), interpolation=cv2.INTER_AREA)
            m = cv2.resize(a, (pw, ph), interpolation=cv2.INTER_AREA)
            x0, y0 = W - pw - 30, H - ph - 20
        h, w = f.shape[:2]
        roi = frame[y0:y0 + h, x0:x0 + w]
        frame[y0:y0 + h, x0:x0 + w] = f * m[..., None] + roi * (1 - m[..., None])
        enc.stdin.write(np.clip(frame, 0, 255).astype(np.uint8).tobytes())
    enc.stdin.close()
    enc.wait()
    return out


def migrate_old_names():
    """One-off: pieces rendered under position numbers (NNN.*) are renamed to their content hash (read from NNN.key)."""
    for k in sorted(SEG.glob("[0-9][0-9][0-9].key")):
        key = k.read_text().strip().replace("m2:", "")
        for ext in ("mp4", "wav"):
            src, dst = k.with_suffix("." + ext), SEG / f"{key}.{ext}"
            if src.exists() and not dst.exists():
                src.replace(dst)
            elif src.exists():
                src.unlink()
        (SEG / f"{key}.key").write_text(key)
        k.unlink()


# ---------------------------------------------------------------- one segment
def seg_key(job):
    rev = FOOTAGE_REV if (job["g"].get("broll") or {}).get("clips") else ""
    return hashlib.sha1((VERSION + rev + json.dumps(job["g"], sort_keys=True, default=str) + job["section"]
                         + json.dumps(job.get("bg"), sort_keys=True, default=str)).encode()).hexdigest()[:10]


def render_segment(job):
    idx, g, sec = job["idx"], job["g"], job["section"]
    tag = f"{idx:03d}"                      # temp-file tag only
    key = seg_key(job)                      # the piece's permanent name: a hash of its contents, so reordering re-renders nothing
    stamp = SEG / f"{key}.key"
    if stamp.exists() and (SEG / f"{key}.mp4").exists() and (SEG / f"{key}.wav").exists():
        return idx, dur(SEG / f"{key}.wav")
    TMP.mkdir(parents=True, exist_ok=True)
    out_v, out_w = SEG / f"{key}.mp4", SEG / f"{key}.wav"
    if g["kind"] == "blooper":
        D = math.ceil((dur(DEVLOG / g["text"]) + 0.3) * FPS) / FPS
        ff("-i", DEVLOG / g["text"], "-af", f"apad=whole_dur={D:.5f},atrim=0:{D:.5f}", "-ar", 48000, "-ac", 2, "-c:a", "pcm_s16le", out_w)
        picture({"visual": "black"}, D, out_v, tag)
        stamp.write_text("m2:" + key)      # m2 = toast blur follows the window actually used
        return idx, D
    mode = g["frame"]
    video = mode in "CP"
    raw = build_voice(g, TMP / f"{tag}_voice", video)
    D = math.ceil((dur(raw) + TAIL) * FPS) / FPS
    finish_voice(raw, out_w, D)
    if sec == "00":                      # cold open: only the voice is needed, the picture is coldopen.html
        stamp.write_text("m2:" + key)      # m2 = toast blur follows the window actually used
        out_v.write_bytes(b"")
        return idx, D
    if mode == "B":
        picture(g, D, out_v, tag)
        if "older than them thar hills" in (g.get("visual") or ""):
            add_caption(out_v, "I am older than them thar hills.", tag)
    else:
        if mode == "C":
            bgv = cam_backdrop()                       # a calm plum backdrop; blurred game footage behind a face looked like junk
        else:
            bgv = picture(g, D, TMP / f"{tag}_bg.mp4", tag + "bg")
        composite(mode, raw, bgv, out_w, D, out_v, wall_x_for(g))
    stamp.write_text("m2:" + key)      # m2 = toast blur follows the window actually used
    return idx, D


# ---------------------------------------------------------------- cold open (section 0)
TTS_BEATS = {0: "co_b0_claude", 1: "co_b1_gpt", 3: "co_b3_claude", 5: "co_b5_claude", 6: "co_b6_claude",
             7: "co_b7_claude", 8: "co_b8_claude", 9: "co_b9_claude"}   # coldopen.html beat -> voiced reply (tts_coldopen.py)


def read_wav(path):
    with wave.open(str(path)) as w:
        assert w.getframerate() == 48000
        a = np.frombuffer(w.readframes(w.getnframes()), np.int16).astype(np.float32) / 32768
        return a.reshape(-1, w.getnchannels())


def write_wav(path, a):
    with wave.open(str(path), "wb") as w:
        w.setnchannels(2); w.setsampwidth(2); w.setframerate(48000)
        w.writeframes((np.clip(a, -1, 1) * 32767).astype(np.int16).tobytes())


def click_track(events, n, rng):
    sr = 48000
    out = np.zeros((n, 2), np.float32)
    for e in events:
        if e["evt"] != "key":
            continue
        L = int(sr * (0.03 + rng.random() * 0.02))
        x = (rng.random(L) * 2 - 1) * (1 - np.arange(L) / L) ** 6
        x = x - np.convolve(x, np.ones(24) / 24, "same")          # crude high-pass: keyboard clack
        x *= (0.35 + rng.random() * 0.25) * 0.5
        s = int(e["t"] * sr)
        if s + L < n:
            out[s:s + L, 0] += x
            out[s:s + L, 1] += x
    return out


def tts_stereo(tid, tempo):
    """Voiced reply -> 48 kHz stereo, sped up a little, levelled to about -17 LUFS."""
    out = SEG / f"tts_{tid}.wav"
    ff("-i", HERE / "build" / "tts" / f"{tid}.wav", "-af", f"atempo={tempo}", "-ar", 48000, "-ac", 2, "-c:a", "pcm_s16le", out)
    relevel(out, target=-17.0)
    return out


def build_coldopen(vo_wavs):
    """Cold open = coldopen.html render + Dylan's four lines at the press times + replies at their EVT resp events + clicks."""
    plan = json.loads((SEG / "coldopen.plan.json").read_text())
    D = math.ceil(plan["total"])
    out_v, out_w = SEG / "000.mp4", SEG / "000.wav"
    ff("-i", SEG / "coldopen.mp4", "-t", D, "-vf", f"fps={FPS},tpad=stop_mode=clone:stop_duration=3", "-frames:v", D * FPS,
       "-an", "-c:v", "libx264", "-crf", "17", "-preset", "veryfast", "-pix_fmt", "yuv420p", out_v)
    n = D * 48000
    mix = np.zeros((n, 2), np.float32)
    events = json.loads((SEG / "coldopen.events.json").read_text())
    mix += click_track(events, n, np.random.default_rng(7))
    for at, w in zip(plan["voice_at"], vo_wavs):
        a = read_wav(w); s = int(at * 48000)
        mix[s:s + len(a)] += a[: n - s]
    for e in events:
        if e["evt"].startswith("resp "):
            tid = TTS_BEATS.get(int(e["evt"].split()[1]))
            if tid and (HERE / "build" / "tts" / f"{tid}.wav").exists():
                a = read_wav(tts_stereo(tid, plan["tempo"])); s = int(e["t"] * 48000)
                mix[s:s + len(a)] += a[: max(0, n - s)]
    write_wav(out_w, mix)
    return D


# ---------------------------------------------------------------- assembly
def main():
    args = sys.argv[1:]
    fresh = "--fresh" in args
    only = None
    if "--only" in args:
        only = {int(x) for x in args[args.index("--only") + 1:]}
    SEG.mkdir(parents=True, exist_ok=True)
    TMP.mkdir(parents=True, exist_ok=True)
    if fresh:
        for f in SEG.glob("*.key"):
            f.unlink()
    edit = json.loads((HERE / "edit.json").read_text(encoding="utf-8"))
    migrate_old_names()

    # flatten; section 0 = indices 1..4 voices feed the cold open (idx 0)
    jobs, order = [], []
    idx = 1
    for sec in edit:
        segs = sec["segments"]
        for k, g in enumerate(segs):
            if g["kind"] == "pickup" and not g.get("recorded"):
                print("skipping unrecorded pickup", g["id"]); continue
            bg = None
            if g["frame"] == "C":
                sib = [s for s in segs[k + 1:] + segs[:k][::-1] if s["frame"] in "BP" and (s.get("broll") or s.get("visual"))]
                bg = sib[0] if sib else None
            jobs.append(dict(idx=idx, g=g, section=sec["section"], bg=bg, bed=sec["bed"]))
            order.append((idx, sec["section"], sec["bed"]))
            idx += 1
        if sec["section"] == "10":     # credits tail: footage under credits music, no voice
            order.append(("credits", "10c", "music/credits_take1.mp3"))

    names = {j["idx"]: seg_key(j) for j in jobs}
    (SEG / "names.json").write_text(json.dumps(names), encoding="utf-8")   # idx -> piece name, for the other tools
    todo = [j for j in jobs if only is None or j["idx"] in only]
    durs = {}
    with ProcessPoolExecutor(max_workers=4) as ex:
        for i, d in ex.map(render_segment, todo):
            durs[i] = d
            print(f"segment {i:03d} done ({d:.1f}s)", flush=True)
    if only is not None:
        return
    for j in jobs:
        durs.setdefault(j["idx"], dur(SEG / f"{names[j['idx']]}.wav"))

    # level every voice clip to -16 LUFS (+-0.3), cold-open lines included; the blooper is already matched
    skip = {j["idx"] for j in jobs if j["g"]["kind"] == "blooper"}
    report = []
    for j in jobs:
        if j["idx"] in skip:
            continue
        w = SEG / f"{names[j['idx']]}.wav"
        report.append((j["idx"], relevel(w)))
    bad = [(i, round(v, 1)) for i, v in report if v is not None and abs(v + 16) > 0.5]
    print(f"voice levels: {len(report)} clips levelled to -16 LUFS; outside +-0.5 dB: {bad or 'none'}")

    # cold open: its voice lines are the first four segments of section "00"
    s0 = [j["idx"] for j in jobs if j["section"] == "00"]
    cold = build_coldopen([SEG / f"{names[i]}.wav" for i in s0])

    # credits tail
    cd = 15.0
    footage_video([dict(src="../GameFootage/budget-out.mp4", i=25, o=40)], cd, SEG / "credits.mp4", "credits")
    write_wav(SEG / "credits.wav", np.zeros((int(cd * 48000), 2), np.float32))

    # timeline: cold open, then sections in order (the cold open's four voice segments are inside it)
    timeline, t = [], 0.0
    parts_v, parts_w = [SEG / "000.mp4"], [SEG / "000.wav"]
    timeline.append(("00", 0.0, cold, None)); t = cold
    cur_sec, cur_start, cur_bed = None, None, None
    sections = {}
    for i, sec, bed in order:
        if sec == "00":
            continue
        if i == "credits":
            parts_v.append(SEG / "credits.mp4"); parts_w.append(SEG / "credits.wav"); d = cd
        else:
            parts_v.append(SEG / f"{names[i]}.mp4"); parts_w.append(SEG / f"{names[i]}.wav"); d = durs[i]
        s = sections.setdefault(sec, [t, 0.0, bed])
        s[1] += d
        t += d
    total = t
    lst = TMP / "all_v.txt"
    lst.write_text("\n".join(f"file '{p.as_posix()}'" for p in parts_v), encoding="utf-8")
    ff("-f", "concat", "-safe", 0, "-i", lst, "-c", "copy", BUILD / "assembled_video.mp4")
    lst = TMP / "all_w.txt"
    lst.write_text("\n".join(f"file '{p.as_posix()}'" for p in parts_w), encoding="utf-8")
    ff("-f", "concat", "-safe", 0, "-i", lst, "-c", "copy", BUILD / "assembled_voice.wav")

    # music beds, ducked under the voice
    beds = [(sec, st, d, bed) for sec, (st, d, bed) in sections.items() if bed]
    inputs, fc = ["-i", BUILD / "assembled_voice.wav"], []
    for k, (sec, st, d, bed) in enumerate(beds, start=1):
        inputs += ["-stream_loop", "-1", "-i", ROOT / bed]
        L = d + 1.2
        vol = BED_VOL.get(Path(bed).stem, BED_DEFAULT_VOL)
        fc.append(f"[{k}:a]atrim=0:{L:.2f},asetpts=PTS-STARTPTS,aformat=channel_layouts=stereo,aresample=48000,"
                  f"afade=t=in:d=1.0,afade=t=out:st={max(L - 1.6, 0):.2f}:d=1.6,volume={vol}dB,"
                  f"adelay={int(st * 1000)}|{int(st * 1000)}[m{k}]")
    n = len(beds)
    fc.append("[0:a]asplit=2[v1][v2]")
    fc.append("".join(f"[m{k}]" for k in range(1, n + 1)) + f"amix=inputs={n}:normalize=0:duration=longest[mus]")
    fc.append("[mus][v2]sidechaincompress=threshold=0.03:ratio=3:attack=15:release=500:makeup=1[duck]")
    fc.append(f"[v1][duck]amix=inputs=2:normalize=0:duration=first,alimiter=limit=0.84:level=disabled,atrim=0:{total:.3f}[aout]")
    scr = TMP / "mix.filter.txt"
    scr.write_text(";".join(fc), encoding="utf-8")
    ff(*inputs, "-/filter_complex", scr, "-map", "[aout]", "-ar", 48000, "-c:a", "pcm_s16le", BUILD / "assembled_audio.wav")
    ff("-i", BUILD / "assembled_video.mp4", "-i", BUILD / "assembled_audio.wav", "-map", "0:v", "-map", "1:a",
       "-c:v", "copy", "-c:a", "aac", "-b:a", "192k", "-movflags", "+faststart", BUILD / "devlog_rough_v1.mp4")
    lines = [f"{sec:>4}  start {st // 60:.0f}:{st % 60:04.1f}  length {d:5.1f}s  bed {bed}" for sec, (st, d, bed) in
             {"00": [0.0, cold, None], **sections}.items()]
    (BUILD / "rough_v1_timeline.txt").write_text("\n".join(lines) + f"\nTOTAL {total / 60:.1f} min\n", encoding="utf-8")
    print(f"done: {total / 60:.1f} min -> build/devlog_rough_v1.mp4")


if __name__ == "__main__":
    main()
