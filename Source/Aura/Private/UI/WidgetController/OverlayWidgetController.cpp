// Copyright LeeSeungwon


#include "UI/WidgetController/OverlayWidgetController.h"

#include "AbilitySystem/AuraAbilitySystemComponent.h"
#include "AbilitySystem/AuraAttributeSet.h"
#include "AbilitySystem/Data/AbilityInfo.h"


void UOverlayWidgetController::BroadcastInitialValue() //OverlayWidgetController가 가지고 있는 모든 위젯에 브로드캐스팅(Overlay위젯 하나에 설정된 컨트롤러에 많은 다른 위젯들이 바인딩되어있다.)
{
	const UAuraAttributeSet* AuraAttributeSet = CastChecked<UAuraAttributeSet>(AttributeSet);
	
	OnHealthChanged.Broadcast(AuraAttributeSet->GetHealth()); 
	OnMaxHealthChanged.Broadcast(AuraAttributeSet->GetMaxHealth());

	OnManaChanged.Broadcast(AuraAttributeSet->GetMana());
	OnMaxManaChanged.Broadcast(AuraAttributeSet->GetMaxMana());
	
}

void UOverlayWidgetController::BindCallbacksToDependencies()
//AuraHUD의 InitOverlay함수에서 WidgetController가 GetOverlayWidgetController함수를 통해 초기화됨. ->GetOverlayWidgetController함수를 호출한다.
{
	const UAuraAttributeSet* AuraAttributeSet = CastChecked<UAuraAttributeSet>(AttributeSet);
	
	AbilitySystemComponent->GetGameplayAttributeValueChangeDelegate(
	AuraAttributeSet->GetHealthAttribute()).AddLambda(
		[this](const FOnAttributeChangeData& Data)
		{
			OnHealthChanged.Broadcast(Data.NewValue);
		}
	);//해당 어트리뷰트가 바뀌었을 때의 델리게이트
	// //GetGameplayAttributeValueChangeDelegate는 dynamic이 아닌 그냥 multicast delegate이므로 AddDynamic을 사용할 수 없어 AddUObject를 사용해야한다.

	AbilitySystemComponent->GetGameplayAttributeValueChangeDelegate(
		AuraAttributeSet->GetMaxHealthAttribute()).AddLambda(
			[this](const FOnAttributeChangeData& Data)
			{
				OnMaxHealthChanged.Broadcast(Data.NewValue);
			}
		);

	AbilitySystemComponent->GetGameplayAttributeValueChangeDelegate(
	AuraAttributeSet->GetManaAttribute()).AddLambda(
		[this](const FOnAttributeChangeData& Data)
		{
			OnManaChanged.Broadcast(Data.NewValue);
		}
	);
	
	AbilitySystemComponent->GetGameplayAttributeValueChangeDelegate(
	AuraAttributeSet->GetMaxManaAttribute()).AddLambda(
		[this](const FOnAttributeChangeData& Data)
		{
			OnMaxManaChanged.Broadcast(Data.NewValue);
		}
	);

	
	if (UAuraAbilitySystemComponent* AuraASC = Cast<UAuraAbilitySystemComponent>(AbilitySystemComponent))
	{
		//아래 if문은 서버와 클라이언트에서 무슨 일이 일어나느냐에 따라 또는 다른 일에 따라 달라질 수 있는 상황이므로 두 경우를 다 생각해줘야 한다.  
		if (AuraASC->bStartupAbilitiesGiven) //bStartupAbilitiesGiven는 캐릭터의 ASC에 어빌리티가 주어졌을 때 true가 됨
		{
			OnInitializeStartupAbilities(AuraASC); //직접 호출해서 ASC에 위젯 바인딩 시키면 됨.
		}
		else //안 주어졌을 경우 델리게이트에 바인딩하고, 그 뒤에 AddCharacterAbilities안에서 해당 델리게이트에 브로드캐스팅하면서 호출하게 됨
		{
			AuraASC->AbilitiesGivenDelegate.AddUObject(this, &UOverlayWidgetController::OnInitializeStartupAbilities);
		}
		
		AuraASC->EffectAssetTags.AddLambda( //EffectAssetTags는 FGameplayTagContainer를 인자로하는 델리게이트->Tag받았을 때 Message달고 있는 것을 찾아서 데이터테이블에서 그에 해당하는 Row를 브로드캐스트한다. 이 브로드캐스트는 Overlay에 바인딩된 함수에 보내진다.
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

void UOverlayWidgetController::OnInitializeStartupAbilities(UAuraAbilitySystemComponent* AuraAbilitySystemComponent)
{
	//TODO: Get Information about all given abilities, look up their ability info, and broadcast it to widgets.
	if (!AuraAbilitySystemComponent->bStartupAbilitiesGiven) return; //어빌리티 안 주어져 있으면 빠른 return

	//각 어빌리티에 다 바인드 할 수 있도록
	FForEachAbility BroadcastDelegate;
	BroadcastDelegate.BindLambda([this, AuraAbilitySystemComponent](const FGameplayAbilitySpec& AbilitySpec)
	{
		//TODO: need a way to figure out the ability tag for a given ability spec.
		FAuraAbilityInfo Info = AbilityInfo->FindAbilityInfoForTag(AuraAbilitySystemComponent->GetAbilityTagFromSpec(AbilitySpec));
		Info.InputTag = AuraAbilitySystemComponent->GetInputTagFromSpec(AbilitySpec);
		AbilityInfoDelegate.Broadcast(Info);
	});
	AuraAbilitySystemComponent->ForEachAbility(BroadcastDelegate);
}
