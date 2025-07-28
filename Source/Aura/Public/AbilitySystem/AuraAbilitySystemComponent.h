// Copyright LeeSeungwon

#pragma once

#include "CoreMinimal.h"
#include "AbilitySystemComponent.h"
#include "AuraAbilitySystemComponent.generated.h"
DECLARE_MULTICAST_DELEGATE_OneParam(FEffectAssetTags, const FGameplayTagContainer& /*AssetTags*/);
DECLARE_MULTICAST_DELEGATE_OneParam(FAbilitiesGiven, UAuraAbilitySystemComponent*);
DECLARE_DELEGATE_OneParam(FForEachAbility, const FGameplayAbilitySpec&);
/**
 * 
 */
UCLASS()
class AURA_API UAuraAbilitySystemComponent : public UAbilitySystemComponent
{
	GENERATED_BODY()

public:
	void AbilityActorInfoSet();

	//Widget controller가 bind할 수 있어야 하므로 public
	FEffectAssetTags EffectAssetTags;
	FAbilitiesGiven AbilitiesGivenDelegate;
	
	void AddCharacterAbilities(const TArray<TSubclassOf<UGameplayAbility>>& StartupAbilities);
	void AddCharacterPassiveAbilities(const TArray<TSubclassOf<UGameplayAbility>>& StartupPassiveAbilities);
	bool bStartupAbilitiesGiven = false;
	
	void AbilityInputTagHeld(const FGameplayTag& InputTag);
	void AbilityInputTagReleased(const FGameplayTag& InputTag);
	void ForEachAbility(const FForEachAbility& Delegate);

	static FGameplayTag GetAbilityTagFromSpec(const FGameplayAbilitySpec& AbilitySpec);
	static FGameplayTag GetInputTagFromSpec(const FGameplayAbilitySpec& AbilitySpec);

	void UpgradeAttribute(const FGameplayTag& AttributeTag);

	UFUNCTION(Server, Reliable)
	void ServerUpgradeAttribute(const FGameplayTag& AttributeTag);
protected:
	
	//어빌리티가 한 번 주어지면 ASC, Activatable abilities 컨테이너는 Replicate된다.
	//Activatable Ability들이 Replicate되면 OnRep_ActivateAbilities()가 호출된다.GiveAbilities가 호출된 후 OnRep_ActivateAbilities는 레플리케이트 된다.
	//그 뿐만 아니라 새로 주어진 어빌리티들의 어빌리티 스펙을 포함하므로 우리는 OnRep_ActivateAbilities를 오버라이드할 수 있다.
	virtual void OnRep_ActivateAbilities() override;
	
	
	UFUNCTION(Client, Reliable)// Client: 서버에서 호출하지만 클라에서 실행되는 RPC. Reliable: 클라이언트에 도달하는 것을 보장(패킷 손실과 같은 상황에서도 받았다는 Corfirmation받을 때까지 계속 보낸다)
	void ClientEffectApplied(UAbilitySystemComponent* AbilitySystemComponent, const FGameplayEffectSpec& EffectSpec, FActiveGameplayEffectHandle ActiveEffectHandle);

};
