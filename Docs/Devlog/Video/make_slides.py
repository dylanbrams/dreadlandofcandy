"""Simple slides for beats that were placeholder cards: graphics/slide_*.png (1920x1080), same look as the diagram.

    python -I make_slides.py

Wording comes only from Tomorrow/SCRIPT.md, Declared.md and the project records; nothing here is new fact.
"""
import textwrap
from pathlib import Path

from PIL import Image, ImageDraw, ImageFont

HERE = Path(__file__).parent
OUT = HERE / "graphics"
W, H = 1920, 1080
FD = "C:/Windows/Fonts/"
BG, GRID, INK, DIM = (18, 18, 22), (28, 28, 34), (236, 235, 230), (150, 148, 160)
MINT, ORANGE, GREEN, GOLD, PINK = (120, 222, 190), (217, 119, 87), (110, 200, 140), (242, 193, 78), (240, 130, 170)


def F(name, size):
    return ImageFont.truetype(FD + name, int(size))


def base():
    im = Image.new("RGB", (W, H), BG)
    d = ImageDraw.Draw(im)
    for x in range(0, W, 60):
        d.line([(x, 0), (x, H)], fill=GRID)
    for y in range(0, H, 60):
        d.line([(0, y), (W, y)], fill=GRID)
    return im, d


def frame(d, accent):
    box = (70, 70, W - 70, H - 70)
    d.rounded_rectangle(box, radius=8, outline=(70, 70, 86), width=2)
    x0, y0, x1, y1 = box
    for (x, y, dx, dy) in ((x0, y0, 1, 1), (x1, y0, -1, 1), (x0, y1, 1, -1), (x1, y1, -1, -1)):
        d.line([(x, y), (x + dx * 30, y)], fill=accent, width=4)
        d.line([(x, y), (x, y + dy * 30)], fill=accent, width=4)


def tag(d, text, accent):
    d.text((120, 108), text, font=F("CascadiaMono.ttf", 26), fill=accent)


def title(d, text, y=190, size=84, accent=INK, width=28):
    for k, line in enumerate(textwrap.wrap(text, width)):
        d.text((120, y + k * (size + 14)), line, font=F("georgiab.ttf", size), fill=accent)
    return y + len(textwrap.wrap(text, width)) * (size + 14)


def bullets(d, items, y, accent, size=44, gap=26, width=62, dx=0):
    for it in items:
        d.rectangle((120 + dx, y + size * 0.35, 120 + dx + 16, y + size * 0.35 + 16), fill=accent)
        for k, line in enumerate(textwrap.wrap(it, width)):
            d.text((160 + dx, y + k * (size + 8)), line, font=F("segoeui.ttf", size), fill=INK)
        y += len(textwrap.wrap(it, width)) * (size + 8) + gap
    return y


def save(im, name):
    OUT.mkdir(exist_ok=True)
    im.save(OUT / name)
    print("wrote", name)


def main():
    # 1. the experiment
    im, d = base(); frame(d, MINT); tag(d, "THE EXPERIMENT", MINT)
    title(d, "Can one person, directing AI, make something worth playing?", y=300, size=96, width=26)
    save(im, "slide_idea.png")

    # 2. who did what
    im, d = base(); frame(d, MINT); tag(d, "THE TEAM", MINT)
    d.text((120, 200), "I directed and playtested.", font=F("georgiab.ttf", 78), fill=INK)
    for x, name, col in ((120, "CLAUDE", ORANGE), (1000, "CHATGPT", GREEN)):
        d.rounded_rectangle((x, 360, x + 780, 700), radius=8, fill=(26, 26, 34), outline=col, width=3)
        d.rectangle((x, 360, x + 10, 700), fill=col)
        d.text((x + 40, 385), name, font=F("CascadiaMono.ttf", 40), fill=col)
        for k, line in enumerate(["planning · code · art", "terrain · audio · integration"]):
            d.text((x + 40, 470 + k * 70), line, font=F("segoeui.ttf", 46), fill=INK)
    d.text((120, 760), "plus a pile of generation tools", font=F("segoeui.ttf", 44), fill=DIM)
    d.text((120, 830), "Handoffs, parallel work, and a lot of checking whether the thing that looked finished", font=F("segoeui.ttf", 36), fill=DIM)
    d.text((120, 880), "actually worked in the game.", font=F("segoeui.ttf", 36), fill=DIM)
    save(im, "slide_team.png")

    # 3. declared work
    im, d = base(); frame(d, GOLD); tag(d, "DECLARED", GOLD)
    y = title(d, "Pre-jam work is declared.", y=250, size=96, width=24)
    d.text((120, y + 50), "Setup and design work that existed before the jam is listed", font=F("segoeui.ttf", 46), fill=INK)
    d.text((120, y + 118), "on the itch.io page and in Docs/Declared.md.", font=F("segoeui.ttf", 46), fill=INK)
    save(im, "slide_declared.png")

    # 4. release plan
    im, d = base(); frame(d, PINK); tag(d, "THE RELEASE PLAN", PINK)
    y = title(d, "What can we actually ship?", y=220, size=92, width=26)
    y = bullets(d, ["The release plan protected time for packaging and testing.",
                    "Then I changed the plan: the village quest givers were too central to cut.",
                    "Five villagers and ten linked jobs went back in."], y + 60, PINK, size=48, width=56)
    save(im, "slide_release.png")

    # 5. generated models need work
    im, d = base(); frame(d, ORANGE); tag(d, "GENERATED ASSETS", ORANGE)
    y = title(d, "A generated model is a starting point.", y=220, size=88, width=26)
    bullets(d, ["Judge the look", "Clean the meshes", "Sort out scale and animation",
                "Does it work at the game camera?"], y + 70, ORANGE, size=54, width=40)
    save(im, "slide_cleanup.png")

    # 7. sound
    im, d = base(); frame(d, ORANGE); tag(d, "SOUND", ORANGE)
    y = title(d, "A voiced cast, and a full score.", y=220, size=88, width=26)
    bullets(d, ["A voiced faerie director: 88 lines", "Voiced heroes, one cast per class", "47 sound effects",
                "About twenty pieces of orchestral score, two takes each"], y + 60, ORANGE, size=50, width=44)
    save(im, "slide_sound.png")

    # 8. the bot
    im, d = base(); frame(d, MINT); tag(d, "TESTING", MINT)
    y = title(d, "A bot plays the whole game by itself.", y=220, size=88, width=26)
    bullets(d, ["Every build gets checked, start to credits", "89 automated tests", "A packaged build, smoke-tested"],
            y + 60, MINT, size=52, width=44)
    save(im, "slide_bot.png")

    # 6. what's in here
    im, d = base(); frame(d, MINT); tag(d, "WHAT'S IN HERE", MINT)
    items = ["154 battle cries in other languages", "A voiced faerie director and a voiced cast",
             "3D models auditioned across Meshy, Rodin, Hunyuan3D and Tripo, then cleaned up and rigged",
             "About twenty pieces of score, two takes each, from ACE Studio",
             "Fifteen generated candy PBR materials, picked from a review page",
             "Generated buildings, bridge, foliage and marshmallows",
             "A bot that plays the whole game unattended, so every build gets checked"]
    bullets(d, items, 190, MINT, size=44, gap=20, width=70)
    save(im, "slide_inside.png")


if __name__ == "__main__":
    main()
