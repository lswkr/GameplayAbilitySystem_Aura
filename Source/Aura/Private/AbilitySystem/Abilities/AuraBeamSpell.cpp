// Copyright LeeSeungwon


#include "AbilitySystem/Abilities/AuraBeamSpell.h"
#include "GameFramework/Character.h"
#include "Kismet/KismetSystemLibrary.h"
#include "AbilitySystem/AuraAbilitySystemLibrary.h"

void UAuraBeamSpell::StoreMouseDataInfo(const FHitResult& HitResult)
{
	if (HitResult.bBlockingHit)
	{
		MouseHitLocation = HitResult.ImpactPoint;
		MouseHitActor = HitResult.GetActor();
	}
	else
	{
		CancelAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true);
	}
}

void UAuraBeamSpell::StoreOwnerVariables()
{
	if (CurrentActorInfo)
	{
		OwnerPlayerController = CurrentActorInfo->PlayerController.Get();
		OwnerCharacter = Cast<ACharacter>(CurrentActorInfo->AvatarActor);//캐릭터 얻고 거기서 무브먼트 얻어서 스펠 도중 움직일 수 없게 하려고
	}
}

void UAuraBeamSpell::TraceFirstTarget(const FVector& BeamTargetLocation)
{
	check(OwnerCharacter);
	if (OwnerCharacter->Implements<UCombatInterface>())
	{
		if (USkeletalMeshComponent* Weapon = ICombatInterface::Execute_GetWeapon(OwnerCharacter))
		{
			TArray<AActor*> ActorsToIgnore;
			ActorsToIgnore.Add(OwnerCharacter);
			FHitResult HitResult;
			const FVector SocketLocation = Weapon->GetSocketLocation(FName("TipSocket"));//나중에 멤버 변수로 만들어서 따로 설정도 가능
			//너무 정확하게 말고 근방에 있으면 잡히도록 Sphere(시작점에서 끝점까지 Sphere기둥이 생긴다.선 대신 Sphere 모양의 선으로 한 것으로 보면 되겠다.)
			UKismetSystemLibrary::SphereTraceSingle(
				OwnerCharacter, 
				SocketLocation, 
				BeamTargetLocation, 
				10.f, 
				TraceTypeQuery1, 
				false, 
				ActorsToIgnore, 
				EDrawDebugTrace::None,
				HitResult,
				true
			);

			if (HitResult.bBlockingHit)
			{
				MouseHitLocation = HitResult.ImpactPoint;
				MouseHitActor = HitResult.GetActor();
			}	
		}
	}
	if (ICombatInterface* CombatInterface = Cast<ICombatInterface>(MouseHitActor))
	{
		if (!CombatInterface->GetOnDeathDelegate().IsAlreadyBound(this, &UAuraBeamSpell::PrimaryTargetDied))//this, &UAuraBeamSpell::PrimaryTargetDied가 바인드 되어있는지 체크
		{
			CombatInterface->GetOnDeathDelegate().AddDynamic(this, &UAuraBeamSpell::PrimaryTargetDied);
		}
	}
	
}

void UAuraBeamSpell::StoreAdditionalTarget(TArray<AActor*>& OutAdditionalTargets)
{
	TArray<AActor*> ActorsToIgnore;
	ActorsToIgnore.Add(GetAvatarActorFromActorInfo());
	ActorsToIgnore.Add(MouseHitActor);//이미 적용한 액터이므로 무시하도록 한다.

	TArray<AActor*> OverlappingActors;
	
	UAuraAbilitySystemLibrary::GetLivePlayerWithinRadius(
		GetAvatarActorFromActorInfo(),
		OverlappingActors,
		ActorsToIgnore,
		850.f,
		MouseHitActor->GetActorLocation()
	);

	int32 NumAdditionalTargets = FMath::Min(GetAbilityLevel()-1, MaxNumShockTarget);
	//int32 NumAdditionalTargets = 5;

	UAuraAbilitySystemLibrary::GetClosestTargets(NumAdditionalTargets, OverlappingActors, OutAdditionalTargets, MouseHitActor->GetActorLocation());

	for (AActor* Target : OutAdditionalTargets) {
		if (ICombatInterface* CombatInterface = Cast<ICombatInterface>(Target))
		{
			if (!CombatInterface->GetOnDeathDelegate().IsAlreadyBound(this, &UAuraBeamSpell::AdditionalTargetDied))
			{
				CombatInterface->GetOnDeathDelegate().AddDynamic(this, &UAuraBeamSpell::AdditionalTargetDied);
			}
		}
	}
}

void UAuraBeamSpell::RemoveOnDeathNotify(AActor* Actor)//333강 Q&A 참고
{
	if (const auto CombatInterface = Cast<ICombatInterface>(Actor))
	{
		if (CombatInterface->GetOnDeathDelegate().IsAlreadyBound(this, &ThisClass::AdditionalTargetDied))
		{
			CombatInterface->GetOnDeathDelegate().RemoveDynamic(this, &ThisClass::AdditionalTargetDied);
		}
		if (CombatInterface->GetOnDeathDelegate().IsAlreadyBound(this, &ThisClass::PrimaryTargetDied))
		{
			CombatInterface->GetOnDeathDelegate().RemoveDynamic(this, &ThisClass::PrimaryTargetDied);
		}
	}
}
