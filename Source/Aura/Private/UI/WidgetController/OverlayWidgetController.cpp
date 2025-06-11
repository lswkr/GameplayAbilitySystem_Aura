// Copyright LeeSeungwon


#include "UI/WidgetController/OverlayWidgetController.h"
#include "AbilitySystem/AuraAttributeSet.h"

void UOverlayWidgetController::BroadcastInitialValue()
{
	const UAuraAttributeSet* AuraAttributeSet = CastChecked<UAuraAttributeSet>(AttributeSet);
	
	OnHealthChanged.Broadcast(AuraAttributeSet->GetHealth());
	OnMaxHealthChanged.Broadcast(AuraAttributeSet->GetMaxHealth());

	// AbilitySystemComponent->GetGameplayAttributeValueChangeDelegate(AuraAttributeSet->GetHealthAttribute().AddUObject(this, )); //해당 어트리뷰트가 바뀌었을 때의 델리게이트
	// //GetGameplayAttributeValueChangeDelegate는 dynamic이 아닌 그냥 multicast delegate이므로 AddDynamic을 사용할 수 없어 AddUObject를 사용해야한다.
}
