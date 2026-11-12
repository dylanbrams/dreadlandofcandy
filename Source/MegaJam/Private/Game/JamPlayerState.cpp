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
		// Skills 2 to 4 are granted now and refuse until the progression unlocks them (T05a).
		SkillHandles[(int32)EJamSkill::FormALine] = AbilitySystem->GiveAbility(FGameplayAbilitySpec(UJamGA_FormALine::StaticClass(), 1));
		SkillHandles[(int32)EJamSkill::DisappointedSigh] = AbilitySystem->GiveAbility(FGameplayAbilitySpec(UJamGA_DisappointedSigh::StaticClass(), 1));
		SkillHandles[(int32)EJamSkill::LastJudgment] = AbilitySystem->GiveAbility(FGameplayAbilitySpec(UJamGA_LastJudgment::StaticClass(), 1));
	}
}
