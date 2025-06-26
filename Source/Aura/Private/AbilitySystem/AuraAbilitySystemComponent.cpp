// Copyright LeeSeungwon


#include "AbilitySystem/AuraAbilitySystemComponent.h"

#include "AuraGameplayTags.h"

void UAuraAbilitySystemComponent::AbilityActorInfoSet()
{
	OnGameplayEffectAppliedDelegateToSelf.AddUObject(this, &UAuraAbilitySystemComponent::EffectApplied);//Dynamic아니므로 Adduboject
	//OnGameplayEffectAppliedDelegateToSelf: 서버에서 GE가 자기 자신에게 적용될 때마다 호출.Instant, Duration GE가 포함된다.
}

void UAuraAbilitySystemComponent::AddCharacterAbilities(const TArray<TSubclassOf<UGameplayAbility>>& StartupAbilities)
{
	for (TSubclassOf<UGameplayAbility> AbilityClass:StartupAbilities)
	{
		//abilityspec만들고 넣어주기
		FGameplayAbilitySpec AbilitySpec = FGameplayAbilitySpec(AbilityClass, 1);
	//	GiveAbility(AbilitySpec); const FGameplayAbilitySpec이어도 ㄱㅊ
 		GiveAbilityAndActivateOnce(AbilitySpec);  //const FGameplayAbilitySpec면 안 됨
	}
}

void UAuraAbilitySystemComponent::EffectApplied(UAbilitySystemComponent* AbilitySystemComponent,
                                                const FGameplayEffectSpec& EffectSpec, FActiveGameplayEffectHandle ActiveEffectHandle)
{
	//태그는 Tarray말고 GameplayTagContainer에 담는다.
	FGameplayTagContainer TagContainer;
	EffectSpec.GetAllAssetTags(TagContainer);

	EffectAssetTags.Broadcast(TagContainer);
}
