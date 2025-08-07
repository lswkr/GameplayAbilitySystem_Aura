// Copyright LeeSeungwon


#include "Actor/AuraFireBall.h"

#include "AbilitySystemBlueprintLibrary.h"
#include "AuraGameplayTags.h"
#include "AbilitySystem/AuraAbilitySystemGlobals.h"
#include "AbilitySystem/AuraAbilitySystemLibrary.h"
#include "Components/AudioComponent.h"
#include "GameplayCueManager.h"

void AAuraFireBall::BeginPlay()
{
	Super::BeginPlay();
	StartOutgoingTimeline();
}

void AAuraFireBall::OnSphereOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
                                    UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	//다른 로직 할 것이므로 Super없이 돌림
	if (!IsValidOverlap(OtherActor)) return;

	if (HasAuthority())
	{
		//Projectile의 DamageGameplayEffect로 인한 Attribute의 변화는 그냥 서버에서만 해주면 알아서 레플리케이트 된다. 서버측에서만 해주자.
		if (UAbilitySystemComponent* TargetASC = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(OtherActor))
		{
			DamageEffectParams.TargetAbilitySystemComponent = TargetASC;//SpawnProjectiles에서 DamageEffectParams의 TargetActor가 nullptr로 들어온다. 그래서 여기에서 설정해준다. 
			//이펙트 적용되기 전에 DeathImpulse를 줘야 함
			const FVector DeathImpulse = GetActorForwardVector() * DamageEffectParams.DeathImpulseMagnitude;
			DamageEffectParams.DeathImpulse = DeathImpulse;
			UAuraAbilitySystemLibrary::ApplyDamageEffect(DamageEffectParams);//DamageEffectParams완성해서 Effect를 적용
		}
	}
}

void AAuraFireBall::OnHit()
{
	//DamageEffectParams은 서버에서 세팅->클라이언트에 셋되는지 어떻게 알릴까?->모든 Actor의 Owner는 레플리케이트됨, FireBall은 Spawn되면서 owner세팅
	//레플리케이트 전에 OnHit이 호출될 수 있으므로 Owner가 설정되어있는지 확인해야함
	
	if (GetOwner())
	{
		FGameplayCueParameters CueParams;
		CueParams.Location = GetActorLocation();
		UGameplayCueManager::ExecuteGameplayCue_NonReplicated(GetOwner(), FAuraGameplayTags::Get().GameplayCue_FireBlast,CueParams);
	}
	
	////게임플레이 큐는 태그로 작동
	if (LoopingSoundComponent)
	{
		LoopingSoundComponent->Stop();
		LoopingSoundComponent->DestroyComponent();
	}
	bHit = true;
}
