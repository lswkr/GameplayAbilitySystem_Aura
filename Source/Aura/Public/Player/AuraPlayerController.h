// Copyright LeeSeungwon

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "GameplayTagContainer.h"
#include "AuraPlayerController.generated.h"

class UInputMappingContext;
class UInputAction;
struct FInputActionValue;
class UAuraInputConfig;
class UAuraAbilitySystemComponent;
class USplineComponent;
class UDamageTextComponent;
class UNiagaraSystem;
class AMagicCircle;
class IHighlightInterface;

enum class ETargetingStatus : uint8
{
	TargetingEnemy,
	TargetingNonEnemy,
	NotTargeting
};

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

	
	//클라RPC를 만들것이다(서버에 호출, 서버에서 작동.컨트롤링플레이어가 local인 경우. 컨트롤링플레이어가 Remote인 경우 이 함수는 클라이언트에게 실행)
	UFUNCTION(Client, Reliable)
	void ShowDamageNumber(float DamageAmount, ACharacter* TargetCharacter, bool bBlockedHit, bool bCriticalHit); //TargetCharacter: 데미지 위젯 붙일 캐릭터
	
	UFUNCTION(BlueprintCallable)
	void ShowMagicCircle(UMaterialInterface* DecalMaterial = nullptr);

	UFUNCTION(BlueprintCallable)
	void HideMagicCircle();
protected:
	virtual void BeginPlay() override;
	virtual void SetupInputComponent() override;

private:
	UPROPERTY(EditAnywhere, Category = "Input")
	TObjectPtr<UInputMappingContext> AuraContext;
	
	UPROPERTY(EditAnywhere, Category = "Input")
	TObjectPtr<UInputAction> MoveAction; //MappingContext에서 설정된 IA 데이터

	UPROPERTY(EditAnywhere, Category = "Input")
	TObjectPtr<UInputAction> ShiftAction; 

	void ShiftPressed() { bShiftKeyDown = true; };
	void ShiftReleased() { bShiftKeyDown = false; };
	bool bShiftKeyDown = false;
	
	void Move(const FInputActionValue& InputActionValue); //매 프레임 호출

	void CursorTrace();
	//이번 프레임에 hover할 캐릭터, 지난 프레임에 hover한 캐릭터에 대한 포인터 두 개, 같을 수도, 다를 수도 있다.
	TObjectPtr<AActor> LastActor;
	TObjectPtr<AActor> ThisActor;
	FHitResult CursorHit;//한 번 구하면 다른 곳에서도 사용할 수 있도록 멤버변수로 설정
	static void HighlightActor(AActor* InActor);
	static void UnHighlightActor(AActor* InActor);
	
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
	ETargetingStatus TargetingStatus = ETargetingStatus::NotTargeting;//마우스커서가 뭘 Targeting하는지
	
	UPROPERTY(EditDefaultsOnly)
	float AutoRunAcceptanceRadius = 50.f;

	UPROPERTY(VisibleAnywhere)
	TObjectPtr<USplineComponent> Spline;

	UPROPERTY(EditDefaultsOnly)
	TObjectPtr<UNiagaraSystem> ClickNiagaraSystem;

	void AutoRun();

	UPROPERTY(EditDefaultsOnly)
	TSubclassOf<UDamageTextComponent> DamageTextComponentClass;

	UPROPERTY(EditDefaultsOnly)
	TSubclassOf<AMagicCircle> MagicCircleClass;

	UPROPERTY()
	TObjectPtr<AMagicCircle> MagicCircle; //저장용 변수

	void UpdateMagicCircleLocation();

};
