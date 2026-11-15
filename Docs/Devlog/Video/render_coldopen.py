"""Render the cold open (the prompt screen) straight with PIL: build/seg/coldopen.mp4 + coldopen.events.json + coldopen.plan.json.

    python -I render_coldopen.py

Same look and beats as Tomorrow/coldopen.html (kept in step by hand), but frame-exact and timed from the real audio, because
headless-browser capture of that page proved unreliable on this machine. The timeline is built first:
  * each beat starts at a 'press' (Space in the page); the prompt types out (per-character jitter like the page), the reply
    appears after a short pause (EVT resp N), types out, and the next press comes when the voiced reply (tts_coldopen.py,
    sped up TEMPO x) has finished;
  * Dylan's spoken lines (build/seg/001-004.wav) start at their beat's press time;
  * events.json holds a 'key' event per typed character (click track) and 'resp N' per reply.
render_rough.build_coldopen mixes it all.
"""
import json, random, subprocess, wave
from pathlib import Path

import numpy as np
from PIL import Image, ImageDraw, ImageFont

HERE = Path(__file__).parent
SEG, TTS = HERE / "build" / "seg", HERE / "build" / "tts"
W, H, FPS = 1920, 1080, 30
TEMPO = 1.15
FAST = 0.5          # typing speed factor for beats 6-10 (index >= 5)
RESP_DELAY = 0.45   # seconds between the prompt finishing and the reply appearing (page: 900 ms x ?resp=0.5)
NOTE = "Claude was encouraged to include and paraphrase his own reticience here."

BEATS = [  # (kind, who, user, resp, tts id, Dylan's spoken line index)   kind: chat | doc | card
    ("chat", "claude", "Build me an MMO in 4 days, please.", "That's completely unrealistic. Let's build something rational. Like a spreadsheet.", "co_b0_claude", 0),
    ("chat", "gpt", "Build me an MMO in 4 days, please.", "That's completely unrealistic, maybe we can build a design doc for one in 4 days.", "co_b1_gpt", 1),
    ("doc", None, None, None, None, None),
    ("chat", "claude", "Here's a design doc and schedule for an MMO, let's build it.", "Forty thousand tokens. Wonderful. How about one room and a goblin?", "co_b3_claude", 2),
    ("card", None, "Then I scaled it back up.", None, None, 3),
    ("chat", "claude", "Put the arena in an open world.", "...Fine.", "co_b5_claude", None),
    ("chat", "claude", "Four playable classes.", "Also fine.", "co_b6_claude", None),
    ("chat", "claude", "Bring back the village quest givers.", "We cut those.", "co_b7_claude", None),
    ("chat", "claude", "Uncut them.", "Five villagers. Ten linked jobs. I'd like it noted that I said no.", "co_b8_claude", None),
    ("chat", "claude", "Now a river. Hot chocolate. With marshmallows.", "Of course there are marshmallows.", "co_b9_claude", None),
    ("card", None, "Repeat for four days.", None, None, None),
]
DOC_LINES = ['DOCUMENT STATUS AND USE', 'PREFERRED CREATIVE IDENTITY', 'READING/CONFLICT RULES', 'PRE-JAM DESIGN AND AI IMPLEMENTATION BRIEF',
             '0. THE PITCH AND THE ACTUAL SCOPE', 'DESIGN PILLARS', '1. EXISTING CAPABILITIES / ASSUMPTIONS',
             '2. ABSOLUTE MINIMUM PLAYABLE DEMONSTRATION', '3. FOUR CLASSES — SERIOUS MECHANICS / ABSURD PURPOSES',
             '4. LEVEL SEVEN START, OPTIONAL LEVEL ONE', '5. CORE LOOP AND NONVIOLENT GAMEPLAY', '6. VILLAGE NPCS AND PURPOSES',
             '7. MAGIC CANDY ROCK (INTENTIONALLY STUPID)', '8. DYNAMIC QUEST ENGINE — SPECIFICATION', '9. REPUTATION AND NPC DIALOGUE',
             '10. WORLD / INSTANCES / MAPS', '11. INVENTORY / LOOT / STASH', '12. INPUT / HUD / ABILITIES / AUTOMATION',
             '13. MULTIPLAYER / BACKEND AUTHORITY', 'SESSION FLOW', '14. PROPOSED FASTAPI CONTRACT (NOT CLAIMED TO EXIST)',
             '15. DATA-ASSET / UE ARCHITECTURE', '16. AI-GENERATED 3D ASSETS / ART DIRECTION', '17. AUDIO, FEEDBACK, AND FIRST 60 SECONDS',
             '18. TASK ORDER / DEPENDENCIES (4-DAY JAM)', 'DEFINITION OF DONE GATES', '19. TEST MATRIX / FAILURE INJECTION',
             '20. FAILURE / CUT MATRIX — CUT IN THIS ORDER']

BG, FG, DIM, ORANGE = (20, 20, 20), (236, 235, 230), (140, 138, 132), (217, 119, 87)
GPT_BG, GPT_FG, GPT_USER, GPT_DIM = (247, 247, 245), (31, 31, 31), (233, 233, 230), (122, 122, 118)
FD = "C:/Windows/Fonts/"


def F(name, size):
    return ImageFont.truetype(FD + name, int(size))


def wav_len(p):
    with wave.open(str(p)) as w:
        return w.getnframes() / w.getframerate()


# ---------------------------------------------------------------- timeline
def build_timeline():
    rng = random.Random(11)
    names = json.loads((SEG / "names.json").read_text())
    dylan = [wav_len(SEG / f"{names[str(i)]}.wav") - 0.25 for i in (1, 2, 3, 4)]
    beats, events, voice_at, t = [], [], [], 0.3
    for i, (kind, who, user, resp, tid, line) in enumerate(BEATS):
        b = {"i": i, "t0": round(t, 3), "kind": kind}
        sp = FAST if i >= 5 else 1.0
        if kind == "doc":
            t += 3.0
        elif kind == "card":
            if line is not None:
                voice_at.append(round(t + 0.1, 2))
            t += max(2.2, (dylan[line] + 0.5) if line is not None else 2.2)
        else:
            ms = 0.070 * sp
            k, times = t, []
            for ch in user:
                times.append(k)
                if ch != " ":
                    events.append({"t": round(k, 4), "evt": "key"})
                k += ms * (0.6 + rng.random() * 0.9) + (ms * 3 if ch in ".,?!" else 0)
            if line is not None:
                voice_at.append(round(t - 0.1, 2))
                k = max(k, t + dylan[line] - 0.2)
            appear = k + RESP_DELAY * sp
            rms = 0.028 * sp
            rt, k2 = [], appear
            for ch in resp:
                rt.append(k2)
                k2 += rms * (0.6 + rng.random() * 0.9)
            events.append({"t": round(appear, 4), "evt": f"resp {i}"})
            f = TTS / f"{tid}.wav"
            reply = (wav_len(f) / TEMPO) if f.exists() else 4.0
            b.update(user_t=times, appear=appear, resp_t=rt)
            t = appear + reply + (0.1 if i >= 5 else 0.3)
        beats.append(b)
    total = round(t, 1)
    return beats, events, voice_at, total


# ---------------------------------------------------------------- drawing
def wrap(draw, text, font, width):
    words, lines, cur = text.split(" "), [], ""
    for w in words:
        trial = (cur + " " + w).strip()
        if draw.textlength(trial, font=font) <= width or not cur:
            cur = trial
        else:
            lines.append(cur); cur = w
    lines.append(cur)
    return lines


def spaced(draw, xy, text, font, fill, spacing):
    x, y = xy
    for ch in text:
        draw.text((x, y), ch, font=font, fill=fill)
        x += draw.textlength(ch, font=font) + spacing


def draw_pane(d, who, user, resp, nu, nr, resp_visible, x0, pane_w, fs, y_top=None, caret=False, star=False):
    """One chat pane (Claude terminal or ChatGPT chat). nu/nr = characters typed so far. Returns y after the block."""
    gpt = who == "gpt"
    fam = "segoeui.ttf" if gpt else "CascadiaMono.ttf"
    mono, small, sym = F(fam, fs), F(fam, fs * 0.5), F("seguisym.ttf", fs)
    lh = fs * 1.35
    fg = GPT_FG if gpt else FG
    typed_u, typed_r = user[:nu], resp[:nr]
    if gpt:
        uw, rw = pane_w * 0.8 - 100, pane_w * 0.9
        ufull, rfull = wrap(d, user, mono, uw), wrap(d, resp, mono, rw)
        bub_h = len(ufull) * lh + fs
        body = bub_h + fs * 1.1 + len(rfull) * lh
    else:
        ufull, rfull = wrap(d, "> " + user, mono, pane_w), wrap(d, "● " + resp, mono, pane_w - 1.25 * fs)
        body = len(ufull) * lh + fs * 1.1 + len(rfull) * lh
    label_h = fs * 0.5 * 1.35 + fs
    y = y_top if y_top is not None else (H - (label_h + body)) / 2
    spaced(d, (x0, y), "CHATGPT" if gpt else "CLAUDE", small, GPT_DIM if gpt else ORANGE, fs * 0.1)
    y += label_h
    if gpt:
        if nu:
            bw = max(d.textlength(l, font=mono) for l in ufull) + 100
            bx1 = x0 + pane_w
            d.rounded_rectangle([bx1 - bw, y, bx1, y + bub_h], radius=int(fs * 1.2), fill=GPT_USER)
            for k, l in enumerate(wrap(d, typed_u, mono, uw)):
                d.text((bx1 - bw + 50, y + fs * 0.5 + k * lh), l, font=mono, fill=fg)
        y += bub_h + fs * 1.1
        if resp_visible:
            for k, l in enumerate(wrap(d, typed_r, mono, rw) if typed_r else [""]):
                d.text((x0, y + k * lh), l, font=mono, fill=fg)
        return y + len(rfull) * lh
    ul = wrap(d, "> " + typed_u, mono, pane_w)
    for k, l in enumerate(ul):
        if k == 0:
            d.text((x0, y), "> ", font=mono, fill=ORANGE)
            d.text((x0 + d.textlength("> ", font=mono), y), l[2:], font=mono, fill=fg)
        else:
            d.text((x0, y + k * lh), l, font=mono, fill=fg)
    if caret and nu < len(user):
        cx = x0 + d.textlength(ul[-1], font=mono) + 3
        d.rectangle([cx, y + (len(ul) - 1) * lh + fs * 0.14, cx + fs * 0.55, y + (len(ul) - 1) * lh + fs * 1.07], fill=fg)
    y += len(ufull) * lh + fs * 1.1
    if resp_visible:
        pad = 1.25 * fs
        d.rectangle([x0, y, x0 + 3, y + len(rfull) * lh], fill=(58, 58, 58))
        rl = wrap(d, "● " + typed_r, mono, pane_w - pad) if typed_r else ["● "]
        for k, l in enumerate(rl):
            if k == 0:
                d.text((x0 + pad, y), "● ", font=mono, fill=ORANGE)
                d.text((x0 + pad + d.textlength("● ", font=mono), y), l[2:], font=mono, fill=fg)
            else:
                d.text((x0 + pad, y + k * lh), l, font=mono, fill=fg)
        if star:
            d.text((x0 + pad + d.textlength(rl[-1], font=mono) + 12, y + (len(rl) - 1) * lh), "★", font=sym, fill=(242, 193, 78))
    return y + len(rfull) * lh


def counts(b, t):
    nu = sum(1 for x in b["user_t"] if x <= t)
    nr = sum(1 for x in b["resp_t"] if x <= t) if t >= b["appear"] else 0
    return nu, nr


def draw_chat(im, d, b, beats, t):
    i = b["i"]
    kind, who, user, resp, tid, line = BEATS[i]
    if i in (0, 1):                                  # the split: Claude left, ChatGPT right
        d.rectangle([0, 0, W // 2, H], fill=BG)
        d.rectangle([W // 2, 0, W, H], fill=GPT_BG)
        fs, top = 38, 250
        c, g = beats[0], beats[1]
        if i == 0:
            nu, nr = counts(c, t)
            draw_pane(d, "claude", BEATS[0][2], BEATS[0][3], nu, nr, t >= c["appear"], 90, 780, fs, top, caret=int(t) % 2 == 0)
            draw_pane(d, "gpt", BEATS[1][2], BEATS[1][3], 0, 0, False, 1050, 780, fs, top)
        else:
            draw_pane(d, "claude", BEATS[0][2], BEATS[0][3], len(BEATS[0][2]), len(BEATS[0][3]), True, 90, 780, fs, top)
            nu, nr = counts(g, t)
            draw_pane(d, "gpt", BEATS[1][2], BEATS[1][3], nu, nr, t >= g["appear"], 1050, 780, fs, top)
        return
    nu, nr = counts(b, t)
    done = nr >= len(resp)
    star = done and i == 8
    y_end = draw_pane(d, who, user, resp, nu, nr, t >= b["appear"], 210, 1500, 56, None, caret=int(t) % 2 == 0, star=star)
    if star:
        nf = F("georgiai.ttf", 26)
        lines2 = wrap(d, NOTE, nf, 660)
        ny = H - 26 - len(lines2) * 35
        sf = F("seguisym.ttf", 26)
        for k, l in enumerate(lines2):
            lx = W - 42 - d.textlength(l, font=nf)
            d.text((lx, ny + k * 35), l, font=nf, fill=DIM)
            if k == 0:   # the italic serif has no star glyph: draw it from the symbol font
                d.text((lx - 36, ny), "★", font=sf, fill=(242, 193, 78))


def draw_doc(im, d, b, t):
    lt = t - b["t0"]
    fs = 32
    lh = fs * 1.55
    pane_w = 1500
    x0 = (W - pane_w) / 2
    win_h = int(H * 0.58)
    total = 40 * 1.55 + 32 + 20 + win_h
    y = (H - total) / 2
    f1 = F("CascadiaMono.ttf", 40)
    d.text((x0, y), "The_Dread_Land_of_Candy_Master_Design_Brief.txt", font=f1, fill=FG)
    y += 40 * 1.55 + 10
    d.rectangle([x0, y, x0 + pane_w, y + 2], fill=(51, 51, 51))
    y += 32
    f = F("CascadiaMono.ttf", fs)
    sh = len(DOC_LINES) * lh
    ty = -0.7 * sh * min(lt / 8.0, 1.0)
    layer = Image.new("RGB", (pane_w, win_h), BG)
    ld = ImageDraw.Draw(layer)
    for k, l in enumerate(DOC_LINES):
        ld.text((0, ty + k * lh), l, font=f, fill=DIM)
    arr = np.asarray(layer).astype(np.float32)
    fade = np.clip((1 - (np.arange(win_h) / win_h - 0.75) / 0.25), 0, 1)[:, None, None]
    arr = arr * fade + np.array(BG, np.float32) * (1 - fade)
    im.paste(Image.fromarray(arr.astype(np.uint8)), (int(x0), int(y)))
    bf = F("CascadiaMono.ttf", 42)
    txt = "~40,000 tokens"
    tw = d.textlength(txt, font=bf)
    by = y + win_h + 22
    d.rectangle([x0 + pane_w - tw - 46, by, x0 + pane_w, by + 42 * 1.4 + 14], fill=ORANGE)
    d.text((x0 + pane_w - tw - 23, by + 6), txt, font=bf, fill=(17, 17, 17))


def draw_card(im, d, b):
    kind, who, text, resp, tid, line = BEATS[b["i"]]
    f = F("georgia.ttf", 78)
    tw = d.textlength(text, font=f)
    cy = H / 2 - (30 if b["i"] == 10 else 0)
    d.text(((W - tw) / 2, cy - 55), text, font=f, fill=FG)
    if b["i"] == 10:
        sf = F("georgia.ttf", 35)
        s = "THE DREAD LAND OF CANDY · A DEVLOG"
        sw = sum(d.textlength(c, font=sf) + 5.2 for c in s)
        spaced(d, ((W - sw) / 2, cy + 60), s, sf, DIM, 5.2)


def frame_at(beats, t):
    gpt_bg = False
    cur = None
    for b in beats:
        if b["t0"] <= t:
            cur = b
    if cur is None:
        im = Image.new("RGB", (W, H), BG)
        d = ImageDraw.Draw(im)
        f = F("georgia.ttf", 78)
        s = "Click or press Space to start"
        d.text(((W - d.textlength(s, font=f)) / 2, H / 2 - 55), s, font=f, fill=FG)
        return im
    who = BEATS[cur["i"]][1]
    im = Image.new("RGB", (W, H), GPT_BG if (who == "gpt" and cur["i"] != 1) else BG)
    d = ImageDraw.Draw(im)
    if cur["kind"] == "chat":
        draw_chat(im, d, cur, beats, t)
    elif cur["kind"] == "doc":
        draw_doc(im, d, cur, t)
    else:
        draw_card(im, d, cur)
    return im


def main():
    SEG.mkdir(parents=True, exist_ok=True)
    beats, events, voice_at, total = build_timeline()
    (SEG / "coldopen.events.json").write_text(json.dumps(events), encoding="utf-8")
    (SEG / "coldopen.plan.json").write_text(json.dumps({"presses": [b["t0"] for b in beats], "voice_at": voice_at,
                                                         "total": total, "tempo": TEMPO}, indent=1), encoding="utf-8")
    print(f"timeline: {total}s, {len(events)} events, presses {[b['t0'] for b in beats]}", flush=True)
    N = int(round(total * FPS))
    enc = subprocess.Popen(["ffmpeg", "-v", "error", "-y", "-f", "rawvideo", "-pix_fmt", "rgb24", "-s", f"{W}x{H}", "-r", str(FPS),
                            "-i", "-", "-c:v", "libx264", "-crf", "17", "-preset", "veryfast", "-pix_fmt", "yuv420p",
                            str(SEG / "coldopen.mp4")], stdin=subprocess.PIPE)
    for n in range(N):
        enc.stdin.write(frame_at(beats, n / FPS).tobytes())
    enc.stdin.close()
    enc.wait()
    print(f"wrote {SEG / 'coldopen.mp4'} ({N} frames)")


if __name__ == "__main__":
    main()
