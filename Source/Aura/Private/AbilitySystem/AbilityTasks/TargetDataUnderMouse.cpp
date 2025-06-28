// Copyright LeeSeungwon


#include "AbilitySystem/AbilityTasks/TargetDataUnderMouse.h"

UTargetDataUnderMouse* UTargetDataUnderMouse::CreateTargetDataUnderMouse(UGameplayAbility* OwningAbility)
{
	UTargetDataUnderMouse* MyObj = NewAbilityTask<UTargetDataUnderMouse> (OwningAbility);
	return MyObj;//노드의 Async Task로 리턴된다.
}

void UTargetDataUnderMouse::Activate()
{
	APlayerController* PC = Ability->GetCurrentActorInfo()->PlayerController.Get();
	
	FHitResult CursorHit;
	PC->GetHitResultUnderCursor(ECC_Visibility, false, CursorHit);

	
	ValidData.Broadcast(CursorHit.Location);
	
}
