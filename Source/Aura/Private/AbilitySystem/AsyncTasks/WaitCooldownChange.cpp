// Copyright LeeSeungwon


#include "AbilitySystem/AsyncTasks/WaitCooldownChange.h"
#include "AbilitySystemComponent.h"

UWaitCooldownChange* UWaitCooldownChange::WaitForCooldownChange(UAbilitySystemComponent* AbilitySystemComponent,
	const FGameplayTag& InCooldownTag)
{
	UWaitCooldownChange* WaitCooldownChange = NewObject<UWaitCooldownChange>();//노드 만들기
	WaitCooldownChange->ASC = AbilitySystemComponent;
	WaitCooldownChange->CooldownTag = InCooldownTag;

	if (!IsValid(AbilitySystemComponent) || !InCooldownTag.IsValid())
	{
		WaitCooldownChange->EndTask();
		return nullptr;
	}

	//To know when a cooldown has ended(Cooldown Tag has been removed)
	AbilitySystemComponent->RegisterGameplayTagEvent(
		InCooldownTag,
		EGameplayTagEventType::NewOrRemoved).AddUObject(
			WaitCooldownChange,
			&UWaitCooldownChange::CooldownTagChanged
			);//NewOrRemoved-델리게이트를 이 태그가 사라지거나 생길 때

	//To know when a cooldown effect has been applied
	AbilitySystemComponent-> OnActiveGameplayEffectAddedDelegateToSelf.AddUObject(WaitCooldownChange, &UWaitCooldownChange::OnActiveEffectAdded);//duration 이펙트 적용될 때의 서버, 클라이언트 모두 사용 가능한 델리게이트
	return WaitCooldownChange;
}

void UWaitCooldownChange::EndTask()
{
	if (!IsValid(ASC)) return;
	ASC->RegisterGameplayTagEvent(CooldownTag,EGameplayTagEventType::NewOrRemoved).RemoveAll(this);//콜백삭제

	SetReadyToDestroy(); //ACTION이 완전 끝났을 때 호출. Action을 삭제를 자유롭게 해주고 게임인스턴스에서 unregister한다.
	MarkAsGarbage();//삭제할 것이라고 mark한다
}

void UWaitCooldownChange::CooldownTagChanged(const FGameplayTag InCooldownTag, int32 NewCount)
{
	if (NewCount == 0)
	{
		CooldownEnd.Broadcast(0.f);
	}
}

void UWaitCooldownChange::OnActiveEffectAdded(UAbilitySystemComponent* TargetASC,
	const FGameplayEffectSpec& SpecApplied, FActiveGameplayEffectHandle ActiveEffectHandle)
{
	FGameplayTagContainer AssetTags;
	SpecApplied.GetAllAssetTags(AssetTags);

	FGameplayTagContainer GrantedTags;
	SpecApplied.GetAllGrantedTags(GrantedTags);

	if (AssetTags.HasTagExact(CooldownTag)||GrantedTags.HasTagExact(CooldownTag))
	{
		FGameplayEffectQuery GameplayEffectQuery = FGameplayEffectQuery::MakeQuery_MatchAnyOwningTags(CooldownTag.GetSingleTagContainer());//해당 태그 가진 놈의 남은 시간 얻기. (GetSingleTagContainer는 하나의 태그를 가지는 태그 컨테이너를 반환한다.)
		TArray<float> TimesRemaining = ASC->GetActiveEffectsTimeRemaining(GameplayEffectQuery); //float 어레이 반환(anyowningtags 다 가져오므로 array, 근데 여기선 하나만 나오긴 할 것임)
		if (TimesRemaining.Num() > 0)
		{
			float TimeRemaining = TimesRemaining[0]; //한 개만 생각했는데 만약 여러 개가 나오게 된다면 가장 긴 시간을 cooldown시간으로 할 것이다.

			for (int32 i = 0;i < TimesRemaining.Num();i++)
			{
				if (TimesRemaining[i] > TimeRemaining)
				{
					TimeRemaining = TimesRemaining[i];
				}
			}
			CooldownStart.Broadcast(TimeRemaining);
		}
	}
}
