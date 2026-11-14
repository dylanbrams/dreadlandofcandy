# Devlog outline

Working outline for the post-jam devlog and video. `[ ]` = still to write or capture. Slide numbers refer to `deck/TheDreadLandOfCandy_Devlog.pptx`.

1. **Cold open: the MMO prompt** (slide 2) [ ] video clip of both exchanges
2. **The game in 60 seconds** (slides 3, 6) [ ] gameplay capture
3. **The team: one director, three Claudes, ChatGPT** (slide 8)
   - worker split and folder ownership (`Docs/Design/WORKERS.md`)
   - [ ] Dylan: how you divided work between the two
4. **Day 0: setting the rules** (slide 9): declared tooling, headless-first, bots
5. **Day 1: a vertical slice in a day** (slides 4, 5, 10)
   - before/after screenshots; bot milestones (07:49 first PASS, 11:23 whole film)
   - anecdotes: camera too close, rats the bot could not catch, the 7 ms finale
6. **Day 2: the world, the sound, the cast** (slide 11): terrain by ChatGPT, audio, release plan, first package
7. **Day 3: test, fix, repeat** (slide 12): BLOCKER / SHIP / NICE loop, grey-screen bug, backend redeploy
8. **Feature deep-dives** (slides 13 to 16), pick two or three:
   - the lecture presentation; the budget running out; the typed director's note and backend; the look grade
   - [ ] music and vocals (ACE Studio)
9. **The Gantt: who built what** (slide 7) [ ] add ChatGPT rows
10. **By the numbers** (slide 17)
11. **What went wrong** (slide 18)
12. **What we learned about working with two AIs** (slide 19) [ ] Dylan's own take: where they disagreed
13. **Submission day and what's next** (slide 21) [ ] final results

## Sources for each section

| Section | Where to look |
| --- | --- |
| Day-by-day detail | `Docs/DEVLOG.md` (GAME entries), `git log --format='%h %ad %s' --date=format:'%m-%d %H:%M'` |
| Design intent | `Docs/Design/BUILD_SPEC.md`, `RELEASE_PLAN.md`, `Jokes.md` |
| Tools and AI credits | `Docs/Declared.md` |
| Per-worker detail | `Docs/Workers/*.md` |
| Screenshots | `Docs/Devlog/Screenshots/` (109), `Marketing/Itch/screenshots/`, `Saved/Screenshots/` |
| Dylan's feedback lists | `Docs/Feedback/`, `Docs/STATUS.md`, `Docs/Archive/STATUS-before-batch0-2026-11-14.md` |
