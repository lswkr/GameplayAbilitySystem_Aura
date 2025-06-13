// Copyright LeeSeungwon

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "UI/WidgetController/AuraWidgetController.h"
#include "OverlayWidgetController.generated.h"

class UAuraUserWidget;
struct FOnAttributeChangeData;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnHealthChangedSignature, float, NewHealth); //Dynamic -블루프린트에서 이벤트를 할당하기 위해, multicast - 여러 블루프린트에게 전달하기 위해
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnMaxHealthChangedSignature, float, NewMaxHealth);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnManaChangedSignature, float, NewMana); 
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnMaxManaChangedSignature, float, NewMaxMana);

USTRUCT(BlueprintType)
struct FUIWidgetRow : public FTableRowBase
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FGameplayTag MessageTag = FGameplayTag();

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FText Message = FText();
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	TSubclassOf<UAuraUserWidget> MessageWidget;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	UTexture2D* Image = nullptr;
};

/**
 * 하나만 가지게 된다(singleton)
 */
UCLASS(BlueprintType, Blueprintable) //캐스트할 때와 같이 Type으로 쓸 떄 BlueprintType필요, Blueprintable- 이 클래스를 가지고 Blueprint를 만들 수 있는것
class AURA_API UOverlayWidgetController : public UAuraWidgetController
{
	GENERATED_BODY()

public:
	virtual void BroadcastInitialValue() override;
	virtual void BindCallbacksToDependencies() override;
	
	UPROPERTY(BlueprintAssignable, Category = "GAS|Attribute") // |: 서브 카테고리
	FOnHealthChangedSignature OnHealthChanged;

	UPROPERTY(BlueprintAssignable, Category = "GAS|Attribute") 
	FOnMaxHealthChangedSignature OnMaxHealthChanged;

	UPROPERTY(BlueprintAssignable,Category = "GAS|Attribute")
	FOnManaChangedSignature OnManaChanged;

	UPROPERTY(BlueprintAssignable,Category = "GAS|Attribute")
	FOnMaxManaChangedSignature OnMaxManaChanged;
	

protected:

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Widget Data")
	TObjectPtr<UDataTable> MessageWidgetDataTable;
	
	void HealthChanged(const FOnAttributeChangeData& Data) const; //ASC에서 값 바뀔 때 호출되는 델리게이트에 바인딩 시킬 함수
	void MaxHealthChanged(const FOnAttributeChangeData& Data) const;
	void ManaChanged(const FOnAttributeChangeData& Data)const;
	void MaxManaChanged(const FOnAttributeChangeData& Data)const;
};
