// Copyright LeeSeungwon

#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"

#include "CombatInterface.generated.h"

class UMotionWarpingComponent;
class UAnimMontage;
// This class does not need to be modified.
UINTERFACE(MinimalAPI, BlueprintType)
class UCombatInterface : public UInterface
{
	GENERATED_BODY()
};

/**
 * 
 */
class AURA_API ICombatInterface
{
	GENERATED_BODY()

	// Add interface functions to this class. This is the class that will be inherited to implement this interface.
public:
	virtual int32 GetPlayerLevel(); //BaseCharacter Class에서 상속받도록

	virtual FVector GetCombatSocketLocation(); //불 나올 weapon의 소켓반환

	UFUNCTION(BlueprintImplementableEvent, BlueprintCallable)//BlueprintImplementableEvent이므로 virtual
	void UpdateFacingTarget(const FVector& Target);

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable)
	UAnimMontage* GetHitReactMontage();

	virtual void Die() = 0;
};
