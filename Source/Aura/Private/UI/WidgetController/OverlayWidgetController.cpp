// Copyright LeeSeungwon


#include "UI/WidgetController/OverlayWidgetController.h"

#include "AbilitySystem/AuraAbilitySystemComponent.h"
#include "AbilitySystem/AuraAttributeSet.h"
#include "AbilitySystem/Data/AbilityInfo.h"
#include "AbilitySystem/Data/LevelUpInfo.h"
#include "Player/AuraPlayerState.h"


void UOverlayWidgetController::BroadcastInitialValue() //OverlayWidgetController가 가지고 있는 모든 위젯에 브로드캐스팅(Overlay위젯 하나에 설정된 컨트롤러에 많은 다른 위젯들이 바인딩되어있다.)
{
	
	
	OnHealthChanged.Broadcast(GetAuraAS()->GetHealth()); 
	OnMaxHealthChanged.Broadcast(GetAuraAS()->GetMaxHealth());

	OnManaChanged.Broadcast(GetAuraAS()->GetMana());
	OnMaxManaChanged.Broadcast(GetAuraAS()->GetMaxMana());
	
}

void UOverlayWidgetController::BindCallbacksToDependencies()
//AuraHUD의 InitOverlay함수에서 WidgetController가 GetOverlayWidgetController함수를 통해 초기화됨. ->GetOverlayWidgetController함수를 호출한다.
{

	GetAuraPS()->OnXPChangedDelegate.AddUObject(this, &UOverlayWidgetController::OnXPChanged);//델리게이트에 콜백 바인드 시킬꺼면 AuraPlayerState는 const면 안 된다.
	GetAuraPS()->OnLevelChangedDelegate.AddLambda(
		[this](int32 NewLevel)
	{
		OnPlayerLevelChangedDelegate.Broadcast(NewLevel);
	}
		);
	
	
	AbilitySystemComponent->GetGameplayAttributeValueChangeDelegate(
	GetAuraAS()->GetHealthAttribute()).AddLambda(
		[this](const FOnAttributeChangeData& Data)
		{
			OnHealthChanged.Broadcast(Data.NewValue);
		}
	);//해당 어트리뷰트가 바뀌었을 때의 델리게이트
	// //GetGameplayAttributeValueChangeDelegate는 dynamic이 아닌 그냥 multicast delegate이므로 AddDynamic을 사용할 수 없어 AddUObject를 사용해야한다.

	AbilitySystemComponent->GetGameplayAttributeValueChangeDelegate(
		GetAuraAS()->GetMaxHealthAttribute()).AddLambda(
			[this](const FOnAttributeChangeData& Data)
			{
				OnMaxHealthChanged.Broadcast(Data.NewValue);
			}
		);

	AbilitySystemComponent->GetGameplayAttributeValueChangeDelegate(
	GetAuraAS()->GetManaAttribute()).AddLambda(
		[this](const FOnAttributeChangeData& Data)
		{
			OnManaChanged.Broadcast(Data.NewValue);
		}
	);
	
	AbilitySystemComponent->GetGameplayAttributeValueChangeDelegate(
	GetAuraAS()->GetMaxManaAttribute()).AddLambda(
		[this](const FOnAttributeChangeData& Data)
		{
			OnMaxManaChanged.Broadcast(Data.NewValue);
		}
	);

	
	if (GetAuraASC())
	{
		//아래 if문은 서버와 클라이언트에서 무슨 일이 일어나느냐에 따라 또는 다른 일에 따라 달라질 수 있는 상황이므로 두 경우를 다 생각해줘야 한다.  
		if (GetAuraASC()->bStartupAbilitiesGiven) //bStartupAbilitiesGiven는 캐릭터의 ASC에 어빌리티가 주어졌을 때 true가 됨
		{
			BroadcastAbilityInfo(); //직접 호출해서 ASC에 위젯 바인딩 시키면 됨.
		}
		else //안 주어졌을 경우 델리게이트에 바인딩하고, 그 뒤에 AddCharacterAbilities안에서 해당 델리게이트에 브로드캐스팅하면서 호출하게 됨
		{
			GetAuraASC()->AbilitiesGivenDelegate.AddUObject(this, &UOverlayWidgetController::BroadcastAbilityInfo);
		}
		
		GetAuraASC()->EffectAssetTags.AddLambda( //EffectAssetTags는 FGameplayTagContainer를 인자로하는 델리게이트->Tag받았을 때 Message달고 있는 것을 찾아서 데이터테이블에서 그에 해당하는 Row를 브로드캐스트한다. 이 브로드캐스트는 Overlay에 바인딩된 함수에 보내진다.
			[this](const FGameplayTagContainer& AssetTags)//람다 함수는 Global말고는 해당 클래스에 아는 내용이 없다.->[]에 추가
			{
				for (const FGameplayTag& Tag : AssetTags)
				{
					//"A.1".MatchesTag("A") will return True, "A".MatchesTag("A.1") will return False
					FGameplayTag MessageTag = FGameplayTag::RequestGameplayTag(FName("Message")); // "Message"들만 골라 내도록
					if (Tag.MatchesTag(MessageTag)) 
					{
						const FUIWidgetRow* Row = GetDataTableRowByTag<FUIWidgetRow>(MessageWidgetDataTable, Tag);//이 데이터로 이펙트 적용 시 브로드 캐스트할 것
						MessageWidgetRowDelegate.Broadcast(*Row);
					}
				}
			}
		);//람다 함수를 사용함으로써 굳이 콜백 함수를 만들 필요가 없다.
	}
	
	
	
	
}


void UOverlayWidgetController::OnXPChanged(int32 NewXP)
{
	const ULevelUpInfo* LevelUpInfo = GetAuraPS()->LevelUpInfo;
	checkf(LevelUpInfo, TEXT("Unable to find LevelUpInfo. Please fill out AuraPlayerState Blueprint"));

	const int32 Level = LevelUpInfo->FindLevelForXP(NewXP);
	const int32 MaxLevel = LevelUpInfo->LevelUpInformation.Num();

	if (Level <= MaxLevel && Level > 0)
	{
		const int32 LevelUpRequirement = LevelUpInfo->LevelUpInformation[Level].LevelUpRequirement;
		const int32 PreviousLevelUpRequirement =LevelUpInfo->LevelUpInformation[Level-1].LevelUpRequirement;

		const int32 DeltaLevelRequirement = LevelUpRequirement - PreviousLevelUpRequirement;
		const int32 XPForThisLevel = NewXP - PreviousLevelUpRequirement;	//이전 레벨과 현 레벨 간의 차이가 채워야 하는 XP이므로

		const float XPBarPercent = static_cast<float>(XPForThisLevel) / static_cast<float>(DeltaLevelRequirement);

		OnXPPercentChangedDelegate.Broadcast(XPBarPercent);
	}
}

