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

        auto Colour = [](const TCHAR* Hex) { return FLinearColor::FromSRGBColor(FColor::FromHex(Hex)); };
        const FLinearColor Manners = Colour(TEXT("F0C65D"));
        const FLinearColor Sweetness = Colour(TEXT("FF8FAB"));
        const FLinearColor Hygiene = Colour(TEXT("4AD9D1"));
        const FLinearColor Nutrition = Colour(TEXT("B8D94A"));
        const FLinearColor Domestic = Colour(TEXT("E8D2A0"));
        if (D.Id == TEXT("knight"))
        {
            D.SkillIcons = { TEXT("/Game/Art/UI/Skills/T_Skill_K01"), TEXT("/Game/Art/UI/Skills/T_Skill_K02"), TEXT(""), TEXT("/Game/Art/UI/Skills/T_Skill_K03") };
            D.SkillAnimations = { TEXT("KnightShield"), TEXT("SwordRaise"), TEXT(""), TEXT("KnightWard") };
            D.PrimaryColor = Colour(TEXT("4778D6"));
            D.SkillDamageColors = { Manners, Manners, Manners, Manners };
        }
        else if (D.Id == TEXT("mage"))
        {
            D.SkillIcons = { TEXT("/Game/Art/UI/Skills/T_Skill_M01"), TEXT("/Game/Art/UI/Skills/T_Skill_M03"), TEXT(""), TEXT("/Game/Art/UI/Skills/T_Skill_M02") };
            D.SkillAnimations = { TEXT("MageLecture"), TEXT("MageLine"), TEXT(""), TEXT("MageJudgment") };
            D.PrimaryColor = Colour(TEXT("8D5BDB"));
            D.SkillDamageColors = { Sweetness, Hygiene, Sweetness, Sweetness };
            D.BackOffset = FVector(-38.f, -18.f, 30.f);
            D.BackAxis = FVector(0.f, 0.3f, 0.95f);
        }
        else if (D.Id == TEXT("archer"))
        {
            D.SkillIcons = { TEXT("/Game/Art/UI/Skills/T_Skill_A01"), TEXT("/Game/Art/UI/Skills/T_Skill_A03"), TEXT(""), TEXT("/Game/Art/UI/Skills/T_Skill_A02") };
            D.SkillAnimations = { TEXT("ArcherShot"), TEXT("ArcherLine"), TEXT(""), TEXT("ArcherJudgment") };
            D.PrimaryColor = Colour(TEXT("3C9D68"));
            D.SkillDamageColors = { Nutrition, Hygiene, Nutrition, Nutrition };
            D.WeaponHand = TEXT("hand_l");
            // Turn about the bow's long axis so its limbs face forward without inverting it.
            D.WeaponGrip = (FRotator(0.f, 0.f, -90.f).Quaternion() * FQuat(FVector::UpVector, PI)).Rotator();
            D.BackOffset = FVector(-35.f, 0.f, 25.f);
            D.BackAxis = FVector(0.f, -0.35f, 0.94f);
            D.BackTwist = 90.f;
        }
        else
        {
            D.SkillIcons = { TEXT("/Game/Art/UI/Skills/T_Skill_B01"), TEXT("/Game/Art/UI/Skills/T_Skill_B03"), TEXT(""), TEXT("/Game/Art/UI/Skills/T_Skill_B02") };
            D.SkillAnimations = { TEXT("BarbarianKnead"), TEXT("BarbarianBellow"), TEXT(""), TEXT("BarbarianCyclone") };
            D.PrimaryColor = Colour(TEXT("C65736"));
            D.SkillDamageColors = { Domestic, Manners, Domestic, Domestic };
            D.BackOffset = FVector(-40.f, 16.f, -10.f);
            D.BackAxis = FVector(0.f, -0.45f, 0.89f);
        }
		return D;
	}
}

const TArray<FJamClassDef>& JamCast::All()
{
	// Blurbs: Docs/Design/Jokes.md, "Casting".
	static const TArray<FJamClassDef> Cast = {
		Def(TEXT("knight"), TEXT("The Knight"), TEXT("Knight"), TEXT("Brought his own sword. Has notes about it."), { TEXT("K01"), TEXT("K02"), TEXT("K03") }, true),
		Def(TEXT("mage"), TEXT("The Mage"), TEXT("Mage"), TEXT("Was promised a spell budget. Read the contract."), { TEXT("M01"), TEXT("M02"), TEXT("M03") }, false),
		Def(TEXT("archer"), TEXT("The Archer"), TEXT("Archer"), TEXT("Brought her bow. Props still has the arrows."), { TEXT("A01"), TEXT("A02"), TEXT("A03") }, false),
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
