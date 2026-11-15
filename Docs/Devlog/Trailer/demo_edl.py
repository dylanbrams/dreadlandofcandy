"""The demo cut, v1. build_demo.py renders it. Times in source seconds.
Segments are laid end to end; voice lines are placed relative to the start of the segment they sit on."""
from pathlib import Path

ROOT = Path(__file__).resolve().parents[3]
TAKES = ROOT / 'Docs/Devlog/Video/raw/original'
FEN = ROOT / 'vocals/output/fenella'
ANN = ROOT / 'vocals/output/announcer'
MUSIC = ROOT / 'music'
FLY = 'LandOfCandyFlythrough.mp4'


def tk(name):
    return TAKES / f'2026-11-15 {name}.mp4'


VIDEO, AUDIO = [], []
_t = [0.0]


def seg(kind, dur, voices=(), **p):
    """voices: (offset, file, gain_db[, src_in, src_dur])"""
    start = _t[0]
    VIDEO.append((kind, dur, p))
    for v in voices:
        off, f, g = v[:3]
        s_in = v[3] if len(v) > 3 else 0.0
        s_dur = v[4] if len(v) > 4 else None
        AUDIO.append((start + off, f, g, s_in, s_dur, 'voice'))
    _t[0] += round(dur * 30) / 30
    return start


def music(at, name, gain, src_in, dur):
    AUDIO.append((at, MUSIC / f'{name}.mp3', gain, src_in, dur, 'music'))


# 1. Cold open: Dylan, on camera.
seg('cam', 3.9, [(0.0, tk('12-16-09'), 2.0, 0.55, 3.8)],
    take='12-16-09', t_in=0.55, wall=780, bg=FLY, bg_in=170.0, fade_in=0.2)
# 2. The land.
m1 = seg('clip', 4.6, [(0.5, ANN / 'A01_land.mp3', 0.0)], src=FLY, **{'in': 155.0}, push=0.06)
seg('clip', 3.2, [(0.2, ANN / 'A02_hero.mp3', 0.0)], src='knight-run.mp4', **{'in': 0.6})
seg('clip', 4.4, [(0.3, ANN / 'A03_armed.mp3', 0.0)], src='knight-run.mp4', **{'in': 11.5})
seg('clip', 3.4, [(0.2, FEN / 'VO_LectureCast_02.mp3', 0.0)], src='knight-run.mp4', **{'in': 7.0})
# 3. Dylan PIP over celery.
seg('pip', 4.5, [(0.0, tk('13-09-43'), 2.0, 0.2, 4.4)], take='13-09-43', t_in=0.2, src='mage-run.mp4', **{'in': 17.0})
# 4. Music kicks in: manners, vegetables, montage.
m2 = seg('clip', 2.4, [(0.3, ANN / 'A05_manners.mp3', 0.0)], src='knight-run.mp4', **{'in': 2.2})
seg('clip', 2.6, [(0.3, ANN / 'A06_vegetables.mp3', 0.0)], src='barbarian-run.mp4', **{'in': 20.0})
for src, t in (('archer-run.mp4', 13.0), ('mage-run.mp4', 10.0), ('barbarian-run.mp4', 7.0),
               ('archer-run.mp4', 52.0), ('mage-run.mp4', 12.5), ('barbarian-run.mp4', 2.5)):
    seg('clip', 1.25, src=src, **{'in': t})
# 5. The world.
seg('clip', 3.6, [(0.3, ANN / 'A07_openworld.mp3', 0.0)], src=FLY, **{'in': 57.8})
seg('clip', 3.0, src=FLY, **{'in': 165.0}, push=0.05)
# 6. CUT!
seg('clip', 4.2, [(0.3, FEN / 'VO_CutPitch_04.mp3', 0.0)], src='knight-run.mp4', **{'in': 17.0})
seg('clip', 2.6, src='mage-run.mp4', **{'in': 80.0})
# 7. Wrap and premiere.
seg('clip', 2.4, src='budget-out.mp4', **{'in': 25.0})
m3 = seg('clip', 3.6, [(0.4, FEN / 'VO_Premiere_02.mp3', 0.0)], src='budget-out.mp4', **{'in': 28.0})
# 8. Dylan closes, no music.
d8 = seg('cam', 3.5, [(0.0, tk('13-29-20'), 2.0, 32.45, 3.45)], take='13-29-20', t_in=32.45, bg=FLY, bg_in=200.0)
# 9. Title.
end = seg('card', 5.5, [(0.4, ANN / 'A09_title.mp3', 0.0)], bg=FLY, bg_in=150.0, fade_out=1.0,
          text={'text': 'THE DREAD LAND OF CANDY', 'size': 96, 'sub': 'Made in four days for the 2026 Epic AI MegaJam'})
TOTAL = _t[0]

music(m1 - 0.3, 'title-theme_take1', -6.0, 2.0, m2 - m1 + 0.6)
music(m2, 'boss-battle_take1', -5.0, 6.0, d8 - m2 - 3.6)
music(m3, 'victory_take1', -9.0, 0.0, d8 - m3)
music(end, 'victory_take1', -6.0, 20.0, TOTAL - end)
