"""Record the dense delivery checkpoint from the completed verification runs."""
import json
import re
from pathlib import Path

HERE = Path(__file__).resolve().parent
ROOT = HERE.parents[2]
PARENT = HERE.parent

def save(path, value):
    path.write_text(json.dumps(value, indent=2) + "\n", encoding="utf-8")

report = json.loads((PARENT / "delivery_report.json").read_text(encoding="utf-8"))
report.update(verified_date_utc="2026-11-13", automation={"passed": 57, "failed": 0})
report["editor_physics"].pop("nonblocking_scatter", None)
report["editor_physics"].update(nonblocking_instances=86003, instanced_components=21)
report["dense_ecology"] = json.loads((HERE / "engine_report.json").read_text(encoding="utf-8"))
report["inspected_packaged_captures"] = ["Dense/Review/packaged-grove.png", "Dense/Review/packaged-snow.png", "Dense/Review/packaged-explorer.png", "Dense/Review/packaged-main.png"]
report["packaged_explorer_startup"] = "passed; native DefaultPawn; manual traversal not claimed"
size = json.loads((PARENT / "package_size.json").read_text(encoding="utf-8"))
report["validation_archive"].update(bytes=size["archive_bytes"], crc_verified=size["crc_verified"])
report["remaining"] = ["O1 is merged. O2 follow camera and remaining O2-O9 framework belong to GAME.", "Open quest autoplay awaits O4 and subsequent GAME work.", "Human traversal, performance and final follow-camera readability remain to be reviewed."]
# Only mark the regression from the current package's actual log.
smoke_bytes = (ROOT / "Saved/Logs/DenseEcology-Backlot-Smoke.log").read_bytes()
smoke = smoke_bytes.decode("utf-16" if smoke_bytes.startswith(b"\xff\xfe") else "utf-8-sig")
assert "Packaged build looks good." in smoke and smoke.count("[PASS]") == 7 and "[FAIL]" not in smoke, smoke[-1800:]
game_log = (ROOT / "Saved/Automation/Packaged-Smoke.log").read_text(encoding="utf-8-sig")
autoplay = re.search(r"JamAutoplay: (PASS[^\r\n]+)", game_log)
assert autoplay
report["backlot_regression"]["autoplay"] = autoplay.group(1)
report["backlot_regression"]["log"] = "Saved/Logs/DenseEcology-Backlot-Smoke.log"
save(PARENT / "delivery_report.json", report)

level = json.loads((PARENT / "level_report.json").read_text(encoding="utf-8"))
level["base_pass_placements"] = level.pop("baked_placements", 581)
level.update(dense_baked_instances=86003, dense_instanced_components=21)
graph = "/Game/World/Dense/PCG/PCG_CandyEcology"
if graph not in level["pcg_graphs"]:
    level["pcg_graphs"].append(graph)
level["source_validation"]["scatter_count"] = 86003
level["packaged_validation"] = "passed dense grove/snow/explorer rendering and native Open terrain startup; O1 merged, follow camera/quests await GAME"
save(PARENT / "level_report.json", level)

readme = PARENT / "README.md"
text = readme.read_text(encoding="utf-8")
text = text.replace("# Sugarborough terrain sources\n", "# Sugarborough terrain sources\n\nCurrent delivery (Nov 13): see [Dense/README.md](Dense/README.md) and\n[Dense/HANDOVER.md](Dense/HANDOVER.md). The dense PCG pass adds 86,003 baked\ninstances, seven imported Rodin models, grass wind and sugar snow. Windows\npackaging, 57 tests and packaged visual checks pass; the CRC-verified archive\nis 613,096,338 bytes. O1 is merged; follow camera and quests remain GAME work.\nThe sections below document the original sparse pass.\n")
text = text.replace("GAME's O1 framework remains on its unmerged branch", "O1 is now merged; O2 follow camera and O4 quests remain pending")
readme.write_text(text, encoding="utf-8")

handover = HERE / "HANDOVER.md"
text = handover.read_text(encoding="utf-8")
start = text.index("## Pending at this handover checkpoint")
end = text.index("## Reproduction and known API traps")
text = text[:start] + """## Verified delivery checkpoint - Nov 13

The resumed snow capture and normal Windows package succeeded. Actual packaged
Grove, Snow and Explorer captures were inspected; native Open startup reports
the terrain map and all 19 locations. Current camera in native Open mode is
still the old fixed camera: GAME O2 must replace it. This is not a passing
Open quest loop or manual playthrough.

Packaged Backlot's 70-second smoke passes all seven checks with three-scene
autoplay PASS. Archive `Builds/TerrainValidation.zip`: **613,096,338 bytes**,
CRC verified, below 1 GB. Nothing uploaded or pushed. Current evidence is in
`Review/packaged-grove.png`, `packaged-snow.png`, `packaged-explorer.png` and
`packaged-main.png`. Parent `delivery_report.json` records checks and limits.

TERRAIN releases the engine after this checkpoint. Read worker notes before
any new launch. A fresh context can start with: "Continue TERRAIN from
ImportSource/Terrain/Dense/HANDOVER.md. Review the packaged grove and snow
captures, then improve traversal/readability with the current GAME framework."
Next meaningful work is human visual/traversal feedback, density tuning and
validation with GAME's follow camera once O2 lands. Preserve village, road,
camp and film-set exclusions. Never claim current native quests exist.

""" + text[end:]
handover.write_text(text, encoding="utf-8")
