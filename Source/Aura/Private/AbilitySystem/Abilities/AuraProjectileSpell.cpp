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

	ICombatInterface* CombatInterface = Cast<ICombatInterface>(GetAvatarActorFromActorInfo());

	if (CombatInterface)
	{
		const FVector SocketLocation = CombatInterface->GetCombatSocketLocation();
		FRotator Rotation = (ProjectileTargetLocation - SocketLocation).Rotation();// 타겟이 작아서 기울어질 수 있으나 땅에 평행으로 가게 하고 싶다.
		Rotation.Pitch= 0.f; //평행하게 가도록 피치를 0으로
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
		
		const FGameplayEffectSpecHandle SpecHandle= SourceASC->MakeOutgoingSpec(DamageEffectClass, GetAbilityLevel(),EffectContextHandle); //projectile에 줄 spechandle(Projectile클래스에 DamageEffectSpecHandle 변수에 설정)

		const FAuraGameplayTags GameplayTags = FAuraGameplayTags::Get();
		const float ScaledDamage = Damage.GetValueAtLevel(/*GetAbilityLevel()*/10);

//		GEngine->AddOnScreenDebugMessage(-1,3.f, FColor::Red, FString::Printf(TEXT("FireBolt Damage: %f"),ScaledDamage));

		UAbilitySystemBlueprintLibrary::AssignTagSetByCallerMagnitude(SpecHandle, GameplayTags.Damage, ScaledDamage);
		Projectile->DamageEffectSpecHandle = SpecHandle;
		
		Projectile->FinishSpawning(SpawnTransform);
	}
}
