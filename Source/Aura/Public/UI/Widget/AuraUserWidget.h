// Copyright LeeSeungwon

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "AuraUserWidget.generated.h"

/**
 * 
 */
UCLASS()
class AURA_API UAuraUserWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable)
	void SetWidgetController(UObject* InWidgetController);

	UPROPERTY(BlueprintReadOnly)
	TObjectPtr<UObject> WidgetController;

protected:
	UFUNCTION(BlueprintImplementableEvent) //위젯에 위젯컨트롤러가 set됐을 때 이 함수를 부를 것이다. 그리고 이 함수를 통해 블루프린트에서 할 수 있는 것들을 할 것 
	void WidgetControllerSet();
};
