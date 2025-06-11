// Copyright LeeSeungwon

#pragma once

#include "CoreMinimal.h"
#include "UI/WidgetController/AuraWidgetController.h"
#include "OverlayWidgetController.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnHealthChangedSignature, float, NewHealth); //Dynamic -블루프린트에서 이벤트를 할당하기 위해, multicast - 여러 블루프린트에게 전달하기 위해
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnMaxHealthChangedSignature, float, NewMaxHealth);

/**
 * 하나만 가지게 된다(singleton)
 */
UCLASS(BlueprintType, Blueprintable) //캐스트할 때와 같이 Type으로 쓸 떄 BlueprintType필요, Blueprintable- 이 클래스를 가지고 Blueprint를 만들 수 있는것
class AURA_API UOverlayWidgetController : public UAuraWidgetController
{
	GENERATED_BODY()

public:
	virtual void BroadcastInitialValue() override;

	UPROPERTY(BlueprintAssignable, Category = "GAS|Attribute") // |: 서브 카테고리
	FOnHealthChangedSignature OnHealthChanged;

	UPROPERTY(BlueprintAssignable, Category = "GAS|Attribute") 
	FOnMaxHealthChangedSignature OnMaxHealthChanged;
};
