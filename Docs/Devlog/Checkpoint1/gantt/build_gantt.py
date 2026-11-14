"""Build the Gantt chart from work_log.csv.

    python build_gantt.py                      # Claude + ChatGPT rows in work_log.csv
    python build_gantt.py --slide              # compact chart used by the deck
    python build_gantt.py --candidates         # also draw chatgpt_candidates.csv (hatched = unconfirmed)

Add a row to work_log.csv (agent = Claude or ChatGPT, times as "YYYY-MM-DD HH:MM") and re-run.
Output: gantt.png, gantt.svg, gantt_legend.md next to this script.
Needs: pip install matplotlib
"""
import argparse
import csv
from collections import OrderedDict
from datetime import datetime
from pathlib import Path

import matplotlib
matplotlib.use("Agg")
import matplotlib.pyplot as plt
from matplotlib.patches import Patch

HERE = Path(__file__).parent
COLORS = {"Claude": "#D97757", "ChatGPT": "#10A37F", "Claude?": "#D97757", "ChatGPT?": "#10A37F"}
LANE_ORDER = ["Design", "Setup", "Assets", "Terrain", "FX", "Audio", "Art", "Rules", "Backend", "Game", "Docs"]
FMT = "%Y-%m-%d %H:%M"


def load(path):
    with open(path, newline="", encoding="utf-8") as f:
        return [r for r in csv.DictReader(f)]


def hours(dt):
    return dt.hour + dt.minute / 60


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument("--candidates", action="store_true")
    ap.add_argument("--slide", action="store_true", help="short 16:9-friendly chart -> gantt_slide.png")
    args = ap.parse_args()

    rows = load(HERE / "work_log.csv")
    if args.candidates:
        rows += load(HERE / "chatgpt_candidates.csv")
    for r in rows:
        r["s"], r["e"] = datetime.strptime(r["start"], FMT), datetime.strptime(r["end"], FMT)

    lanes = [l for l in LANE_ORDER if any(r["lane"] == l for r in rows)]
    lanes += sorted({r["lane"] for r in rows} - set(lanes))
    days = sorted({r["s"].date() for r in rows} | {r["e"].date() for r in rows})
    day_lanes = {d: [l for l in lanes if any(r["lane"] == l and d in (r["s"].date(), r["e"].date()) for r in rows)] for d in days}

    fig, axes = plt.subplots(len(days), 1, figsize=(16, (2.0 if args.slide else 3.1) * len(days)), gridspec_kw={"height_ratios": [len(day_lanes[d]) + 1.2 for d in days]})
    if len(days) == 1:
        axes = [axes]
    for ax, day in zip(axes, days):
        ypos = {l: i for i, l in enumerate(day_lanes[day])}
        for r in rows:
            for d in {r["s"].date(), r["e"].date()}:
                if d != day:
                    continue
                s = hours(r["s"]) if r["s"].date() == day else 0
                e = hours(r["e"]) if r["e"].date() == day else 24
                width = max(e - s, 0.12)
                unconfirmed = r["confidence"] == "unconfirmed"
                ax.barh(ypos[r["lane"]], width, left=s, height=0.62,
                        color=COLORS.get(r["agent"], "#888"), alpha=0.45 if unconfirmed else 0.95,
                        hatch="//" if unconfirmed else None, edgecolor="white", linewidth=0.6)
                ax.text(s + width / 2, ypos[r["lane"]], r["id"], ha="center", va="center",
                        fontsize=6.5, color="#222" if unconfirmed else "white")
        ax.set_xlim(0, 24)
        ax.set_xticks(range(0, 25, 2))
        ax.set_xticklabels([f"{h:02d}:00" for h in range(0, 25, 2)], fontsize=8)
        ax.set_yticks(range(len(ypos)))
        ax.set_yticklabels(day_lanes[day], fontsize=9)
        ax.set_ylim(len(ypos) - 0.5, -0.5)
        ax.grid(axis="x", alpha=0.25)
        ax.set_title(day.strftime("%a %b %d").replace(" 0", " "), loc="left", fontsize=11, fontweight="bold")
        for sp in ("top", "right"):
            ax.spines[sp].set_visible(False)

    handles = [Patch(color=COLORS["Claude"], label="Claude (Fable / Opus / Sonnet)"),
               Patch(color=COLORS["ChatGPT"], label="ChatGPT / Codex")]
    if args.candidates:
        handles.append(Patch(facecolor=COLORS["ChatGPT"], alpha=0.45, hatch="//", label="Unconfirmed (no Claude trailer in git)"))
    fig.legend(handles=handles, loc="upper right", ncol=len(handles), fontsize=9, frameon=False)
    fig.suptitle("The Dread Land of Candy: who built what, hour by hour (times from commit stamps; bars are approximate)",
                 x=0.01, ha="left", fontsize=12, fontweight="bold")
    fig.tight_layout(rect=(0, 0, 1, 0.96))
    if args.slide:
        fig.savefig(HERE / "gantt_slide.png", dpi=150)
        return
    fig.savefig(HERE / "gantt.png", dpi=150)
    fig.savefig(HERE / "gantt.svg")

    with open(HERE / "gantt_legend.md", "w", encoding="utf-8") as f:
        f.write("| Id | Agent | Lane | Task | Start | End | Confidence |\n|---|---|---|---|---|---|---|\n")
        for r in sorted(rows, key=lambda r: r["s"]):
            f.write(f"| {r['id']} | {r['agent']} ({r['model']}) | {r['lane']} | {r['task']} | {r['start']} | {r['end']} | {r['confidence']} |\n")
    print("wrote gantt.png, gantt.svg, gantt_legend.md:", len(rows), "rows")


if __name__ == "__main__":
    main()
