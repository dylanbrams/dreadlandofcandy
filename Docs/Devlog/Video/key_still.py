"""Key a still (or a list of stills) and write a contact sheet: raw | matte | over dark | over light.

    python -I key_still.py out.png in1.png in2.png ...

Green-dominance key (works on this wrinkled, unevenly lit screen where a single chromakey colour fails):
  g = (G - max(R, B)) / G ;  alpha = 0 where g is large, 1 where it is small.
Then despill (clamp G to max(R, B)), remove specks, feather. Content is cropped to x < 898 (the right of the
OBS frame is black).
"""
import sys
import cv2
import numpy as np

CROP_W = 898
A_SUBJ, A_SCREEN = -6.0, -14.0   # Lab a*: above A_SUBJ = subject, below A_SCREEN = green/teal screen
WALL_X = None                    # set per take if the screen stops short of the frame edge (garbage matte, px)

def key(bgr, wall_x=WALL_X):
    crop = bgr[:, :CROP_W]
    lab = cv2.cvtColor(crop.astype(np.float32) / 255.0, cv2.COLOR_BGR2Lab)
    lab = cv2.GaussianBlur(lab, (0, 0), 1.0)   # sensor noise
    astar = lab[..., 1]
    a = np.clip((astar - A_SCREEN) / (A_SUBJ - A_SCREEN), 0, 1)
    if wall_x:
        a[:, wall_x:] = 0
    a8 = (a * 255).astype(np.uint8)
    m = (a8 > 128).astype(np.uint8)
    n, lab_, stats, _ = cv2.connectedComponentsWithStats(m, connectivity=8)
    keep = np.zeros_like(m)
    for i in range(1, n):
        if stats[i, cv2.CC_STAT_AREA] > 4000:
            keep[lab_ == i] = 1
    keep = cv2.morphologyEx(keep, cv2.MORPH_CLOSE, np.ones((9, 9), np.uint8))
    a = a * cv2.GaussianBlur(keep.astype(np.float32), (0, 0), 1.2)
    a = cv2.GaussianBlur(a, (0, 0), 0.8)
    im = crop.astype(np.float32)
    b_, g, r = im[..., 0], im[..., 1], im[..., 2]
    g2 = np.minimum(g, np.maximum(r, b_) + 4)           # despill
    return np.stack([b_, g2, r], -1), a

def over(fg, a, bg):
    return (fg * a[..., None] + bg * (1 - a[..., None])).astype(np.uint8)

def tiles(path):
    spec = path.split('@')
    src = cv2.imread(spec[0])
    fg, a = key(src, int(spec[1]) if len(spec) > 1 else None)
    h, w = a.shape
    dark = np.zeros((h, w, 3), np.float32); dark[:] = (60, 40, 45)      # BGR: deep plum
    dark[:, :, 1] += np.linspace(0, 40, w)[None, :]
    light = np.zeros((h, w, 3), np.float32); light[:] = (200, 170, 255) # BGR: pink
    raw = src[:, :CROP_W]
    matte = cv2.cvtColor((a * 255).astype(np.uint8), cv2.COLOR_GRAY2BGR)
    row = np.hstack([raw, matte, over(fg, a, dark), over(fg, a, light)])
    return cv2.resize(row, None, fx=0.5, fy=0.5, interpolation=cv2.INTER_AREA)

if __name__ == "__main__" and sys.argv[1] != "mockup":
    out, ins = sys.argv[1], sys.argv[2:]
    cv2.imwrite(out, np.vstack([tiles(p) for p in ins]))
    print("wrote", out)


def mockup(out, still, bg_path, wall=None):
    """1920x1080 frame: left = CAM (keyed over a blurred copy of the background), right = PIP bottom-right over the sharp background."""
    src = cv2.imread(still.split("@")[0])
    fg, a = key(src, wall)
    bg = cv2.imread(bg_path)
    bg = cv2.resize(bg, (1920, 1080), interpolation=cv2.INTER_AREA).astype(np.float32)
    # PIP: about a quarter of the frame height, bottom-right
    ph = 400
    pw = int(CROP_W * ph / fg.shape[0])
    f = cv2.resize(fg, (pw, ph), interpolation=cv2.INTER_AREA)
    m = cv2.resize(a, (pw, ph), interpolation=cv2.INTER_AREA)
    pip = bg.copy()
    y0, x0 = 1080 - ph - 20, 1920 - pw - 30
    pip[y0:y0 + ph, x0:x0 + pw] = f * m[..., None] + pip[y0:y0 + ph, x0:x0 + pw] * (1 - m[..., None])
    # CAM: keyed at full height over a blurred, darkened background
    blur = cv2.GaussianBlur(bg, (0, 0), 25) * 0.6
    s = 1080 / fg.shape[0]
    cf = cv2.resize(fg, None, fx=s, fy=s, interpolation=cv2.INTER_AREA)
    cm = cv2.resize(a, None, fx=s, fy=s, interpolation=cv2.INTER_AREA)
    cam = blur.copy()
    x1 = (1920 - cf.shape[1]) // 2
    cam[:, x1:x1 + cf.shape[1]] = cf * cm[..., None] + cam[:, x1:x1 + cf.shape[1]] * (1 - cm[..., None])
    cv2.imwrite(out + "_cam.png", cam.astype(np.uint8))
    cv2.imwrite(out + "_pip.png", pip.astype(np.uint8))
    print("wrote", out + "_cam.png", out + "_pip.png")


if __name__ == "__main__" and len(sys.argv) > 1 and sys.argv[1] == "mockup":
    mockup(*sys.argv[2:5])
