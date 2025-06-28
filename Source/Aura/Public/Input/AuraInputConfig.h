// Copyright LeeSeungwon

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "Engine/DataAsset.h"
#include "AuraInputConfig.generated.h"

//IA배열을 만들어야 되는데 IA마다 GameplayTag와 연결시켜야 한다.->구조체
USTRUCT(BlueprintType)
struct FAuraInputAction
{
	GENERATED_BODY()

	UPROPERTY(EditDefaultsOnly)
	const class UInputAction* InputAction = nullptr; //const는 런타임에서는 고칠 수 없다는 것. 블루프린트에서 값 설정은 가능하다

	UPROPERTY(EditDefaultsOnly)
	FGameplayTag InputTag = FGameplayTag(); //const GameplayTag는 못 하는듯하다.
};

/**
 * 
 */
UCLASS()
class AURA_API UAuraInputConfig : public UDataAsset
{
	GENERATED_BODY()

public:

	const UInputAction* FindAbilityInputActionForTag(const FGameplayTag& InputTag, bool bLogNotFound = false) const; //태그에 해당하는 IA반환
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	TArray<FAuraInputAction> AbilityInputActions;


};
