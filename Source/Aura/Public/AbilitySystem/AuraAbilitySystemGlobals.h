// Copyright LeeSeungwon

#pragma once

#include "CoreMinimal.h"
#include "AbilitySystemGlobals.h"
#include "AuraAbilitySystemGlobals.generated.h"

/**
 * 
 */
UCLASS()
class AURA_API UAuraAbilitySystemGlobals : public UAbilitySystemGlobals
{
	GENERATED_BODY()
	virtual FGameplayEffectContext* AllocGameplayEffectContext() const override; //MakeEffectContext에서 Context만들 때 봤던 함수이다.
	// 새로운 Effect Context를 만들기 위해 이 함수를 부르면 이 함수를 가지고 어떤 클래스가 이 오브젝트를 인스턴스화할 때 사용될 지 결정한다.
};
