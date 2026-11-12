#include "Game/JamPlayerState.h"
#include "Gas/JamAbilitySystemComponent.h"
#include "Gas/JamGameplayAbility.h"
#include "Gas/JamGameplayTags.h"

AJamPlayerState::AJamPlayerState()
{
	// No replication during the jam (BUILD_SPEC 5): solo only.
	AbilitySystem = CreateDefaultSubobject<UJamAbilitySystemComponent>(TEXT("AbilitySystem"));
}

UAbilitySystemComponent* AJamPlayerState::GetAbilitySystemComponent() const
{
	return AbilitySystem;
}

void AJamPlayerState::InitForAvatar(AActor* Avatar)
{
	AbilitySystem->InitAbilityActorInfo(this, Avatar);
	if (!LectureHandle.IsValid())
	{
		AbilitySystem->AddLooseGameplayTag(JamGameplayTags::Team_Hero);
		LectureHandle = AbilitySystem->GiveAbility(FGameplayAbilitySpec(UJamGA_LectureCone::StaticClass(), 1));
	}
}
