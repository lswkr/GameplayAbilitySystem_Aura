// Copyright LeeSeungwon


#include "Player/AuraPlayerController.h"
#include "EnhancedInputSubsystems.h"
#include "EnhancedInputComponent.h"
#include "Interaction/EnemyInterface.h"

AAuraPlayerController::AAuraPlayerController()
{
	bReplicates =true;
}

void AAuraPlayerController::PlayerTick(float DeltaTime)
{
	Super::PlayerTick(DeltaTime);
	
	CursorTrace();
}

void AAuraPlayerController::CursorTrace() //매 프레임마다 호출
{
	FHitResult CursorHit;
	GetHitResultUnderCursor(ECC_Visibility, false, CursorHit); //SimpleCollision만 하기 위해 false
	if (!CursorHit.bBlockingHit) return;

	LastActor = ThisActor;
	ThisActor = CursorHit.GetActor();//TScriptInterface<IEnemyInterface>하면 Cast필요없이 바로 이렇게 할 수 있다.

	/*
	 * Line Trace from Cursor. There are several scenarios:
	 * A. LastActor is null && ThisActor is null
	 *		-Do Nothing(벽이나 다른 것들, 에너미 아닌 것들에 걸렸으므로)
	 * B. LastActor is null && ThisActor is Valid
	 *		-Highlight ThisActor(처음으로 hover된 경우이므로)
	 * C. LastActor is Valid && ThisActor is null
	 *		-UnHighlight LastActor(더 이상 hover하지 않으므로)
	 *
	 * D. Both Actors are valid, but LastActor != ThisActor
	 *		-UnHighlight LastActor and Highlight ThisActor(다른 놈으로 옮겨갔을 때)
	 * E. Both Actors are valid and are the same actor
	 *		-Do Nothing(계속 같은 놈 Hover중이므로)
	 */
	
	if (LastActor == nullptr)
	{
		if (ThisActor != nullptr)
		{
			//Case B
			ThisActor->HighlightActor();
		}
		else
		{
			//Case A Both are null, do nothing
		}
	}
	else //LastActor is valid
	{
		if (ThisActor == nullptr) 
		{
			//Case C
			LastActor->UnHighlightActor();
		}
		else // both actors are valid
		{
			if (LastActor != ThisActor)
			{
				//Case D
				LastActor->UnHighlightActor();
				ThisActor->HighlightActor();
			}
			else
			{
				//Case E - Do Nothing
			}
		}
	}
}

void AAuraPlayerController::BeginPlay()
{
	Super::BeginPlay();

	check(AuraContext);//없으면 움직이지 않을 것이므로 어설트

	UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(GetLocalPlayer());//서브시스템은 하나만 존재한다
	if (Subsystem)
	{
		Subsystem->AddMappingContext(AuraContext, 0); //매핑 컨텍스트 설정
	}
	
	bShowMouseCursor=true; //마우스 커서 보이도록
	DefaultMouseCursor = EMouseCursor::Default; //마우스 커서도 여러 종류가 있음(crosshair, 잡는 손 등)

	FInputModeGameAndUI InputModeData;
	InputModeData.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);
	InputModeData.SetHideCursorDuringCapture(false);
	SetInputMode(InputModeData);//커서 세팅
}

void AAuraPlayerController::SetupInputComponent()
{
	Super::SetupInputComponent();

	UEnhancedInputComponent*  EnhancedInputComponent= CastChecked<UEnhancedInputComponent>(InputComponent);//InputComponent는 내장된 변수. UEnhancedInputComponent타입을 저장하도록 언리얼엔진에서 설정

	//MappingContext에 있는 IA들에 대한 변수 필요
	EnhancedInputComponent->BindAction(MoveAction,ETriggerEvent::Triggered, this, &AAuraPlayerController::Move);
}

void AAuraPlayerController::Move(const FInputActionValue& InputActionValue)
{
	const FVector2D InputAxisVector = InputActionValue.Get<FVector2D>();// Move IA을 2D로 했으므로
	const FRotator Rotation = GetControlRotation();
	const FRotator YawRotation(0.f,Rotation.Yaw,0.f);
	
	const FVector ForwardDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X); //앞
	const FVector RightDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y); //오른쪽

	if (APawn* ControlledPawn = GetPawn<APawn>())
	{
		ControlledPawn ->AddMovementInput(ForwardDirection, InputAxisVector.Y);//InputAxisVector.Y이 양수인지 음수인지에 따라 앞 뒤 방향 결정
		ControlledPawn ->AddMovementInput(RightDirection, InputAxisVector.X);//InputAxisVector.X가 양수인지 음수인지에 따라 좌 우 방향 결정
		
	}
}


