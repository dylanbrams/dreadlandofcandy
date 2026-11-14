#include "Film/JamLines.h"

namespace
{
	struct FPool
	{
		EJamLineTrigger Trigger;
		TArray<const TCHAR*> Texts;
	};

	// Jokes.md, "Fenella's lines, by trigger", the commentary pool, the lecture bubbles and the production notes.
	TArray<FPool> BuildPools()
	{
		return {
			{ EJamLineTrigger::RealityIntro, {
				TEXT("Welcome to Heroes of Dread: The Land of Candy! I'm Fenella Flicker, producer, director and the only person here who understands reality television."),
				TEXT("I've assembled these villagers and cast YOU as our hero. They bring the problems. You bring the brooding. I'll bring the camera alarmingly close."),
				TEXT("I want MASSIVE problems. Impossible odds! Tears before breakfast! The villagers asked for a quiet life. Terrible television. I've requested revisions."),
				TEXT("Every little village crisis will become an EPIC catastrophe. Entirely real reactions, naturally. If yours aren't dramatic enough, we'll do another take."),
				TEXT("Now come to my golden triangle, hero. Your starring role awaits. Try to look haunted. It's candy country; you'll have to make an effort."),
			} },
			{ EJamLineTrigger::SceneStart, {
				TEXT("Places, everyone! And by everyone I mean you."),
				TEXT("Quiet on set! It's a field. It was already quiet. Well done, all."),
				TEXT("Remember: you're grim, you're haunted, you're holding celery."),
				TEXT("Rolling! Is the red light on? There's no red light. Rolling!"),
				TEXT("Scene one, take one. No pressure. It's only my whole career."),
				TEXT("And... ACTION! I love saying that. Ninth time today."),
				TEXT("Hit your mark. Your mark is the dirt. All of it."),
				TEXT("Give me dread. Give me doom. Give me dental hygiene."),
			} },
			{ EJamLineTrigger::Level7Intro, {
				TEXT("You start at level seven. Levels one to six tested poorly."),
				TEXT("Don't ask about the first six levels. Continuity."),
			} },
			{ EJamLineTrigger::FailedTake, {
				TEXT("Cut! Beautiful. Unusable, but beautiful."),
				TEXT("That's what take two is for. I've budgeted for four."),
				TEXT("The rats were perfect. We'll go again for you."),
				TEXT("Back to one! That's film talk for \"you did it wrong.\""),
				TEXT("That was a take. Technically. RETAKE!"),
				TEXT("Don't apologise. You weren't going to. Again!"),
			} },
			{ EJamLineTrigger::Take3, { TEXT("Take three. Still within budget. Barely. Emotionally.") } },
			{ EJamLineTrigger::Take4, { TEXT("Take four. This is the last one I budgeted for.") } },
			{ EJamLineTrigger::Take5, { TEXT("Take five. We're now spending your cut.") } },
			{ EJamLineTrigger::Take6, { TEXT("Take six. I'm telling people this was a choice.") } },
			{ EJamLineTrigger::CutPitch, {
				TEXT("CUT! The rats have no motivation. Options, darling."),
				TEXT("CUT! Needs a stronger antagonist. I'm thinking: bigger goblin."),
				TEXT("CUT! Where's the ticking clock? I brought a clock."),
				TEXT("CUT! Test audiences wanted more rats. I asked two rats."),
				TEXT("CUT! Same scene, but sadder. Lecture them gently."),
				TEXT("CUT! It's good. Make it more. Just... more."),
			} },
			{ EJamLineTrigger::NoteTaken, {
				TEXT("YES. This is why I hired you. Found you. Same thing."),
				TEXT("See? Collaboration. I talk, you agree."),
				TEXT("Write that down. \"The hero had a great idea.\" Mine."),
			} },
			{ EJamLineTrigger::NoteRefused, {
				TEXT("Fine. Do it your way. It'll be worse. I'll say so in the credits."),
				TEXT("Noted. In my other notebook. The grudge one."),
				TEXT("No, no. You're the artist. I'm only the director."),
			} },
			{ EJamLineTrigger::ForcedRetake, {
				TEXT("Perfect! Again."),
				TEXT("This time the rats know their lines."),
			} },
			{ EJamLineTrigger::NarrationCut, { TEXT("CUT. Nobody reads these. Skip to the rats.") } },
			{ EJamLineTrigger::Wrap, {
				TEXT("That's a wrap! Everyone gets paid. \"Everyone\" is a term of art."),
				TEXT("You're going to be a star! My cut is the star part."),
				TEXT("Great work. I'll send you the bill."),
				TEXT("Here's your share. Don't count it. It's not for counting."),
			} },
			{ EJamLineTrigger::BudgetOut, {
				TEXT("Small thing. We're out of money. Keep rolling!"),
				TEXT("Pretend the box is a rat. I believe in you. I have to."),
				TEXT("This is a stylistic choice now. Say it with me."),
			} },
			{ EJamLineTrigger::Premiere, {
				TEXT("A triumph. I'd like to thank myself. Thank you."),
				TEXT("Four stars! Out of a number I'll confirm later."),
			} },
			{ EJamLineTrigger::Pause, {
				TEXT("Why did everyone stop? Who called lunch?"),
				TEXT("Walking off set? Your contract has thoughts about that."),
				TEXT("Fine. Go. I'll film the rats. They're easier to light."),
			} },
			{ EJamLineTrigger::Idle, {
				TEXT("Take your time. Film is cheap. It isn't. Hurry up."),
				TEXT("Is this a dramatic pause, or are you buffering?"),
				TEXT("I'm paying you by the hour. I'm not. Move anyway."),
				TEXT("Brooding is good. Brooding near the rats is better."),
				TEXT("We'll fix the standing still in post. I can't afford post."),
				TEXT("The audience is bored. I'm the audience."),
				TEXT("Lovely stillness. Very brave. Now do the other thing. Acting."),
				TEXT("I can hear the budget ticking. It's a small sound."),
			} },
			{ EJamLineTrigger::Strayed, {
				TEXT("This is a closed set! Get back in frame!"),
				TEXT("The set ends there. Past it is nothing. I didn't build it."),
				TEXT("Where are you going? The plot is over here."),
				TEXT("That's not in the script. Nothing is, but that especially."),
				TEXT("Come back! I can't fly that far. Union rules. My union."),
				TEXT("If you find a better film out there, don't tell me."),
			} },
			{ EJamLineTrigger::LectureCast, {
				TEXT("Less violence! More character development!"),
				TEXT("The sword is a metaphor! Keep it a metaphor!"),
				TEXT("Don't swing it! Do you know what rat insurance costs?"),
				TEXT("Louder! The cheap seats are in another village!"),
				TEXT("Yes! Disappoint them!"),
			} },
			{ EJamLineTrigger::Pacified, {
				TEXT("He's learning! That's an arc!"),
				TEXT("Hold on his little face. Hold... gorgeous."),
				TEXT("That rat just earned a supporting credit."),
				TEXT("Look at the remorse. You can't direct that. I directed that."),
				TEXT("One down. Emotionally."),
			} },
			{ EJamLineTrigger::Interact, {
				TEXT("Circular motions! I want to feel the plaque leaving."),
				TEXT("Offer the celery like it's a crown."),
				TEXT("He's chewing! Nobody move. Nobody breathe. Got it."),
				TEXT("Gums too. We're not animals. Well. He is."),
				TEXT("That's the shot. A knight, a goblin, a minty silence."),
			} },
			{ EJamLineTrigger::Sulking, {
				TEXT("I'm not filming this. I'm filming that bush. The bush takes notes."),
				TEXT("Oh, was that good? I wasn't looking."),
				TEXT("Carry on. Don't mind the director. Nobody does."),
				TEXT("The bush is giving me so much right now."),
			} },
			{ EJamLineTrigger::Commentary, {
				TEXT("Fun fact: this rat is played by three smaller rats."),
				TEXT("We shot this scene in one day. We shot everything in one day. Four, legally."),
				TEXT("The talent did all their own standing."),
				TEXT("That house isn't real. None of them are. One is a painting of a house. I won't say which."),
				TEXT("The original cut was nine hours. The studio lost six of them. I know where they are."),
				TEXT("This field was meant to be a volcano. Then I saw the price of a volcano."),
				TEXT("People ask what the film is about. It's about ninety minutes. It's eight. It's about eight minutes."),
				TEXT("Watch the goblin on the left. He's improvising. I docked him for it."),
			} },
			{ EJamLineTrigger::BubbleDefault, {
				TEXT("ABSOLUTELY NOT."), TEXT("WE'VE TALKED ABOUT THIS."), TEXT("I'M NOT ANGRY."), TEXT("I'M DISAPPOINTED."),
				TEXT("FORM A LINE!"), TEXT("MIND YOUR TEETH!"), TEXT("IS THIS WHO YOU WANT TO BE?"), TEXT("ELBOWS."),
				TEXT("INDOOR VOICE!"), TEXT("EXCUSE ME."), TEXT("PER MY LAST LECTURE"), TEXT("WE DO NOT EAT THE CROPS."),
				TEXT("LOOK AT ME WHEN I'M LECTURING."),
			} },
			{ EJamLineTrigger::BubbleSadder, {
				TEXT("if it's not too much trouble"), TEXT("i just worry"), TEXT("no, you go ahead"), TEXT("i'm sure you had your reasons"),
			} },
			{ EJamLineTrigger::BubbleBudget, { TEXT("[SHOUTING]"), TEXT("[LECTURE]"), TEXT("[BUBBLE GOES HERE]") } },
			{ EJamLineTrigger::NuisanceReply, {
				TEXT("ok"), TEXT("sorry"), TEXT("fine."), TEXT("...mum?"), TEXT("i'll floss"), TEXT("he started it"),
			} },
			{ EJamLineTrigger::ProductionNote, {
				TEXT("This walk was restored for the Director's Cut. It was cut for pacing. You can see why."),
				TEXT("The following scene contains 37% more dental hygiene than the theatrical release."),
				TEXT("Colour corrected by a faerie with strong opinions."),
				TEXT("No rats were lectured more than necessary."),
				TEXT("Scene missing. Pretend it was great."),
			} },
		};
	}

	const TCHAR* const GNames[] = {
		TEXT("SceneStart"), TEXT("Level7Intro"), TEXT("RealityIntro"), TEXT("FailedTake"), TEXT("Take3"), TEXT("Take4"), TEXT("Take5"), TEXT("Take6"),
		TEXT("CutPitch"), TEXT("NoteTaken"), TEXT("NoteRefused"), TEXT("ForcedRetake"), TEXT("NarrationCut"), TEXT("Wrap"),
		TEXT("BudgetOut"), TEXT("Premiere"), TEXT("Pause"), TEXT("Idle"), TEXT("Strayed"), TEXT("LectureCast"), TEXT("Pacified"),
		TEXT("Interact"), TEXT("Sulking"), TEXT("Commentary"), TEXT("BubbleDefault"), TEXT("BubbleSadder"), TEXT("BubbleBudget"),
		TEXT("NuisanceReply"), TEXT("ProductionNote"),
	};
	static_assert(UE_ARRAY_COUNT(GNames) == (int32)EJamLineTrigger::Count, "trigger names out of step with the enum");
}

namespace JamLines
{
	const TCHAR* TriggerName(EJamLineTrigger Trigger)
	{
		return GNames[(int32)Trigger];
	}

	const TArray<FJamLine>& All()
	{
		static const TArray<FJamLine> Table = []()
		{
			TArray<FJamLine> Out;
			for (const FPool& Pool : BuildPools())
			{
				int32 N = 0;
				for (const TCHAR* Text : Pool.Texts)
				{
					FJamLine L;
					L.Trigger = Pool.Trigger;
					L.Text = Text;
					L.Id = FName(*FString::Printf(TEXT("%s_%02d"), TriggerName(Pool.Trigger), ++N));
					Out.Add(MoveTemp(L));
				}
			}
			return Out;
		}();
		return Table;
	}

	TArray<const FJamLine*> ForTrigger(EJamLineTrigger Trigger)
	{
		TArray<const FJamLine*> Out;
		for (const FJamLine& L : All())
		{
			if (L.Trigger == Trigger) { Out.Add(&L); }
		}
		return Out;
	}

	const FJamLine* Find(FName Id)
	{
		return All().FindByPredicate([Id](const FJamLine& L) { return L.Id == Id; });
	}

	bool IsFenella(EJamLineTrigger Trigger) { return Trigger <= EJamLineTrigger::Commentary; }

	bool IsStory(EJamLineTrigger Trigger) { return Trigger <= EJamLineTrigger::Pause; }

	bool IsOnce(EJamLineTrigger Trigger)
	{
		return Trigger == EJamLineTrigger::Level7Intro
			|| Trigger == EJamLineTrigger::RealityIntro
			|| (Trigger >= EJamLineTrigger::Take3 && Trigger <= EJamLineTrigger::Take6);
	}

	EJamLineTrigger TriggerForTake(int32 Take)
	{
		if (Take <= 2) { return EJamLineTrigger::SceneStart; }
		return (EJamLineTrigger)((int32)EJamLineTrigger::Take3 + FMath::Min(Take, 6) - 3);
	}
}
