// Copyright LeeSeungwon


#include "UI/Widget/AuraUserWidget.h"

void UAuraUserWidget::SetWidgetController(UObject* InWidgetController)
{
	WidgetController = InWidgetController;
	WidgetControllerSet(); //위젯 컨트롤러 Set된 후 호출
}
