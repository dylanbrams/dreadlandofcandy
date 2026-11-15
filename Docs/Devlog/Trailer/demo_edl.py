"""The demo cut, v2 (30 fps re-capture). build_demo.py renders it. Times in source seconds.
Segments are laid end to end; voice lines are placed relative to the start of the segment they sit on.
Rules (Dylan): no shots of the bot walking into walls or trees, nothing that reads as unattended play; short shots.
Every gameplay pick avoids the Discord pop-up ranges in GameFootage/toasts.json (the renderer also masks them)."""
from pathlib import Path

ROOT = Path(__file__).resolve().parents[3]
TAKES = ROOT / 'Docs/Devlog/Video/raw/original'
FEN = ROOT / 'vocals/output/fenella'
ANN = ROOT / 'vocals/output/announcer'
MUSIC = ROOT / 'music'


def tk(name):
    return TAKES / f'2026-11-15 {name}.mp4'


VIDEO, AUDIO = [], []
_t = [0.0]


def seg(kind, dur, voices=(), **p):
    """voices: (offset, file, gain_db[, src_in, src_dur[, fade_out]])"""
    start = _t[0]
    VIDEO.append((kind, dur, p))
    for v in voices:
        off, f, g = v[:3]
        s_in = v[3] if len(v) > 3 else 0.0
        s_dur = v[4] if len(v) > 4 else None
        fo = v[5] if len(v) > 5 else 0.0
        AUDIO.append((start + off, f, g, s_in, s_dur, 'voice', fo))
    _t[0] += round(dur * 30) / 30
    return start


def clip(src, t_in, dur, voices=(), **p):
    return seg('clip', dur, voices, src=src, **{'in': t_in}, **p)


def music(at, name, gain, src_in, dur):
    AUDIO.append((at, MUSIC / f'{name}.mp3', gain, src_in, dur, 'music', 0.0))


# 1. Cold open: the devlog's own (coldopen.html prompt screen, Claude / ChatGPT split, Dylan's voice and the click
#    track), as the devlog renders it. Trailer rule (Dylan, Nov 15): gameplay within ~5 s, so only Dylan's ask
#    (0.2-2.9 s: "Build me an MMO in four days... please.") then a hard cut to the land; the game is the answer.
#    The wav sits ~1 dB under -15 LUFS: +1 dB.
COLD = ROOT / 'Docs/Devlog/Video/build/seg/000'
seg('clip', 3.0, [(0.0, Path(f'{COLD}.wav'), 1.0, 0.0, 3.0, 0.08)], src=str(Path(f'{COLD}.mp4')), **{'in': 0.0})
# 2. The land, the hero, the lecture.
m1 = clip('tour-high.mp4', 75.0, 4.6, [(0.5, ANN / 'A01_land.mp3', 0.0)])
clip('hero-close.mp4', 0.3, 3.2, [(0.2, ANN / 'A02_hero.mp3', 0.0)])
clip('knight-run.mp4', 10.0, 4.3, [(0.3, ANN / 'A03_armed.mp3', 0.0)])
clip('barbarian-run.mp4', 9.0, 3.4, [(0.2, FEN / 'VO_LectureCast_02.mp3', 0.0)])
# 3. Dylan in the corner over the giant celery.
seg('pip', 4.5, [(0.0, tk('13-09-43'), 0.0, 0.2, 4.4)], take='13-09-43', t_in=0.2, src='hero-close.mp4', **{'in': 20.8})
# 4. Music kicks in: manners, vegetables, a fast montage of all four classes.
m2 = clip('typed-note.mp4', 15.0, 2.4, [(0.3, ANN / 'A05_manners.mp3', 0.0)])
clip('typed-note.mp4', 30.0, 2.6, [(0.3, ANN / 'A06_vegetables.mp3', 0.0)])
for src, t in (('archer-run.mp4', 15.0), ('barbarian-run.mp4', 15.3), ('archer-run.mp4', 11.5),
               ('barbarian-run.mp4', 13.0), ('knight-run.mp4', 78.1)):
    clip(src, t, 1.3)
# 5. The world: chocolate river from above, the bridge, quests in the village.
clip('tour-high.mp4', 30.0, 3.6, [(0.3, ANN / 'A07_openworld.mp3', 0.0)])
clip('tour-high.mp4', 40.0, 2.6, push=0.04)
clip('village-improvise.mp4', 121.2, 2.8, [(0.3, ANN / 'A08_quests.mp3', 0.0)])
clip('tour-high.mp4', 100.0, 2.8, push=0.04)
# 6. CUT!, the sad-rats rewrite, budget boxes.
clip('knight-run.mp4', 46.9, 1.5, [(0.1, FEN / 'VO_CutPitch_04.mp3', 0.0)])
clip('typed-note.mp4', 25.5, 3.0)
clip('budget-out.mp4', 10.0, 3.9, [(0.2, FEN / 'VO_BudgetOut_02.mp3', 0.0)])
# 7. Premiere.
m3 = clip('budget-out.mp4', 24.6, 3.6, [(0.3, FEN / 'VO_Premiere_02.mp3', 0.0)])
# 8. Dylan closes, no music.
d8 = seg('cam', 3.5, [(0.0, tk('13-29-20'), 0.0, 32.45, 3.45)], take='13-29-20', t_in=32.45, bg='tour-high.mp4', bg_in=60.0)
# 9. Title.
end = seg('card', 5.5, [(0.4, ANN / 'A09_title.mp3', 0.0)], bg='tour-high.mp4', bg_in=15.0, fade_out=1.0,
          text={'text': 'THE DREAD LAND OF CANDY', 'size': 96, 'sub': 'Made in four days for the 2026 Epic AI MegaJam'})
TOTAL = _t[0]

music(m1 - 0.3, 'title-theme_take1', -6.0, 2.0, m2 - m1 + 0.6)
music(m2, 'boss-battle_take1', -5.0, 6.0, m3 - m2)
music(m3, 'victory_take1', -9.0, 0.0, d8 - m3)
music(end, 'victory_take1', -6.0, 20.0, TOTAL - end)
