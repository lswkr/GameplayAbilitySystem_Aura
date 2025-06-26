// Copyright LeeSeungwon

#pragma once

#include "CoreMinimal.h"
#include "EnhancedInputComponent.h"
#include "AuraInputConfig.h"//BindAbilityActions를 템플릿함수로 만들것이고 이 헤더파일에서 그대로 정의할 것이라 전방선언 없이 바로 헤더파일
#include "AuraInputComponent.generated.h"

/**
 * 
 */
UCLASS()
class AURA_API UAuraInputComponent : public UEnhancedInputComponent
{
	GENERATED_BODY()

public:
	template<class UserClass, typename PressedFuncType, typename ReleasedFuncType, typename HeldFuncType>
	void BindAbilityActions(const UAuraInputConfig* InputConfig, UserClass* Object, PressedFuncType PressedFunc, ReleasedFuncType ReleasedFunc,HeldFuncType HeldFunc); //bindaction을 호출할 것이므로 userobject가 필요->object변수
};

template <class UserClass, typename PressedFuncType, typename ReleasedFuncType, typename HeldFuncType> //player controller에서 활용
void UAuraInputComponent::BindAbilityActions(const UAuraInputConfig* InputConfig, UserClass* Object,
	PressedFuncType PressedFunc, ReleasedFuncType ReleasedFunc, HeldFuncType HeldFunc)
{
	check(InputConfig);

	for (const FAuraInputAction& Action : InputConfig->AbilityInputActions)
	{
		//(101강 9분에 설명 있음)
		if (Action.InputAction && Action.InputTag.IsValid())
		{
			
			if (PressedFunc)
			{
				BindAction(Action.InputAction, ETriggerEvent::Started, Object, PressedFunc, Action.InputTag); //Started: 눌렀을 때 한 번 콜백
			}
			if (ReleasedFunc)
			{
				BindAction(Action.InputAction, ETriggerEvent::Completed, Object, ReleasedFunc, Action.InputTag); //Completed: 누르는 걸 땠을 때 콜백
			}
			if (HeldFunc)
			{
				BindAction(Action.InputAction, ETriggerEvent::Triggered, Object, HeldFunc, Action.InputTag);//Triggered: 누르고 있는 동안 모든 프레임에서 콜백
				//BindAction은 템플릿 함수고 여러 개의 인풋 파라미터를 가질 수 있다. InputTag와 연관된 액션이 콜백에 의해 전달될 것이다.
			}
		}
	}
}
