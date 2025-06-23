// Copyright LeeSeungwon


#include "UI/Widget/AuraUserWidget.h"

void UAuraUserWidget::SetWidgetController(UObject* InWidgetController)
{
	WidgetController = InWidgetController;
	WidgetControllerSet(); //위젯 컨트롤러 Set된 후 호출, 프로그레스바 위젯의 경우 이걸 따로 자신들의 블루프린트에서 호출
}
