// Copyright LeeSeungwon

#pragma once

#include "CoreMinimal.h"
#include "Abilities/GameplayAbility.h"
#include "AuraGameplayAbility.generated.h"

/**
 * 
 */
UCLASS()
class AURA_API UAuraGameplayAbility : public UGameplayAbility
{
	GENERATED_BODY()

public:
	
	UPROPERTY(EditDefaultsOnly, Category="Input")
	FGameplayTag StartUpInputTag;//인풋태그를 런타임에 바꿀 수도 있다. 현재 StartUpInputTag는 진짜 시작할 때 처음 사용되는 인풋 태그이다.
};
