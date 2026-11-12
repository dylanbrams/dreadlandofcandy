"""Check saved GLBs, triangle budgets, UVs, texture size and raw provenance."""
import hashlib
import json
import math
from pathlib import Path
import struct

HERE = Path(__file__).resolve().parent


def read_glb(path):
    raw = path.read_bytes()
    magic, version, total = struct.unpack_from("<III", raw)
    assert magic == 0x46546C67 and version == 2 and total == len(raw), path
    length, kind = struct.unpack_from("<II", raw, 12)
    assert kind == 0x4E4F534A
    doc = json.loads(raw[20:20 + length])
    binary_offset = 20 + length
    binary_length, binary_kind = struct.unpack_from("<II", raw, binary_offset)
    assert binary_kind == 0x004E4942
    binary = raw[binary_offset + 8:binary_offset + 8 + binary_length]
    assert all(not entry.get("uri") or entry["uri"].startswith("data:") for entry in doc.get("images", []) + doc.get("buffers", []))
    return doc, binary


def accessor_values(doc, binary, index):
    accessor = doc["accessors"][index]
    view = doc["bufferViews"][accessor["bufferView"]]
    code = {5126: "f", 5125: "I", 5123: "H", 5121: "B"}[accessor["componentType"]]
    dimensions = {"SCALAR": 1, "VEC2": 2, "VEC3": 3, "VEC4": 4}[accessor["type"]]
    item_size = struct.calcsize("<" + code * dimensions)
    stride = view.get("byteStride", item_size)
    offset = view.get("byteOffset", 0) + accessor.get("byteOffset", 0)
    return [struct.unpack_from("<" + code * dimensions, binary, offset + i * stride) for i in range(accessor["count"])]


def main():
    manifest = json.loads((HERE / "prepared_manifest.json").read_text(encoding="utf-8"))
    assert len(manifest["assets"]) == 8
    results = []
    for asset in manifest["assets"]:
        name = asset["name"]
        raw = HERE / "Raw" / (name + ".glb")
        clean = HERE / "Clean" / (name + ".glb")
        assert hashlib.sha256(raw.read_bytes()).hexdigest() == asset["raw_sha256"]
        assert hashlib.sha256(clean.read_bytes()).hexdigest() == asset["clean_sha256"]
        doc, binary = read_glb(clean)
        triangles = 0
        for mesh in doc["meshes"]:
            for primitive in mesh["primitives"]:
                assert primitive.get("mode", 4) == 4
                positions = accessor_values(doc, binary, primitive["attributes"]["POSITION"])
                assert positions and all(math.isfinite(v) for p in positions for v in p)
                uv = accessor_values(doc, binary, primitive["attributes"]["TEXCOORD_0"])
                assert len(uv) == len(positions)
                assert all(math.isfinite(v) for p in uv for v in p)
                indices = accessor_values(doc, binary, primitive["indices"])
                assert len(indices) % 3 == 0 and all(0 <= i[0] < len(positions) for i in indices)
                triangles += len(indices) // 3
        assert triangles == asset["triangles"] and triangles <= 1500
        for material in doc["materials"]:
            assert "baseColorTexture" in material["pbrMetallicRoughness"]
        for texture in asset["textures"]:
            png = (HERE / "Textures" / texture["file"]).read_bytes()
            assert png[:8] == b"\x89PNG\r\n\x1a\n"
            width, height = struct.unpack_from(">II", png, 16)
            assert width <= 1024 and height <= 1024
        assert (HERE / "FBX" / (name + ".fbx")).read_bytes().startswith(b"Kaydara FBX Binary")
        results.append(dict(name=name, triangles=triangles, textures=len(asset["textures"]), raw_unchanged=True, status="passed"))
    generation = json.loads((HERE / "generation_manifest.json").read_text(encoding="utf-8"))
    credits = sum(j.get("credits", 0) for j in generation["jobs"].values())
    assert len(results) == 8 and credits == 4
    report = dict(status="passed", assets=results, actual_credits=credits, runtime_imported=False, in_world=False)
    (HERE / "validation.json").write_text(json.dumps(report, indent=2) + "\n", encoding="utf-8")
    print(json.dumps(report, indent=2))


if __name__ == "__main__":
    main()
