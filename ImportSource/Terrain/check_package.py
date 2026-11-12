"""Create a local validation archive and measure the real submission-size budget.

No upload. Excludes debug symbols and runtime Saved directories, as a clean
delivery should. The normal jam submission script remains GAME's responsibility.
"""
import json
from pathlib import Path
import zipfile

ROOT = Path(__file__).resolve().parents[2]
BUILD = ROOT / "Builds/Win64-Development"
OUT = ROOT / "Builds/TerrainValidation.zip"
files = sorted(p for p in BUILD.rglob("*") if p.is_file() and p.suffix.lower() != ".pdb" and "Saved" not in p.relative_to(BUILD).parts)
if not (BUILD / "MegaJam.exe").is_file() or not files:
    raise RuntimeError("No packaged game to validate")
temporary = OUT.with_suffix(".zip.part")
with zipfile.ZipFile(temporary, "w", compression=zipfile.ZIP_DEFLATED, compresslevel=6) as archive:
    for path in files:
        archive.write(path, path.relative_to(BUILD).as_posix())
temporary.replace(OUT)
with zipfile.ZipFile(OUT) as archive:
    bad_file = archive.testzip()
if bad_file is not None:
    raise RuntimeError("Archive CRC failed: " + bad_file)
report = dict(archive=str(OUT.relative_to(ROOT)), file_count=len(files), uncompressed_bytes=sum(p.stat().st_size for p in files), archive_bytes=OUT.stat().st_size, limit_bytes=1_000_000_000, under_limit=OUT.stat().st_size < 1_000_000_000, excluded=["*.pdb", "runtime Saved directories"], crc_verified=True, uploaded=False)
(ROOT / "ImportSource/Terrain/package_size.json").write_text(json.dumps(report, indent=2) + "\n", encoding="utf-8")
print(json.dumps(report, indent=2))
if not report["under_limit"]:
    raise SystemExit("Validation archive exceeds the jam size limit")
