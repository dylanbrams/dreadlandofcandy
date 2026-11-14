#include "Game/JamAttackParticles.h"
#include "Game/JamHero.h"
#include "Game/JamNuisance.h"
#include "Misc/CommandLine.h"
#include "Misc/Parse.h"
#include "Game/JamGameState.h"
#include "NiagaraComponent.h"
#include "NiagaraSystem.h"
#include "Components/SceneComponent.h"
#include "Engine/World.h"
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
    Fx->SetLifeSpan(Fx->bPresentationReview ? 40.f : 3.6f);
    for (int32 Layer = 0; Layer < 2; ++Layer)
    {
        const FString AssetName = Name + (Layer == 1 ? TEXT("_Layer1") : TEXT(""));
        const FString Path = FString::Printf(TEXT("/Game/FX/Attacks/Systems/%s.%s"), *AssetName, *AssetName);
        UNiagaraSystem* System = LoadObject<UNiagaraSystem>(nullptr, *Path, nullptr, LOAD_NoWarn | LOAD_Quiet);
        if (!System) { if (Layer == 0) { UE_LOG(LogJam, Warning, TEXT("AttackParticles: missing %s"), *Path); } continue; }
        auto* Component = NewObject<UNiagaraComponent>(Fx);
        Component->SetAutoActivate(false);
        Component->SetupAttachment(Fx->GetRootComponent());
        Fx->AddInstanceComponent(Component);
        Component->SetAsset(System);
        Component->RegisterComponent();
        const float Extent = FMath::Max(1000.f, FMath::Max(Rank.Radius, Rank.Length) + 200.f);
        Component->SetSystemFixedBounds(FBox(FVector(-Extent, -Extent, -100.f), FVector(Extent, Extent, 900.f)));
        Component->SetVariableFloat(TEXT("User.Radius"), FMath::Max(100.f, Rank.Radius));
        Component->SetVariableFloat(TEXT("User.Length"), FMath::Max(100.f, Rank.Length > 0.f ? Rank.Length : Rank.Radius));
        Component->SetVariableFloat(TEXT("User.HalfWidth"), Rank.Width * 0.5f);
        const AJamGameState* GS = Hero->GetWorld()->GetGameState<AJamGameState>();
        Component->SetVariableBool(TEXT("User.BudgetOut"), GS && GS->bBudgetOut);
        if (Fx->bPresentationReview) { Component->SetForceSolo(true); }
        Component->Activate(true);
        if (Fx->bPresentationReview) { Component->AdvanceSimulation(12, 1.f / 60.f); Component->SetPaused(true); }
        Fx->ParticleComponents.Add(Component);
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
