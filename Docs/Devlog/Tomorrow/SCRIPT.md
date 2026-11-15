# The Dread Land of Candy — devlog script, working draft

Read naturally; slide prompts are cues. This draft leaves the ending open. Replace bracketed prompts after tomorrow's work. Roughly 6–8 minutes depending on gameplay and pauses.

## 1. Cold open — an unreasonable idea

I wanted to make a game in a game jam. Somewhere along the way, the conversation started sounding suspiciously like I wanted to make an MMO. In a few days. With AI collaborators.

Now, I have a history with MMOs. I started with Ancient Anguish and TsunamiMUD, went through City of Heroes, Star Wars Galaxies, EVE Online, and later Path of Exile. I skipped the big early Ultima Online and World of Warcraft wave, though I dabbled later. So my sense of reasonable scope may be a little compromised.

[On screen: “I am older than them thar hills.” Find the actual exchange before quoting it.]

## 2. The game we actually made

This is The Dread Land of Candy. You're a hero in a candy world, but you're also an actor in a rather chaotic production. Fenella, your faerie director, has opinions. There are scenes to shoot, creatures to deal with, takes to ruin, and a budget that eventually runs out.

The joke is that the heroic fantasy and the film production are both happening at once. You aren't just saving the day. You're trying to get a usable take.

[Show gameplay: Fenella, a scene, a CUT, and the budget gag. Verify the footage matches the final build.]

## 3. The setup

I directed and playtested. Claude and ChatGPT worked across planning, code, art, terrain, audio and integration, with other generation tools in the mix. There were handoffs, parallel work, and a lot of checking whether the thing that looked finished actually worked in the game.

Some setup and design work existed before the jam and was declared. The project records have the detailed credits. A caution about the chart: a commit records a snapshot, not how long somebody worked. The bars are estimates, and some attribution still needs checking.

[Show Gantt; point out events and overlap, not a leaderboard.]

## 4. Day one — get a film on its feet

The early goal was to make a complete loop before polishing everything. By the first day, the records show a hero on screen and then an automated run through the whole film. That bot became useful because a game can compile perfectly and still fail somewhere between the title screen and the ending.

From there came the HUD, Fenella, the director's notes, the wrap, skills, music, characters, and the visual style. There were plenty of strange little problems along the way: a camera too close, rats the bot couldn't catch, and generated objects that needed a human eye.

[Show early and later captures. Check anecdotes against sources before embellishing.]

## 4b. The asset audition

We didn't just ask one generator for a world and accept whatever came back. We compared candidates. Here are three rats: Meshy, Hyper3D Rodin, and Hunyuan3D through fal. They have very different personalities. I picked the Rodin rat for the game. We also used Tripo for later candy foliage; that tree is a different subject, so this isn't a four-way test of the same prompt. The Hunyuan rat used a revised prompt too.

The generated model was a starting point. We still had to judge the look, clean up meshes, sort out scale and animation, and see whether it worked at the game camera. A good-looking object on a review screen is not automatically a good game asset.

[Show the labeled comparison; then cut to the actual rat and foliage in game. Add your own reaction to each model.]

## 5. The world gets bigger

The project started with a contained film-shoot structure, but the candy world kept pulling us outward. Sugarborough became a place to travel through, with sets in the landscape and a compass to guide the player. A cast of four classes followed, along with sound and voices.

Then came the important question: what can we actually ship? The release plan tried to protect time for packaging and testing. And then I changed the plan: the village quest givers were too central to cut. Five villagers and ten linked jobs became part of the game.

[Show the world and village; explain in your own words why that decision mattered.]

## 6. The part where we play it

This is where a project stops being a list of completed tasks and starts being a game. I played, made lists, and we worked through the problems: a compass label covering the compass, missing HUD information, the world going grey at the wrong time, controls, progression, and the many little things that make a character feel wrong in your hands.

Some fixes passed automated checks before anyone had seen or heard them in a real play session. That distinction matters. A green test can tell us a lot, but it can't tell me whether an attack feels funny, whether a voice lands, or whether the walk across the river is enjoyable.

[Insert your favourite specific playtest surprise here.]

## 7. Late changes and the handoff

The village grew, the landscape got a river and a bridge, and the attack effects got more personality: peppermints, celery arrows, flour clouds and storms of cards. The records also show multilingual battle cries and a series of fixes for focus, equipment and the quest flow.

At this checkpoint, several of those late changes still needed to be seen or heard live. A fresh build had packaged, passed a basic smoke check and been zipped, but I hadn't playtested that build yet. So I'm not going to pretend that a passing test means the final game is finished.

[Tomorrow: replace this paragraph with what actually happened, and show the result.]

## 8. What this collaboration taught me

[PERSONAL INSERT: Where did the tools surprise you? Where did they disagree? What did you have to decide or fix yourself?]

The useful pattern was a tight loop: make something concrete, run it, look at it, and give the next instruction based on what happened. The hard part was keeping the different pieces, handoffs and expectations in sync while the clock kept moving.

One wonderfully specific example: one packaging command failed, but its exit code got lost in a pipe. The smoke test then ran the previous executable. The lesson wasn't that testing is useless. It was that you have to know what you just tested.

[Avoid attributing whole features solely from commit trailers.]

## 9. Ending — to be written

[Fill in after the remaining work: final build, jam submission, any separate itch release, what survived, what broke, and how you feel about it. Add a link and call to action when the destination is ready.]
