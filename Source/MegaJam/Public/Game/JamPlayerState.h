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
	/** The ability for a skill slot (Lecture, Form A Line, the Sigh, the Judgment). */
	FGameplayAbilitySpecHandle GetSkillHandle(EJamSkill Skill) const { return Skill == EJamSkill::Lecture ? LectureHandle : SkillHandles[(int32)Skill]; }

	/** Level, points, ranks, loot and stats (BUILD_SPEC 3c). Written only by the game mode. */
	const FJamProgression& GetProgression() const { return Progression; }
	FJamProgression& EditProgression() { return Progression; }

	/** Every note the player typed, oldest first (BUILD_SPEC 3a), and the motif the last one came back with. */
	TArray<FString> TypedNotes;
	FString Motif;

	/** The score: the hero's cut over the run. */
	int32 Sprinkles = 0;
	/** Skills already used by this player; their first attack cry is in English. */
	uint8 HeardFirstAttackMask = 0;

private:
	UPROPERTY(VisibleAnywhere)
	TObjectPtr<UJamAbilitySystemComponent> AbilitySystem;

	FGameplayAbilitySpecHandle LectureHandle;
	FGameplayAbilitySpecHandle SkillHandles[(int32)EJamSkill::Count];
	FJamProgression Progression;
};
