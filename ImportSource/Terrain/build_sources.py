"""Reproducible Sugarborough terrain, original low-poly props, and baked scatter.

Run: python -I ImportSource/Terrain/build_sources.py
No engine, network, paid services or third-party Python packages required.
Coordinates are Unreal centimetres; +X north, +Y east, +Z up.
"""
import json
import math
import random
import struct
import zlib
from pathlib import Path

ROOT = Path(__file__).resolve().parent
PALETTE = {
    "Village": [0.57, 0.34, 0.19], "Meadow": [0.30, 0.63, 0.29],
    "Molars": [0.49, 0.30, 0.40], "Orchard": [0.19, 0.43, 0.29],
    "Path": [0.91, 0.67, 0.36], "Pink": [0.95, 0.22, 0.49],
    "Mint": [0.26, 0.87, 0.69], "Cream": [0.96, 0.86, 0.63],
    "Chocolate": [0.17, 0.065, 0.035], "Lavender": [0.54, 0.33, 0.76],
    "Red": [0.80, 0.06, 0.12], "White": [0.97, 0.94, 0.85],
    "Window": [0.10, 0.35, 0.43], "Dark": [0.045, 0.025, 0.055],
}
YAWS = {"Meadow": 0, "Molars": 120, "Orchard": -120}


def polar(yaw, distance):
    return (math.cos(math.radians(yaw)) * distance, math.sin(math.radians(yaw)) * distance)


def smooth(a, b, x):
    t = max(0, min(1, (x - a) / (b - a)))
    return t * t * (3 - 2 * t)


def biome(x, y):
    if math.hypot(x, y) < 2500:
        return "Village"
    yaw = math.degrees(math.atan2(y, x))
    return min(YAWS, key=lambda b: abs((yaw - YAWS[b] + 180) % 360 - 180))


def locations():
    rows = []
    def add(name, kind, b, yaw, distance, radius):
        x, y = polar(yaw, distance)
        rows.append(dict(id=name, kind=kind, biome=b, x=x, y=y, radius=radius))
    add("Village", "Village", "Village", 0, 0, 2500)
    add("Signpost", "Landmark", "Village", 60, 500, 100)
    add("GiverMayor", "Giver", "Village", 180, 700, 150)
    add("GiverBaker", "Giver", "Village", -60, 900, 150)
    for b, distance, radius in [("Meadow", 6000, 900), ("Molars", 6500, 700), ("Orchard", 6000, 1000)]:
        add({"Meadow": "SetField", "Molars": "SetFort", "Orchard": "SetOrchard"}[b], "FilmSet", b, YAWS[b], distance, radius)
        add("Giver" + b, "Giver", b, YAWS[b] + (20 if b == "Orchard" else -20), 3200, 150)
    for b, camps in {
        "Meadow": [("West", -35, 4200), ("East", 35, 4200), ("Far", 10, 9800)],
        "Molars": [("Near", 30, 4300), ("Ridge", -35, 8000), ("Far", 15, 9500)],
        "Orchard": [("Near", -30, 4300), ("Grove", 35, 8200), ("Far", -10, 9600)],
    }.items():
        for suffix, angle, distance in camps:
            add("Camp" + b + suffix, "Camp", b, YAWS[b] + angle, distance, 600)
    return rows


LOCATIONS = locations()


def road_distance(x, y):
    # Three broad radial routes and a connecting ring make every biome reachable.
    r = math.hypot(x, y)
    distances = [abs(r - 3600)] if r > 2700 else []
    for yaw in YAWS.values():
        a = math.radians(yaw)
        along, across = x * math.cos(a) + y * math.sin(a), -x * math.sin(a) + y * math.cos(a)
        if 0 <= along <= 10500:
            distances.append(abs(across))
    return min(distances, default=1e6)


def height(x, y):
    r = math.hypot(x, y)
    wave = 145 * math.sin(x / 1950) * math.sin(y / 2100) + 80 * math.sin((x + y) / 1650)
    h = wave * smooth(2400, 3400, r)
    # Reserve a completely level pad INCLUDING square set corners and a buffer.
    flatten = 1.0
    for row in LOCATIONS:
        if row["kind"] == "Village":
            continue
        d = math.hypot(x - row["x"], y - row["y"])
        radius = row["radius"] * (math.sqrt(2) if row["kind"] == "FilmSet" else 1)
        flatten = min(flatten, smooth(radius + 250, radius + 1100, d))
    h *= flatten * smooth(260, 850, road_distance(x, y))
    # Outside the playable square, a continuous steep candy mountain wall.
    edge = max(abs(x), abs(y))
    h += smooth(11300, 14750, edge) * (2600 + 600 * math.sin(x / 1800) * math.cos(y / 1600))
    return h


def clear(x, y, footprint=0):
    if max(abs(x), abs(y)) > 10800 or math.hypot(x, y) < 2650:
        return False
    if road_distance(x, y) < 330 + footprint:
        return False
    for row in LOCATIONS:
        radius = row["radius"]
        if row["kind"] == "FilmSet":
            radius *= math.sqrt(2)
        elif row["kind"] in ("Giver", "Landmark"):
            radius = max(300, radius)
        if math.hypot(x - row["x"], y - row["y"]) < radius + footprint + 160:
            return False
    return True


class Mesh:
    def __init__(self, name):
        self.name, self.vertices, self.faces = name, [], []

    def face(self, indices, material):
        # Triangles only, so budgets have an unambiguous meaning.
        for i in range(1, len(indices) - 1):
            self.faces.append(([indices[0], indices[i], indices[i + 1]], material))

    def ellipsoid(self, center, size, material, segments=12, rings=7):
        start = len(self.vertices)
        for j in range(rings + 1):
            lat = -math.pi / 2 + math.pi * j / rings
            for i in range(segments):
                angle = 2 * math.pi * i / segments
                self.vertices.append(tuple(center[k] + size[k] * v for k, v in enumerate((math.cos(lat) * math.cos(angle), math.cos(lat) * math.sin(angle), math.sin(lat)))))
        for j in range(rings):
            for i in range(segments):
                a, b = start + j * segments + i, start + j * segments + (i + 1) % segments
                self.face([a, b, b + segments, a + segments], material)

    def box(self, center, size, material):
        start = len(self.vertices)
        for z in (-1, 1):
            for y in (-1, 1):
                for x in (-1, 1):
                    self.vertices.append(tuple(center[k] + size[k] * v / 2 for k, v in enumerate((x, y, z))))
        for face in [(0, 2, 3, 1), (4, 5, 7, 6), (0, 1, 5, 4), (2, 6, 7, 3), (0, 4, 6, 2), (1, 3, 7, 5)]:
            self.face([start + i for i in face], material)

    def cylinder(self, center, radius, depth, material, segments=12):
        start = len(self.vertices)
        for z in (-depth / 2, depth / 2):
            for i in range(segments):
                a = i * 2 * math.pi / segments
                self.vertices.append((center[0] + radius * math.cos(a), center[1] + radius * math.sin(a), center[2] + z))
        self.face([start + i for i in reversed(range(segments))], material)
        self.face([start + segments + i for i in range(segments)], material)
        for i in range(segments):
            j = (i + 1) % segments
            self.face([start + i, start + j, start + segments + j, start + segments + i], material)

    def save(self):
        target = ROOT / "Sources" / (self.name + ".obj")
        target.parent.mkdir(parents=True, exist_ok=True)
        lines = ["# Original procedural Sugarborough mesh; centimetres, Z up", "mtllib terrain.mtl", "o " + self.name]
        lines.extend("v %.4f %.4f %.4f" % p for p in self.vertices)
        if self.name.startswith("SM_Terrain"):
            lines.extend("vt %.6f %.6f" % ((p[0] + 16000) / 32000, (p[1] + 16000) / 32000) for p in self.vertices)
        current = None
        slots = []
        for indices, mat in self.faces:
            if mat != current:
                lines.append("usemtl " + mat)
                current = mat
            if mat not in slots:
                slots.append(mat)
            lines.append("f " + " ".join(f"{i + 1}/{i + 1}" if self.name.startswith("SM_Terrain") else str(i + 1) for i in indices))
        target.write_text("\n".join(lines) + "\n", encoding="utf-8")
        return dict(name=self.name, triangles=len(self.faces), materials=slots)


def props():
    result = []
    for name, color in [("Gumdrop", "Pink"), ("MintBoulder", "Mint"), ("SugarBoulder", "Lavender")]:
        m = Mesh("SM_" + name)
        m.ellipsoid((0, 0, 75), (120, 105, 115), color, 14, 9)
        m.ellipsoid((55, 30, 25), (85, 90, 55), "Cream", 10, 5)
        result.append(m.save())
    m = Mesh("SM_LollipopPine")
    m.cylinder((0, 0, 200), 24, 400, "Cream")
    for z, radius, color in [(310, 155, "Mint"), (420, 125, "Pink"), (510, 90, "Cream")]:
        m.ellipsoid((0, 0, z), (radius, radius, 110), color, 12, 7)
    result.append(m.save())
    m = Mesh("SM_CottonCandyTree")
    m.cylinder((0, 0, 200), 30, 400, "Chocolate")
    for x, y, z, color in [(-85, 0, 420, "Pink"), (85, 0, 450, "Lavender"), (0, 50, 540, "Pink")]:
        m.ellipsoid((x, y, z), (160, 140, 160), color, 12, 7)
    result.append(m.save())
    m = Mesh("SM_WaferCliff")
    m.box((0, 0, 550), (800, 450, 1100), "Chocolate")
    for z in range(100, 1100, 180):
        m.box((0, -232, z), (810, 25, 55), "Cream")
    for x in range(-300, 400, 150):
        m.box((x, -250, 550), (35, 25, 1100), "Village")
    result.append(m.save())
    m = Mesh("SM_SugarMountain")
    for j, (z, radius) in enumerate([(0, 1400), (550, 1350), (1250, 1000), (1900, 620), (2400, 300), (2650, 0)]):
        for i in range(14):
            a = i * math.pi / 7
            ripple = 1 + .15 * math.sin(i * 2.3 + j * .8)
            m.vertices.append((radius * ripple * math.cos(a) + j * 35, radius * .85 * ripple * math.sin(a), z))
    for j in range(5):
        for i in range(14):
            a, b = j * 14 + i, j * 14 + (i + 1) % 14
            m.face([a, b, b + 14, a + 14], "White" if j >= 3 else "Lavender")
    result.append(m.save())
    m = Mesh("SM_GingerbreadCottage")
    m.box((0, 0, 190), (500, 460, 380), "Village")
    # Gabled roof, two solid triangular ends; front faces -X.
    start = len(m.vertices)
    m.vertices += [(-290, -280, 380), (-290, 280, 380), (-290, 0, 620), (290, -280, 380), (290, 280, 380), (290, 0, 620)]
    for f in [(0, 2, 1), (3, 4, 5), (0, 3, 5, 2), (1, 2, 5, 4), (0, 1, 4, 3)]:
        m.face([start + i for i in f], "Pink")
    m.box((-256, 0, 115), (18, 95, 230), "Chocolate")
    for y in (-155, 155):
        m.box((-260, y, 240), (24, 95, 100), "Cream")
        m.box((-275, y, 240), (12, 65, 70), "Window")
    for x in (-255, 255):
        for y in (-235, 235):
            m.cylinder((x, y, 195), 24, 400, "Cream", 10)
    result.append(m.save())
    m = Mesh("SM_CandyWell")
    m.cylinder((0, 0, 55), 150, 110, "Cream", 16)
    m.cylinder((0, 0, 113), 110, 10, "Window", 16)
    for y in (-125, 125):
        m.cylinder((0, y, 200), 18, 290, "Red", 10)
    m.ellipsoid((0, 0, 350), (180, 180, 75), "Pink", 16, 6)
    result.append(m.save())
    m = Mesh("SM_CandyCaveMouth")
    # Thick horseshoe around a real opening. Dark rear lies beyond the opening.
    for i in range(11):
        a = i * math.pi / 10
        m.ellipsoid((0, 360 * math.cos(a), 110 + 410 * math.sin(a)), (200, 115, 140), "Lavender", 10, 6)
    m.box((190, 0, 230), (25, 550, 460), "Dark")
    result.append(m.save())
    m = Mesh("SM_FondantTower")
    m.cylinder((0, 0, 350), 180, 700, "Cream", 14)
    m.cylinder((0, 0, 700), 220, 90, "Pink", 14)
    for i in range(8):
        a = i * math.pi / 4
        m.box((180 * math.cos(a), 180 * math.sin(a), 790), (90, 90, 150), "Cream")
    m.box((-182, 0, 490), (15, 65, 140), "Window")
    result.append(m.save())
    m = Mesh("SM_FondantGate")
    for y in (-330, 330):
        m.box((0, y, 260), (160, 150, 520), "Cream")
        m.ellipsoid((0, y, 560), (110, 110, 130), "Pink", 12, 7)
    m.box((0, 0, 490), (170, 600, 110), "Cream")
    m.box((-90, 0, 500), (15, 280, 55), "Chocolate")
    result.append(m.save())
    return result


def terrain():
    result = []
    for tx in range(4):
        for ty in range(4):
            m = Mesh(f"SM_Terrain_{tx}_{ty}")
            for ix in range(33):
                for iy in range(33):
                    x, y = -16000 + tx * 8000 + ix * 250, -16000 + ty * 8000 + iy * 250
                    m.vertices.append((x, y, height(x, y)))
            for ix in range(32):
                for iy in range(32):
                    a = ix * 33 + iy
                    x, y = -16000 + tx * 8000 + (ix + .5) * 250, -16000 + ty * 8000 + (iy + .5) * 250
                    m.face([a, a + 33, a + 34, a + 1], "Terrain")
            result.append(m.save())
    return result


def placements():
    rng = random.Random(20261112)
    result = []
    def add(mesh, x, y, scale=1, yaw=0, blocking=False, category="Scatter", z=None):
        result.append(dict(mesh=mesh, location=[x, y, height(x, y) if z is None else z], scale=[scale] * 3, yaw=yaw, blocking=blocking, category=category, biome=biome(x, y)))
    palettes = {"Meadow": ["Gumdrop", "MintBoulder", "CottonCandyTree"], "Molars": ["SugarBoulder", "MintBoulder", "LollipopPine"], "Orchard": ["LollipopPine", "CottonCandyTree", "Gumdrop"]}
    occupied = []
    for _ in range(3500):
        x, y = rng.uniform(-10800, 10800), rng.uniform(-10800, 10800)
        b = biome(x, y)
        if b == "Village":
            continue
        choice = rng.choice(palettes[b])
        scale = rng.uniform(.65, 1.35)
        footprint = (270 if "Tree" in choice or "Pine" in choice else 180) * scale
        if not clear(x, y, footprint) or any(math.hypot(x - px, y - py) < footprint + pr + 90 for px, py, pr in occupied):
            continue
        occupied.append((x, y, footprint))
        add("SM_" + choice, x, y, scale, rng.uniform(0, 360))
        if len(occupied) >= 520:
            break
    # Village homes face the square. Keep radial roads and all NPCs unobstructed.
    for yaw in (30, 85, 150, 210, 265, 330):
        x, y = polar(yaw, 1750)
        add("SM_GingerbreadCottage", x, y, 1, yaw, True, "Village")
    add("SM_CandyWell", -350, 350, 1, category="Village", blocking=True)
    # Fort silhouette outside the reserved gameplay floor. The gate faces home.
    fx, fy = polar(120, 6500)
    a = math.radians(120)
    for along, across in [(-1250, -1200), (-1250, 1200), (1250, -1200), (1250, 1200)]:
        x, y = fx + along * math.cos(a) - across * math.sin(a), fy + along * math.sin(a) + across * math.cos(a)
        add("SM_FondantTower", x, y, 1.3, 120, True, "Fort")
    gx, gy = polar(120, 5100)
    add("SM_FondantGate", gx, gy, 1.4, 120, True, "Fort")
    # Mountains are outside gameplay bounds and mask the continuous perimeter collision.
    for side in range(4):
        for step in range(12):
            along = -13750 + step * 2500 + rng.uniform(-500, 500)
            edge = rng.uniform(12700, 13800)
            x, y = [(edge, along), (-edge, along), (along, edge), (along, -edge)][side]
            add("SM_SugarMountain", x, y, 1, rng.uniform(0, 360), True, "Boundary", z=100)
            result[-1]["scale"] = [rng.uniform(1.1, 1.65), rng.uniform(1.1, 1.65), rng.uniform(.8, 1.7)]
    x, y = polar(65, 10400)
    add("SM_CandyCaveMouth", x, y, 1.6, 65, True, "Cave")
    return result


def validate(data):
    # Sample the design surface; the engine pass also verifies imported collision.
    worst = 0
    for row in LOCATIONS:
        if row["kind"] not in ("FilmSet", "Camp", "Village"):
            continue
        radius = row["radius"]
        for ix in range(-10, 11):
            for iy in range(-10, 11):
                x, y = row["x"] + radius * ix / 10, row["y"] + radius * iy / 10
                if row["kind"] != "FilmSet" and math.hypot(x - row["x"], y - row["y"]) > radius:
                    continue
                slope = math.degrees(math.atan(math.hypot(height(x + 2, y) - height(x - 2, y), height(x, y + 2) - height(x, y - 2)) / 4))
                worst = max(worst, slope)
    assert worst < 15, worst
    for p in data["placements"]:
        if p["category"] == "Scatter":
            assert clear(*p["location"][:2]), p
    assert all(m["triangles"] <= 1500 for m in data["meshes"] if not m["name"].startswith("SM_Terrain"))
    # Conservatively check every actual terrain triangle whose XY box touches a
    # reserved gameplay footprint. This catches coarse-grid slope artifacts.
    mesh_worst = 0
    for mesh in data["meshes"]:
        if not mesh["name"].startswith("SM_Terrain"):
            continue
        vertices = []
        for line in (ROOT / "Sources" / (mesh["name"] + ".obj")).read_text().splitlines():
            fields = line.split()
            if fields and fields[0] == "v":
                vertices.append(tuple(map(float, fields[1:])))
            elif fields and fields[0] == "f":
                a, b, c = [vertices[int(v.split("/")[0]) - 1] for v in fields[1:]]
                xmin, xmax = min(a[0], b[0], c[0]), max(a[0], b[0], c[0])
                ymin, ymax = min(a[1], b[1], c[1]), max(a[1], b[1], c[1])
                touches = False
                for row in LOCATIONS:
                    if row["kind"] not in ("FilmSet", "Camp", "Village"):
                        continue
                    dx = max(xmin - row["x"], row["x"] - xmax, 0)
                    dy = max(ymin - row["y"], row["y"] - ymax, 0)
                    if (max(dx, dy) if row["kind"] == "FilmSet" else math.hypot(dx, dy)) <= row["radius"]:
                        touches = True
                        break
                if touches:
                    ab, ac = [b[i] - a[i] for i in range(3)], [c[i] - a[i] for i in range(3)]
                    n = (ab[1] * ac[2] - ab[2] * ac[1], ab[2] * ac[0] - ab[0] * ac[2], ab[0] * ac[1] - ab[1] * ac[0])
                    mesh_worst = max(mesh_worst, math.degrees(math.atan2(math.hypot(n[0], n[1]), abs(n[2]))))
    assert mesh_worst < 15, mesh_worst
    return dict(max_gameplay_slope_degrees=worst, max_import_triangle_slope_degrees=mesh_worst, scatter_count=sum(p["category"] == "Scatter" for p in data["placements"]), reserved_locations=len(LOCATIONS), terrain_triangles=sum(m["triangles"] for m in data["meshes"] if m["name"].startswith("SM_Terrain")))


def ground_texture():
    # A single original 1k atlas gives smooth paths without dense terrain geometry.
    width = 1024
    pixels = bytearray()
    for iy in range(width):
        pixels.append(0)
        y = 16000 - (iy + .5) * 32000 / width
        for ix in range(width):
            x = -16000 + (ix + .5) * 32000 / width
            r = math.hypot(x, y)
            rgb = list(PALETTE[biome(x, y)])
            if 2300 < r < 2700:
                outside = list(PALETTE[min(YAWS, key=lambda b: abs((math.degrees(math.atan2(y, x)) - YAWS[b] + 180) % 360 - 180))])
                blend = smooth(2300, 2700, r)
                rgb = [PALETTE["Village"][k] * (1 - blend) + outside[k] * blend for k in range(3)]
            path = (1 - smooth(200, 280, road_distance(x, y))) * (1 - smooth(10300, 10700, max(abs(x), abs(y))))
            path = max(path, 1 - smooth(950, 1050, r))
            grain = 1 + .025 * math.sin(x * .08) * math.sin(y * .067)
            rgb = [(rgb[k] * (1 - path) + PALETTE["Path"][k] * path) * grain for k in range(3)]
            pixels.extend(round(max(0, min(1, c)) * 255) for c in rgb)
    def chunk(kind, contents):
        return struct.pack(">I", len(contents)) + kind + contents + struct.pack(">I", zlib.crc32(kind + contents))
    png = b"\x89PNG\r\n\x1a\n" + chunk(b"IHDR", struct.pack(">IIBBBBB", width, width, 8, 2, 0, 0, 0))
    png += chunk(b"IDAT", zlib.compress(pixels, 9)) + chunk(b"IEND", b"")
    (ROOT / "Sources" / "T_SugarboroughGround.png").write_bytes(png)


def main():
    (ROOT / "Sources").mkdir(parents=True, exist_ok=True)
    ground_texture()
    (ROOT / "Sources" / "terrain.mtl").write_text("\n".join(f"newmtl {key}\nKd {' '.join(map(str, rgb))}\nPr 0.85\n" for key, rgb in PALETTE.items()) + "\nnewmtl Terrain\nKd 1 1 1\nmap_Kd T_SugarboroughGround.png\n", encoding="utf-8")
    data = dict(seed=20261112, world_half_size=12000, village_radius=2500, palette=PALETTE, locations=LOCATIONS, meshes=props() + terrain(), placements=placements())
    data["validation"] = validate(data)
    (ROOT / "world_manifest.json").write_text(json.dumps(data, indent=2) + "\n", encoding="utf-8")
    print(json.dumps(data["validation"], indent=2))
    print(f"Wrote {len(data['meshes'])} original meshes and {len(data['placements'])} placements")


if __name__ == "__main__":
    main()
