"""Group the raw takes into raw/by-section/<section>/<take-id>.mp4 (hard links, no extra disk).

    python -I label_takes.py

The mapping comes from NARRATIVE.md's take map. Re-run after editing TAKES.
"""
import os
from pathlib import Path

HERE = Path(__file__).parent
SRC = HERE / "raw" / "original"
DST = HERE / "raw" / "by-section"

# section folder -> take ids (clock times)
TAKES = {
    "00_coldopen": ["12-16-09", "12-16-38", "12-16-58"],
    "01_idea": ["12-17-20", "12-17-35", "12-18-01", "12-18-42", "12-18-57", "12-20-17", "12-24-53"],
    "02_hook": ["13-08-18", "13-08-28", "13-08-34", "13-08-46", "13-09-00", "13-09-07", "13-09-43", "13-10-07",
                "13-10-34", "13-11-11", "13-11-24", "13-12-05", "13-12-35"],
    "03_setup": ["12-27-32", "13-13-39", "12-30-44"],
    "03b_gantt": ["13-54-05", "13-57-31", "13-58-12", "13-58-41"],
    "04_dayone": ["12-34-50", "12-35-10", "12-35-30", "12-36-41"],
    "04b_audition": ["12-38-12", "12-39-11", "13-16-35"],
    "05_world": ["12-40-31", "13-17-38"],
    "07_longnight": ["13-18-57", "13-20-06", "13-20-55"],
    "08_sunday": ["13-22-00", "13-22-27", "13-24-31"],
    "08b_river": ["13-25-32", "13-25-53", "13-29-20"],
    "09_why": ["13-30-15"],
    "11_postcredits": ["13-35-40"],
    "_offscript": ["12-25-56", "12-29-51"],
    "_falsestart": ["12-27-22", "12-29-14", "12-29-39", "12-37-51", "12-40-18", "13-17-24", "13-30-08", "13-58-34"],
    "_keyplate": ["12-15-53", "12-16-00", "13-18-49"],
}

def main():
    n = 0
    seen = set()
    for sec, ids in TAKES.items():
        d = DST / sec
        d.mkdir(parents=True, exist_ok=True)
        for i in ids:
            src = SRC / f"2026-11-15 {i}.mp4"
            if not src.exists():
                print("MISSING", i); continue
            dst = d / f"{i}.mp4"
            if not dst.exists():
                os.link(src, dst)
            seen.add(i); n += 1
    allids = {p.stem.replace("2026-11-15 ", "") for p in SRC.glob("*.mp4")}
    print(f"{n} linked; unassigned: {sorted(allids - seen)}")

main()
