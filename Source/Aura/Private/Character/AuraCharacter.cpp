// Copyright LeeSeungwon


#include "Character/AuraCharacter.h"

#include "AbilitySystemComponent.h"
#include "AbilitySystem/AuraAbilitySystemComponent.h"
#include "Player/AuraPlayerState.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Player/AuraPlayerController.h"
#include "UI/HUD/AuraHUD.h"


AAuraCharacter::AAuraCharacter()
{
	GetCharacterMovement()->bOrientRotationToMovement = true;
	GetCharacterMovement()->RotationRate = FRotator(0.f,400.f,0.f);
	GetCharacterMovement()->bConstrainToPlane = true;
	GetCharacterMovement()->bSnapToPlaneAtStart = true;

	bUseControllerRotationPitch = false;
	bUseControllerRotationYaw = false;
	bUseControllerRotationRoll = false;


}

void AAuraCharacter::PossessedBy(AController* NewController)
{
	Super::PossessedBy(NewController);

	//init ability actor info for the server
	InitAbilityActorInfo();
	AddCharacterAbilities();
}

void AAuraCharacter::OnRep_PlayerState() //OnRep_PlayerState 함수의 호출은 PlayerState 객체의 변경사항이 복제(replication) 될 때 발생, 게임 시작 후 캐릭터가 생성되거나, 죽어서 새로 생성될 때 OnRep_PlayerState가 호출되는 이유는 PlayerState 내의 특정 값이 초기화되거나 변경되어, 이 변화가 클라이언트에 전달되기 때문.
{
	Super::OnRep_PlayerState();

	//init ability actor info for the client
	InitAbilityActorInfo();
}

int32 AAuraCharacter::GetPlayerLevel()
{
	const AAuraPlayerState* AuraPlayerState = GetPlayerState<AAuraPlayerState>();
	check(AuraPlayerState);

	return AuraPlayerState->GetPlayerLevel();
}

void AAuraCharacter::InitAbilityActorInfo()
{
	AAuraPlayerState* AuraPlayerState = GetPlayerState<AAuraPlayerState>(); //pawn함수. PlayerState를 AuraPlayerState로 캐스트
	check(AuraPlayerState);
	AuraPlayerState->GetAbilitySystemComponent()->InitAbilityActorInfo(AuraPlayerState,this); //사용하고 있는 PlayerState에 있는 ASC를 해당 캐릭터로 초기화, Owner는 당연히 PlayerState이고 World에서 보이는 Actor는 AuraCharacter
	Cast<UAuraAbilitySystemComponent> (AuraPlayerState->GetAbilitySystemComponent())->AbilityActorInfoSet(); //AuraAbilitySystemComponent 클래스의 EffectApplied 함수를 Binding시키기 위해 한 번 호출
	AbilitySystemComponent = AuraPlayerState->GetAbilitySystemComponent(); //초기화 마친 PlayerState의 ASC를 캐릭터에 설정
	AttributeSet=AuraPlayerState->GetAttributeSet();

	//아래 컨트롤러가 null인지 체크해서 assert를 해야하나? 클라이언트는 각각의 컨트롤러만 신경쓰면 됨. null이면 진행은 시키지 않되 crash내지 않는다. 
	if (AAuraPlayerController* AuraPlayerController = Cast<AAuraPlayerController>(GetController()))
	{
		if(AAuraHUD* AuraHUD = Cast<AAuraHUD>(AuraPlayerController->GetHUD()))
		{
			AuraHUD->InitOverlay(AuraPlayerController, AuraPlayerState, AbilitySystemComponent, AttributeSet); //PC, PS, ASC, AS 4개 다 설정된 상태이므로 HUD의 오버레이를 초기화할 수 있게 되었다.
		}
	}
	InitializeDefaultAttributes();// 서버에서만불러도 어차피 Replicate되게 되어있어 클라이언트에 자동으로 가게 되어 있지만 두 군데에서 다 호출해도 상관없다. ASC가 결정되어 있는 곳에서 불러야하므로 여기서 호출한다.
}
