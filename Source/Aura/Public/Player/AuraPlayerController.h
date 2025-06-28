// Copyright LeeSeungwon

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "GameplayTagContainer.h"
#include "AuraPlayerController.generated.h"

class UInputMappingContext;
class UInputAction;
struct FInputActionValue;
class IEnemyInterface;
class UAuraInputConfig;
class UAuraAbilitySystemComponent;
class USplineComponent;

/**
 * 
 */
UCLASS()
class AURA_API AAuraPlayerController : public APlayerController
{
	GENERATED_BODY()

public:
	AAuraPlayerController();
	virtual void PlayerTick(float DeltaTime) override;
	
protected:
	virtual void BeginPlay() override;
	virtual void SetupInputComponent() override;

private:
	UPROPERTY(EditAnywhere, Category = "Input")
	TObjectPtr<UInputMappingContext> AuraContext;
	
	UPROPERTY(EditAnywhere, Category = "Input")
	TObjectPtr<UInputAction> MoveAction; //MappingContext에서 설정된 IA 데이터

	void Move(const FInputActionValue& InputActionValue); //매 프레임 호출

	void CursorTrace();
	//이번 프레임에 hover할 캐릭터, 지난 프레임에 hover한 캐릭터에 대한 포인터 두 개, 같을 수도, 다를 수도 있다.
	TScriptInterface<IEnemyInterface> LastActor;
	TScriptInterface<IEnemyInterface> ThisActor;
	FHitResult CursorHit;//한 번 구하면 다른 곳에서도 사용할 수 있도록 멤버변수로 설정
	
	void AbilityInputTagPressed(FGameplayTag InputTag);
	void AbilityInputTagReleased(FGameplayTag InputTag);
	void AbilityInputTagHeld(FGameplayTag InputTag);
	
	
	UPROPERTY(EditDefaultsOnly, Category="Input")
	TObjectPtr<UAuraInputConfig> InputConfig;

	UPROPERTY()
	TObjectPtr<UAuraAbilitySystemComponent> AuraAbilitySystemComponent;

	UAuraAbilitySystemComponent* GetASC();

	FVector CachedDestination = FVector::ZeroVector;
	float FollowTime = 0.f;//얼마나 마우스를 누르고 있는지 측정하는 시간변수
	float ShortPressThreshold = 0.5f; //얼마나 짧게 눌러야 ShortPress인지 판단하는 기준 변수
	bool bAutoRunning = false;//true일 동안 매 프레임마다 AddMovementInput사용할 것임
	bool bTargeting = false;//적을 마우스커서가 Targeting하는지
	UPROPERTY(EditDefaultsOnly)
	float AutoRunAcceptanceRadius = 50.f;

	UPROPERTY(VisibleAnywhere)
	TObjectPtr<USplineComponent> Spline;

	void AutoRun();

	
	
};
