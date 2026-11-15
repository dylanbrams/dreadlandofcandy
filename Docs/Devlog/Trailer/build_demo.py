"""Builds the demo video from DEMO_EDIT.md's plan: python -I build_demo.py [out.mp4] [--from S --to S]

Picture: every frame composited in Python (cv2), piped to ffmpeg. Segment kinds:
  clip  gameplay/flythrough footage, optional slow push-in and a text card over it
  cam   Dylan keyed (key_still.key) over a blurred, darkened still of some footage
  pip   footage with Dylan keyed into the bottom-right corner
  card  text on plum, for the title
Sound: voices on a bus, music beds ducked under it (sidechaincompress), loudnorm at the end.
Footage: Docs/Devlog/GameFootage (git-ignored). Output: Docs/Devlog/Trailer/build/ (git-ignored).
"""
import subprocess
import sys
from pathlib import Path

import cv2
import numpy as np
from PIL import Image, ImageDraw, ImageFont

ROOT = Path(__file__).resolve().parents[3]
sys.path.insert(0, str(ROOT / 'Docs/Devlog/Video'))
sys.path.insert(0, str(Path(__file__).parent))
from key_still import key  # noqa: E402  (green-dominance key, despill)

FOOT = ROOT / 'Docs/Devlog/GameFootage'
TAKES = ROOT / 'Docs/Devlog/Video/raw/original'
FEN = ROOT / 'vocals/output/fenella'
ANN = ROOT / 'vocals/output/announcer'
MUSIC = ROOT / 'music'
FONT_TITLE = str(ROOT / 'Content/UI/Fonts/Cinzel-Medium.ttf')
FONT_BODY = str(ROOT / 'Content/UI/Fonts/CormorantGaramond-SemiBold.ttf')
OUT_DIR = Path(__file__).parent / 'build'
W, H, FPS = 1920, 1080, 30
PLUM = (45, 30, 40)  # BGR

FLY = 'LandOfCandyFlythrough.mp4'


def take(name):
    return str(TAKES / f'2026-11-15 {name}.mp4')


# ---------------------------------------------------------------- edit decision list
# Video: (kind, seconds, params). Times inside sources are in seconds.
# Audio: (start, file, gain_db, src_in, src_dur or None, bus, fade_out)  bus = 'voice' | 'music'
from demo_edl import VIDEO, AUDIO  # noqa: E402  (kept separate so the cut can be changed without touching this)


# ---------------------------------------------------------------- picture
class Source:
    cache = {}

    @classmethod
    def get(cls, path):
        if path not in cls.cache:
            cls.cache[path] = cv2.VideoCapture(path)
        return cls.cache[path]

    @classmethod
    def frame(cls, path, t, native=False):
        cap = cls.get(path)
        fps = cap.get(cv2.CAP_PROP_FPS) or 30
        want = int(round(t * fps))
        last = cls.last.get(path)
        if last and last[0] == (want, native):   # a still background asks for the same frame every time
            return last[1]
        im = cls._read(cap, want, native)
        cls.last[path] = ((want, native), im)
        return im

    last = {}

    @classmethod
    def _read(cls, cap, want, native):
        pos = int(cap.get(cv2.CAP_PROP_POS_FRAMES))
        if want < pos or want > pos + 90:
            cap.set(cv2.CAP_PROP_POS_FRAMES, want)
            pos = want
        ok, im = True, None
        while pos <= want:
            ok, im = cap.read()
            pos += 1
            if not ok:
                break
        if im is None:
            return np.zeros((720, 1280, 3) if native else (H, W, 3), np.uint8)
        if not native and im.shape[:2] != (H, W):
            im = cv2.resize(im, (W, H), interpolation=cv2.INTER_LANCZOS4)
        return im


import json  # noqa: E402
try:
    TOASTS = json.loads((FOOT / 'toasts.json').read_text(encoding='utf-8'))
except OSError:
    TOASTS = {}


def mask_toast(im, name, t):
    """Discord notifications got drawn over the window's bottom-right corner in some captures (FOOT/toasts.json,
    detected ranges). Blur that corner around any detected range, with a margin."""
    if not any(a - 2.0 <= t <= b + 2.0 for a, b in TOASTS.get(name, [])):
        return im
    x0, y0 = int(W * 0.885), int(H * 0.905)
    reg = im[y0:, x0:]
    small = cv2.resize(reg, (max(1, reg.shape[1] // 16), max(1, reg.shape[0] // 16)), interpolation=cv2.INTER_AREA)
    im = im.copy()
    im[y0:, x0:] = cv2.GaussianBlur(cv2.resize(small, (reg.shape[1], reg.shape[0])), (0, 0), 6)
    return im


def footage(path, t, push=0.0, u=0.0):
    im = Source.frame(path if Path(path).is_absolute() else str(FOOT / path), t)
    im = mask_toast(im, Path(path).name, t)
    if push:
        s = 1.0 + push * u
        M = cv2.getRotationMatrix2D((W / 2, H / 2), 0, s)
        im = cv2.warpAffine(im, M, (W, H), flags=cv2.INTER_LINEAR, borderMode=cv2.BORDER_REFLECT)
    return im


_text_cache = {}


def text_layer(text, size, font=FONT_TITLE, y=None, sub=None):
    k = (text, size, font, y, sub)
    if k in _text_cache:
        return _text_cache[k]
    img = Image.new('RGBA', (W, H), (0, 0, 0, 0))
    d = ImageDraw.Draw(img)
    f = ImageFont.truetype(font, size)
    bb = d.textbbox((0, 0), text, font=f)
    tw, th = bb[2] - bb[0], bb[3] - bb[1]
    yy = (H - th) // 2 if y is None else y
    x = (W - tw) // 2
    for dx, dy in ((0, 4), (2, 4), (-2, 4)):
        d.text((x + dx, yy + dy - bb[1]), text, font=f, fill=(0, 0, 0, 170))
    d.text((x, yy - bb[1]), text, font=f, fill=(255, 244, 228, 255))
    if sub:
        fs = ImageFont.truetype(FONT_BODY, int(size * 0.42))
        sb = d.textbbox((0, 0), sub, font=fs)
        sx = (W - (sb[2] - sb[0])) // 2
        sy = yy + th + int(size * 0.5)
        d.text((sx, sy + 3), sub, font=fs, fill=(0, 0, 0, 170))
        d.text((sx, sy), sub, font=fs, fill=(255, 220, 235, 255))
    a = np.asarray(img).astype(np.float32)
    rgb, al = a[..., [2, 1, 0]], a[..., 3:] / 255.0
    _text_cache[k] = (rgb, al)
    return rgb, al


def apply_text(im, spec, u_local, dur_local):
    if not spec:
        return im
    rgb, al = text_layer(spec['text'], spec.get('size', 84), spec.get('font', FONT_TITLE), spec.get('y'), spec.get('sub'))
    t = u_local
    fade = min(1.0, t / 0.35, max(0.0, (dur_local - t) / 0.35))
    a = al * fade
    return (im * (1 - a) + rgb * a).astype(np.uint8)


def keyed(take_name, t, wall=None):
    src = Source.frame(take(take_name), t, native=True)  # key_still works on the native 1280x720 take
    return key(src, wall)


def place(base, fg, a, height, x_right=None, x_center=None, bottom=0):
    h0, w0 = a.shape
    s = height / h0
    fw, fh = int(w0 * s), height
    fg = cv2.resize(fg, (fw, fh), interpolation=cv2.INTER_AREA)
    a = cv2.resize(a, (fw, fh), interpolation=cv2.INTER_AREA)[..., None]
    x = (W - fw) // 2 if x_center else (x_right - fw)
    y = H - fh - bottom
    x0, x1 = max(0, x), min(W, x + fw)
    y0 = max(0, y)
    reg = base[y0:y0 + fh, x0:x1].astype(np.float32)
    f = fg[y0 - y:, x0 - x:x0 - x + (x1 - x0)]
    aa = a[y0 - y:, x0 - x:x0 - x + (x1 - x0)]
    base[y0:y0 + fh, x0:x1] = (f * aa + reg * (1 - aa)).astype(np.uint8)
    return base


def soft_blur(im, sigma):
    """Blur at 1/4 size and scale back: same look as a full-size blur, a fraction of the cost, no banding."""
    s = cv2.resize(im, (W // 4, H // 4), interpolation=cv2.INTER_AREA)
    s = cv2.GaussianBlur(s, (0, 0), sigma / 4)
    return cv2.resize(s, (W, H), interpolation=cv2.INTER_CUBIC)


def render_frame(kind, p, u, dur):
    if kind == 'clip':
        im = footage(p['src'], p['in'] + min(u, p.get('hold', 1e9)), p.get('push', 0.0), u / dur)  # hold: freeze there
    elif kind == 'card':
        im = np.zeros((H, W, 3), np.uint8); im[:] = PLUM
        if p.get('bg'):
            bg = footage(p['bg'], p['bg_in'] + u)
            bg = soft_blur(bg, 14)
            im = (bg * 0.35 + im * 0.65).astype(np.uint8)
    elif kind in ('cam', 'pip'):
        if kind == 'cam':
            bg = footage(p['bg'], p['bg_in'] + (u if p.get('bg_moves') else 0))
            im = (soft_blur(bg, 18) * 0.55).astype(np.uint8)
        else:
            im = footage(p['src'], p['in'] + u)
        fg, a = keyed(p['take'], p['t_in'] + u, p.get('wall'))
        if kind == 'cam':
            im = place(im, fg, a, 1000, x_center=True)
        else:
            im = place(im, fg, a, 430, x_right=W - 30)
    else:
        raise ValueError(kind)
    im = apply_text(im, p.get('text'), u, dur)
    # fades
    fi, fo = p.get('fade_in', 0), p.get('fade_out', 0)
    g = 1.0
    if fi and u < fi: g = u / fi
    if fo and u > dur - fo: g = min(g, (dur - u) / fo)
    if g < 1.0: im = (im * max(0.0, g)).astype(np.uint8)
    return im


def render_video(path, t_from=0.0, t_to=1e9):
    proc = subprocess.Popen(['ffmpeg', '-hide_banner', '-loglevel', 'error', '-y', '-f', 'rawvideo', '-pix_fmt', 'bgr24',
                             '-s', f'{W}x{H}', '-r', str(FPS), '-i', '-', '-c:v', 'h264_nvenc', '-preset', 'p5', '-cq', '19',
                             '-pix_fmt', 'yuv420p', str(path)], stdin=subprocess.PIPE)
    t0 = 0.0
    for kind, dur, p in VIDEO:
        n = int(round(dur * FPS))
        for i in range(n):
            T = t0 + i / FPS
            if t_from <= T < t_to:
                proc.stdin.write(render_frame(kind, p, i / FPS, dur).tobytes())
        t0 += n / FPS
        print(f'  {t0:6.2f}s  {kind} {p.get("src") or p.get("take") or ""}', flush=True)
    proc.stdin.close()
    proc.wait()
    return t0


# ---------------------------------------------------------------- sound
VOICE_FX = ('highpass=f=80,afftdn=nf=-30,acompressor=threshold=-24dB:ratio=3:attack=5:release=120:makeup=6,'
            'equalizer=f=3000:t=q:w=1:g=3')
VOICE_LUFS = -15.0


def lufs(path):
    out = subprocess.run(['ffmpeg', '-hide_banner', '-i', str(path), '-af', 'ebur128', '-f', 'null', '-'],
                         capture_output=True, text=True).stderr
    vals = [l.split()[1] for l in out.splitlines() if l.strip().startswith('I:')]
    return float(vals[-1])


def prepare_take(src, s_in, s_dur):
    """One of Dylan's lines: cut, clean, measure, then gain to exactly VOICE_LUFS. Cached in build/voice/."""
    d = OUT_DIR / 'voice'
    d.mkdir(parents=True, exist_ok=True)
    out = d / f'{Path(src).stem.replace(" ", "_")}_{s_in:.2f}_{s_dur or 0:.2f}.wav'
    if out.exists():
        return out
    raw = out.with_suffix('.raw.wav')
    subprocess.run(['ffmpeg', '-hide_banner', '-loglevel', 'error', '-y', '-ss', str(s_in)] + (['-t', str(s_dur)] if s_dur else [])
                   + ['-i', str(src), '-vn', '-af', VOICE_FX, '-ar', '48000', '-ac', '2', str(raw)], check=True)
    g = VOICE_LUFS - lufs(raw)
    for _ in range(3):   # short lines measure a little differently after gain: correct until within 0.3 dB
        subprocess.run(['ffmpeg', '-hide_banner', '-loglevel', 'error', '-y', '-i', str(raw), '-af',
                        f'volume={g:.2f}dB,alimiter=limit=0.89', str(out)], check=True)
        err = VOICE_LUFS - lufs(out)
        if abs(err) <= 0.3:
            break
        g += err
    raw.unlink()
    print(f'  voice {out.name}: {g:+.1f} dB -> {lufs(out):.1f} LUFS', flush=True)
    return out


def render_audio(path, total):
    args, voice, music = ['ffmpeg', '-hide_banner', '-loglevel', 'error', '-y'], [], []
    filt = []
    for i, (at, f, gain, s_in, s_dur, bus, v_fade) in enumerate(AUDIO):
        args += ['-i', str(f)]
        trim = f'atrim=start={s_in}' + (f':duration={s_dur}' if s_dur else '')
        fades = ''
        if bus == 'music':
            fades = f',afade=t=in:d=0.6' + (f',afade=t=out:st={max(0, s_dur - 1.5)}:d=1.5' if s_dur else '')
        elif v_fade and s_dur:
            fades = f',afade=t=out:st={max(0, s_dur - v_fade)}:d={v_fade}'
        voice_fx = ''
        if str(f).startswith(str(TAKES)) or Path(f).parent == ROOT / 'Docs/Devlog/Video':  # Dylan's takes and pickups
            # Dylan's raw takes differ in level from take to take (around -42 to -49 LUFS) and carry room tone:
            # clean each line, then bring it to exactly the announcer's level (two-pass, see prepare_take).
            f, s_in, s_dur = prepare_take(f, s_in, s_dur), 0.0, None
            trim = 'atrim=start=0'
            args[-1] = str(f)
        filt.append(f'[{i}:a]{trim},asetpts=PTS-STARTPTS{voice_fx},aformat=sample_rates=48000:channel_layouts=stereo{fades},'
                    f'volume={gain}dB,adelay={int(at * 1000)}:all=1[a{i}]')
        (voice if bus == 'voice' else music).append(f'[a{i}]')
    filt.append(f'{"".join(voice)}amix=inputs={len(voice)}:normalize=0,apad=whole_dur={total}[v]')
    filt.append('[v]asplit=2[vk][vm]')
    filt.append(f'{"".join(music)}amix=inputs={len(music)}:normalize=0,apad=whole_dur={total}[m]')
    filt.append('[m][vk]sidechaincompress=threshold=0.03:ratio=8:attack=20:release=400[md]')
    filt.append(f'[vm][md]amix=inputs=2:normalize=0,atrim=duration={total},loudnorm=I=-16:TP=-1.5:LRA=11[out]')
    args += ['-filter_complex', ';'.join(filt), '-map', '[out]', '-ar', '48000', '-c:a', 'aac', '-b:a', '192k', str(path)]
    subprocess.run(args, check=True)


def main():
    OUT_DIR.mkdir(exist_ok=True)
    out = Path(sys.argv[1]) if len(sys.argv) > 1 and not sys.argv[1].startswith('--') else OUT_DIR / 'demo_v1.mp4'
    a = sys.argv
    t_from = float(a[a.index('--from') + 1]) if '--from' in a else 0.0
    t_to = float(a[a.index('--to') + 1]) if '--to' in a else 1e9
    vid, aud = OUT_DIR / '_video.mp4', OUT_DIR / '_audio.m4a'
    total = render_video(vid, t_from, t_to)
    if t_from > 0 or t_to < 1e9:
        print('partial render (picture only):', vid)
        return
    render_audio(aud, total)
    subprocess.run(['ffmpeg', '-hide_banner', '-loglevel', 'error', '-y', '-i', str(vid), '-i', str(aud), '-c', 'copy',
                    '-shortest', '-movflags', '+faststart', str(out)], check=True)
    print('wrote', out, f'{total:.1f}s')


if __name__ == '__main__':
    main()
