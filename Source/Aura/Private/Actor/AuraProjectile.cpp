// Copyright LeeSeungwon


#include "Actor/AuraProjectile.h"

#include "AbilitySystemBlueprintLibrary.h"
#include "AbilitySystemComponent.h"
#include "Components/SphereComponent.h"
#include "Aura/Aura.h"
#include "GameFramework/ProjectileMovementComponent.h"
#include "Kismet/GameplayStatics.h"
#include "NiagaraFunctionLibrary.h"
#include "AbilitySystem/AuraAbilitySystemLibrary.h"
#include "Components/AudioComponent.h"

// Sets default values
AAuraProjectile::AAuraProjectile()
{
 	PrimaryActorTick.bCanEverTick = false;
	bReplicates = true; //projectile을 서버에서 소환하고 싶고 레플리케이트시키고 싶음, 클라이언트는 레플리케이트버전을 활용하도록 true
	
	Sphere = CreateDefaultSubobject<USphereComponent> ("Sphere");
	SetRootComponent(Sphere);
	Sphere->SetCollisionObjectType(ECC_Projectile);
	Sphere->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	Sphere->SetCollisionResponseToAllChannels(ECR_Ignore);//다 무시 때리고
	Sphere->SetCollisionResponseToChannel(ECC_WorldDynamic, ECR_Overlap); //특정 몇개만 콜리전
	Sphere->SetCollisionResponseToChannel(ECC_WorldStatic, ECR_Overlap);
	Sphere->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);

	ProjectileMovement = CreateDefaultSubobject<UProjectileMovementComponent>("ProjectileMovement");
	ProjectileMovement->InitialSpeed = 550.f;
	ProjectileMovement->MaxSpeed = 550.f;
	ProjectileMovement->ProjectileGravityScale = 0.f; //중력없이 날아감
}



void AAuraProjectile::BeginPlay()
{
	Super::BeginPlay();
	
	SetLifeSpan(LifeSpan);
	
	Sphere->OnComponentBeginOverlap.AddDynamic(this, &AAuraProjectile::OnSphereOverlap);
	
	LoopingSoundComponent = UGameplayStatics::SpawnSoundAttached(LoopingSound,GetRootComponent()); //sound컴포넌트를 반환하는 함수.->이후에 사운드를 멈출 수 있다. 액터에 붙지 않고 월드에 떠돌아 다니는 그런 특이한 컴포넌트이다.
}

void AAuraProjectile::Destroyed() 
{
	if (!bHit && !HasAuthority())
	{//destroy가 먼저 레플리케이트되어 Destroyed함수가 클라의OnSphereOverlap이전에 호출될 때, bhit은 false && 이건 클라에서만 관련됨(!HasAuthority)
		UGameplayStatics::PlaySoundAtLocation(this, ImpactSound, GetActorLocation(), FRotator::ZeroRotator);
		UNiagaraFunctionLibrary::SpawnSystemAtLocation(this, ImpactEffect, GetActorLocation());
		if (LoopingSoundComponent) LoopingSoundComponent->Stop();
		bHit = true;
	}
	Super::Destroyed();
}

void AAuraProjectile::OnSphereOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
                                      UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{	//UAuraProjectileSpell에서 SpawnProjectile할 때 서버가 아니면 빠른 Return을 하도록 했다.DamageEffectSpecHandle를 거기서 만들기에 클라에서는 DamageEffectSpecHandle가 Notvalid하다. 그래서 if문에 isValid를 추가해준다

	//if (OtherActor == GetOwner()) return;//162강 Q&A 답변
	if (!DamageEffectSpecHandle.Data.IsValid() || DamageEffectSpecHandle.Data.Get()->GetContext().GetEffectCauser() == OtherActor) // 쏜 놈이랑 맞은 놈이 같으면 아무것도 안 하도록(뭔가를 하면 닿은 뒤 파괴되므로)
	{
		return;
	}
	if (!UAuraAbilitySystemLibrary::IsNotFriend(DamageEffectSpecHandle.Data.Get()->GetContext().GetEffectCauser(), OtherActor))
	{
		return;
	}
	if (!bHit) //이렇게 해서 클라에서 두 번 소리가 나는 것을 방지
	{
		
		UGameplayStatics::PlaySoundAtLocation(this, ImpactSound, GetActorLocation(), FRotator::ZeroRotator);
		UNiagaraFunctionLibrary::SpawnSystemAtLocation(this, ImpactEffect, GetActorLocation());

		if (LoopingSoundComponent) LoopingSoundComponent->Stop();
		bHit = true;
	}
	
	
	if (HasAuthority())
	{
		//Projectile의 DamageGameplayEffect로 인한 Attribute의 변화는 그냥 서버에서만 해주면 알아서 레플리케이트 된다. 서버측에서만 해주자.
		if (UAbilitySystemComponent* TargetASC = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(OtherActor))
		{
			TargetASC->ApplyGameplayEffectSpecToSelf(*DamageEffectSpecHandle.Data.Get());
		}
		Destroy();
	}
	else
	{
		bHit=true; 
		//OnSphereOverlap이 destruction이 레플리케이트되기 전에 일어나면 bHit을 true로하여 Destroyed에서는 효과가 일어나지 못하도록 한다.
	}
}


