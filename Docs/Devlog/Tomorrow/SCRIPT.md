# The Dread Land of Candy — devlog script, recording draft

Read naturally; slide prompts are cues. Each section is a **take** you can record on its own: it opens with a music bed and ends on a clean beat, so the takes can be cut together in any order. Sections 1–6 come from ChatGPT's Nov 15 01:45 draft, lightly tightened. Section 7 covers the long night, and sections 8 and 8b reconstruct Sunday morning, 05:00–11:00 project time, from git and `Docs/DEVLOG.md`. The ending is still open. About 9–11 minutes in all.

**Music.** Everything in `music/` is the game's own score, generated in ACE Studio from `music/RPG-music-prompts.md`, which asked for epic music played completely straight: the comedy comes from heroic orchestras over rats and candy. Put the beds below under your voice at roughly −18 to −22 dB, and bring them up between takes. They are the mp3 masters, so nothing needs ripping from the game. Check `theme-song.mp3` for vocals before talking over it; if it has them, use it only under titles and the ending.

| Take | Bed | Why |
| --- | --- | --- |
| 1 Cold open | `music/title-theme_take1.mp3` | The title music, utterly sincere |
| 2 Premise | `music/theme-song.mp3` (titles only), then `music/village_take1.mp3` | Introduces the game |
| 3 Collaboration | `music/setting-out_take1.mp3` | The journey begins |
| 4 Day one | `music/quest-accepted_take1.mp3` | |
| 4b Asset audition | `music/mystery_take1.mp3` | Three rats, one choice |
| 5 World gets bigger | `music/open-world-exploration_take1.mp3` | |
| 6 Playtest | `music/standard-combat_take1.mp3` | |
| 7 The long night | `music/hard-combat_take1.mp3` | |
| 8 Sunday morning | `music/village_take2.mp3`; `music/feeding-candy_take1.mp3` under the marshmallows | The candy world gets a coat of paint |
| 8b "The river is terrible" | `music/boss-battle_take1.mp3` | A bug list scored as a boss fight |
| 9 Why do this | `music/emotional-revelation_take1.mp3` | The sincere bit |
| 10 Ending | `music/victory_take1.mp3` → `music/credits_take1.mp3` | |

## 1. Cold open — an unreasonable idea

I wanted to make a game in a game jam. Somewhere along the way the conversation started sounding suspiciously like I wanted to make an MMO. In a few days. With AI collaborators.

I have a history with MMOs. I started on MUDs, Ancient Anguish and TsunamiMUD, back when the graphics were your imagination. Then City of Heroes, Star Wars Galaxies, EVE Online, and much later Path of Exile one and two. I skipped Ultima Online and the big World of Warcraft wave and only dabbled later. I've tried plenty of the modern ones on Steam (Aion, Guild Wars, the Warhammer one, MechWarrior Online, and so on) without ever settling in.

So my sense of reasonable scope may be a little compromised.

[On screen, in the background while you talk: a slow pan across the Molar Hills with the caption **"I am older than them thar hills."** Don't mention it; let it sit there.]

## 2. The game we actually made

This is The Dread Land of Candy. You're a hero in a candy world, but you're also an actor in a rather chaotic production. Fenella, your faerie director, has opinions. There are scenes to shoot, creatures to deal with, takes to ruin, and a budget that eventually runs out.

The joke is that the heroic fantasy and the film production happen at once. You aren't just saving the day. You're trying to get a usable take.

[Gameplay: Fenella, a scene, a CUT, the budget gag. Record from the final build.]

## 3. The setup

I directed and playtested. Claude and ChatGPT worked across planning, code, art, terrain, audio and integration, with a pile of generation tools in the mix. There were handoffs, parallel work, and a lot of checking whether the thing that looked finished actually worked in the game.

Some setup and design work existed before the jam and is declared. A caution about the chart: a commit records a snapshot, not how long somebody worked. The bars are estimates.

[Gantt; point out events and overlap, not a leaderboard.]

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
