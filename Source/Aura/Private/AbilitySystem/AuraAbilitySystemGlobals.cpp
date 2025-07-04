// Copyright LeeSeungwon


#include "AbilitySystem/AuraAbilitySystemGlobals.h"

#include "AuraAbilityTypes.h"

FGameplayEffectContext* UAuraAbilitySystemGlobals::AllocGameplayEffectContext() const
{
	return new FAuraGameplayEffectContext();//우리가 만들었던 커스텀 EffectContext, 이제 이 UAuraAbilitySystemGlobals를 사용하면 Context만들 때 마다 FAuraGameplayEffectContext가 된다.
}
