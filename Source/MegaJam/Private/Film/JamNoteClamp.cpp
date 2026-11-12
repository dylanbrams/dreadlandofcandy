#include "Film/JamNoteClamp.h"
#include "Film/JamLines.h"
#include "Film/JamMiniJson.h"
#include "Film/JamNagScheduler.h"

int32 FJamNoteContext::CapacityOf(EJamTargetKind Kind) const
{
	for (const TPair<EJamTargetKind, int32>& K : Kinds)
	{
		if (K.Key == Kind) { return K.Value; }
	}
	return 0;
}

int32 FJamNoteContext::MaxCapacity() const
{
	int32 Max = 0;
	for (const TPair<EJamTargetKind, int32>& K : Kinds) { Max = FMath::Max(Max, K.Value); }
	return Max;
}

namespace
{
	constexpr int32 HardMaxCount = 8;

	const TCHAR* KindName(EJamTargetKind Kind)
	{
		switch (Kind)
		{
		case EJamTargetKind::Rat: return TEXT("rat");
		case EJamTargetKind::Goblin: return TEXT("goblin");
		case EJamTargetKind::Bug: return TEXT("bug");
		default: return TEXT("almond");
		}
	}

	/** Control characters out, whitespace collapsed, trimmed, cut to MaxLen (never in the middle of a surrogate pair). */
	FString CleanText(const FString& In, int32 MaxLen)
	{
		FString Out;
		bool bSpace = false;
		for (int32 i = 0; i < In.Len(); ++i)
		{
			const TCHAR C = In[i];
			if (C < 0x20 || C == 0x7F || C == 0x2028 || C == 0x2029)
			{
				if (C == '\t' || C == '\n' || C == '\r' || C == 0x2028 || C == 0x2029) { bSpace = true; }
				continue;
			}
			if (C == ' ') { bSpace = true; continue; }
			if (bSpace && !Out.IsEmpty()) { Out.AppendChar(' '); }
			bSpace = false;
			Out.AppendChar(C);
		}
		if (Out.Len() > MaxLen) { Out.LeftInline(MaxLen); }
		if (!Out.IsEmpty() && Out[Out.Len() - 1] >= 0xD800 && Out[Out.Len() - 1] <= 0xDBFF) { Out.LeftChopInline(1); }
		Out.TrimEndInline();
		return Out;
	}

	bool ReadNumber(const FJamJson& Form, const TCHAR* Key, double& Out)
	{
		const FJamJson* V = Form.Find(Key);
		if (!V || V->Type != FJamJson::EType::Number) { return false; }
		Out = V->Number;
		return true;
	}

	bool ReadText(const FJamJson& Form, const TCHAR* Key, int32 MaxLen, bool bRequireNonEmpty, FString& Out)
	{
		const FJamJson* V = Form.Find(Key);
		if (!V || V->Type != FJamJson::EType::String) { return false; }
		Out = CleanText(V->String, MaxLen);
		return !bRequireNonEmpty || !Out.IsEmpty();
	}

	/** Strings in an array: cleaned, empties dropped, cut to MaxItems. Fails if fewer than MinItems remain. */
	bool ReadTextList(const FJamJson& Form, const TCHAR* Key, int32 MinItems, int32 MaxItems, int32 MaxLen, TArray<FString>& Out)
	{
		const FJamJson* V = Form.Find(Key);
		if (!V || V->Type != FJamJson::EType::Array) { return false; }
		Out.Reset();
		for (const FJamJson& Item : V->Array)
		{
			if (Item.Type != FJamJson::EType::String) { continue; }
			const FString Clean = CleanText(Item.String, MaxLen);
			if (!Clean.IsEmpty() && Out.Num() < MaxItems) { Out.Add(Clean); }
		}
		return Out.Num() >= MinItems;
	}

	/** At most three words, at most 24 characters. */
	FString CleanMotif(const FString& In)
	{
		TArray<FString> Words;
		In.ParseIntoArrayWS(Words);
		if (Words.Num() > 3) { Words.SetNum(3); }
		return CleanText(FString::Join(Words, TEXT(" ")), 24);
	}

	float ClampTime(double Seconds)
	{
		if (Seconds < 1.0) { return 0.f; }
		return (float)FMath::Clamp(Seconds, 30.0, 120.0);
	}

	EJamTint TintFrom(const FString& S)
	{
		if (S == TEXT("pink")) { return EJamTint::Pink; }
		if (S == TEXT("mint")) { return EJamTint::Mint; }
		if (S == TEXT("gold")) { return EJamTint::Gold; }
		if (S == TEXT("grey")) { return EJamTint::Grey; }
		if (S == TEXT("purple")) { return EJamTint::Purple; }
		return EJamTint::None;
	}

	EJamWeather WeatherFrom(const FString& S)
	{
		if (S == TEXT("rain")) { return EJamWeather::Rain; }
		if (S == TEXT("sprinkle_snow")) { return EJamWeather::SprinkleSnow; }
		if (S == TEXT("fog")) { return EJamWeather::Fog; }
		return EJamWeather::None;
	}

	EJamMood MoodFrom(const FString& S)
	{
		if (S == TEXT("epic")) { return EJamMood::Epic; }
		if (S == TEXT("sad")) { return EJamMood::Sad; }
		return EJamMood::Cheerful;
	}
}

namespace JamNoteClamp
{
	FJamNoteContext MakeContext(const FJamSceneDef& Scene, int32 StageIndex)
	{
		FJamNoteContext C;
		C.SceneId = Scene.Id;
		if (Scene.Stages.IsValidIndex(StageIndex)) { C.BaseStage = Scene.Stages[StageIndex]; }
		for (EJamTargetKind Kind : { EJamTargetKind::Rat, EJamTargetKind::Goblin, EJamTargetKind::Bug, EJamTargetKind::Almond })
		{
			const int32 Cap = JamScenes::SetCapacity(Scene, Kind);
			if (Cap > 0) { C.Kinds.Emplace(Kind, Cap); }
		}
		return C;
	}

	float BaseResolve(EJamTargetKind Kind)
	{
		switch (Kind)
		{
		case EJamTargetKind::Rat: return 100.f;
		case EJamTargetKind::Goblin: return 150.f;
		case EJamTargetKind::Bug: return 60.f;
		default: return 0.f;
		}
	}

	FString SanitizeNote(const FString& Typed)
	{
		return CleanText(Typed, NoteMaxChars);
	}

	bool IsFinishable(const FJamStageDef& Stage, const FJamNoteContext& Ctx)
	{
		if (Stage.Count < 1) { return false; }
		if (Stage.TimeLimit <= 0.f) { return true; }

		// A generous, sequential model of a player who only has the Lecture: travel two seconds per target, then hit
		// until pacified (or hold E, or pick up). The player must manage it in 60% of the time on the clock.
		float PerTarget = 2.f;
		// An Interact stage with a changed Resolve spawns fresh targets (a boss), which must be pacified before they are treated.
		const bool bFreshTargets = Stage.Verb == EJamStageVerb::Interact && Stage.ResolveMultiplier != 1.f;
		if (bFreshTargets)
		{
			const float Hits = FMath::CeilToFloat(BaseResolve(Stage.Target) * Stage.ResolveMultiplier / FMath::Max(1.f, Ctx.LectureDamage));
			PerTarget += FMath::Max(1.f, Hits) * Ctx.LectureCooldown;
		}
		switch (Stage.Verb)
		{
		case EJamStageVerb::Pacify:
		{
			const float Damage = FMath::Max(1.f, Ctx.LectureDamage);
			const float Hits = FMath::CeilToFloat(BaseResolve(Stage.Target) * Stage.ResolveMultiplier / Damage);
			PerTarget += FMath::Max(1.f, Hits) * Ctx.LectureCooldown;
			break;
		}
		case EJamStageVerb::Interact: PerTarget += 0.6f; break;
		default: break;
		}
		return Stage.Count * PerTarget <= 0.6f * Stage.TimeLimit;
	}

	float GrossFromKnobs(const FJamStageDef& Stage, float Scale, const FJamStageDef& Base)
	{
		float G = 1.1f;
		G += 0.05f * FMath::Max(0, Stage.Count - Base.Count);
		G += 0.10f * FMath::Max(0.f, Stage.ResolveMultiplier - Base.ResolveMultiplier);
		G += 0.10f * FMath::Max(0.f, Stage.SpeedMultiplier - Base.SpeedMultiplier);
		G += 0.05f * FMath::Max(0.f, Scale - 1.f);
		if (Stage.TimeLimit > 0.f && (Base.TimeLimit <= 0.f || Stage.TimeLimit < Base.TimeLimit)) { G += 0.1f; }
		return FMath::Clamp(G, 1.1f, 2.0f);
	}

	bool FromResponse(const FString& Json, const FJamNoteContext& Ctx, FJamClampedNote& Out, FString& OutReason)
	{
		FJamJson Root;
		if (!FJamJson::Parse(Json, Root) || Root.Type != FJamJson::EType::Object)
		{
			OutReason = TEXT("not a JSON object");
			return false;
		}
		const FJamJson* V = Root.Find(TEXT("v"));
		if (!V || V->Type != FJamJson::EType::Number || V->Number != 1.0)
		{
			OutReason = TEXT("wrong or missing version");
			return false;
		}
		const FJamJson* Source = Root.Find(TEXT("source"));
		if (!Source || Source->Type != FJamJson::EType::String || (Source->String != TEXT("cache") && Source->String != TEXT("model")))
		{
			OutReason = TEXT("no form on offer (budget, refused or unknown source)");
			return false;
		}
		const FJamJson* Form = Root.Find(TEXT("form"));
		if (!Form || Form->Type != FJamJson::EType::Object)
		{
			OutReason = TEXT("missing form");
			return false;
		}

		double Count = 0, Resolve = 0, Speed = 0, Scale = 0, Time = 0;
		if (!ReadNumber(*Form, TEXT("count"), Count) || !ReadNumber(*Form, TEXT("resolve_mult"), Resolve)
			|| !ReadNumber(*Form, TEXT("speed_mult"), Speed) || !ReadNumber(*Form, TEXT("scale"), Scale)
			|| !ReadNumber(*Form, TEXT("time_limit_s"), Time))
		{
			OutReason = TEXT("missing or non-numeric knob");
			return false;
		}

		FString TargetKind, Tint, Weather, Mood;
		FJamNoteCosmetics C;
		FString Objective;
		if (!ReadText(*Form, TEXT("target_kind"), 16, true, TargetKind) || !ReadText(*Form, TEXT("tint"), 16, true, Tint)
			|| !ReadText(*Form, TEXT("weather"), 16, true, Weather) || !ReadText(*Form, TEXT("mood"), 16, true, Mood)
			|| !ReadText(*Form, TEXT("motif"), 200, true, C.Motif)
			|| !ReadText(*Form, TEXT("scene_title"), 40, true, C.SceneTitle)
			|| !ReadText(*Form, TEXT("fenella_reaction"), 90, true, C.FenellaReaction)
			|| !ReadText(*Form, TEXT("objective"), 60, true, Objective)
			|| !ReadText(*Form, TEXT("loot_name"), 48, true, C.LootName)
			|| !ReadText(*Form, TEXT("loot_flavour"), 80, true, C.LootFlavour))
		{
			OutReason = TEXT("missing, non-text or empty text field");
			return false;
		}
		if (!ReadTextList(*Form, TEXT("lecture_bubbles"), 3, 5, 24, C.LectureBubbles)
			|| !ReadTextList(*Form, TEXT("replies"), 2, 3, 16, C.Replies))
		{
			OutReason = TEXT("too few bubbles or replies");
			return false;
		}
		C.Motif = CleanMotif(C.Motif);
		if (C.Motif.IsEmpty())
		{
			OutReason = TEXT("empty motif");
			return false;
		}

		// Knobs: clamped, never rejected for being out of range.
		FJamStageDef Stage = Ctx.BaseStage;
		Stage.Id = TEXT("TypedNote");
		Stage.Objective = Objective;
		Stage.Target = Ctx.BaseStage.Target;
		if (Ctx.BaseStage.Verb != EJamStageVerb::Collect)
		{
			for (const TPair<EJamTargetKind, int32>& K : Ctx.Kinds)
			{
				if (TargetKind == KindName(K.Key) && K.Value > 0) { Stage.Target = K.Key; }
			}
		}
		const int32 Capacity = FMath::Min(HardMaxCount, Ctx.CapacityOf(Stage.Target));
		if (Capacity < 1)
		{
			OutReason = TEXT("the set cannot spawn that target");
			return false;
		}
		Stage.Count = FMath::Clamp((int32)FMath::RoundToInt((float)FMath::Clamp(Count, -1000.0, 1000.0)), 1, Capacity);
		Stage.ResolveMultiplier = (float)FMath::Clamp(Resolve, 0.5, 3.0);
		Stage.SpeedMultiplier = (float)FMath::Clamp(Speed, 0.5, 2.0);
		Stage.TimeLimit = ClampTime(Time);
		C.Scale = (float)FMath::Clamp(Scale, 0.5, 3.0);
		C.Tint = TintFrom(Tint);
		C.Weather = WeatherFrom(Weather);
		C.Mood = MoodFrom(Mood);

		if (!IsFinishable(Stage, Ctx))
		{
			OutReason = TEXT("unfinishable with the Lecture alone");
			return false;
		}

		Out = FJamClampedNote();
		Out.Source = Source->String;
		Out.Cosmetics = MoveTemp(C);
		Out.Def.Id = TEXT("TypedNote");
		Out.Def.Replacement = Stage;
		Out.Def.GrossMultiplier = GrossFromKnobs(Stage, Out.Cosmetics.Scale, Ctx.BaseStage);
		Out.Def.BubblePool = TEXT("TypedNote");
		OutReason.Reset();
		return true;
	}

	namespace
	{
		struct FWordSet
		{
			TArray<const TCHAR*> Words;
			bool Has(const TSet<FString>& Tokens) const
			{
				for (const TCHAR* W : Words)
				{
					if (Tokens.Contains(W)) { return true; }
				}
				return false;
			}
		};
	}

	bool FromKeywords(const FString& Typed, const FJamNoteContext& Ctx, int32 Seed, FJamClampedNote& Out)
	{
		// Tokens: lower case letters and digits.
		TSet<FString> Tokens;
		{
			FString Cur;
			for (TCHAR C : SanitizeNote(Typed).ToLower())
			{
				if (FChar::IsAlnum(C)) { Cur.AppendChar(C); }
				else if (!Cur.IsEmpty()) { Tokens.Add(Cur); Cur.Reset(); }
			}
			if (!Cur.IsEmpty()) { Tokens.Add(Cur); }
		}

		const FWordSet Bigger{ { TEXT("big"), TEXT("bigger"), TEXT("biggest"), TEXT("huge"), TEXT("enormous"), TEXT("giant"), TEXT("massive"), TEXT("large"), TEXT("larger") } };
		const FWordSet Smaller{ { TEXT("small"), TEXT("smaller"), TEXT("tiny"), TEXT("little"), TEXT("mini") } };
		const FWordSet More{ { TEXT("more"), TEXT("many"), TEXT("lots"), TEXT("crowd"), TEXT("crowded"), TEXT("army"), TEXT("horde"), TEXT("swarm") } };
		const FWordSet Fewer{ { TEXT("fewer"), TEXT("few"), TEXT("less"), TEXT("single"), TEXT("alone") } };
		const FWordSet Faster{ { TEXT("fast"), TEXT("faster"), TEXT("quick"), TEXT("quicker"), TEXT("hurry"), TEXT("rush"), TEXT("speedy"), TEXT("sprint") } };
		const FWordSet Slower{ { TEXT("slow"), TEXT("slower"), TEXT("sleepy"), TEXT("lazy"), TEXT("calm") } };
		const FWordSet Tougher{ { TEXT("strong"), TEXT("stronger"), TEXT("tough"), TEXT("tougher"), TEXT("boss"), TEXT("villain"), TEXT("antagonist"), TEXT("scary"), TEXT("mean") } };
		const FWordSet Weaker{ { TEXT("weak"), TEXT("weaker"), TEXT("easy"), TEXT("easier"), TEXT("soft") } };
		const FWordSet Sad{ { TEXT("sad"), TEXT("sadder"), TEXT("cry"), TEXT("crying"), TEXT("sorry"), TEXT("ashamed"), TEXT("gloomy"), TEXT("lonely") } };
		const FWordSet Epic{ { TEXT("epic"), TEXT("dramatic"), TEXT("heroic"), TEXT("grand"), TEXT("legendary") } };
		const FWordSet Happy{ { TEXT("happy"), TEXT("cheerful"), TEXT("fun"), TEXT("party"), TEXT("joy") } };
		const FWordSet Rain{ { TEXT("rain"), TEXT("rainy"), TEXT("storm"), TEXT("wet") } };
		const FWordSet Snow{ { TEXT("snow"), TEXT("snowy"), TEXT("sprinkle"), TEXT("sprinkles") } };
		const FWordSet Fog{ { TEXT("fog"), TEXT("foggy"), TEXT("mist"), TEXT("misty"), TEXT("spooky"), TEXT("haunted") } };
		const FWordSet Clock{ { TEXT("clock"), TEXT("timer"), TEXT("deadline"), TEXT("urgent"), TEXT("panic") } };

		FJamStageDef Stage = Ctx.BaseStage;
		Stage.Id = TEXT("TypedNote");
		FJamNoteCosmetics C;
		bool bAny = false;
		auto Hit = [&bAny](bool bFound) { bAny |= bFound; return bFound; };

		if (Hit(Bigger.Has(Tokens))) { C.Scale = 2.f; Stage.ResolveMultiplier *= 1.5f; }
		if (Hit(Smaller.Has(Tokens))) { C.Scale = 0.6f; Stage.ResolveMultiplier *= 0.7f; }
		if (Hit(More.Has(Tokens))) { Stage.Count += 2; }
		if (Hit(Fewer.Has(Tokens))) { Stage.Count = FMath::Max(1, Stage.Count - 2); }
		if (Hit(Faster.Has(Tokens))) { Stage.SpeedMultiplier *= 1.5f; }
		if (Hit(Slower.Has(Tokens))) { Stage.SpeedMultiplier *= 0.7f; }
		if (Hit(Tougher.Has(Tokens))) { Stage.ResolveMultiplier *= 2.f; }
		if (Hit(Weaker.Has(Tokens))) { Stage.ResolveMultiplier *= 0.7f; }
		if (Hit(Sad.Has(Tokens))) { C.Mood = EJamMood::Sad; C.Tint = EJamTint::Grey; }
		if (Hit(Epic.Has(Tokens))) { C.Mood = EJamMood::Epic; }
		if (Hit(Happy.Has(Tokens))) { C.Mood = EJamMood::Cheerful; }
		if (Hit(Rain.Has(Tokens))) { C.Weather = EJamWeather::Rain; }
		if (Hit(Snow.Has(Tokens))) { C.Weather = EJamWeather::SprinkleSnow; }
		if (Hit(Fog.Has(Tokens))) { C.Weather = EJamWeather::Fog; }
		if (Hit(Clock.Has(Tokens)) && Stage.TimeLimit <= 0.f) { Stage.TimeLimit = 45.f; }
		if (Hit(Tokens.Contains(TEXT("pink")))) { C.Tint = EJamTint::Pink; }
		if (Hit(Tokens.Contains(TEXT("mint")) || Tokens.Contains(TEXT("green")))) { C.Tint = EJamTint::Mint; }
		if (Hit(Tokens.Contains(TEXT("gold")) || Tokens.Contains(TEXT("golden")) || Tokens.Contains(TEXT("yellow")))) { C.Tint = EJamTint::Gold; }
		if (Hit(Tokens.Contains(TEXT("grey")) || Tokens.Contains(TEXT("gray")))) { C.Tint = EJamTint::Grey; }
		if (Hit(Tokens.Contains(TEXT("purple")) || Tokens.Contains(TEXT("violet")))) { C.Tint = EJamTint::Purple; }
		if (!bAny) { return false; }

		// Same limits as a service response, then make it finishable by easing it off.
		const int32 Capacity = FMath::Max(1, FMath::Min(HardMaxCount, Ctx.CapacityOf(Stage.Target)));
		Stage.Count = FMath::Clamp(Stage.Count, 1, Capacity);
		Stage.ResolveMultiplier = FMath::Clamp(Stage.ResolveMultiplier, 0.5f, 3.f);
		Stage.SpeedMultiplier = FMath::Clamp(Stage.SpeedMultiplier, 0.5f, 2.f);
		C.Scale = FMath::Clamp(C.Scale, 0.5f, 3.f);
		while (!IsFinishable(Stage, Ctx))
		{
			if (Stage.Count > 1) { --Stage.Count; }
			else if (Stage.ResolveMultiplier > 1.f) { Stage.ResolveMultiplier = FMath::Max(1.f, Stage.ResolveMultiplier - 0.5f); }
			else { Stage.TimeLimit = 0.f; }
		}

		// Text comes from the authored pools (Jokes.md); the scene keeps its own title, objective and loot.
		FJamNagScheduler Picker(Seed);
		const TArray<const FJamLine*> Reactions = JamLines::ForTrigger(EJamLineTrigger::NoteTaken);
		C.FenellaReaction = Reactions[FMath::Abs(Seed) % Reactions.Num()]->Text;
		const EJamLineTrigger BubblePool = C.Mood == EJamMood::Sad ? EJamLineTrigger::BubbleSadder : EJamLineTrigger::BubbleDefault;
		for (int32 i = 0; i < 3; ++i) { C.LectureBubbles.Add(Picker.PickFromPool(BubblePool)->Text); }
		for (int32 i = 0; i < 2; ++i) { C.Replies.Add(Picker.PickFromPool(EJamLineTrigger::NuisanceReply)->Text); }
		C.Motif = TEXT("");
		Stage.Objective = Ctx.BaseStage.Objective;

		Out = FJamClampedNote();
		Out.Source = TEXT("keywords");
		Out.Def.Id = TEXT("TypedNote");
		Out.Def.Replacement = Stage;
		Out.Def.GrossMultiplier = GrossFromKnobs(Stage, C.Scale, Ctx.BaseStage);
		Out.Def.BubblePool = TEXT("TypedNote");
		Out.Cosmetics = MoveTemp(C);
		return true;
	}
}
