// Copyright LeeSeungwon

#pragma once

#include "CoreMinimal.h"
#include "AbilitySystemInterface.h"
#include "GameFramework/PlayerState.h"
#include "AuraPlayerState.generated.h"

class UAbilitySystemComponent;
class UAttributeSet;
class ULevelUpInfo;

DECLARE_MULTICAST_DELEGATE_OneParam (FOnPlayerStatChanged, int32/*StatValue*/);
/**
 * 
 */
UCLASS()
class AURA_API AAuraPlayerState : public APlayerState, public IAbilitySystemInterface
{
	GENERATED_BODY()

public:
	AAuraPlayerState();
	virtual void GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const override;
	virtual UAbilitySystemComponent* GetAbilitySystemComponent() const override;
	UAttributeSet* GetAttributeSet() const { return AttributeSet; }

	UPROPERTY(EditDefaultsOnly)
	TObjectPtr<ULevelUpInfo> LevelUpInfo;
	
	FOnPlayerStatChanged OnXPChangedDelegate;
	FOnPlayerStatChanged OnLevelChangedDelegate;
	
	FORCEINLINE int32 GetPlayerLevel() const { return Level; }
	FORCEINLINE int32 GetXP() const { return XP; }

	void AddToLevel(int32 InLevel);
	void AddToXP(int32 InXP);

	void SetLevel(int32 InLevel);
	void SetXP(int32 InXP);
	
protected:
	UPROPERTY(VisibleAnywhere)//PlayerState블루프린트에서 볼 수 있도록 Visibleanywhere로 설정
	TObjectPtr<UAbilitySystemComponent> AbilitySystemComponent;

	UPROPERTY()
	TObjectPtr<UAttributeSet> AttributeSet;

	
	
private:
	UPROPERTY(VisibleAnywhere, ReplicatedUsing = OnRep_Level)
	int32 Level = 1; //replicate될 때마다 보일 수 있도록 GetLifetimeReplicatedProps활용, Aura만 사용, Enemy의 경우는 따로 가지도록 

	UPROPERTY(VisibleAnywhere, ReplicatedUsing = OnRep_XP)
	int32 XP = 1;
	
	UFUNCTION() //RepNotify 할 수 있도록 UFUNCTION붙이기
	void OnRep_Level(int32 OldLevel);

	UFUNCTION()
	void OnRep_XP(int32 OldXP);
};
