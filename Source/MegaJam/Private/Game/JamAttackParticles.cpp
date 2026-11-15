#include "Game/JamAttackParticles.h"
#include "Game/JamHero.h"
#include "Game/JamNuisance.h"
#include "Misc/CommandLine.h"
#include "Misc/Parse.h"
#include "Game/JamGameState.h"
#include "NiagaraComponent.h"
#include "NiagaraSystem.h"
#include "Components/PointLightComponent.h"
#include "Components/SceneComponent.h"
#include "Engine/World.h"
#include "EngineUtils.h"
#include "MegaJam.h"

AJamAttackParticles::AJamAttackParticles()
{
    PrimaryActorTick.bCanEverTick = true;
    SetRootComponent(CreateDefaultSubobject<USceneComponent>(TEXT("Root")));
    SetActorEnableCollision(false);
}

void AJamAttackParticles::Spawn(AJamHero* Hero, EJamSkill Skill, const FJamSkillRank& Rank, const FVector& Floor)
{
    if (!Hero || !Hero->GetWorld()) { return; }
    // Each row presents the four existing skills using that cast member's props.
    static const TCHAR* Presets[4][4] = {
        { TEXT("NS_K01_PoliteShield"), TEXT("NS_K02_SternInstruction"), TEXT("NS_FX_Impact"), TEXT("NS_K03_GoodManners") },
        { TEXT("NS_M01_SpoonfulOfSugar"), TEXT("NS_M03_ArcaneSanitization"), TEXT("NS_FX_Impact"), TEXT("NS_M02_PeppermintPopfall") },
        { TEXT("NS_A01_CeleryShot"), TEXT("NS_A03_FlossSnare"), TEXT("NS_FX_Impact"), TEXT("NS_A02_CeleryVolley") },
        { TEXT("NS_B01_FuriousKneading"), TEXT("NS_B03_AggressiveEncouragement"), TEXT("NS_FX_Impact"), TEXT("NS_B02_RollingPinCyclone") }
    };
    const int32 Index = FMath::Clamp(Hero->GetCastIndex(), 0, 3);
    if (Skill >= EJamSkill::Count) { return; }
    const FString Name = Presets[Index][static_cast<int32>(Skill)];
    FActorSpawnParameters Params;
    Params.Owner = Hero;
    auto* Fx = Hero->GetWorld()->SpawnActor<AJamAttackParticles>(Floor, Hero->GetActorRotation(), Params);
    if (!Fx) { return; }
    Fx->Caster = Hero;
    Fx->CastIndex = Index;
    Fx->Started = Hero->GetWorld()->GetTimeSeconds();
    const AJamGameState* SpawnState = Hero->GetWorld()->GetGameState<AJamGameState>();
    Fx->bSpawnedDuringTake = SpawnState && SpawnState->GetShoot().GetState() == EJamShootState::Rolling;
    Fx->bPresentationReview = FParse::Param(FCommandLine::Get(), TEXT("classreview")) || FParse::Value(FCommandLine::Get(), TEXT("castreview="), Fx->CastIndex);
    Fx->CastIndex = Index;
    // Fixes4 #6: the barbarian's cyclone cloud lingers for 4.2 s, so its actor must outlive it.
    const bool bLongCloud = Name == TEXT("NS_B02_RollingPinCyclone");
    Fx->SetLifeSpan(Fx->bPresentationReview ? 40.f : bLongCloud ? 4.8f : 3.6f);
    // Fixes3 #2: each class's own system, every authored layer (up to three), at its authored prop cap.
    // Fixes3 #2: flashes. A bright light in the class colour pops at the cast and fades in a quarter second;
    // ultimates pop again when their big props land.
    if (Name != TEXT("NS_FX_Impact"))
    {
        static const FLinearColor ClassFlash[] = { FLinearColor(1.f, 0.85f, 0.35f), FLinearColor(1.f, 0.35f, 0.75f), FLinearColor(0.45f, 1.f, 0.3f), FLinearColor(1.f, 0.6f, 0.25f) };
        auto* Light = NewObject<UPointLightComponent>(Fx);
        Light->SetupAttachment(Fx->GetRootComponent());
        Fx->AddInstanceComponent(Light);
        Light->SetRelativeLocation(FVector(Rank.Length > 0.f ? FMath::Min(Rank.Length * 0.4f, 500.f) : 150.f, 0.f, 180.f));
        Light->SetIntensityUnits(ELightUnits::Candelas);
        // Fixes4 #5-8: the flashes blinded the player. About a fifth of the old brightness, a tighter reach
        // (the archer's tightest), and the ultimate's second pop no brighter than its first.
        static const float ClassBright[] = { 0.9f, 0.8f, 0.7f, 0.75f };
        static const float ClassReach[] = { 0.8f, 0.8f, 0.55f, 0.8f };
        Light->SetAttenuationRadius(FMath::Max(Rank.Radius, Rank.Length) * ClassReach[Index] + 450.f);
        Light->SetLightColor(ClassFlash[Index]);
        Light->SetCastShadows(false);
        Light->RegisterComponent();
        Fx->Flash = Light;
        Fx->FlashPeak = (Skill == EJamSkill::LastJudgment ? 26000.f : 12000.f) * ClassBright[Index];
        if (Skill == EJamSkill::LastJudgment) { Fx->SecondFlashAt = Index == 1 ? 2.0f : Index == 2 ? 1.1f : Index == 3 ? 1.3f : 0.4f; }
        Light->SetIntensity(Fx->FlashPeak);
    }
    // Fixes3 #2: the giant peppermints of Peppermint Popfall home in, one dropped on each nearby nuisance.
    TArray<FVector> HomingTargets;
    if (Name == TEXT("NS_M02_PeppermintPopfall"))
    {
        TArray<AJamNuisance*> Near;
        const float Reach = FMath::Max(Rank.Radius, 400.f) + 600.f;
        for (TActorIterator<AJamNuisance> It(Hero->GetWorld()); It; ++It)
        {
            if (!It->IsPacified() && !It->IsTreated() && FVector::Dist2D(It->GetActorLocation(), Floor) <= Reach) { Near.Add(*It); }
        }
        Near.Sort([Floor](const AJamNuisance& A, const AJamNuisance& B) { return FVector::DistSquared2D(A.GetActorLocation(), Floor) < FVector::DistSquared2D(B.GetActorLocation(), Floor); });
        for (int32 i = 0; i < FMath::Min(5, Near.Num()); ++i) { HomingTargets.Add(Near[i]->GetActorLocation()); }
    }
    const AJamGameState* GS = Hero->GetWorld()->GetGameState<AJamGameState>();
    auto AddLayer = [&](UNiagaraSystem* System, const FVector* At, int32 PropCount, float Radius)
    {
        auto* Component = NewObject<UNiagaraComponent>(Fx);
        Component->SetAutoActivate(false);
        Component->SetupAttachment(Fx->GetRootComponent());
        Fx->AddInstanceComponent(Component);
        Component->SetAsset(System);
        Component->RegisterComponent();
        if (At) { Component->SetWorldLocation(FVector(At->X, At->Y, Floor.Z)); }
        const float Extent = FMath::Max(1000.f, FMath::Max(Rank.Radius, Rank.Length) + 200.f);
        Component->SetSystemFixedBounds(FBox(FVector(-Extent, -Extent, -100.f), FVector(Extent, Extent, 1800.f)));
        Component->SetVariableInt(TEXT("User.PropCount"), PropCount);   // each emitter clamps to its own authored cap
        Component->SetVariableFloat(TEXT("User.Radius"), Radius);
        Component->SetVariableFloat(TEXT("User.Length"), FMath::Max(100.f, Rank.Length > 0.f ? Rank.Length : Rank.Radius));
        Component->SetVariableFloat(TEXT("User.HalfWidth"), Rank.Width * 0.5f);
        Component->SetVariableBool(TEXT("User.BudgetOut"), GS && GS->bBudgetOut);
        if (Fx->bPresentationReview) { Component->SetForceSolo(true); }
        Component->Activate(true);
        if (Fx->bPresentationReview) { Component->AdvanceSimulation(12, 1.f / 60.f); Component->SetPaused(true); }
        Fx->ParticleComponents.Add(Component);
    };
    for (int32 Layer = 0; Layer < 4; ++Layer)
    {
        const FString AssetName = Layer == 0 ? Name : FString::Printf(TEXT("%s_Layer%d"), *Name, Layer);
        const FString Path = FString::Printf(TEXT("/Game/FX/Attacks/Systems/%s.%s"), *AssetName, *AssetName);
        UNiagaraSystem* System = LoadObject<UNiagaraSystem>(nullptr, *Path, nullptr, LOAD_NoWarn | LOAD_Quiet);
        if (!System) { if (Layer == 0) { UE_LOG(LogJam, Warning, TEXT("AttackParticles: missing %s"), *Path); } continue; }
        if (Layer == 1 && HomingTargets.Num() > 0)
        {
            for (const FVector& Target : HomingTargets) { AddLayer(System, &Target, 1, 10.f); }
            continue;
        }
        AddLayer(System, nullptr, 1000, FMath::Max(100.f, Rank.Radius));
    }
    UE_LOG(LogJam, Log, TEXT("AttackParticles: class=%d skill=%d preset=%s layers=%d"), Index, static_cast<int32>(Skill), *Name, Fx->ParticleComponents.Num());
}

void AJamAttackParticles::Tick(float DeltaSeconds)
{
    Super::Tick(DeltaSeconds);
    const AJamGameState* GS = GetWorld()->GetGameState<AJamGameState>();
    if (bMouthFoam)
    {
        if (!Brushee.IsValid()) { Destroy(); }
        return;
    }
    if (!Caster.IsValid() || Caster->GetCastIndex() != CastIndex ||
        (!bPresentationReview && bSpawnedDuringTake && GS && (GS->GetShoot().GetState() != EJamShootState::Rolling || GS->TakeFailedAt >= Started)))
    { Destroy(); return; }
    for (const auto& Component : ParticleComponents) { if (Component) { Component->SetVariableBool(TEXT("User.BudgetOut"), GS && GS->bBudgetOut); } }
    if (Flash)
    {
        const float Age = GetWorld()->GetTimeSeconds() - Started;
        float Glow = FMath::Exp(-Age * 11.f);
        if (SecondFlashAt > 0.f && Age >= SecondFlashAt) { Glow = FMath::Max(Glow, 0.8f * FMath::Exp(-(Age - SecondFlashAt) * 9.f)); }
        Flash->SetIntensity(FlashPeak * Glow);
    }
}

void AJamAttackParticles::EndPlay(const EEndPlayReason::Type Reason)
{
    for (const auto& Component : ParticleComponents)
    {
        if (Component) { Component->SetVariableBool(TEXT("User.Stop"), true); Component->DeactivateImmediate(); Component->DestroyComponent(); }
    }
    Super::EndPlay(Reason);
}

void AJamAttackParticles::SpawnBrushFoam(AJamHero* Hero, AJamNuisance* Target, const FVector& Mouth)
{
    if (!Hero || !Target) { return; }
    const float Size = Target->GetActorScale3D().GetAbsMax();
    FActorSpawnParameters Params;
    Params.Owner = Target;
    auto* Fx = Hero->GetWorld()->SpawnActor<AJamAttackParticles>(Mouth - FVector(0.f, 0.f, 20.f * Size), Target->GetActorRotation(), Params);
    if (!Fx) { return; }
    Fx->Caster = Hero;
    Fx->Brushee = Target;
    Fx->bMouthFoam = true;
    Fx->SetLifeSpan(1.2f);
    Fx->AttachToActor(Target, FAttachmentTransformRules::KeepWorldTransform);
    for (const TCHAR* Name : { TEXT("NS_M03_ArcaneSanitization"), TEXT("NS_M03_ArcaneSanitization_Layer1") })
    {
        const FString Path = FString::Printf(TEXT("/Game/FX/Attacks/Systems/%s.%s"), Name, Name);
        UNiagaraSystem* System = LoadObject<UNiagaraSystem>(nullptr, *Path);
        if (!System) { continue; }
        auto* Component = NewObject<UNiagaraComponent>(Fx);
        Component->SetAutoActivate(false);
        Component->SetupAttachment(Fx->GetRootComponent());
        Fx->AddInstanceComponent(Component);
        Component->SetAsset(System);
        Component->RegisterComponent();
        Component->SetRelativeScale3D(FVector(0.25f * Size));
        Component->SetVariableFloat(TEXT("User.Length"), 60.f);
        Component->SetVariableFloat(TEXT("User.Duration"), 1.2f);
        Component->SetVariableInt(TEXT("User.PropCount"), 6);
        Component->SetSystemFixedBounds(FBox(FVector(-200.f), FVector(300.f)));
        Component->Activate(true);
        Fx->ParticleComponents.Add(Component);
    }
    UE_LOG(LogJam, Log, TEXT("BrushFoam: %s layers=%d"), *Target->GetName(), Fx->ParticleComponents.Num());
}
