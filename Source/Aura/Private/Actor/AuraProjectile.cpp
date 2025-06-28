// Copyright LeeSeungwon


#include "Actor/AuraProjectile.h"

#include "Components/SphereComponent.h"
#include "GameFramework/ProjectileMovementComponent.h"

// Sets default values
AAuraProjectile::AAuraProjectile()
{
 	PrimaryActorTick.bCanEverTick = false;
	bReplicates = true; //projectile을 서버에서 소환하고 싶고 레플리케이트시키고 싶음, 클라이언트는 레플리케이트버전을 활용하도록 true
	
	Sphere = CreateDefaultSubobject<USphereComponent> ("Sphere");
	SetRootComponent(Sphere);
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
	Sphere->OnComponentBeginOverlap.AddDynamic(this, &AAuraProjectile::OnSphereOverlap);
}

void AAuraProjectile::OnSphereOverlap(UPrimitiveComponent* OverlappedCOmponent, AActor* OtherActor,
	UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	
}


