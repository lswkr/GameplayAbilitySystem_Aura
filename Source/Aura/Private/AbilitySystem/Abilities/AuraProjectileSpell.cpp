// Copyright LeeSeungwon


#include "AbilitySystem/Abilities/AuraProjectileSpell.h"

#include "Actor/AuraProjectile.h"
#include "Interaction/CombatInterface.h"

void UAuraProjectileSpell::ActivateAbility(const FGameplayAbilitySpecHandle Handle,
                                           const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo,
                                           const FGameplayEventData* TriggerEventData)
{
	Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);
	
	
	
	
}

void UAuraProjectileSpell::SpawnProjectile()
{
	//projectile을 서버에서 소환하고 싶고 레플리케이트시키고 싶음, 클라이언트는 레플리케이트버전을 가지고 노는 것

	
	const bool bIsServer = GetAvatarActorFromActorInfo()->HasAuthority(); //HasAuthority: 이 함수를 통해 서버에 있는지 확인 가능

	if (!bIsServer) return; //서버 아니면 바로 return;

	ICombatInterface* CombatInterface = Cast<ICombatInterface>(GetAvatarActorFromActorInfo());

	if (CombatInterface)
	{
		const FVector SocketLocation = CombatInterface->GetCombatSocketLocation();

		FTransform SpawnTransform; //특정 소켓을 반환받아 그 소켓의 트랜스폼을 활용할 것임. CombatInterface활용, AuraCharacterBase에서 확인
		SpawnTransform.SetLocation(SocketLocation);
		//TODO: Set the Projectile Rotation
		
		AAuraProjectile* Projectile = GetWorld()->SpawnActorDeferred<AAuraProjectile>(
			ProjectileClass,
			SpawnTransform,
			GetOwningActorFromActorInfo(),
			Cast<APawn>(GetOwningActorFromActorInfo()),
			ESpawnActorCollisionHandlingMethod::AlwaysSpawn
			);
		//TODO: Give the Projectile a GameplayEffectSpec for Causing Damage.
		Projectile->FinishSpawning(SpawnTransform);
		
	
	}
	
}
