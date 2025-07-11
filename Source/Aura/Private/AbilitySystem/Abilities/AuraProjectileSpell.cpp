// Copyright LeeSeungwon


#include "AbilitySystem/Abilities/AuraProjectileSpell.h"

#include "AbilitySystemBlueprintLibrary.h"
#include "AbilitySystemComponent.h"
#include "Actor/AuraProjectile.h"
#include "Interaction/CombatInterface.h"
#include "Aura/Public/AuraGameplayTags.h"

void UAuraProjectileSpell::ActivateAbility(const FGameplayAbilitySpecHandle Handle,
                                           const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo,
                                           const FGameplayEventData* TriggerEventData)
{
	Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);
	
	
	
	
}

void UAuraProjectileSpell::SpawnProjectile(const FVector& ProjectileTargetLocation)
{
	//projectile을 서버에서 소환하고 싶고 레플리케이트시키고 싶음, 클라이언트는 레플리케이트버전을 가지고 노는 것
	
	const bool bIsServer = GetAvatarActorFromActorInfo()->HasAuthority(); //HasAuthority: 이 함수를 통해 서버에 있는지 확인 가능

	if (!bIsServer) return; //서버 아니면 바로 return;

	const FVector SocketLocation = ICombatInterface::Execute_GetCombatSocketLocation(GetAvatarActorFromActorInfo(), FAuraGameplayTags::Get().Montage_Attack_Weapon);
	FRotator Rotation = (ProjectileTargetLocation - SocketLocation).Rotation();
	
	if (GetOwningActorFromActorInfo() == nullptr)
	{
		UE_LOG(LogTemp, Error, TEXT("OwningActor is Null"));
	}
	if (!Cast<APawn>(GetAvatarActorFromActorInfo()))//owningactor하면 playerstate라 pawn으로 안되는듯
	{
		UE_LOG(LogTemp, Error, TEXT("OwningActor cast is failed"));
	
	}
	
	FTransform SpawnTransform; //특정 소켓을 반환받아 그 소켓의 트랜스폼을 활용할 것임. CombatInterface활용, AuraCharacterBase에서 확인
	SpawnTransform.SetLocation(SocketLocation);
	//TODO: Set the Projectile Rotation
	SpawnTransform.SetRotation(Rotation.Quaternion());
	AAuraProjectile* Projectile = GetWorld()->SpawnActorDeferred<AAuraProjectile>(
		ProjectileClass,
		SpawnTransform,
		GetAvatarActorFromActorInfo(),//GetAvatarActorFromActorInfo, owningactor에서 avatar로 일단 바꿔놓음
		Cast<APawn>(GetAvatarActorFromActorInfo()),
		ESpawnActorCollisionHandlingMethod::AlwaysSpawn
		);
	//TODO: Give the Projectile a GameplayEffectSpec for Causing Damage.
	
	const UAbilitySystemComponent* SourceASC = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(GetAvatarActorFromActorInfo());
	FGameplayEffectContextHandle EffectContextHandle = SourceASC->MakeEffectContext();
	EffectContextHandle.SetAbility(this);
	EffectContextHandle.AddSourceObject(Projectile);
	TArray<TWeakObjectPtr<AActor>> Actors;
	Actors.Add(Projectile);
	EffectContextHandle.AddActors(Actors);
	FHitResult HitResult;
	HitResult.Location = ProjectileTargetLocation;
	EffectContextHandle.AddHitResult(HitResult);
	
	const FGameplayEffectSpecHandle SpecHandle= SourceASC->MakeOutgoingSpec(DamageEffectClass, GetAbilityLevel(),EffectContextHandle); //projectile에 줄 spechandle(Projectile클래스에 DamageEffectSpecHandle 변수에 설정)

	const FAuraGameplayTags GameplayTags = FAuraGameplayTags::Get();
	

	for (auto& Pair:DamageTypes)
	{
		const float ScaledDamage = Pair.Value.GetValueAtLevel(GetAbilityLevel());
		UAbilitySystemBlueprintLibrary::AssignTagSetByCallerMagnitude(SpecHandle, Pair.Key, ScaledDamage);
	}
	//GEngine->AddOnScreenDebugMessage(-1,3.f, FColor::Red, FString::Printf(TEXT("FireBolt Damage: %f"),ScaledDamage));

	
	Projectile->DamageEffectSpecHandle = SpecHandle;
	
	Projectile->FinishSpawning(SpawnTransform);
	
}
