// Copyright LeeSeungwon


#include "UI/HUD/AuraHUD.h"

#include "UI/Widget/AuraUserWidget.h"
#include "UI/WidgetController/OverlayWidgetController.h"


UOverlayWidgetController* AAuraHUD::GetOverlayWidgetController(const FWidgetControllerParams& WCParams)
{
	if (OverlayWidgetController == nullptr)
	{
		OverlayWidgetController = NewObject<UOverlayWidgetController> (this,OverlayWidgetControllerClass);
		OverlayWidgetController -> SetWidgetControllerParams(WCParams);
		OverlayWidgetController -> BindCallbacksToDependencies();//첫 초기화
		//컨트롤러는 델리게이트에 바인드 된 위젯에 대해 전혀 아는 것이 없다.
		return OverlayWidgetController; 
	}
	return OverlayWidgetController;
}

void AAuraHUD::InitOverlay(APlayerController* PC, APlayerState* PS, UAbilitySystemComponent* ASC, UAttributeSet* AS)
{
	/*
	 *위의 4개 Parameter가 모두 유효하려면 AAuraChacter::InitAbilityActorInfo가 실행되어야 함->이 때 InitOverlay 함수가 호출되어야 함.
	 * & PlayerController만 있으면 HUD에 접근할 수 있음.->InitAbilityActorInfo안에서 이 함수에 접근가능
	*/	
	//위젯과 위젯컨트롤러 생성
	checkf(OverlayWidgetClass, TEXT("Overlay widget class uninitialized, please fill out BP_AuraHUD"))
	checkf(OverlayWidgetControllerClass, TEXT("Overlay Widget Controller Class uninitialized, please fill out BP_AuraHUD"))

	//위젯 넣어주기
	UUserWidget* Widget = CreateWidget<UUserWidget>(GetWorld(),OverlayWidgetClass);
	OverlayWidget = Cast<UAuraUserWidget>(Widget);
	
	const FWidgetControllerParams WidgetControllerParams(PC, PS, ASC, AS);
	UOverlayWidgetController* WidgetController = GetOverlayWidgetController(WidgetControllerParams);

	//WidgetControllerParams를 넣었다는 것은 저것들이 다 세팅됐다는 것.->이를 이용해 WidgetController를 초기화한다.
	OverlayWidget->SetWidgetController(WidgetController);
	WidgetController -> BroadcastInitialValue();
	
	Widget->AddToViewport();
}


