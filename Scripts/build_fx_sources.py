"""Build original low-poly OBJ props and PNG masks. Standard library only.

python Scripts/build_fx_sources.py
Outputs ImportSource/FX; reruns reproduce the same source assets.
"""
import hashlib
import json
import math
from pathlib import Path
import struct
import zlib

ROOT = Path(__file__).resolve().parents[1] / "ImportSource" / "FX"


class Mesh:
    def __init__(self):
        self.vertices, self.uvs, self.faces = [], [], []

    def vertex(self, xyz, uv=(0, 0)):
        self.vertices.append(xyz)
        self.uvs.append(uv)
        return len(self.vertices)

    def tri(self, *indices):
        self.faces.append(indices)

    def quad(self, a, b, c, d):
        self.tri(a, b, c)
        self.tri(a, c, d)

    def tube(self, rings, segments=12):
        """Closed lathe along +X; rings contain (x, radius)."""
        rows = []
        for i, (x, radius) in enumerate(rings):
            row = []
            for j in range(segments):
                angle = math.tau * j / segments
                row.append(self.vertex((x, radius * math.cos(angle), radius * math.sin(angle)),
                                       (j / segments, i / (len(rings) - 1))))
            rows.append(row)
        for near, far in zip(rows, rows[1:]):
            for j in range(segments):
                k = (j + 1) % segments
                self.quad(near[j], near[k], far[k], far[j])
        for row, x, reverse in ((rows[0], rings[0][0], True), (rows[-1], rings[-1][0], False)):
            center = self.vertex((x, 0, 0), (0.5, 0.5))
            for j in range(segments):
                a, b = row[j], row[(j + 1) % segments]
                self.tri(center, b, a) if reverse else self.tri(center, a, b)

    def save(self, name):
        lines = ["# Original procedural FX mesh; centimetres, no collision"]
        lines += ["v %.6f %.6f %.6f" % tuple(v) for v in self.vertices]
        lines += ["vt %.6f %.6f" % uv for uv in self.uvs]
        lines += ["f " + " ".join(f"{i}/{i}" for i in face) for face in self.faces]
        path = ROOT / (name + ".obj")
        path.write_text("\n".join(lines) + "\n", encoding="ascii")
        return {"name": name, "triangles": len(self.faces), "vertices": len(self.vertices),
                "bounds_min": [min(v[i] for v in self.vertices) for i in range(3)],
                "bounds_max": [max(v[i] for v in self.vertices) for i in range(3)]}


def annulus(half_angle=math.pi, segments=32):
    mesh = Mesh()
    for j in range(segments + 1):
        a = -half_angle + 2 * half_angle * j / segments
        for radius in (84, 100):
            mesh.vertex((radius * math.cos(a), radius * math.sin(a), 0), (j / segments, (radius - 84) / 16))
    for j in range(segments):
        n = j * 2 + 1
        mesh.quad(n, n + 1, n + 3, n + 2)
    return mesh


def plane(width=40, length=40, start=False):
    mesh = Mesh()
    x = 0 if start else -length / 2
    for v, uv in zip(((x, -width / 2, 0), (x + length, -width / 2, 0),
                      (x + length, width / 2, 0), (x, width / 2, 0)),
                     ((0, 0), (1, 0), (1, 1), (0, 1))):
        mesh.vertex(v, uv)
    mesh.quad(1, 2, 3, 4)
    return mesh


def png(name, pixel, size=128):
    raw = bytearray()
    for y in range(size):
        raw.append(0)
        for x in range(size):
            raw.extend(pixel((x + 0.5) / size * 2 - 1, (y + 0.5) / size * 2 - 1))
    def chunk(kind, data):
        return struct.pack(">I", len(data)) + kind + data + struct.pack(">I", zlib.crc32(kind + data))
    data = b"\x89PNG\r\n\x1a\n" + chunk(b"IHDR", struct.pack(">2I5B", size, size, 8, 6, 0, 0, 0))
    data += chunk(b"IDAT", zlib.compress(bytes(raw), 9)) + chunk(b"IEND", b"")
    (ROOT / (name + ".png")).write_bytes(data)


def main():
    ROOT.mkdir(parents=True, exist_ok=True)
    meshes = {"SM_FX_Ring": annulus(), "SM_FX_Arc": annulus(math.radians(35), 20),
              "SM_FX_Card": plane(), "SM_FX_Napkin": plane(28, 36),
              "SM_FX_Strand": plane(1.5, 100, True)}
    pin = Mesh()
    pin.tube([(-30, 2.5), (-20, 2.5), (-18, 6), (-16, 7), (16, 7), (18, 6), (20, 2.5), (30, 2.5)])
    meshes["SM_FX_Pin"] = pin
    candy = Mesh()
    candy.tube([(-3, 15), (-2, 17.5), (2, 17.5), (3, 15)], 16)
    meshes["SM_FX_Peppermint"] = candy
    celery = Mesh()
    celery.tube([(-32, 1.5), (-26, 3), (16, 4), (25, 2.5)], 8)
    for tip_y, tip_z in ((10, 3), (-8, 4), (0, 10)):
        base = celery.vertex((12, 0, 0), (0, 0.5))
        a = celery.vertex((24, tip_y - 4, tip_z), (0.5, 0))
        b = celery.vertex((33, tip_y, tip_z + 3), (1, 0.5))
        c = celery.vertex((24, tip_y + 4, tip_z), (0.5, 1))
        celery.quad(base, a, b, c)
    meshes["SM_FX_Celery"] = celery
    cube = Mesh()
    for axis, sign in ((0, -1), (0, 1), (1, -1), (1, 1), (2, -1), (2, 1)):
        indices = []
        other = [i for i in range(3) if i != axis]
        for (a, b), uv in zip(((-1, -1), (1, -1), (1, 1), (-1, 1)), ((0, 0), (1, 0), (1, 1), (0, 1))):
            v = [0, 0, 0]
            v[axis], v[other[0]], v[other[1]] = sign * 5, a * 5, b * 5
            indices.append(cube.vertex(v, uv))
        # outward winding depends on axis parity
        if sign * (-1 if axis == 1 else 1) < 0:
            indices.reverse()
        cube.quad(*indices)
    meshes["SM_FX_SugarCube"] = cube
    dome = Mesh()
    for i in range(9):
        theta = math.pi / 2 * i / 8
        for j in range(25):
            phi = math.tau * j / 24
            dome.vertex((100 * math.sin(theta) * math.cos(phi), 100 * math.sin(theta) * math.sin(phi),
                         100 * math.cos(theta)), (j / 24, i / 8))
    for i in range(8):
        for j in range(24):
            a = i * 25 + j + 1
            if i == 0:
                dome.tri(a, a + 25, a + 26)
            else:
                dome.quad(a, a + 25, a + 26, a + 1)
    meshes["SM_FX_Dome"] = dome
    records = [mesh.save(name) for name, mesh in meshes.items()]
    def rgba(alpha):
        return (255, 255, 255, round(max(0, min(1, alpha)) * 255))
    png("T_FX_SoftDisc", lambda x, y: rgba(max(0, 1 - x*x - y*y) ** 2))
    png("T_FX_Bubble", lambda x, y: rgba(math.exp(-((math.hypot(x, y) - 0.78) / 0.055) ** 2)
        * (0.35 + 0.65 * max(0, (-x-y) / 1.414))))
    png("T_FX_Star", lambda x, y: rgba(max(0, 1 - (abs(x)**0.65 + abs(y)**0.65)) ** 0.8))
    png("T_FX_Peppermint", lambda x, y: (240, 28, 65, 255) if int((math.atan2(y, x) / math.tau + 1) * 12) % 2 else (255, 246, 226, 255))
    manifest = {"units": "centimetres", "origin": "Original procedural geometry and masks", "meshes": records,
                "sha256": {p.name: hashlib.sha256(p.read_bytes()).hexdigest()
                           for p in sorted(ROOT.iterdir()) if p.suffix in (".obj", ".png")}}
    (ROOT / "manifest.json").write_text(json.dumps(manifest, indent=2) + "\n", encoding="utf-8")
    print(f"FX source build: {len(records)} meshes, 4 textures -> {ROOT}")


if __name__ == "__main__":
    main()
