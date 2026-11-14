#pragma once
#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Film/JamProgression.h"
#include "JamAttackParticles.generated.h"
class AJamHero;
class AJamNuisance;
class UNiagaraComponent;

/** Cast-owned cosmetic layers. Gameplay hit areas remain in the abilities. */
UCLASS()
class MEGAJAM_API AJamAttackParticles : public AActor
{
    GENERATED_BODY()
public:
    AJamAttackParticles();
    static void Spawn(AJamHero* Hero, EJamSkill Skill, const FJamSkillRank& Rank, const FVector& Floor);
    static void SpawnBrushFoam(AJamHero* Hero, AJamNuisance* Target, const FVector& Mouth);
    virtual void Tick(float DeltaSeconds) override;
    virtual void EndPlay(const EEndPlayReason::Type Reason) override;
private:
    UPROPERTY(Transient) TArray<TObjectPtr<UNiagaraComponent>> ParticleComponents;
    TWeakObjectPtr<AJamHero> Caster;
    int32 CastIndex = 0;
    float Started = 0.f;
    TWeakObjectPtr<AJamNuisance> Brushee;
    bool bMouthFoam = false;
    bool bPresentationReview = false;
    bool bSpawnedDuringTake = false;
};
