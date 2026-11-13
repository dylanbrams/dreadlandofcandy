"""Seed a layered candy ecosystem, preserving quest and traversal clearings."""
import importlib.util
import json
import math
from pathlib import Path
import random

HERE = Path(__file__).resolve().parent
TERRAIN = HERE.parent
spec = importlib.util.spec_from_file_location("terrain_source", TERRAIN / "build_sources.py")
t = importlib.util.module_from_spec(spec)
spec.loader.exec_module(t)
SOURCE = HERE / "Sources"


def allowed(x, y, radius, layer):
    if max(abs(x), abs(y)) > 10900 or math.hypot(x, y) < 2620:
        return False
    if t.road_distance(x, y) < (190 if layer == "grass" else 280) + radius:
        return False
    for row in t.LOCATIONS:
        if row["kind"] == "Village":
            continue
        pad = row["radius"] * (math.sqrt(2) if row["kind"] == "FilmSet" else 1)
        if row["kind"] in ("Giver", "Landmark"):
            pad = max(300, pad)
        if math.hypot(x - row["x"], y - row["y"]) < pad + radius + 100:
            return False
    return True


def save_mesh(mesh):
    old = t.ROOT
    t.ROOT = HERE
    try:
        return mesh.save()
    finally:
        t.ROOT = old


def models():
    result = []
    rng = random.Random(702)
    for name, color in [("SM_CandyGrassMint", "Mint"), ("SM_CandyGrassRose", "Pink"), ("SM_CandyGrassIce", "White")]:
        mesh = t.Mesh(name)
        for i in range(14):
            angle = i * math.tau / 14 + rng.uniform(-.3, .3)
            length = rng.uniform(30, 55)
            width = rng.uniform(2.5, 5)
            base = (rng.uniform(-20, 20), rng.uniform(-20, 20))
            out = (math.cos(angle), math.sin(angle))
            cross = (-out[1], out[0])
            first = len(mesh.vertices)
            for distance, z, w in [(0, 0, width * .6), (length * .22, length * .55, width), (length * .5, length, 0)]:
                for side in (-1, 1):
                    mesh.vertices.append((base[0] + out[0] * distance + cross[0] * w * side,
                                          base[1] + out[1] * distance + cross[1] * w * side, z))
            mesh.face([first, first + 1, first + 3, first + 2], color)
            mesh.face([first + 2, first + 3, first + 4], color)
        result.append(save_mesh(mesh))
    mesh = t.Mesh("SM_CandyflossThicket")
    for i, (x, y, z, r) in enumerate([(-65, 0, 63, 74), (65, 15, 65, 78), (0, -45, 85, 83), (0, 40, 118, 70)]):
        mesh.ellipsoid((x, y, z), (r, r * .88, r * .83), "Pink" if i % 2 else "Cream", 10, 6)
    result.append(save_mesh(mesh))
    mesh = t.Mesh("SM_SugarSnowflake")
    mesh.vertices = [(-3, 0, -3), (3, 0, -3), (3, 0, 3), (-3, 0, 3),
                     (0, -3, -3), (0, 3, -3), (0, 3, 3), (0, -3, 3)]
    mesh.face([0, 1, 2, 3], "White")
    mesh.face([4, 5, 6, 7], "White")
    result.append(save_mesh(mesh))
    return result


def ecology():
    rng = random.Random(2026111301)
    groups = {}
    def place(mesh, x, y, scale, yaw, layer, z_offset=0):
        b = t.biome(x, y)
        key = b + "_" + mesh
        group = groups.setdefault(key, dict(mesh=mesh, biome=b, layer=layer, instances=[]))
        group["instances"].append([round(x, 2), round(y, 2), round(t.height(x, y) + z_offset, 2), round(yaw, 2), round(scale, 3)])
    # Short tufts make the surface continuous, with correlated patches of colour.
    for ix in range(-154, 155):
        for iy in range(-154, 155):
            x, y = ix * 70 + rng.uniform(-24, 24), iy * 70 + rng.uniform(-24, 24)
            if not allowed(x, y, 32, "grass"):
                continue
            if rng.random() < .06:
                continue
            b = t.biome(x, y)
            patch = math.sin(x / 800) * math.cos(y / 730)
            name = "SM_CandyGrassIce" if b == "Molars" else "SM_CandyGrassRose" if patch > .55 else "SM_CandyGrassMint"
            place(name, x, y, rng.uniform(.65, 1.25), rng.uniform(0, 360), "grass", -3)
    # Groves are denser in some regions and open up into glades elsewhere.
    occupied = []
    for _ in range(18000):
        x, y = rng.uniform(-10800, 10800), rng.uniform(-10800, 10800)
        patch = .5 + .5 * math.sin(x / 1500 + math.cos(y / 1300))
        if rng.random() > .25 + .7 * patch:
            continue
        scale = rng.uniform(.8, 1.4)
        radius = 300 * scale
        if not allowed(x, y, radius, "tree"):
            continue
        if any((x - px) ** 2 + (y - py) ** 2 < (radius * .65 + pr * .65) ** 2 for px, py, pr in occupied):
            continue
        b = t.biome(x, y)
        choices = ["SM_Rodin_PeppermintBirch", "SM_Rodin_BubblegumBlossom"] if b != "Molars" else ["SM_Rodin_CandyCaneFir"]
        place(rng.choice(choices), x, y, scale, rng.uniform(0, 360), "tree", -4)
        occupied.append((x, y, radius))
        if len(occupied) >= 1050:
            break
    for ix in range(-29, 30):
        for iy in range(-29, 30):
            x, y = ix * 370 + rng.uniform(-140, 140), iy * 370 + rng.uniform(-140, 140)
            if not allowed(x, y, 160, "bush") or rng.random() < .25:
                continue
            place("SM_CandyflossThicket", x, y, rng.uniform(.65, 1.3), rng.uniform(0, 360), "bush", -12)
            for _ in range(rng.randrange(1, 4)):
                px, py = x + rng.uniform(-220, 220), y + rng.uniform(-220, 220)
                if not allowed(px, py, 90, "flower"):
                    continue
                b = t.biome(px, py)
                choice = rng.choice(["SM_Rodin_CandyFlowerClump", "SM_Rodin_SugarFern", "SM_Rodin_JellybeanBush"]) if b != "Molars" else "SM_Rodin_GummySucculent"
                size = rng.uniform(.85, 1.35) if "Flower" in choice else rng.uniform(.65, 1.05) if "Fern" in choice else rng.uniform(.45, .95)
                place(choice, px, py, size, rng.uniform(0, 360), "flower", -12 if "Flower" in choice or "Fern" in choice else -4)
    # Powdered sugar falls in the snowy molar-country. Animated by a light shader.
    for _ in range(2600):
        x, y = rng.uniform(-9500, 2000), rng.uniform(2500, 10500)
        if t.biome(x, y) == "Molars":
            place("SM_SugarSnowflake", x, y, rng.uniform(.4, 1), rng.uniform(0, 360), "snow", 1700)
    return list(groups.values())


def snow_ground():
    # Original atlas plus a soft powdered-sugar biome. No external imagery.
    from PIL import Image
    image = Image.open(TERRAIN / "Sources/T_SugarboroughGround.png").convert("RGB")
    pixels = image.load()
    for iy in range(1024):
        y = 16000 - (iy + .5) * 32000 / 1024
        for ix in range(1024):
            x = -16000 + (ix + .5) * 32000 / 1024
            yaw = math.degrees(math.atan2(y, x))
            angle = abs((yaw - 120 + 180) % 360 - 180)
            snow = (1 - t.smooth(48, 63, angle)) * t.smooth(2800, 4300, math.hypot(x, y))
            if snow < .001:
                continue
            grain = .96 + .04 * math.sin(x / 260) * math.cos(y / 370)
            color = (.63 * grain, .73 * grain, .83 * grain)
            pixels[ix, iy] = tuple(round(pixels[ix, iy][c] * (1 - snow * .92) + color[c] * 255 * snow * .92) for c in range(3))
    image.save(SOURCE / "T_CandySnowGround.png")


def main():
    SOURCE.mkdir(parents=True, exist_ok=True)
    meshes = models()
    groups = ecology()
    snow_ground()
    counts = {}
    for group in groups:
        counts[group["layer"]] = counts.get(group["layer"], 0) + len(group["instances"])
    result = dict(seed=2026111301, meshes=meshes, groups=groups, counts=counts, excluded_willow=True)
    (HERE / "ecology.json").write_text(json.dumps(result, separators=(",", ":")), encoding="utf-8")
    print(json.dumps(counts, indent=2))
    print("DENSE ECOLOGY SOURCE PASS")


if __name__ == "__main__":
    main()
