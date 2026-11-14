"""Build the devlog deck. Edit SLIDES below (or open the .pptx and edit by hand), then:

    python build_deck.py

Slide kinds: title, bullets, image (picture + caption), two (picture + bullets), todo (placeholder for Dylan).
Needs: pip install python-pptx
Gantt image is read from ../gantt/gantt_slide.png, so re-run gantt/build_gantt.py --slide first if the CSV changed.
"""
from pathlib import Path

from pptx import Presentation
from pptx.dml.color import RGBColor
from pptx.util import Inches, Pt

HERE = Path(__file__).parent
A = HERE / "assets"
GANTT = HERE.parent / "gantt" / "gantt_slide.png"

BG = RGBColor(0x2B, 0x1B, 0x3A)      # grape
INK = RGBColor(0xFF, 0xF4, 0xE0)     # cream
ACCENT = RGBColor(0xFF, 0x7A, 0xB6)  # candy pink
MUTED = RGBColor(0xC9, 0xB8, 0xD9)

SLIDES = [
    dict(kind="title", title="THE DREAD LAND OF CANDY",
         sub="A weekend project made by a director who should know better\nand machines that can't say no\n\nEpic AI MegaJam 2026  |  Devlog, checkpoint 1 (Nov 14, morning)"),

    dict(kind="bullets", title="The pitch that did not survive contact",
         bullets=["Prompt to both AIs: \"Build me an MMO in 4 days, please.\"",
                  "Claude: \"That's completely unrealistic. We'll build something rational.\"",
                  "ChatGPT: \"Maybe we can build a design doc for one in 4 days.\"",
                  "Round two, with a design doc: \"Yeah, uh... how about some of it. Want a closed arena battle?\"",
                  "What shipped instead: a candy-world film shoot with four classes, a nagging faerie and a typed director's note."],
         notes="Source: Dylan's notes at the top of Docs/DEVLOG.md. Add the real video clip here."),

    dict(kind="bullets", title="The game in one slide",
         bullets=["You are a knight who has been cast in a film about a candy land, and you lecture the nuisances (rats, goblins, bugs, almonds) into good behaviour.",
                  "Fenella the faerie films you, narrates (88 voiced lines) and sulks when you refuse her notes.",
                  "Three scenes, takes and retakes, a Production Ledger at the wrap, loot, ranks and three skills.",
                  "At CUT! you can type your own director's note; a Claude Haiku backend rewrites the next shot.",
                  "The budget runs out in the finale: the world goes grey and the bugs become boxes labelled BUG."],
         notes="Design text: Docs/Design/BUILD_SPEC.md and Jokes.md."),

    dict(kind="two", title="Where it started: Day 1, 07:49", image="t01.png",
         bullets=["Grey cylinders, an orange wireframe cone.", "First autoplay bot PASS: pacified=3/3.",
                  "Two misses on the very first screenshot: the camera was too close and tints did nothing."]),

    dict(kind="two", title="Where it was 7 hours later", image="t20-play-14.png",
         bullets=["Whole film plays under a bot: title to Premiere to credits.", "HUD, takes, skills, captions, speech bubbles.",
                  "Still placeholder floors; art pass and rigged knight land that afternoon."]),

    dict(kind="two", title="Where it is now", image="01-candy-world.png",
         bullets=["Open candy world with PCG foliage, compass, follow camera.", "Four playable classes, theme song, biome music, 47 SFX.",
                  "Packaged Win64 build, 689 to 721 MB zip."]),

    dict(kind="image", title="Who built what, hour by hour", image=str(GANTT),
         caption="Gantt of the Claude side, from git commit times (approximate). ChatGPT rows to be added from work_log.csv."),

    dict(kind="bullets", title="The cast of workers",
         bullets=["Dylan: director, taster, approver. Plays every build, owns every \"is this funny\" call.",
                  "Claude Fable 5.2: specs and plans (build spec, release plan, open-world spec).",
                  "Claude Opus 5.5: GAME worker, the only one that opens the editor. Most of Day 1.",
                  "Claude Sonnet 5.5: RULES (ledger, progression, note clamp), BACKEND (director service), checkpoints.",
                  "ChatGPT / Codex: terrain (PCG world), Niagara FX, cast generation and rigging, ElevenLabs audio, itch page, R7 fix batches. [ChatGPT to confirm]"],
         notes="WORKERS.md describes the folder-ownership rules that let them run in parallel."),

    dict(kind="bullets", title="Day 0, Nov 11 (before the jam): rules of engagement",
         bullets=["Tooling only, declared in Docs/Declared.md: build/test/package scripts, mesh generator clients, a GAS test template.",
                  "Fable wrote the build spec and joke sheet in about 55 minutes of commits.",
                  "Decision: headless first. Rules are plain C++ with tests; a bot plays the game.",
                  "Decision: split into workers with owned folders so sessions can run side by side."]),

    dict(kind="bullets", title="Day 1, Nov 12: the vertical slice in a day",
         bullets=["07:49 first hero on screen. 11:23 whole film plays under autoplay.",
                  "By 18:00: HUD, Fenella, CUT! notes, wrap card, title, pause, Premiere, credits, skills, music, art pass, rigged knight, voice hookup, typed notes.",
                  "About 70 commits from Claude in one day.",
                  "Evening: Dylan's open-world spec (3e); terrain goes to ChatGPT."]),

    dict(kind="bullets", title="Day 2, Nov 13: the world, the sound, the cast",
         bullets=["ChatGPT overnight: Sugarborough terrain, dense PCG candy world, Niagara attack pipeline.",
                  "Release plan R0 to R8: checkpoint, sound, theme song, open world, enemies hit back, package.",
                  "47 SFX and 88 Fenella lines imported by one script; theme song (Big Rock Candy Mountain, adjusted) over the credits.",
                  "Four classes playable; class gestures, stowed weapons, brushing foam.",
                  "Evening: first package, 689 MB zip; Dylan plays and writes an 18-item list."]),

    dict(kind="bullets", title="Day 3, Nov 14: test, fix, repeat",
         bullets=["Loop: Dylan plays in the editor, tags each finding BLOCKER / SHIP / NICE, a session fixes a batch, repeat.",
                  "Batch 0 to 2 done: clickable HUD, compass fix, grey-screen bug, camera controls, numeric XP, controls card.",
                  "72 Jam tests pass; Open autoplay passes all three scenes.",
                  "Backend redeployed (prompt n2, live check passes).",
                  "Feature freeze tonight 11 PM PST."]),

    dict(kind="image", title="Review: the wrap card", image="t10-wrap.png",
         caption="Day 1: Production Ledger, loot cards and Fenella's stamp. First screenshot had text running off the card."),

    dict(kind="image", title="Review: the lecture", image="t19-lecture-9.5.png",
         caption="The brand moment: sword up, inhale, a flash shaped like the real hit area, jagged bubbles."),

    dict(kind="image", title="Review: the budget runs out", image="t21-budget-7.png",
         caption="Grey grid, boxes labelled BUG, [SHOUTING] bubbles, music cut dead."),

    dict(kind="image", title="Look pass: before and after", image="look-after3.png",
         caption="Three tries at the grade: lime and glaring, dark olive, then in between. Before shot: assets/look-before.png"),

    dict(kind="bullets", title="By the numbers (Nov 14, 10:00)",
         bullets=["142 commits in 4 days: 113 carry a Claude co-author trailer, 29 do not (ChatGPT / Codex or Dylan).",
                  "About 17,500 lines of C++ in Source/ (headers and sources).",
                  "72 automated Jam.* tests; a bot that plays the whole game in about 2 minutes.",
                  "47 SFX, 88 voiced lines, 18 music tracks, a theme song with Dylan's vocals.",
                  "4 playable classes, 3 scenes, 12 skill icons, 30+ generated models.",
                  "Zip 689 to 721 MB against a 1 GB limit."],
         notes="Recompute with git log if needed; the commands are in README.md."),

    dict(kind="bullets", title="Things that went wrong (the honest slide)",
         bullets=["Printed DefaultEngine.ini on stream, including a local token (not committed). Rule added to CLAUDE.md.",
                  "Three background packages killed for low memory; packaging moved to the foreground.",
                  "A package \"passed\" because piping through tail lost the exit code; the smoke test ran the old exe.",
                  "Unity build clashes (same helper names in two files) failed only the game target.",
                  "An ini value with // is cut at the slashes: typed notes silently fell back to keywords until the URL was quoted.",
                  "A rolling-pin generated with extra handles; rebuilt by hand in Blender.",
                  "Cast gestures reviewed by Dylan as \"inappropriate\": shared knight moves on a barbarian."]),

    dict(kind="bullets", title="What we learned working with two AIs",
         bullets=["A bot that plays the game finds more bugs than a reviewer: stuck behind rats, goblins out of reach, a finale cleared in 7 ms.",
                  "Screenshots are evidence; tests passing is not the same as it being fun.",
                  "Folder ownership and a STATUS hand-over file made thread resets cheap.",
                  "Quota is a schedule: Claude saved for the last day, ChatGPT took the overnight and fix batches.",
                  "[Dylan: add the part where the models disagreed]"]),

    dict(kind="todo", title="Dylan's section: add tomorrow",
         bullets=["Video clip: the MMO prompt exchange", "Your own screenshots and gameplay capture",
                  "What each AI was best and worst at (your opinion)", "The music and vocals story (ACE Studio)",
                  "Playtest reactions", "Submission day"]),

    dict(kind="bullets", title="What is left (Nov 14 to 15)",
         bullets=["Hands-on checks: swapped mouse buttons, camera feel, typed directions, cast visuals, golden rain.",
                  "Verify game HTTPS to the backend.", "One package, Open smoke test, zip under 1 GB, itch tester push on Dylan's go.",
                  "Nov 15: trailer/video, blocker fixes, submit before 5:59 PM PST (target upload 2 PM)."]),
]


def style(tf, size, color=INK, bold=False):
    for p in tf.paragraphs:
        for r in p.runs:
            r.font.size = Pt(size)
            r.font.color.rgb = color
            r.font.bold = bold
            r.font.name = "Segoe UI"


def add_text(slide, left, top, width, height, text, size, color=INK, bold=False):
    tb = slide.shapes.add_textbox(Inches(left), Inches(top), Inches(width), Inches(height))
    tf = tb.text_frame
    tf.word_wrap = True
    tf.text = text
    style(tf, size, color, bold)
    return tb


def add_bullets(slide, left, top, width, height, items, size=20):
    tb = slide.shapes.add_textbox(Inches(left), Inches(top), Inches(width), Inches(height))
    tf = tb.text_frame
    tf.word_wrap = True
    for i, t in enumerate(items):
        p = tf.paragraphs[0] if i == 0 else tf.add_paragraph()
        p.text = "•  " + t
        p.space_after = Pt(10)
    style(tf, size)


def resolve(name):
    p = Path(name)
    return p if p.is_absolute() else A / name


def main():
    prs = Presentation()
    prs.slide_width, prs.slide_height = Inches(13.333), Inches(7.5)
    blank = prs.slide_layouts[6]
    for s in SLIDES:
        sl = prs.slides.add_slide(blank)
        sl.background.fill.solid()
        sl.background.fill.fore_color.rgb = BG
        k = s["kind"]
        if k == "title":
            add_text(sl, 0.8, 2.0, 11.7, 1.5, s["title"], 54, ACCENT, True)
            add_text(sl, 0.8, 3.7, 11.7, 3, s["sub"], 22, MUTED)
        else:
            add_text(sl, 0.6, 0.35, 12.1, 0.9, s["title"], 34, ACCENT, True)
            if k in ("bullets", "todo"):
                add_bullets(sl, 0.7, 1.4, 11.9, 5.5, s["bullets"], 22 if len(s["bullets"]) < 6 else 19)
            elif k == "image":
                pic = resolve(s["image"])
                sl.shapes.add_picture(str(pic), Inches(0.6), Inches(1.3), height=Inches(5.3))
                if pic.name == "gantt_slide.png":  # wide chart: fit to width instead
                    sl.shapes._spTree.remove(sl.shapes[-1]._element)
                    sl.shapes.add_picture(str(pic), Inches(1.4), Inches(1.25), width=Inches(10.4))
                add_text(sl, 0.6, 6.75, 12.1, 0.6, s["caption"], 14, MUTED)
            elif k == "two":
                sl.shapes.add_picture(str(resolve(s["image"])), Inches(0.6), Inches(1.4), width=Inches(7.4))
                add_bullets(sl, 8.3, 1.6, 4.5, 5, s["bullets"], 20)
        if s.get("notes"):
            sl.notes_slide.notes_text_frame.text = s["notes"]
    out = HERE / "TheDreadLandOfCandy_Devlog.pptx"
    prs.save(out)
    print("wrote", out, len(SLIDES), "slides")


if __name__ == "__main__":
    main()
