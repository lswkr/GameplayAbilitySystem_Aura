// Copyright LeeSeungwon

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "AbilitySystemInterface.h"
#include "Interaction/CombatInterface.h"
#include "AuraCharacterBase.generated.h"

class UAbilitySystemComponent;
class UAttributeSet;
class UGameplayEffect;
class UGameplayAbility;
class UAnimMontage;

UCLASS(Abstract)
class AURA_API AAuraCharacterBase : public ACharacter, public IAbilitySystemInterface, public ICombatInterface
{
	GENERATED_BODY()

public:
	AAuraCharacterBase();
	//아래 두 개를 AuraCharacter에서 직접 호출하면 Aura는 이걸 직접가지고 있지 않으므로 nullptr을 반환
	virtual UAbilitySystemComponent* GetAbilitySystemComponent() const override;
	UAttributeSet* GetAttributeSet() const { return AttributeSet; }

	virtual UAnimMontage* GetHitReactMontage_Implementation() override; //GA_HitReact에서 AnimMontage반환하는 함수
	virtual void Die() override; //서버에서만 호출

	UFUNCTION(NetMulticast, Reliable)
	virtual void MulticastHandleDeath(); //클, 썹 다 연관
protected:
	virtual void BeginPlay() override;

	UPROPERTY(EditAnywhere, Category= "Combat")
	TObjectPtr<USkeletalMeshComponent> Weapon;

	UPROPERTY(EditAnywhere, Category = "Combat")
	FName WeaponTipSocketName;

	virtual FVector GetCombatSocketLocation() override;
	

	UPROPERTY()
	TObjectPtr<UAbilitySystemComponent> AbilitySystemComponent;

	UPROPERTY()
	TObjectPtr<UAttributeSet> AttributeSet;

	virtual void InitAbilityActorInfo(); //초기화를 델리게이트로 하려고 만든 함수

	UPROPERTY(BlueprintReadOnly, EditAnywhere, Category = "Attributes")
	TSubclassOf<UGameplayEffect> DefaultPrimaryAttributes; //default 어트리뷰트들을 초기화시키는 GE를 설정

	UPROPERTY(BlueprintReadOnly, EditAnywhere, Category = "Attributes")
	TSubclassOf<UGameplayEffect> DefaultSecondaryAttributes; //Infinite GE일 예정

	UPROPERTY(BlueprintReadOnly, EditAnywhere, Category = "Attributes")
	TSubclassOf<UGameplayEffect> DefaultVitalAttributes;
	
	void ApplyEffectToSelf(TSubclassOf<UGameplayEffect> GameplayEffectClass, float Level) const; //위 Attributes들을 초기화하는 GE를 적용하는 함수 
	virtual void InitializeDefaultAttributes() const;
	
	void AddCharacterAbilities();

	/*Dissolve Effects*/

	
	void Dissolve(); //머티리얼 인스턴스를 가지고 DynamicMaterialInstance를 만들고 이로 바꿔치기 하는 함수

	UFUNCTION(BlueprintImplementableEvent) //타임라인 활용은 블루프린트가 더 쉬움->BlueprintImplementable
	void StartDissolveTimeline(UMaterialInstanceDynamic* DynamicMaterialInstance);

	UFUNCTION(BlueprintImplementableEvent) 
	void StartWeaponDissolveTimeline(UMaterialInstanceDynamic* DynamicMaterialInstance);
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	TObjectPtr<UMaterialInstance> DissolveMaterialInstance;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	TObjectPtr<UMaterialInstance> WeaponDissolveMaterialInstance;
	
private:
	UPROPERTY(EditAnywhere, Category="Abilities")
	TArray<TSubclassOf<UGameplayAbility>> StartupAbilities;
	
	UPROPERTY(EditAnywhere, Category="Combat")
	TObjectPtr<UAnimMontage> HitReactMontage;
	// void InitializePrimaryAttributes() const; //Attribute GE를 적용시키는 함수
	// void InitializeSecondaryAttributes() const; //Secondary Attribute를 적용할 함수,
};
