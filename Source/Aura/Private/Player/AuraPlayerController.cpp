// Copyright LeeSeungwon


#include "Player/AuraPlayerController.h"
#include "EnhancedInputSubsystems.h"
//#include "EnhancedInputComponent.h" AuraInputComponent쓰면서 필요없어짐
#include "AbilitySystemBlueprintLibrary.h"
#include "AuraGameplayTags.h"
#include "NavigationPath.h"
#include "NavigationSystem.h"
#include "AbilitySystem/AuraAbilitySystemComponent.h"
#include "Components/SplineComponent.h"
#include "Input/AuraInputComponent.h"
#include "Interaction/EnemyInterface.h"
#include "GameFramework/Character.h"
#include "UI/Widget/DamageTextComponent.h"
#include "NiagaraFunctionLibrary.h"
#include "Actor/MagicCircle.h"
#include "Aura/Aura.h"
#include "Components/DecalComponent.h"
#include "Interaction/HighlightInterface.h"

AAuraPlayerController::AAuraPlayerController()
{
	bReplicates =true;
	Spline = CreateDefaultSubobject<USplineComponent>("Spline");
}

void AAuraPlayerController::PlayerTick(float DeltaTime)
{
	Super::PlayerTick(DeltaTime);
	CursorTrace();
	AutoRun();
	UpdateMagicCircleLocation();
}

void AAuraPlayerController::ShowDamageNumber_Implementation(float DamageAmount, ACharacter* TargetCharacter, bool bBlockedHit, bool bCriticalHit)//클라 캐릭터입장에서, 서버에서 호출되지만 클라이언트에서 execute->클라에서 보임
{
	if (IsValid(TargetCharacter) && DamageTextComponentClass && IsLocalController()) //IsValid는 포인터가 null인지 그리고 Pending kill인지 보는 함수. TextComponentClass는 블루프린트에서 설정하거나 안 하거나 하는 값이기에 굳이 PendingKill을 걱정안해도 되기에 캐릭터 포인터에만 사용함
	{
		UDamageTextComponent* DamageText = NewObject<UDamageTextComponent>(TargetCharacter, DamageTextComponentClass);//새 오브젝트 생성
		DamageText->RegisterComponent();//Dynamic하게 위젯컴포넌트를 만든 뒤에는 register해야한다.(생성자에서 생성한게 아닐 경우 이렇게 수동으로 register한다)
		DamageText->AttachToComponent(TargetCharacter->GetRootComponent(),FAttachmentTransformRules::KeepRelativeTransform);//루트 컴포넌트에 붙음
		DamageText->DetachFromComponent(FDetachmentTransformRules::KeepWorldTransform); //애니메이션 끝나고 사라지게 붙인 뒤 바로 떼버린다.
		DamageText->SetDamageText(DamageAmount,bBlockedHit, bCriticalHit); //Damage숫자 설정(Blueprint함수)
		//Destroy는 SetDamageText에 구현되어있다.
	}
}

void AAuraPlayerController::AutoRun()
{
	if (!bAutoRunning) return;
	
	if (APawn* ControlledPawn = GetPawn())
	{
		//일반적으로 정확히 Spline위에 서 있지 않다.약간 빗겨져 있을 것이다. 최대한 덜 빗기도록 
		const FVector LocationOnSpline = Spline->FindLocationClosestToWorldLocation(ControlledPawn->GetActorLocation(), ESplineCoordinateSpace::World);//폰에 가장 가까운 스플라인 위의 점
		const FVector Direction = Spline->FindDirectionClosestToWorldLocation(LocationOnSpline,  ESplineCoordinateSpace::World); //액터 위치가 어차피 LocationOnSpline로 설정되어있으므로 이렇게 변수를 넣어도 문제 없음
		ControlledPawn->AddMovementInput(Direction);

		const float DistanceToDestination = (LocationOnSpline - CachedDestination).Length();
		
		if (DistanceToDestination <= AutoRunAcceptanceRadius)
		{
			bAutoRunning = false;//클릭 거리가 자동으로 움직이는 기준보다 낮으면 자동으로 움직이지 않는다.
		}
	}
}

void AAuraPlayerController::UpdateMagicCircleLocation()
{
	//커서의 매 프레임마다 위치 찾기(Tick 함수에서 활용)
	if (IsValid(MagicCircle))
	{
		MagicCircle->SetActorLocation(CursorHit.ImpactPoint);
	}
}

void AAuraPlayerController::HighlightActor(AActor* InActor)
{
	if (IsValid(InActor) &&InActor->Implements<UHighlightInterface>())
	{
		IHighlightInterface::Execute_HighlightActor(InActor);
	}
}

void AAuraPlayerController::UnHighlightActor(AActor* InActor)
{
	if (IsValid(InActor) &&InActor->Implements<UHighlightInterface>())
	{
		IHighlightInterface::Execute_UnHighlightActor(InActor);
	}
}

void AAuraPlayerController::CursorTrace() //매 프레임마다 호출
{
	if (GetASC() && GetASC()->HasMatchingGameplayTag(FAuraGameplayTags::Get().Player_Block_CursorTrace))
	{
		UnHighlightActor(LastActor);
		UnHighlightActor(ThisActor);
		
		LastActor = nullptr;
		ThisActor = nullptr;
		return;
	}//Cursor_Trace태그 가지는 중일 때에는 아무것도 하지 않도록(예를 들어 공격 같은 것 하는 도중)
	
	const ECollisionChannel TraceChannel = IsValid(MagicCircle)? ECC_ExcludePlayers : ECC_Visibility; //마법진에 들어갈 때, 아닐 때 
	GetHitResultUnderCursor(TraceChannel, false, CursorHit); //SimpleCollision만 하기 위해 false
	if (!CursorHit.bBlockingHit) return;

	LastActor = ThisActor;
	if (IsValid(CursorHit.GetActor()) && CursorHit.GetActor()->Implements<UHighlightInterface>()) //ThisActor가 AActor* 이므로 인터페이스 구현여부만 보는 것
	{
		ThisActor = CursorHit.GetActor();
	}
	else//닿은 대상이 Highlight인터페이스를 구현하지 않았다면
	{
		ThisActor = nullptr;
	}
		
	
	if (LastActor != ThisActor)
	{
		UnHighlightActor(LastActor);
		HighlightActor(ThisActor);
	}
}


void AAuraPlayerController::AbilityInputTagPressed(FGameplayTag InputTag)
{
	if (GetASC() && GetASC()->HasMatchingGameplayTag(FAuraGameplayTags::Get().Player_Block_InputPressed))
	{	//GA에서 block_Pressed 라는 'Activation Owned Tags'가 있으면 멈추도록
		return;
	}
	if (InputTag.MatchesTagExact(FAuraGameplayTags::Get().InputTag_LMB))
	{
		if (IsValid(ThisActor))
		{
			TargetingStatus = ThisActor->Implements<UEnemyInterface>()? ETargetingStatus::TargetingEnemy: ETargetingStatus::TargetingNonEnemy;//ThisActor는 이 시점에서는 null이 아닌 경우에만 오게 됨
			
		}
		else
		{
			TargetingStatus = ETargetingStatus::NotTargeting;
		}
		bAutoRunning = false;//Actor가 있건 없건 누르면 AutoRunning은 끝나야 함. 
		
	}
	if (GetASC()) GetASC()->AbilityInputTagPressed(InputTag);
}

void AAuraPlayerController::AbilityInputTagReleased(FGameplayTag InputTag)
{
	if (GetASC() && GetASC()->HasMatchingGameplayTag(FAuraGameplayTags::Get().Player_Block_InputReleased))
	{
		return;
	}
	if (!InputTag.MatchesTagExact(FAuraGameplayTags::Get().InputTag_LMB)) //LMB아닌 경우 InputTag에 해당하는 Ability를 발동시키고 싶다.
	{
		if (GetASC())//Set이 안 되어 nullptr가 되는경우 대비
		{
			GetASC()->AbilityInputTagReleased(InputTag);
		}
		return;
	}
	
	if (GetASC()) GetASC()->AbilityInputTagReleased(InputTag); //released됐다고 알려줌
	
	if (TargetingStatus != ETargetingStatus::TargetingEnemy && !bShiftKeyDown) //마우스 뗀 상태에서는 targeting하거나 shiftkey누른 상태가 아니면 shortpressthreshold를 측정할 필요 없다.
	{
		const APawn* ControlledPawn = GetPawn();
		if (FollowTime <= ShortPressThreshold && ControlledPawn)//FollowTime: AbilityInputTagHeld 함수에서 모아지고 있는 시간
		{
			if (IsValid(ThisActor) && ThisActor->Implements<UHighlightInterface>()) //적은 이 인터페이스 구현해서 적용됨
			{
				IHighlightInterface::Execute_SetMoveToLocation(ThisActor, CachedDestination);
			}
			else if (GetASC() && !GetASC()->HasMatchingGameplayTag(FAuraGameplayTags::Get().Player_Block_InputPressed)) //Pressed가 아닐 때에만 생기도록
			{
				UNiagaraFunctionLibrary::SpawnSystemAtLocation(this, ClickNiagaraSystem, CachedDestination);
			}
			
			if (UNavigationPath* NavPath = UNavigationSystemV1::FindPathToLocationSynchronously(this, ControlledPawn->GetActorLocation(), CachedDestination))
			{
				Spline->ClearSplinePoints();//위치 Point 받기 전에 일단 배열 비우기
				
				for (const FVector& PointLoc : NavPath->PathPoints)// PathPoints: FVector의 TArray ->우리가 스플라인에 활용할 포인트들을 모아놓은 배열
				{
					Spline->AddSplinePoint(PointLoc, ESplineCoordinateSpace::World);//스플라인에 포인트 넣기
				}
				if (NavPath->PathPoints.Num()>0)
				{
					CachedDestination =NavPath->PathPoints[NavPath->PathPoints.Num()-1];//마지막 점을 CachedDestination으로 만들어 장애물 우회할 때 오류 방지
					bAutoRunning = true;
				}
			}
			

		}
		//Released됐으면 초기화
		FollowTime = 0;
		TargetingStatus = ETargetingStatus::NotTargeting;
	}

}

void AAuraPlayerController::AbilityInputTagHeld(FGameplayTag InputTag)
{
	if (GetASC() && GetASC()->HasMatchingGameplayTag(FAuraGameplayTags::Get().Player_Block_InputHeld))
	{
		return;
	}
	if (!InputTag.MatchesTagExact(FAuraGameplayTags::Get().InputTag_LMB)) //LMB아닌 경우 InputTag에 해당하는 Ability를 발동시키고 싶다.
	{
		if (GetASC())//Set이 안 되어 nullptr가 되는경우 대비
		{
			GetASC()->AbilityInputTagHeld(InputTag);
		}
		return;
	}
	
	if (TargetingStatus == ETargetingStatus::TargetingEnemy || bShiftKeyDown) //LMB이고 Targeting이면(Enemy에 마우스를 갖다대고 있는 상태이면), 또는 shift key가 눌러져있는 상태이면
	{
		if (GetASC())
		{
			GetASC()->AbilityInputTagHeld(InputTag);
		}
	}
	else //Targeting이 아니라면
	{
		FollowTime += GetWorld()->GetDeltaSeconds(); //마우스 누르는 시간

		// if (GetHitResultUnderCursor(ECC_Visibility, false, Hit))// 마우스에 닿은 것 있는지 확인
		// {
		// 	CachedDestination = Hit.ImpactPoint;
		// } 
		if (CursorHit.bBlockingHit) //위 코드를 사용하면 CursorTrace()에서 구한 걸 한 번 더 구하게 됨. 그래서 CursurHit을 멤버변수로 승격시키고 이것만 활용하도록 한다. 
		{
			CachedDestination = CursorHit.ImpactPoint;
		}

		if (APawn* ControlledPawn = GetPawn())
		{
			const FVector WorldDirection = (CachedDestination - ControlledPawn-> GetActorLocation()).GetSafeNormal();
;			ControlledPawn->AddMovementInput(WorldDirection);
		}
	}
	
}

UAuraAbilitySystemComponent* AAuraPlayerController::GetASC()
{
	if (AuraAbilitySystemComponent == nullptr)
	{
		AuraAbilitySystemComponent = Cast<UAuraAbilitySystemComponent>(UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(GetPawn<APawn>()));//한 번만 Cast하고 그 이후에는 멤버변수에 set
	}
	return AuraAbilitySystemComponent;
}


void AAuraPlayerController::ShowMagicCircle(UMaterialInterface* DecalMaterial)
{
	//없으면 새로 만들어서 저장
	if (!IsValid(MagicCircle)) 
	{
		MagicCircle = GetWorld()->SpawnActor<AMagicCircle>(MagicCircleClass);
		if (DecalMaterial)
		{
			MagicCircle->MagicCircleDecal->SetMaterial(0, DecalMaterial);
		}
	}
}

void AAuraPlayerController::HideMagicCircle()
{
	if (IsValid(MagicCircle))
	{
		MagicCircle->Destroy();
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

	UAuraInputComponent*  AuraInputComponent= CastChecked<UAuraInputComponent>(InputComponent);//InputComponent는 내장된 변수. UEnhancedInputComponent타입을 저장하도록 언리얼엔진에서 설정
	//MappingContext에 있는 IA들에 대한 변수 필요
	AuraInputComponent->BindAction(MoveAction,ETriggerEvent::Triggered, this, &AAuraPlayerController::Move);
	AuraInputComponent->BindAction(ShiftAction, ETriggerEvent::Started, this, &AAuraPlayerController::ShiftPressed);
	AuraInputComponent->BindAction(ShiftAction, ETriggerEvent::Completed, this, &AAuraPlayerController::ShiftReleased);
	AuraInputComponent->BindAbilityActions(InputConfig, this, &ThisClass::AbilityInputTagPressed, &ThisClass::AbilityInputTagReleased, &ThisClass::AbilityInputTagHeld);
	
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


