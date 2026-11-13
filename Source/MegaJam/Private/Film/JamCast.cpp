#include "Film/JamCast.h"

namespace
{
	FJamClassDef Def(const TCHAR* Id, const TCHAR* Name, const TCHAR* Rig, const TCHAR* Blurb, std::initializer_list<const TCHAR*> Cries, bool bFull)
	{
		FJamClassDef D;
		D.Id = Id;
		D.Name = Name;
		D.Rig = Rig;
		D.Blurb = Blurb;
		for (const TCHAR* C : Cries) { D.Cries.Add(C); }
		D.bFullClips = bFull;
		if (D.Id == TEXT("knight")) { D.SkillNames = { TEXT("Polite Shield"), TEXT("Stern Instruction"), TEXT(""), TEXT("Good Manners") }; }
		else if (D.Id == TEXT("mage")) { D.SkillNames = { TEXT("Spoonful of Sugar"), TEXT("Arcane Sanitization"), TEXT(""), TEXT("Peppermint Popfall") }; }
		else if (D.Id == TEXT("archer")) { D.SkillNames = { TEXT("Celery Shot"), TEXT("Floss Snare"), TEXT(""), TEXT("Celery Volley") }; }
		else { D.SkillNames = { TEXT("Furious Kneading"), TEXT("Aggressive Encouragement"), TEXT(""), TEXT("Rolling Pin Cyclone") }; }
		// Temporary art until the class-specific icon delivery replaces these paths.
		D.SkillIcons = { TEXT("/Game/Art/UI/Icons/T_Skill_Lecture"), TEXT("/Game/Art/UI/Icons/T_Skill_FormALine"), TEXT(""), TEXT("/Game/Art/UI/Icons/T_Skill_LastJudgment") };
		return D;
	}
}

const TArray<FJamClassDef>& JamCast::All()
{
	// Blurbs: Docs/Design/Jokes.md, "Casting".
	static const TArray<FJamClassDef> Cast = {
		Def(TEXT("knight"), TEXT("The Knight"), TEXT("Knight"), TEXT("Brought his own sword. Has notes about it."), { TEXT("K01"), TEXT("K02"), TEXT("K03") }, true),
		Def(TEXT("mage"), TEXT("The Mage"), TEXT("Mage"), TEXT("Was promised a spell budget. Read the contract."), { TEXT("M01"), TEXT("M02"), TEXT("M03") }, false),
		Def(TEXT("archer"), TEXT("The Archer"), TEXT("Archer"), TEXT("No bow. Props said it was in the van."), { TEXT("A01"), TEXT("A02"), TEXT("A03") }, false),
		Def(TEXT("barbarian"), TEXT("The Barbarian"), TEXT("Barbarian"), TEXT("Has been spoken to about his volume."), { TEXT("B01"), TEXT("B03") }, false),
	};
	return Cast;
}

int32 JamCast::IndexOf(const FString& Id)
{
	return All().IndexOfByPredicate([&Id](const FJamClassDef& D) { return D.Id.Equals(Id, ESearchCase::IgnoreCase); });
}

const FJamClassDef& JamCast::Get(int32 Index)
{
	return All()[FMath::Clamp(Index, 0, All().Num() - 1)];
}

int32 JamCast::Step(int32 Index, int32 Delta)
{
	const int32 N = All().Num();
	return ((FMath::Clamp(Index, 0, N - 1) + Delta) % N + N) % N;
}

FString JamCast::CryFor(const FJamClassDef& Def, int32 CastCount)
{
	if (Def.Cries.Num() == 0) { return FString(); }
	return Def.Cries[FMath::Abs(CastCount) % Def.Cries.Num()];
}
