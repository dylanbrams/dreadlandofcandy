#include "Misc/AutomationTest.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "AbilitySystemGlobals.h"
#include "Gas/JamAoePatternLibrary.h"
#include "Gas/JamGameplayAbility.h"
#include "Gas/JamGameplayEffects.h"
#include "Gas/JamGameplayTags.h"
#include "Gas/JamResolveAttributeSet.h"
#include "Testing/JamAbilityTestRig.h"
#include "Game/JamPlayerState.h"

#if WITH_DEV_AUTOMATION_TESTS

#define JamFlags (EAutomationTestFlags::EditorContext | EAutomationTestFlags::ClientContext | EAutomationTestFlags::EngineFilter)

namespace
{
	// A bare game world for tests that need actors. Destroyed by the caller via DestroyTestWorld.
	UWorld* CreateTestWorld()
	{
		UWorld* World = UWorld::CreateWorld(EWorldType::Game, false);
		FWorldContext& Context = GEngine->CreateNewWorldContext(EWorldType::Game);
		Context.SetCurrentWorld(World);
		World->InitializeActorsForPlay(FURL());
		return World;
	}

	void DestroyTestWorld(UWorld* World)
	{
		GEngine->DestroyWorldContext(World);
		World->DestroyWorld(false);
	}

	AJamTestCombatant* SpawnCombatant(UWorld* World, const FVector& Location, FGameplayTag Team)
	{
		AJamTestCombatant* A = World->SpawnActorDeferred<AJamTestCombatant>(
			AJamTestCombatant::StaticClass(), FTransform(Location), nullptr, nullptr,
			ESpawnActorCollisionHandlingMethod::AlwaysSpawn);
		A->TeamTag = Team;
		A->FinishSpawning(FTransform(Location));
		return A;
	}
}

// ---- Pure geometry: no world, no renderer -------------------------------------------------------------

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FJamAoeShapesTest, "Jam.Gas.AoeShapes", JamFlags)
bool FJamAoeShapesTest::RunTest(const FString& Parameters)
{
	const FVector Origin = FVector::ZeroVector;
	const FVector Forward = FVector::ForwardVector;

	FJamAoePatternParams Cone;
	Cone.Shape = EJamAoeShape::Cone;
	Cone.Radius = 800.f;
	Cone.ConeHalfAngleDeg = 35.f;
	TestTrue(TEXT("cone: dead ahead"),            UJamAoePatternLibrary::TestPoint(Cone, Origin, Forward, FVector(500, 0, 0)));
	TestTrue(TEXT("cone: inside the half angle"), UJamAoePatternLibrary::TestPoint(Cone, Origin, Forward, FVector(500, 300, 0)));
	TestFalse(TEXT("cone: outside the half angle"), UJamAoePatternLibrary::TestPoint(Cone, Origin, Forward, FVector(300, 500, 0)));
	TestFalse(TEXT("cone: beyond the range"),     UJamAoePatternLibrary::TestPoint(Cone, Origin, Forward, FVector(900, 0, 0)));
	TestFalse(TEXT("cone: behind the caster"),    UJamAoePatternLibrary::TestPoint(Cone, Origin, Forward, FVector(-300, 0, 0)));
	// Facing +Y: the same world point that was ahead is now off to the side.
	TestFalse(TEXT("cone: rotates with the caster"), UJamAoePatternLibrary::TestPoint(Cone, Origin, FVector::RightVector, FVector(500, 0, 0)));
	TestTrue(TEXT("cone: rotated caster sees +Y"), UJamAoePatternLibrary::TestPoint(Cone, Origin, FVector::RightVector, FVector(0, 500, 0)));

	FJamAoePatternParams Ring;
	Ring.Shape = EJamAoeShape::Ring;
	Ring.Radius = 400.f;
	Ring.InnerRadius = 200.f;
	TestFalse(TEXT("ring: hole in the middle"), UJamAoePatternLibrary::TestPoint(Ring, Origin, Forward, FVector(100, 0, 0)));
	TestTrue(TEXT("ring: on the band"),         UJamAoePatternLibrary::TestPoint(Ring, Origin, Forward, FVector(300, 0, 0)));
	TestFalse(TEXT("ring: outside"),            UJamAoePatternLibrary::TestPoint(Ring, Origin, Forward, FVector(500, 0, 0)));

	FJamAoePatternParams Line;
	Line.Shape = EJamAoeShape::Line;
	Line.Radius = 600.f;
	Line.Width = 100.f;
	TestTrue(TEXT("line: in the lane"),     UJamAoePatternLibrary::TestPoint(Line, Origin, Forward, FVector(400, 50, 0)));
	TestFalse(TEXT("line: outside the lane"), UJamAoePatternLibrary::TestPoint(Line, Origin, Forward, FVector(400, 200, 0)));

	FJamAoePatternParams Holey;
	Holey.Shape = EJamAoeShape::Circle;
	Holey.Radius = 500.f;
	FJamAoeHole Hole;
	Hole.LocalOffset = FVector2D(200.f, 0.f);
	Hole.Radius = 50.f;
	Holey.StaticHoles.Add(Hole);
	TestFalse(TEXT("hole: safe spot is not hit"), UJamAoePatternLibrary::TestPoint(Holey, Origin, Forward, FVector(200, 0, 0)));
	TestTrue(TEXT("hole: elsewhere is hit"),      UJamAoePatternLibrary::TestPoint(Holey, Origin, Forward, FVector(-200, 0, 0)));
	return true;
}

// ---- Resolve damage and the once-only pacify ----------------------------------------------------------

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FJamResolveDamageTest, "Jam.Gas.ResolveDamage", JamFlags)
bool FJamResolveDamageTest::RunTest(const FString& Parameters)
{
	UAbilitySystemGlobals::Get().InitGlobalData();
	UWorld* World = CreateTestWorld();

	AJamTestCombatant* Hero = SpawnCombatant(World, FVector::ZeroVector, JamGameplayTags::Team_Hero);
	AJamTestCombatant* Target = SpawnCombatant(World, FVector(300, 0, 0), JamGameplayTags::Team_Nuisance);
	UAbilitySystemComponent* HeroASC = Hero->GetAbilitySystemComponent();
	UAbilitySystemComponent* TargetASC = Target->GetAbilitySystemComponent();

	int32 PacifiedEvents = 0;
	Target->GetResolveSet()->OnPacified.AddLambda([&PacifiedEvents](AActor*) { ++PacifiedEvents; });

	auto Hit = [&](float Base)
	{
		FGameplayEffectSpecHandle Spec = HeroASC->MakeOutgoingSpec(UJamGE_ResolveDamage::StaticClass(), 1.f, HeroASC->MakeEffectContext());
		Spec.Data->SetSetByCallerMagnitude(JamGameplayTags::SetByCaller_Resolve_Base, Base);
		HeroASC->ApplyGameplayEffectSpecToTarget(*Spec.Data, TargetASC);
	};

	TestEqual(TEXT("starts at full Resolve"), Target->GetResolve(), 100.f);
	Hit(30.f);
	TestEqual(TEXT("30 Resolve damage"), Target->GetResolve(), 70.f);
	TestFalse(TEXT("not pacified yet"), TargetASC->HasMatchingGameplayTag(JamGameplayTags::State_Pacified));

	// ResolveReduction is subtracted from every hit by the magnitude calculation.
	TargetASC->SetNumericAttributeBase(UJamResolveAttributeSet::GetResolveReductionAttribute(), 10.f);
	Hit(30.f);
	TestEqual(TEXT("reduction of 10 turns a 30 hit into 20"), Target->GetResolve(), 50.f);

	Hit(500.f);
	TestEqual(TEXT("Resolve clamps at zero"), Target->GetResolve(), 0.f);
	TestTrue(TEXT("pacified at zero"), TargetASC->HasMatchingGameplayTag(JamGameplayTags::State_Pacified));
	TestEqual(TEXT("pacify broadcast once"), PacifiedEvents, 1);

	Hit(50.f);
	TestEqual(TEXT("a further hit does not re-pacify"), PacifiedEvents, 1);
	TestEqual(TEXT("Resolve stays at zero"), Target->GetResolve(), 0.f);

	DestroyTestWorld(World);
	return true;
}

// ---- The Lecture cone end to end: filter, cooldown, skip-pacified -------------------------------------

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FJamLectureConeTest, "Jam.Gas.LectureCone", JamFlags)
bool FJamLectureConeTest::RunTest(const FString& Parameters)
{
	UAbilitySystemGlobals::Get().InitGlobalData();
	UWorld* World = CreateTestWorld();

	AJamTestCombatant* Hero = SpawnCombatant(World, FVector::ZeroVector, JamGameplayTags::Team_Hero);
	AJamTestCombatant* InCone   = SpawnCombatant(World, FVector(500, 0, 0),   JamGameplayTags::Team_Nuisance);
	AJamTestCombatant* TooWide  = SpawnCombatant(World, FVector(100, 600, 0), JamGameplayTags::Team_Nuisance);
	AJamTestCombatant* Behind   = SpawnCombatant(World, FVector(-300, 0, 0),  JamGameplayTags::Team_Nuisance);
	AJamTestCombatant* Friendly = SpawnCombatant(World, FVector(300, 50, 0),  JamGameplayTags::Team_Hero);

	UAbilitySystemComponent* ASC = Hero->GetAbilitySystemComponent();
	const FGameplayAbilitySpecHandle Handle = ASC->GiveAbility(FGameplayAbilitySpec(UJamGA_LectureCone::StaticClass(), 1));

	TestTrue(TEXT("first cast activates"), ASC->TryActivateAbility(Handle));
	TestEqual(TEXT("target in the cone takes 60"), InCone->GetResolve(), 40.f);
	TestEqual(TEXT("target outside the angle untouched"), TooWide->GetResolve(), 100.f);
	TestEqual(TEXT("target behind untouched"), Behind->GetResolve(), 100.f);
	TestEqual(TEXT("same-team actor in the cone untouched"), Friendly->GetResolve(), 100.f);
	TestTrue(TEXT("cooldown tag granted"), ASC->HasMatchingGameplayTag(JamGameplayTags::Cooldown_Lecture));

	TestFalse(TEXT("second cast refused during cooldown"), ASC->TryActivateAbility(Handle));
	TestEqual(TEXT("refused cast does no damage"), InCone->GetResolve(), 40.f);

	// Clear the cooldown the way time would, then cast again.
	FGameplayTagContainer CooldownTags;
	CooldownTags.AddTag(JamGameplayTags::Cooldown_Lecture);
	ASC->RemoveActiveEffectsWithGrantedTags(CooldownTags);
	TestTrue(TEXT("cast after cooldown activates"), ASC->TryActivateAbility(Handle));
	TestEqual(TEXT("second hit pacifies"), InCone->GetResolve(), 0.f);
	TestTrue(TEXT("pacified tag set"), InCone->GetAbilitySystemComponent()->HasMatchingGameplayTag(JamGameplayTags::State_Pacified));

	ASC->RemoveActiveEffectsWithGrantedTags(CooldownTags);
	TestTrue(TEXT("third cast still activates (nothing left is a bug, not a refusal)"), ASC->TryActivateAbility(Handle));
	if (const FGameplayAbilitySpec* Spec = ASC->FindAbilitySpecFromHandle(Handle))
	{
		const UJamGA_AoeAttack* Ability = Cast<UJamGA_AoeAttack>(Spec->GetPrimaryInstance());
		TestTrue(TEXT("ability instance found"), Ability != nullptr);
		if (Ability) { TestEqual(TEXT("pacified targets are skipped"), Ability->LastHitTargets.Num(), 0); }
	}

	DestroyTestWorld(World);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FJamSkillsTest, "Jam.Gas.Skills", JamFlags)
bool FJamSkillsTest::RunTest(const FString& Parameters)
{
	// Skills 2 to 4 at rank 1 (no PlayerState: the rig may cast anything; numbers come from the rank table).
	UAbilitySystemGlobals::Get().InitGlobalData();
	UWorld* World = CreateTestWorld();
	AJamTestCombatant* Hero = SpawnCombatant(World, FVector::ZeroVector, JamGameplayTags::Team_Hero);
	UAbilitySystemComponent* ASC = Hero->GetAbilitySystemComponent();
	const FJamSkillRank Line = JamProgression::RankData(EJamSkill::FormALine, 1);
	const FJamSkillRank Sigh = JamProgression::RankData(EJamSkill::DisappointedSigh, 1);

	// Form A Line: a rectangle ahead, Length long and Width wide in total.
	AJamTestCombatant* OnLine  = SpawnCombatant(World, FVector(Line.Length * 0.8f, 0, 0), JamGameplayTags::Team_Nuisance);
	AJamTestCombatant* Beside  = SpawnCombatant(World, FVector(Line.Length * 0.5f, Line.Width * 0.5f + 60.f, 0), JamGameplayTags::Team_Nuisance);
	AJamTestCombatant* Behind  = SpawnCombatant(World, FVector(-200, 0, 0), JamGameplayTags::Team_Nuisance);
	const FGameplayAbilitySpecHandle LineHandle = ASC->GiveAbility(FGameplayAbilitySpec(UJamGA_FormALine::StaticClass(), 1));
	TestTrue(TEXT("line casts"), ASC->TryActivateAbility(LineHandle));
	TestEqual(TEXT("on the line takes its damage"), OnLine->GetResolve(), 100.f - Line.Damage);
	TestEqual(TEXT("beside the line untouched"), Beside->GetResolve(), 100.f);
	TestEqual(TEXT("behind untouched"), Behind->GetResolve(), 100.f);
	TestTrue(TEXT("line has its own cooldown"), ASC->HasMatchingGameplayTag(JamGameplayTags::Cooldown_Line));
	TestFalse(TEXT("and not the Lecture's"), ASC->HasMatchingGameplayTag(JamGameplayTags::Cooldown_Lecture));
	TestFalse(TEXT("line refused on cooldown"), ASC->TryActivateAbility(LineHandle));

	// The Sigh: a ring around the hero, behind included.
	const FGameplayAbilitySpecHandle SighHandle = ASC->GiveAbility(FGameplayAbilitySpec(UJamGA_DisappointedSigh::StaticClass(), 1));
	const float BehindBefore = Behind->GetResolve();
	TestTrue(TEXT("sigh casts while the line cools down"), ASC->TryActivateAbility(SighHandle));
	TestEqual(TEXT("sigh reaches behind"), Behind->GetResolve(), BehindBefore - Sigh.Damage);
	TestTrue(TEXT("sigh radius is smaller than the line is long"), Sigh.Radius < Line.Length * 0.8f);
	TestEqual(TEXT("far target outside the sigh"), OnLine->GetResolve(), 100.f - Line.Damage);
	TestTrue(TEXT("sigh cooldown"), ASC->HasMatchingGameplayTag(JamGameplayTags::Cooldown_Sigh));

	// The Last Judgment: the big ring catches everything here.
	const FGameplayAbilitySpecHandle JudgmentHandle = ASC->GiveAbility(FGameplayAbilitySpec(UJamGA_LastJudgment::StaticClass(), 1));
	TestTrue(TEXT("judgment casts"), ASC->TryActivateAbility(JudgmentHandle));
	TestEqual(TEXT("judgment pacifies the far one"), OnLine->GetResolve(), 0.f);
	TestTrue(TEXT("judgment cooldown"), ASC->HasMatchingGameplayTag(JamGameplayTags::Cooldown_Judgment));

	DestroyTestWorld(World);
	return true;
}

// Diagnostic/regression for the actual PlayerState-owned cooldown path used by AJamHUD.
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FJamCooldownHudReadTest, "Jam.Gas.CooldownHudRead", JamFlags)
bool FJamCooldownHudReadTest::RunTest(const FString&)
{
    UAbilitySystemGlobals::Get().InitGlobalData();
    UWorld* World = CreateTestWorld();
    AJamTestCombatant* Avatar = SpawnCombatant(World, FVector::ZeroVector, JamGameplayTags::Team_Hero);
    AJamPlayerState* Player = World->SpawnActor<AJamPlayerState>();
    Player->InitForAvatar(Avatar);
    UAbilitySystemComponent* ASC = Player->GetAbilitySystemComponent();
    const EJamSkill Skills[] = { EJamSkill::Lecture, EJamSkill::FormALine, EJamSkill::LastJudgment };
    const FGameplayTag Tags[] = { JamGameplayTags::Cooldown_Lecture, JamGameplayTags::Cooldown_Line, JamGameplayTags::Cooldown_Judgment };
    for (int32 i = 0; i < UE_ARRAY_COUNT(Skills); ++i)
    {
        const float Expected = Player->GetProgression().GetSkillNumbers(Skills[i]).Cooldown;
        TestTrue(TEXT("starting skill casts on the real owner"), ASC->TryActivateAbility(Player->GetSkillHandle(Skills[i])));
        const FGameplayEffectQuery Query = FGameplayEffectQuery::MakeQuery_MatchAnyOwningTags(FGameplayTagContainer(Tags[i]));
        const TArray<TPair<float, float>> Effects = ASC->GetActiveEffectsTimeRemainingAndDuration(Query);
        TestEqual(TEXT("HUD query finds one cooldown"), Effects.Num(), 1);
        if (Effects.Num() != 1) { continue; }
        AddInfo(FString::Printf(TEXT("%s: remaining=%.3f duration=%.3f expected=%.3f ratio=%.3f"),
            *Tags[i].ToString(), Effects[0].Key, Effects[0].Value, Expected, Effects[0].Key / FMath::Max(Effects[0].Value, SMALL_NUMBER)));
        TestTrue(TEXT("real progression duration survives application"), FMath::IsNearlyEqual(Effects[0].Value, Expected));
        TestTrue(TEXT("newly cast ratio is one"), Effects[0].Value > 0.f && FMath::IsNearlyEqual(Effects[0].Key / Effects[0].Value, 1.f));
    }
    World->Tick(LEVELTICK_All, 0.25f);
    for (const FGameplayTag& Tag : Tags)
    {
        const auto Effects = ASC->GetActiveEffectsTimeRemainingAndDuration(
            FGameplayEffectQuery::MakeQuery_MatchAnyOwningTags(FGameplayTagContainer(Tag)));
        TestTrue(TEXT("cooldown still present after a quarter second"), Effects.Num() == 1);
        if (Effects.Num() == 1) { TestTrue(TEXT("HUD ratio decreases with world time"), Effects[0].Key > 0.f && Effects[0].Key < Effects[0].Value); }
    }
    DestroyTestWorld(World);
    return true;
}

#endif
