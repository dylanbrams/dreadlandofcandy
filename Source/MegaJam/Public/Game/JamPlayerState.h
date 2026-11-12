#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerState.h"
#include "AbilitySystemInterface.h"
#include "GameplayAbilitySpecHandle.h"
#include "Film/JamProgression.h"
#include "JamPlayerState.generated.h"

class UJamAbilitySystemComponent;

/** Per-player state. Owns the hero's ability system; the hero pawn is only its avatar. */
UCLASS()
class MEGAJAM_API AJamPlayerState : public APlayerState, public IAbilitySystemInterface
{
	GENERATED_BODY()

public:
	AJamPlayerState();

	virtual UAbilitySystemComponent* GetAbilitySystemComponent() const override;
	UJamAbilitySystemComponent* GetJamAbilitySystem() const { return AbilitySystem; }

	/** Binds the ASC to a new avatar and grants the starting abilities once. */
	void InitForAvatar(AActor* Avatar);

	FGameplayAbilitySpecHandle GetLectureHandle() const { return LectureHandle; }

	/** Level, points, ranks, loot and stats (BUILD_SPEC 3c). Written only by the game mode. */
	const FJamProgression& GetProgression() const { return Progression; }
	FJamProgression& EditProgression() { return Progression; }

	/** The score: the hero's cut over the run. */
	int32 Sprinkles = 0;

private:
	UPROPERTY(VisibleAnywhere)
	TObjectPtr<UJamAbilitySystemComponent> AbilitySystem;

	FGameplayAbilitySpecHandle LectureHandle;
	FJamProgression Progression;
};
