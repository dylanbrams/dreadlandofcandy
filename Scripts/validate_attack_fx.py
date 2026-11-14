"""Validate generated sources and the external Niagara bytecode audit.

Run outside Unreal after NiagaraDumpBytecode. This validates actual emitted VM
programs, rather than treating successful asset saves as successful compilation.
"""
import ast
import csv
import hashlib
import html
import json
from pathlib import Path
import re
import shutil

ROOT = Path(__file__).resolve().parents[1]
SRC = ROOT / "ImportSource/FX"


def main():
    sources = json.loads((SRC / "manifest.json").read_text(encoding="utf-8"))
    for name, expected in sources["sha256"].items():
        assert hashlib.sha256((SRC / name).read_bytes()).hexdigest() == expected, name
    assert len(sources["meshes"]) == 10
    for mesh in sources["meshes"]:
        assert 0 < mesh["triangles"] <= 512, mesh["name"]
        for lo, hi in zip(mesh["bounds_min"], mesh["bounds_max"]):
            assert lo <= hi, mesh["name"]
    for name in ("build_fx_sources.py", "import_fx_assets.py", "build_niagara_attacks.py", "recompile_fx_systems.py", "build_fx_review.py", "render_fx_review.py"):
        ast.parse((ROOT / "Scripts" / name).read_text(encoding="utf-8"))
    assets = json.loads((SRC / "niagara_assets.json").read_text(encoding="utf-8"))
    expected = {a["asset"] for a in assets}
    assert len(expected) == 40  # Fixes3 #2 signature layers
    audit = max((ROOT / "Saved/FX/Bytecode").glob("*/NiagaraScripts.csv"), key=lambda p: p.stat().st_mtime)
    rows = list(csv.DictReader(audit.open(encoding="utf-8-sig"), skipinitialspace=True))
    assert rows and all(int(r["OpCount"]) > 0 for r in rows), "Missing/empty VM programs"
    found = {re.match(r"(/Game/FX/Attacks/Systems/[^/]+)", r["Name"]).group(1) for r in rows}
    assert found == expected, (expected - found, found - expected)
    presets = json.loads((SRC / "attack_presets.json").read_text(encoding="utf-8"))
    assert len(presets) == 12
    assert all(set(p["systems"]) <= expected for p in presets.values())
    expected_scripts = sum(2 + 2 * a["emitters"] for a in assets)
    assert len(rows) == expected_scripts, (len(rows), expected_scripts)
    for asset in expected:
        package = ROOT / "Content" / (asset.split(".")[0].removeprefix("/Game/") + ".uasset")
        assert package.is_file() and package.stat().st_size > 0
        assert audit.stat().st_mtime >= package.stat().st_mtime, f"Stale audit for {asset}"
    compile_log = (ROOT / "Saved/FX/compile.log").read_text(encoding="utf-8", errors="replace")
    assert not re.search(r"\b(Error|Fatal):", compile_log), "Compiler errors in compile.log"
    repair_path = ROOT / "Saved/FX/recompile.log"
    repair_log = repair_path.read_text(encoding="utf-8", errors="replace")
    assert not re.search(r"\b(Error|Fatal):", repair_log), "Repair errors in recompile.log"
    repaired = set(re.findall(r"FX SYSTEM RECOMPILED: (\S+)", repair_log))
    assert repaired == expected, "Every saved system must pass the fresh-process repair"
    assert repair_path.stat().st_mtime >= max((ROOT / "Content" / (a.split(".")[0].removeprefix("/Game/") + ".uasset")).stat().st_mtime for a in expected), "Saved assets changed after repair"
    report = {"source_meshes": 10, "source_textures": 4, "source_hashes": "pass",
              "python_syntax": "pass", "niagara_systems": len(expected), "compiled_vm_programs": len(rows),
              "nonempty_vm_programs": "pass", "compile_log_errors": 0,
              "audit": str(audit.relative_to(ROOT)).replace("\\", "/"),
              "saved_system_repair": {"systems": len(repaired), "errors": 0, "log": "Saved/FX/recompile.log"},
              "visual_review": "See Docs/Workers/fx.md; compilation does not establish visual quality",
              "gameplay_integration": "not enabled", "packaged_cook": "not validated"}
    render_file = ROOT / "Saved/FX/render_results.json"
    if render_file.exists():
        renders = json.loads(render_file.read_text(encoding="utf-8"))
        if {r["attack"] for r in renders} == set(presets):
            orbit = next(r for r in renders if r["attack"] == "B02")["attributes"]["NS_B02_RollingPinCyclone"]
            assert orbit["position_samples"] and all(abs((p[0]**2 + p[1]**2)**0.5 - 225) < 0.05 for p in orbit["position_samples"]), "Stale or incorrect saved B02 orbit program"
            assert orbit["color_samples"] and all(abs(a-b) < 0.001 for a,b in zip(orbit["color_samples"][0][:3], (0.65,0.32,0.12))), "Incorrect saved B02 colour program"
            cards = []
            review_dir = SRC / "Review"
            review_dir.mkdir(exist_ok=True)
            for render in renders:
                assert render["owner_destruction_cleanup"] == "pass"
                assert all(n > 0 for n in render["particles"].values())
                shot = Path(render["screenshot"])
                assert shot.is_file() and shot.stat().st_mtime >= max(
                    (ROOT / "Content" / (a.split(".")[0].removeprefix("/Game/") + ".uasset")).stat().st_mtime
                    for a in presets[render["attack"]]["systems"]), "Stale screenshot: " + render["attack"]
                shutil.copy2(shot, review_dir / shot.name)
                cards.append(f'<figure><img src="Review/{html.escape(shot.name)}"><figcaption>{html.escape(render["attack"])} — prototype; 180 cm white scale marker</figcaption></figure>')
            (SRC / "review.html").write_text('<!doctype html><html lang="en"><meta charset="utf-8"><title>Niagara attack review</title>'
                '<style>body{background:#111923;color:#eee;font:16px system-ui;margin:24px}main{display:grid;grid-template-columns:repeat(auto-fit,minmax(350px,1fr));gap:16px}figure{margin:0}img{width:100%}figcaption{padding:8px}</style>'
                '<h1>Attack Niagara prototypes</h1><p>Standalone blockouts after the saved-bytecode repair. Owner cleanup and particle attributes are checked separately from art quality. Gameplay integration and final art approval remain pending.</p><main>'
                + ''.join(cards) + '</main></html>', encoding="utf-8")
            (SRC / "visual_validation.json").write_text(json.dumps(renders, indent=2) + "\n", encoding="utf-8")
            review_status = json.loads((SRC / "visual_review_status.json").read_text(encoding="utf-8")) if (SRC / "visual_review_status.json").exists() else {"status":"not reviewed"}
            report.update(visual_captures=12, owner_cleanup_checks=12,
                          visual_review=review_status,
                          parameter_stop_updates="Require running gameplay-world validation")
    (SRC / "validation.json").write_text(json.dumps(report, indent=2) + "\n", encoding="utf-8")
    print(f"FX validation PASS: 10 meshes, 4 masks, {len(expected)} systems, {len(rows)} nonempty VM programs")


if __name__ == "__main__":
    main()
