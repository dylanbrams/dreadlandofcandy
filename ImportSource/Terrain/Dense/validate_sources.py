"""Validate layered placement exclusions, finite transforms and source budgets."""
import importlib.util
import json
import math
from pathlib import Path

HERE = Path(__file__).resolve().parent
spec = importlib.util.spec_from_file_location("ecology_source", HERE / "build_ecology.py")
e = importlib.util.module_from_spec(spec)
spec.loader.exec_module(e)
data = json.loads((HERE / "ecology.json").read_text())
counts = {}
for group in data["groups"]:
    layer = group["layer"]
    counts[layer] = counts.get(layer, 0) + len(group["instances"])
    assert "Willow" not in group["mesh"]
    assert (HERE / "FBX" / (group["mesh"] + ".fbx")).exists() or (HERE.parent / "Foliage/FBX" / (group["mesh"] + ".fbx")).exists()
    for x, y, z, yaw, scale in group["instances"]:
        assert all(math.isfinite(v) for v in (x, y, z, yaw, scale)) and scale > 0
        assert e.t.biome(x, y) == group["biome"]
        if layer == "snow":
            assert group["biome"] == "Molars"
            continue
        radius = 32 if layer == "grass" else 300 * scale if layer == "tree" else 160 if layer == "bush" else 90
        # Transform serialization rounds centimetres and scales. Allow 1 cm at
        # exclusion boundaries; the authored safety margins are 100 cm.
        assert e.allowed(x, y, max(0, radius - 1), layer), (group["mesh"], x, y)
for record in data["meshes"]:
    assert record["triangles"] <= 1500
assert counts == data["counts"]
report = dict(status="passed", layer_counts=counts, total_instances=sum(counts.values()), source_groups=len(data["groups"]), finite_transforms=True, location_and_road_exclusions=True, excluded_willow=True)
(HERE / "source_validation.json").write_text(json.dumps(report, indent=2) + "\n", encoding="utf-8")
print(json.dumps(report, indent=2))
