// Copyright LeeSeungwon


#include "UI/WidgetController/AttributeMenuWidgetController.h"

#include "AbilitySystem/AuraAttributeSet.h"
#include "AbilitySystem/Data/AttributeInfo.h"
#include "AuraGameplayTags.h"

void UAttributeMenuWidgetController::BroadcastInitialValue()
{
	UAuraAttributeSet* AS = CastChecked<UAuraAttributeSet> (AttributeSet);

	check(AttributeInfo);

	for (auto& Pair: AS->TagsToAttributes)
	{
		BroadcastAttributeInfo(Pair.Key, Pair.Value());//Value그 자체가 함수이므로 ()해야됨
	}
	
	
	
}

void UAttributeMenuWidgetController::BindCallbacksToDependencies()
{
	UAuraAttributeSet* AS = CastChecked<UAuraAttributeSet> (AttributeSet);//값 바뀐거를 AS에 알려야 하므로 AS 필요

	for (auto& Pair : AS->TagsToAttributes)
	{
		AbilitySystemComponent->GetGameplayAttributeValueChangeDelegate(Pair.Value()).AddLambda(
			[this, Pair](const FOnAttributeChangeData& Data)
			{
				BroadcastAttributeInfo(Pair.Key, Pair.Value());
				
			}
			);//값 바뀔 때 호출할 델리게이트
	}
}

void UAttributeMenuWidgetController::BroadcastAttributeInfo(const FGameplayTag& AttributeTag,
	const FGameplayAttribute& Attribute) const
{
	FAuraAttributeInfo Info = AttributeInfo->FindAttributeInfoForTag(AttributeTag);
	Info.AttributeValue = Attribute.GetNumericValue(AttributeSet);
	/*FAuraAttributeInfo Info = AttributeInfo->FindAttributeInfoForTag(Pair.Key);
				Info.AttributeValue = Pair.Value().GetNumericValue(AS); //	Info.AttributeValue = Pair.Value.Execute() /* 함수가 Value이므로 Execute()*///.GetNumericValue(AS);//Attr은 static function으로부터 반환.-> 어떤 AS에서 오는지 알아야하므로 AS가 input(이전에 사용했던 방식. 비교위해 주석처리)
//				AttributeInfoDelegate.Broadcast(Info);*/
	
	AttributeInfoDelegate.Broadcast(Info);
}
