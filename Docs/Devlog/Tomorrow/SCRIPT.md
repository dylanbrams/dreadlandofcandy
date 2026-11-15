# The Dread Land of Candy — devlog script, recording draft

Read naturally; slide prompts are cues. Section 0 is a screen recording of `coldopen.html` with your voice over it. Each other section is a **take** you can record on its own: it opens with a music bed and ends on a clean beat, so the takes can be cut together in any order. Sections 1–6 come from ChatGPT's Nov 15 01:45 draft, lightly tightened. Section 7 covers the long night, and sections 8 and 8b reconstruct Sunday morning, 05:00–11:00 project time, from git and `Docs/DEVLOG.md`. The ending is still open. About 9–11 minutes in all.

**Music.** Everything in `music/` is the game's own score, generated in ACE Studio from `music/RPG-music-prompts.md`, which asked for epic music played completely straight: the comedy comes from heroic orchestras over rats and candy. Put the beds below under your voice at roughly −18 to −22 dB, and bring them up between takes. They are the mp3 masters, so nothing needs ripping from the game. **Do not use `theme-song.mp3` anywhere; the theme song is cut from the game, the video and the trailer.** The one exception is the a cappella blooper in section 11.

| Take | Bed | Why |
| --- | --- | --- |
| 0 Prompt screen | None: typing sounds and your voice only | Silence makes the typing land |
| 1 Unreasonable idea | `music/title-theme_take1.mp3`, hitting on the first word after the prompt screen | The title music, utterly sincere |
| 2 Hook montage | `music/standard-combat_take1.mp3`, ducked under voice, up in the gaps | Energy for the features |
| 3 Collaboration | `music/setting-out_take1.mp3` | The journey begins |
| 3b Reading the chart | `music/setting-out_take1.mp3` continued, low | |
| 4 Day one | `music/quest-accepted_take1.mp3` | |
| 4b Asset audition | `music/mystery_take1.mp3` | Three rats, one choice |
| 5 World gets bigger | `music/open-world-exploration_take1.mp3` | |
| 6 Playtest | `music/standard-combat_take2.mp3` | |
| 7 The long night | `music/hard-combat_take1.mp3` | |
| 8 Sunday morning | `music/village_take2.mp3`; `music/feeding-candy_take1.mp3` under the marshmallows | The candy world gets a coat of paint |
| 8b "The river is terrible" | `music/boss-battle_take1.mp3` | A bug list scored as a boss fight |
| 9 Why do this | `music/emotional-revelation_take1.mp3` | The sincere bit |
| 10 Ending | `music/victory_take1.mp3` under the last words, then `music/credits_take1.mp3` | |

**Picture: you're on screen about a quarter of the time.** Shoot every take on the green screen, reading the whole section straight through. In the edit, your voice runs continuously while the picture cuts away from your face. Three framings:

- **CAM**: you, full frame or keyed over a blurred game shot. Use it for openers, jokes you deliver, and the personal beats.
- **PIP**: you keyed small in a bottom corner over game footage or a slide, about a quarter of the frame height.
- **B-ROLL**: game footage, slides or screenshots only, with your voice over them.

Rule of thumb: stay on CAM for no more than 6–8 seconds, then cut away for at least 10. Cut on the start of a sentence, not mid-word.

| Take | CAM | Cut away to (PIP or B-ROLL) |
| --- | --- | --- |
| 0 Prompt screen | None: screen only | The prompt screen full frame |
| 1 Unreasonable idea | "That's pretty much the story of the last four days"; "my sense of reasonable scope may be a little compromised" | B-ROLL: Molar Hills pan with the "older than them thar hills" caption under the whole MMO list |
| 2 Hook montage | "So what did I end up with?"; "The AI is on a leash"; "You're trying to get a usable take" | B-ROLL with game audio up after every beat: cards and battle cries, Resolve and pacified rat, improvise screen, typed proposal, encores, CUT!, typed note, budget boxes |
| 3 Setup | First sentence | B-ROLL: checkpoint slides |
| 3b Reading the chart | "A warning before anyone reads it like a leaderboard"; "what's not on the chart is me" | PIP over the Gantt the whole time, with highlights on the bars named |
| 4 Day one | None | B-ROLL throughout: first-hero and autoplay captures, early screenshots |
| 4b Asset audition | "I picked the Rodin rat" (react to it) | B-ROLL: rat comparison card, then the rat in game |
| 5 World bigger | "And then I changed the plan" | B-ROLL: the world and the village; PIP over the quest givers |
| 6 Playtest | Your playtest surprise | B-ROLL: compass, HUD and controls captures |
| 7 Long night | None | B-ROLL: attack effects in motion, a village encore |
| 8 Sunday morning | "Instead, three things happened at once"; "Nobody needed this" | B-ROLL: PBR contact sheet → village from above → marshmallow sheet → marshmallows drifting → UI concepts → Black Sugar title |
| 8b River | Reading the Fixes4 quotes aloud (PIP over the text on screen) | B-ROLL: river before/after, camera lifting over a canopy |
| 9 Why do this | Most of it; this is the personal one. "The real limit" on CAM | B-ROLL under the draft list (one shot per item); river-before under "the river is terrible" |
| 10 Ending | Your closing words | B-ROLL: title screen, itch page |
| 11 Post-credits | Both lines, deadpan | Black screen for the clip |

**Green screen: yes, keep it.** It lets you be keyed in small (PIP) over the game, which is what makes the regular cutaways feel natural instead of jumpy. Tips:
- Light the screen evenly and separately from yourself, and stand 1–2 m in front of it to avoid green spill.
- **Don't wear mint or green.** Half this game is mint, and so is the screen.
- Lock focus, exposure and white balance.
- Record a few seconds of the empty screen first, to help the key.

If the key comes out ragged (uneven light, hair edges), drop the PIP framing, shoot in front of something plain, and use straight cuts between CAM and B-ROLL instead.

## 0. Cold open — the prompt screen

*Screen-record `coldopen.html` full screen (open it in a browser, press F11; Z zooms the text; H hides the key hint before you record). Press Space to advance each beat; the page synthesizes keyboard clicks while the prompts type. No music. If you'd rather use real typing sounds, press M to mute and record your keyboard separately.*

**Your voice over it.** Say each prompt as it types, a little ahead of the text, flat and hopeful: "Build me an MMO in four days... please." Then let the response type out in silence; the reader gets the joke from the screen. On the ChatGPT beat, say the same prompt again, slightly more tired. Say nothing over the design-doc scroll. On "Here's a design doc and schedule for an MMO, let's build it," sound confident.

| Beat | Screen | You say |
| --- | --- | --- |
| 1 | Claude: "Build me an MMO in 4 days, please." → "That's completely unrealistic. Don't waste my time, we'll build something rational." | "Build me an MMO in four days... please." |
| 2 | ChatGPT: same prompt → "That's completely unrealistic, maybe we can build a design doc for one in 4 days." | The same line, more tired |
| 3 | The real master brief's section headings scroll by; "~40,000 tokens" | Nothing |
| 4 | Claude: "Here's a design doc and schedule for an MMO, let's build it." → "Yeah, uh.... How about some of it. Want a closed arena battle?" | The prompt, confidently |
| 5 | Card: "Then I scaled it back up." | Nothing, or read the card |
| 6–10 | Claude: open world, four classes, quest givers ("We cut those." / "Uncut them."), the hot chocolate river with marshmallows | Each prompt, faster and faster |
| 11 | Card: "Repeat for four days." | Nothing. Hard cut to section 1 and the music hits |

Beats 1, 2 and 4 are your own wording from the top of `Docs/DEVLOG.md`, which are comic exaggerations, as you intended. Beats 6–10 are true to the records: the arena moved into an open world, four classes, the quest givers were cut in the release plan and you restored them on Nov 14, and the river with marshmallows came on Nov 14–15. Edit the `beats` list at the top of the page's script to change any wording.

## 1. An unreasonable idea

[On CAM, straight after the cut:] That's pretty much the story of the last four days. I ask Claude for something huge. ChatGPT specs it out. Claude chops it down to size. Then I scale it back up.

I wanted to make a game in a game jam. Somewhere along the way the conversation started sounding suspiciously like I wanted to make an MMO. In a few days. With AI collaborators.

I have a history with MMOs. I started on MUDs, Ancient Anguish and TsunamiMUD, back when the graphics were your imagination. Then City of Heroes, Star Wars Galaxies, EVE Online, and much later Path of Exile one and two. I skipped Ultima Online and the big World of Warcraft wave and only dabbled later. I've tried plenty of the modern ones on Steam (Aion, Guild Wars, the Warhammer one, MechWarrior Online, and so on) without ever settling in.

So my sense of reasonable scope may be a little compromised.

[On screen, in the background while you talk: a slow pan across the Molar Hills with the caption **"I am older than them thar hills."** Don't mention it; let it sit there.]

## 2. What's actually in it — the hook montage

*Fast. Each beat is one sentence on CAM or PIP, then 3–5 seconds of the thing itself with the game audio up. Bed: `music/standard-combat_take1.mp3`, ducked under your voice and swelling in the gaps. About 75–90 seconds.*

So what did I end up with? Here's what's actually in it.

**The attacks talk back.** Every attack throws comic word cards into the world, in several languages, and the hero shouts battle cries in a hundred and fifty-four translations. The first time you use a skill, it's English. After that, you never know what you'll get.

[B-ROLL: a signature attack with the cards tumbling out; then the same skill again with a non-English cry. Game audio up.]

**Nothing dies.** You lecture, you feed things celery, you brush their teeth. Enemies don't have health, they have Resolve. Wear it down and they sit there, chastened.

[B-ROLL: knight placards on a group, a Resolve bar emptying, a pacified rat. Fenella: "That rat just earned a supporting credit."]

**The quests can be rewritten while you play.** Every villager has authored jobs. But you can also hit "Ask Fenella to improvise," and an AI writes two new ways to do the job, which really change what you do. Lecture the rats becomes pacify the rats, then feed them celery.

[B-ROLL: the offer screen, the improvise button, the two new routes appearing, then playing one.]

**Or write your own.** Type how you'd handle it, "bribe them with snacks so they go home," and the director turns it into a playable route. Or refuses, in character, if it's nonsense.

[B-ROLL: typing a proposal, the route appearing; then a refusal for something silly.]

**The AI is on a leash.** It only gets to choose from pieces we built and tested. It can't break the game, and if the server's down, you just get the authored version.

[CAM: one sentence. This is the line that answers "won't the AI just break it?"]

**And the villagers keep coming back.** Two to four encore jobs each, with twists: a time limit, a crowd, rats that fight back. Then a goodbye.

[B-ROLL: a giver's encore offer, a Rush timer, a Crowd swarm.]

**The whole thing is a film shoot.** Quests are scenes. Fenella, your faerie director, yells CUT! and pitches rewrites. Takes fail and get retaken. You can type director's notes mid-scene, "make the goblins enormous and ashamed," and the scene changes. And when the budget runs out, the rats are replaced by cardboard boxes.

[B-ROLL: Fenella's CUT! pitch ("Test audiences wanted more rats. I asked two rats."), a typed note changing a scene, then the budget-out boxes. Hold on the boxes. Fenella: "Pretend the box is a rat. I believe in you. I have to."]

[CAM:] You're not saving the day. You're trying to get a usable take.

*Capture these from a build with the backend live: the improvise, proposal, director's-note and premiere beats call Claude Haiku on the server, and offline they fall back to authored content. Check the backend is up (`readyz`) before you record.*

## 3. The setup

I directed and playtested. Claude and ChatGPT worked across planning, code, art, terrain, audio and integration, with a pile of generation tools in the mix. There were handoffs, parallel work, and a lot of checking whether the thing that looked finished actually worked in the game.

Some setup and design work existed before the jam and is declared. A caution about the chart: a commit records a snapshot, not how long somebody worked. The bars are estimates.

[Gantt; point out events and overlap, not a leaderboard.]

## 3b. Reading the chart — who built what, hour by hour

*A pickup take, recorded separately (`03b`), so the section 3 you already recorded still stands. Picture: the Gantt (`../Checkpoint1/gantt/gantt.png`) almost full screen, with you in PIP. Slow pan down one day at a time; when you name a bar, highlight it (a box or a dim on everything else). Bed: continue `music/setting-out_take1.mp3`, low.*

This is the whole jam on one chart. Each row is a kind of work: design, assets, terrain, art, backend, game code, docs. Each bar is a stretch of work, orange for Claude, green for ChatGPT and Codex. The times come from git: two hundred and twenty-nine commits over five days.

[Highlight the colour key, then one bar of each colour.]

A warning before anyone reads it like a leaderboard. A commit tells you when something was finished, not when it started, so the start of most bars is an estimate. And where a commit doesn't say which AI made it, I've marked it as a best guess.

[Point at a hatched or approximate bar, or put a caption on screen: "68 bars · 42 from commit times · 26 estimated".]

Here's what I find interesting. It looks like a team working side by side. It mostly isn't. Add it up and there are only about four hours where both were working at the same time. It's a relay race: one picks up when the other stops.

[Dim everything except the overlapping bars on Saturday evening, around 18:00.]

Some of that is just quota. Saturday evening, Claude ran out, and Codex took over all eight open fixes until Claude came back after midnight.

[Highlight Saturday 18:00 to midnight: the green G19 and G20 bars, the gap in orange.]

You can also see who's good at what, or at least who I handed what to. Claude spent most of its time in game code. ChatGPT did the terrain, the big generated world, overnight on Thursday, and a lot of the art.

[Highlight Friday's long green Terrain bar, G06, then Claude's dense Game row.]

And what's not on the chart is me. I'm the one bar that runs the whole length, playing it, writing the fix lists, and deciding what we keep.

[Pull back to the full chart. Hold for a beat.]

*Numbers: from `Checkpoint1/gantt/work_log.csv` as of Nov 15 noon, about 45 bar-hours for Claude and 32 for ChatGPT/Codex, but don't say these as hours worked; the bar lengths are estimates. The four hours of overlap is computed from the same bars, so it's approximate too; "about four hours" is fair, anything more precise isn't.*

## 4. Day one — get a film on its feet

The early goal was a complete loop before polishing anything. By the first morning there was a hero on screen, and by late morning a bot could play through the whole film by itself. That bot earned its keep: a game can compile perfectly and still fall over somewhere between the title screen and the credits.

Then came the HUD, Fenella, the director's notes, the wrap, skills, music, characters and the look. And plenty of strange little problems: a camera too close, rats the bot couldn't catch, generated objects that needed a human eye.

[Early and later captures.]

## 4b. The asset audition

We didn't ask one generator for a world and accept whatever came back. We auditioned. Three rats: Meshy, Hyper3D Rodin and Hunyuan3D through fal. Very different personalities. I picked the Rodin rat. Tripo came later for candy foliage, so the tree isn't a same-prompt comparison.

A generated model is a starting point. We still had to judge the look, clean meshes, sort out scale and animation, and see whether it worked at the game camera. Looking good on a review screen doesn't make it a good game asset.

[Labeled comparison, then the rat and foliage in game. Add your reaction to each model.]

## 5. The world gets bigger

The film-shoot structure kept getting pulled outward by the candy world. Sugarborough became a place to travel through, with sets in the landscape and a compass. Four classes followed, with sound and voices.

Then the question: what can we actually ship? The release plan protected time for packaging and testing. And then I changed the plan: the village quest givers were too central to cut. Five villagers and ten linked jobs went back in.

[World and village; say in your own words why that mattered.]

## 6. The part where we play it

This is where a list of completed tasks starts being a game. I played, made lists, and we worked through them: a compass label covering the compass, missing HUD information, the world going grey at the wrong time, controls, progression, the little things that make a character feel wrong in your hands.

Some fixes passed automated checks before anyone had seen or heard them. A green test tells you a lot, but it can't tell me whether an attack feels funny, whether a voice lands, or whether walking across the river is any fun.

[Your favourite specific playtest surprise.]

## 7. The long night

Saturday night into Sunday, the village grew, the landscape got a river and a bridge, and the attacks got personality: peppermints raining down, celery arrows, flour clouds and storms of playing cards. A hundred and fifty-four translated battle cries went in. The first time you use a skill it's in English; after that, you never know what language you'll get.

Around half past two in the morning a fresh build packaged, passed its smoke test and zipped at 737 megabytes. By five, every villager had two to four encore jobs, and a review pass over the night's code had found and fixed seven gameplay bugs, like town errands you couldn't hand in and a defeat that didn't reset your progress.

[Attack effects in motion; a village encore.]

## 8. Sunday morning — the candy world gets a coat of paint

*Reconstructed from the records, 05:00–11:00 project time. Times are commit stamps.*

By half past eight there was a release candidate: repackaged, smoke test clean, and a bot playing all three main scenes and all ten village jobs end to end. That's the point where a sensible person stops adding things.

Instead, three things happened at once.

**The ground.** ChatGPT generated fifteen seamless PBR material sets, three takes each of rock candy, peanut brittle, peppermint, chocolate mint and mint grass, and put them on a review page for me to pick from. The picks went onto the paths and the ground. The river became opaque, glossy hot chocolate, with steam coming off it. Thirty-six more trees, seventy-three shrubs, and twenty-three mountains along the eastern horizon.

[PBR contact sheet, then the village from above.]

**The marshmallows.** Thirteen floating cylinders had been standing in for marshmallows on the river. They were replaced by three generated marshmallow models (classic, squashed and twisted), cleaned up in Blender, drifting downstream at forty-five centimetres a second with a little bob. Nobody needed this. [Your reaction.]

[Marshmallow contact sheet, then them drifting under the bridge.]

**The UI.** Four interface directions were mocked up: Candy Arcade, Frosted Stickerbook, Saccharine Reliquary and Black Sugar. I picked Black Sugar, a dark candy look with pewter frames, peppermint enamel and proper serif fonts, and by half past nine it was in the game: menus, HUD, compass, and cooldown numbers on the skills. I cut the full screen-by-screen review short to save usage. That's a real constraint in this kind of project: the collaborators have a budget too.

[Old title screen, then the Black Sugar title. Point at the subtitle: "A weekend project made by a director who should know better, and machines that can't say no."]

Meanwhile, Claude kept working on the game itself: past level ten you now earn encore levels, five percent more power each. And the submission zip stopped dragging the Saved folder along with it.

## 8b. "The river is terrible"

Then I played it, and wrote Fixes4. Some lines, verbatim:

> Controls are hidden under "Notes" on the title menu. These are the ONE THING THAT SHOULD NOT HAVE A JOKE LABEL.

> The archer's flashes are the right idea, but a little too big and bright. It hurts.

> The river is terrible. It's a single ribbon with no depth above the ground. The bridge is flattened, for some reason, and looks like crap.

> There's a house actually sitting on the path, and a tree in the middle of where a road should be.

So: Controls is called Controls now. The signature skill got a keyboard key, Shift, because laptop trackpads don't have a middle mouse button. You start at level one instead of seven, and the monsters and you scale up together, so the village jobs give you somewhere to grow. The flashes are about five times dimmer. The barbarian baker's ultimate actually spins now, and the flour cloud is bigger and hangs around. The knight's shouts finally come out as text placards, which had been in the design all along.

Then the terrain. The river got carved into a real channel with banks. The bridge stopped being flat. Fourteen hundred and thirty-two instances of things growing in the middle of roads were removed, two buildings moved off the path, and about nineteen hundred bits of brush went down along the east road, so the walk to the river isn't a bare field anymore.

[River before and after: the same crossing, about ninety minutes apart.]

Last, at eleven: the camera. It kept getting stuck in hills and tree canopies. The first fix didn't work, because the trees in this map don't block the camera's trace at all; it had to check the trees' bounds directly. Now the camera swings up over buildings and canopies instead of clipping through them.

[Camera lifting over a canopy, in motion.]

As I record this, the newest of these changes pass their tests but haven't been packaged and smoke-tested together yet. [Update this line once they have.]

## 9. Why do this?

I want to make great stuff. AI is the best tool for making things I've ever encountered, and this was me testing the limits of its speed and flexibility.

Look at what's in here: multilingual speech, generated 3D models, custom voice lines and sound effects, a full orchestral score, rigging, the UI. Doing all of that from scratch would be completely unreasonable for a team to do just as a test. This little vertical slice would take months.

[DRAFT LIST, from the project records; confirm each item before you say it:
- 154 battle cries in other languages, picked at random after your first English one
- a voiced faerie director and a voiced cast
- 3D models auditioned across Meshy, Rodin, Hunyuan3D and Tripo, then cleaned up and rigged
- about twenty pieces of score, two takes each, from ACE Studio
- fifteen generated candy PBR materials, picked from a review page
- generated buildings, bridge, foliage and marshmallows
- a bot that plays the whole game unattended, so every build gets checked]

I originally wanted an MMO, and I didn't get it. But I got a playable chunk that people might actually have fun with.

The whole game is something of a joke. I went for maximum ridiculousness on a lot of levels. But the core experiment was serious: make something huge, make it work pretty well, and see what happens.

**The real limit.** Every time I open the game, I see several things I want to improve. That's the nature of a jam, but it also tells me I really should have more people working with me. I'd love a full team. At the same time, a full team brings communication overhead and discussions. It would make a better game, in more time.

That's the real limitation of AI here. It's hard to get multiple points of view, and AI doesn't play the game as critically as I do on the human side of things. Every test was green when I wrote "the river is terrible." Playtesting, and players, are a core requirement for making a good game.

[This is the thesis of the video. Hold on it, and let the music swell after the last line. If you want a callback, cut to the river-before shot on "the river is terrible".]

[Optional, about quest length. There's solid research that people value what they put effort into, the "IKEA effect" (Norton, Mochon and Ariely, 2011), and it only held when people finished the thing. I found no study that measures attachment to longer quests in games, so say it as your hunch, backed by that research, not as a games finding. Something like: "People value what they work for, as long as they get to finish it. That's why the villagers keep coming back with more jobs."]

## 10. Ending — to be written

[Fill in after the remaining work: the final build, the jam submission, what survived, what broke, and how you feel about it. Add the itch.io link when it's live.]

[End credits: `music/credits_take1.mp3`.]

## 11. Post-credits — a warning

*Hard cut from the credits to black. No music bed. Deadpan.*

> Before you go. AI made a lot of this game possible. It also made this possible. I'm sharing it so you understand the dangers.

[Play `assets/blooper/BigRockCandyMountain_Blooper_8s.mp3` over a black screen: 8.5 seconds, the opening line of the cut theme song, your vocals and whistle only, with no guitar. It's already loudness-matched and faded out. Don't extend it.]

> And that's why the theme song was cut.

[Optional extra button, if you want it: "It's almost as bad as my early Bohlen-Pierce experiments."]

*Use only this clip, never `theme-song.mp3` or `BigRockCandyMountain_Repaired.mp3`: those contain the 1928 guitar, which is still under US copyright. The clip is cut from your own stems (`Vocal_Warm_Repaired.wav` + `Whistle_Unchanged.wav` in `D:\AudacityRecordings\BigRockCandyMountain\output\repaired\`); the lyrics and melody are public domain.*
