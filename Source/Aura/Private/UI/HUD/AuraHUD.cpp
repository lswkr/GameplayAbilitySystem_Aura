// Copyright LeeSeungwon


#include "UI/HUD/AuraHUD.h"

#include "UI/Widget/AuraUserWidget.h"

void AAuraHUD::BeginPlay()
{
	Super::BeginPlay();
	//위젯 넣어주기
	UUserWidget* Widget = CreateWidget<UUserWidget>(GetWorld(),OverlayWidgetClass);
	Widget->AddToViewport();
}
