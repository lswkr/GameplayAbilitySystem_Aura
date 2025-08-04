// Copyright LeeSeungwon


#include "AbilitySystem/Abilities/AuraFireBolt.h"

#include "AbilitySystem/AuraAbilitySystemLibrary.h"
#include "Actor/AuraProjectile.h"
#include "GameFramework/ProjectileMovementComponent.h"

FString UAuraFireBolt::GetDescription(int32 Level)
{
	const int32 ScaledDamage = Damage.GetValueAtLevel(Level);
	const float ManaCost = FMath::Abs(GetManaCost(Level));
	const float Cooldown = GetCooldown(Level);
	if (Level==1)
	{
		return FString::Printf(TEXT(
			//Title
			"<Title>FIRE BOLT</>\n\n"
			//Level
			"<Small>Level: </><Level>%d</>\n"
			//ManaCost
			"<Small>ManaCost: </><ManaCost>%.1f</>\n"
			//Cooldown
			"<Small>Cooldown: </><Cooldown>%.1f</>\n\n"
			
			//Description
			"<Default>Launches a bolt of fire,"
			" exploding on impact and dealing: </>"
			//Damage
			"<Damage>%d</><Default> fire damage with"
			" a chance to burn</>"
			),
			//Values
			Level,
			ManaCost,
			Cooldown,
			ScaledDamage);//""안에 "로 나눠도 상관없다.	
	}
	else
	{
		return FString::Printf(TEXT(
			//Title
			"<Title>FIRE BOLT</>\n\n"
			//Level
			"<Small>Level: </><Level>%d</>\n"
			
			//ManaCost
			"<Small>ManaCost: </><ManaCost>%.1f</>\n"
			
			//Cooldown
			"<Small>Cooldown: </><Cooldown>%.1f</>\n\n"
			//Number of FireBolts
			"<Default>Launches %d bolts of fire,"
			" exploding on impact and dealing: </>"
			//Damage
			"<Damage>%d</><Default> fire damage with\n"
			" a chance to burn</>"
			),
			//Values
			Level,
			ManaCost,
			Cooldown,
			FMath::Min(Level,NumProjectiles),
			ScaledDamage);//""안에 "로 나눠도 상관없다.	
	}
}

FString UAuraFireBolt::GetNextLevelDescription(int32 Level)
{
	const int32 ScaledDamage = Damage.GetValueAtLevel(Level);
	const float ManaCost = FMath::Abs(GetManaCost(Level));
	const float Cooldown = GetCooldown(Level);
	
	return FString::Printf(TEXT(
				//Title
				"<Title>NEXT LEVEL:</>\n\n"
				//Level
				"<Small>Level: </><Level>%d</>\n"
				//ManaCost
				"<Small>ManaCost: </><ManaCost>%.1f</>\n"

				//Cooldown
				"<Small>Cooldown: </><Cooldown>%.1f</>\n\n"
				//Number of FireBolts
				"<Default>Launches %d bolts of fire,"
				" exploding on impact and dealing: </>\n"
				//Damage
				"<Damage>%d</><Default> fire damage with"
				" a chance to burn</>"
				),
				//Values
				Level,
				ManaCost,
				Cooldown,
				FMath::Min(Level,NumProjectiles),
				ScaledDamage);//""안에 "로 나눠도 상관없다.	
}

void UAuraFireBolt::SpawnProjectiles(const FVector& ProjectileTargetLocation, const FGameplayTag& SocketTag, bool bOverridePitch, float PitchOverride, AActor* HomingTarget)
{
	const bool bIsServer = GetAvatarActorFromActorInfo()->HasAuthority(); 

	if (!bIsServer) return; 

	const FVector SocketLocation = ICombatInterface::Execute_GetCombatSocketLocation(GetAvatarActorFromActorInfo(), SocketTag);
	FRotator Rotation = (ProjectileTargetLocation - SocketLocation).Rotation();
	if (bOverridePitch) Rotation.Pitch = PitchOverride;

	const FVector Forward = Rotation.Vector();
	const int32 EffectiveNumProjectiles = FMath::Min(MaxNumProjectiles, GetAbilityLevel());
	TArray <FRotator> Rotations = UAuraAbilitySystemLibrary::EvenlySpacedRotators(Forward, FVector::UpVector, ProjectileSpread, EffectiveNumProjectiles);
	
	for (const FRotator& Rot : Rotations)
	{
		FTransform SpawnTransform; //특정 소켓을 반환받아 그 소켓의 트랜스폼을 활용할 것임. CombatInterface활용, AuraCharacterBase에서 확인
		SpawnTransform.SetLocation(SocketLocation);
		SpawnTransform.SetRotation(Rot.Quaternion());

		AAuraProjectile* Projectile = GetWorld()->SpawnActorDeferred<AAuraProjectile>(
			ProjectileClass,
			SpawnTransform,
			GetAvatarActorFromActorInfo(),//GetAvatarActorFromActorInfo, owningactor에서 avatar로 일단 바꿔놓음
			Cast<APawn>(GetAvatarActorFromActorInfo()),
			ESpawnActorCollisionHandlingMethod::AlwaysSpawn
		);

		//Give the Projectile a GameplayEffectSpec for Causing Damage.
		Projectile->DamageEffectParams = MakeDamageEffectParamsFromClassDefaults();
		
		if (HomingTarget && HomingTarget->Implements<UCombatInterface>()) // HomingTarget이 null인경우가 있을 수 있다.(아무것도 없는 곳을 누른 경우)
		{
			Projectile->ProjectileMovement->HomingTargetComponent = HomingTarget->GetRootComponent();//Movement컴포넌트는 딱히 레퍼런스카운팅할 필요 없으므로 Weakptr이다.
		}
		else //Combatinterface구현 안 한 벽 같은것들
		{
			Projectile->HomingTargetSceneComponent = NewObject<USceneComponent>(USceneComponent::StaticClass()); //HomingTargetSceneComponent이 생성되면 NewObject로는 가비지콜렉트가 안 되는데 이렇게 UPROPERTY가 된 변수에다가 지정해주면 가비지콜렉트가 된다. 
			Projectile->HomingTargetSceneComponent->SetWorldLocation(ProjectileTargetLocation);
			Projectile->ProjectileMovement->HomingTargetComponent = Projectile->HomingTargetSceneComponent;
		}
		Projectile->ProjectileMovement->HomingAccelerationMagnitude = FMath::FRandRange(HomingAccelerationMin, HomingAccelerationMax);
		Projectile->ProjectileMovement->bIsHomingProjectile = bLaunchHomingProjectiles;

		Projectile->FinishSpawning(SpawnTransform);
	}

	
}
