"""Validate delivered GLBs/FBXs and record geometry and rig structure, without Unreal."""
import json
from pathlib import Path
import struct
import sys

ROOT = Path(__file__).resolve().parents[2]
sys.path.insert(0, str(ROOT / "Scripts"))
import genlib as g


def inspect_glb(path):
    g.validate_glb(path)
    data = path.read_bytes()
    length = struct.unpack_from("<I", data, 12)[0]
    doc = json.loads(data[20:20 + length])
    primitives = [p for mesh in doc.get("meshes", []) for p in mesh["primitives"]]
    triangles = sum(doc["accessors"][p["indices"]]["count"] // 3
                    for p in primitives if "indices" in p and p.get("mode", 4) == 4)
    skins = doc.get("skins", [])
    joints = [doc["nodes"][i].get("name", str(i)) for skin in skins for i in skin["joints"]]
    return {"file": str(path.relative_to(ROOT)), "bytes": len(data), "triangles": triangles,
            "materials": len(doc.get("materials", [])), "images": len(doc.get("images", [])),
            "skins": len(skins), "joints": joints,
            "skinned_primitives": sum("JOINTS_0" in p["attributes"] and "WEIGHTS_0" in p["attributes"]
                                      for p in primitives)}


reports = []
characters = json.loads(Path(__file__).with_name("expansion_characters.json").read_text())["characters"]
for character in characters:
    name = character["name"]
    directory = ROOT / "ImportSource/Meshy" / (name + "_Rig")
    static = inspect_glb(ROOT / "ImportSource/Meshy" / (name + ".glb"))
    rig = inspect_glb(directory / (name + "_Rig.glb"))
    assert rig["skins"] and rig["skinned_primitives"], (name, "No skin weights")
    assert {"pelvis", "hand_l", "hand_r", "foot_l", "foot_r"}.issubset(rig["joints"]), (name, "Missing humanoid bones")
    files = [directory / (name + "_Rig.fbx")]
    files += [directory / ("A_" + name + "_Rig_" + label + ".fbx") for label in ("Idle", "Walk", "BasicWalk")]
    for path in files:
        g.validate_fbx(path)
        assert path.stat().st_size > 10000, (path, "Unexpectedly small FBX")
    reports.append({"character": name, "static": static, "rig": rig,
                    "fbx": [{"file": str(p.relative_to(ROOT)), "bytes": p.stat().st_size} for p in files]})
dest = ROOT / "ImportSource/_gen/expansion_characters/validation.json"
dest.write_text(json.dumps(reports, indent=2), encoding="utf-8")
for record in reports:
    print(record["character"], record["static"]["triangles"], "static triangles;",
          len(record["rig"]["joints"]), "joints;", len(record["fbx"]), "valid FBXs")
print("Saved", dest)
