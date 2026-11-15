"""Architecture diagram for the devlog ("the AI is on a leash"): graphics/architecture.png (1920x1080).

    python -I make_architecture.py

Every box, step and endpoint comes from backend/README.md and backend/director/app.py (the Director service):
client -> FastAPI behind nginx -> Postgres (cache, budget, rate counters) and Claude Haiku 5.5 (structured output);
validate -> limit -> screen -> cache -> budget -> model -> clamp; any failure = the game plays its authored content.
"""
from pathlib import Path

from PIL import Image, ImageDraw, ImageFont

HERE = Path(__file__).parent
ROOT = HERE.parents[2]
OUT = HERE / "graphics" / "architecture.png"
W, H = 1920, 1080
FD = "C:/Windows/Fonts/"

BG, GRID, INK, DIM, PEWTER = (18, 18, 22), (28, 28, 34), (236, 235, 230), (140, 138, 150), (150, 150, 165)
MINT, ORANGE, BLUE, PINK, GOLD = (120, 222, 190), (217, 119, 87), (110, 170, 235), (240, 130, 170), (242, 193, 78)


def F(name, size):
    return ImageFont.truetype(FD + name, int(size))


MONO, SERIF, SERIFB, SANS = "CascadiaMono.ttf", "georgia.ttf", "georgiab.ttf", "segoeui.ttf"


def corners(d, box, color, n=14, w=3):
    x0, y0, x1, y1 = box
    for (x, y, dx, dy) in ((x0, y0, 1, 1), (x1, y0, -1, 1), (x0, y1, 1, -1), (x1, y1, -1, -1)):
        d.line([(x, y), (x + dx * n, y)], fill=color, width=w)
        d.line([(x, y), (x, y + dy * n)], fill=color, width=w)


def panel(d, box, accent, title, sub=None, title_size=30, sub_size=19, fill=(26, 26, 34)):
    x0, y0, x1, y1 = box
    d.rounded_rectangle(box, radius=6, fill=fill, outline=(70, 70, 86), width=2)
    corners(d, box, accent)
    d.rectangle([x0 + 18, y0 + 18, x0 + 26, y0 + 26], fill=accent)
    d.text((x0 + 40, y0 + 10), title, font=F(MONO, 20), fill=accent)
    if sub:
        d.text((x0 + 18, y0 + 52), title_text(title, sub)[0], font=F(SERIFB, title_size), fill=INK)
        for k, line in enumerate(title_text(title, sub)[1:]):
            d.text((x0 + 18, y0 + 52 + title_size + 8 + k * (sub_size + 8)), line, font=F(MONO, sub_size), fill=DIM)


def title_text(title, sub):
    parts = sub.split("|")
    return parts


def arrow(d, p0, p1, color, width=4, dashed=False, head=16, label=None, label_pos=None, label_color=None):
    (x0, y0), (x1, y1) = p0, p1
    import math
    L = math.hypot(x1 - x0, y1 - y0)
    ux, uy = (x1 - x0) / L, (y1 - y0) / L
    end = (x1 - ux * head * 0.6, y1 - uy * head * 0.6)
    if dashed:
        pos = 0
        while pos < L - head:
            a = pos, min(pos + 14, L - head)
            d.line([(x0 + ux * a[0], y0 + uy * a[0]), (x0 + ux * a[1], y0 + uy * a[1])], fill=color, width=width)
            pos += 24
    else:
        d.line([(x0, y0), end], fill=color, width=width)
    nx, ny = -uy, ux
    d.polygon([(x1, y1), (x1 - ux * head + nx * head * 0.55, y1 - uy * head + ny * head * 0.55),
               (x1 - ux * head - nx * head * 0.55, y1 - uy * head - ny * head * 0.55)], fill=color)
    if label:
        lx, ly = label_pos or ((x0 + x1) / 2, (y0 + y1) / 2)
        d.text((lx, ly), label, font=F(MONO, 17), fill=label_color or color, anchor="mm")


def bubble(im, d, box, text, accent, font, tail):
    x0, y0, x1, y1 = box
    d.rounded_rectangle(box, radius=22, fill=(34, 34, 44), outline=accent, width=3)
    tx, ty = tail
    d.polygon([(tx, ty), (x0 + 60, y1 - 2), (x0 + 120, y1 - 2)], fill=(34, 34, 44), outline=accent)
    d.line([(x0 + 62, y1 - 1), (x0 + 118, y1 - 1)], fill=(34, 34, 44), width=4)
    y = y0 + 16
    for line in text:
        d.text((x0 + 22, y), line, font=font, fill=INK)
        y += font.size + 6


def paste_fit(im, path, box, flip=False):
    src = Image.open(path).convert("RGBA")
    if flip:
        src = src.transpose(Image.FLIP_LEFT_RIGHT)
    bb = src.getbbox()
    if bb:
        src = src.crop(bb)
    x0, y0, x1, y1 = box
    s = min((x1 - x0) / src.width, (y1 - y0) / src.height)
    src = src.resize((int(src.width * s), int(src.height * s)), Image.LANCZOS)
    im.alpha_composite(src, (int(x0 + (x1 - x0 - src.width) / 2), int(y1 - src.height)))


def main():
    OUT.parent.mkdir(parents=True, exist_ok=True)
    im = Image.new("RGBA", (W, H), BG + (255,))
    d = ImageDraw.Draw(im)
    for x in range(0, W, 60):
        d.line([(x, 0), (x, H)], fill=GRID, width=1)
    for y in range(0, H, 60):
        d.line([(0, y), (W, y)], fill=GRID, width=1)

    # ---- upper left: the chain  LOCAL CLIENT -> REST SERVER -> DATABASE / LLM API
    client = (60, 70, 380, 250)
    server = (480, 70, 860, 250)
    db = (960, 40, 1330, 150)
    llm = (960, 170, 1330, 280)
    panel(d, client, MINT, "LOCAL CLIENT", "Unreal Engine 5.8|C++ game client")
    panel(d, server, PEWTER, "REST SERVER", "Director service|FastAPI · gunicorn · nginx")
    panel(d, db, BLUE, "DATABASE", "Postgres|", title_size=26, sub_size=16)
    d.text((db[0] + 18, db[1] + 84), "cache · daily budget · rate limits", font=F(MONO, 15), fill=DIM)
    panel(d, llm, ORANGE, "LLM API", "Claude Haiku 5.5|", title_size=26, sub_size=16)
    d.text((llm[0] + 18, llm[1] + 84), "structured output, JSON schema", font=F(MONO, 15), fill=DIM)
    arrow(d, (client[2] + 4, 160), (server[0] - 4, 160), MINT, label="REST/JSON", label_pos=(430, 138))
    arrow(d, (server[2] + 4, 125), (db[0] - 4, 100), BLUE, label="SQL", label_pos=(912, 88), label_color=BLUE)
    arrow(d, (server[2] + 4, 195), (llm[0] - 4, 220), ORANGE, label="HTTPS", label_pos=(912, 248))
    # fallback: any failure -> the game's own content
    d.line([(670, 252), (670, 300), (330, 300)], fill=GOLD, width=3)
    arrow(d, (330, 300), (330, 254), GOLD, width=3, dashed=True)
    d.text((500, 330), "any failure or timeout: the game plays its authored quests", font=F(MONO, 16), fill=GOLD, anchor="mm")
    d.text((500, 354), "nothing breaks", font=F(MONO, 16), fill=GOLD, anchor="mm")

    # ---- lower left: the player (knight) talking to a villager; arrow to the client
    d.rounded_rectangle((60, 390, 900, 1010), radius=10, fill=(24, 24, 30), outline=(60, 60, 74), width=2)
    corners(d, (60, 390, 900, 1010), MINT)
    d.text((84, 402), "IN THE GAME", font=F(MONO, 19), fill=MINT)
    paste_fit(im, ROOT / "ImportSource/Meshy/SK_Knight.png", (110, 560, 330, 990))
    paste_fit(im, ROOT / "Assets/Incoming/2D/T_Portrait_Farmer/T_Portrait_Farmer.png", (640, 700, 860, 990), flip=True)
    d.text((120, 995 - 4), "", font=F(MONO, 14), fill=DIM)
    bubble(im, d, (250, 450, 660, 570),
           ["Bribe the rats with snacks", "so they go home."], MINT, F(SERIFB, 27), (232, 622))
    bubble(im, d, (540, 610, 880, 700),
           ["A Quiet Constituency:", "pacify the rats."], PINK, F(SERIFB, 25), (735, 768))
    d.text((84, 1012 - 8), "", font=F(MONO, 14), fill=DIM)
    arrow(d, (150, 390), (150, 256), MINT, width=5, label="typed plan", label_pos=(92, 330), label_color=MINT)

    # ---- right: endpoints and the request pipeline
    d.text((1400, 58), "ENDPOINTS", font=F(MONO, 19), fill=PEWTER)
    for k, e in enumerate(["POST /v1/director/note", "POST /v1/director/premiere", "POST /v1/director/quest/branches",
                           "POST /v1/director/quest/propose", "GET  /v1/director/readyz"]):
        d.text((1400, 92 + k * 34), e, font=F(MONO, 19), fill=INK if k < 4 else DIM)

    d.text((960, 352), "EVERY REQUEST PASSES THESE GATES", font=F(MONO, 19), fill=PEWTER)
    steps = [("1", "validate", "schema, 80 chars"), ("2", "rate limit", "per session + address"), ("3", "screen", "links, contact info"),
             ("4", "cache", "same note, no model"), ("5", "budget", "one atomic daily cap"), ("6", "model", "Haiku, structured"),
             ("7", "clamp", "server enforces limits")]
    x, y = 960, 392
    for k, (n, name, sub) in enumerate(steps):
        bx = x + (k % 4) * 232
        by = y + (k // 4) * 124
        accent = ORANGE if name == "model" else GOLD if name == "clamp" else PEWTER
        d.rounded_rectangle((bx, by, bx + 214, by + 104), radius=6, fill=(26, 26, 34), outline=accent, width=2)
        d.text((bx + 12, by + 8), n, font=F(MONO, 18), fill=accent)
        d.text((bx + 38, by + 4), name, font=F(SERIFB, 27), fill=INK)
        d.text((bx + 12, by + 52), sub, font=F(MONO, 14), fill=DIM)
        if k % 4 < 3 and k != 6:
            arrow(d, (bx + 216, by + 52), (bx + 230, by + 52), accent, width=2, head=9)

    # ---- what's new
    d.text((960, 660), "WHAT'S NEW HERE", font=F(MONO, 19), fill=MINT)
    cards = [(MINT, "The model proposes.", "The server clamps.", "Every field is forced back inside limits the game was built to handle."),
             (GOLD, "It can't break the game.", "", "The AI only picks from pieces we built and tested; nonsense is refused in character."),
             (PINK, "Quests rewrite live.", "", "Pacify the rats, then feed them celery: new routes while you play.")]
    for k, (c, t1, t2, body) in enumerate(cards):
        by = 700 + k * 100
        d.rounded_rectangle((960, by, 1860, by + 88), radius=6, fill=(26, 26, 34), outline=(70, 70, 86), width=2)
        d.rectangle((960, by, 968, by + 88), fill=c)
        d.text((986, by + 8), t1 + (" " + t2 if t2 else ""), font=F(SERIFB, 29), fill=INK)
        d.text((986, by + 50), body, font=F(SANS, 21), fill=DIM)
    d.text((960, 1012), "Tested against 23 canned notes, 13 of them hostile.", font=F(MONO, 17), fill=DIM)

    im.convert("RGB").save(OUT)
    print("wrote", OUT)


if __name__ == "__main__":
    main()
