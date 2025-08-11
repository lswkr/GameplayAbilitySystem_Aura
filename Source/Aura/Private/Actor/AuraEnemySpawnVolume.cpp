// Copyright LeeSeungwon


#include "Actor/AuraEnemySpawnVolume.h"

#include "Components/BoxComponent.h"
#include "Interaction/PlayerInterface.h"
#include "Actor/AuraEnemySpawnPoint.h"

// Sets default values
AAuraEnemySpawnVolume::AAuraEnemySpawnVolume()
{
	PrimaryActorTick.bCanEverTick = false;

	Box = CreateDefaultSubobject<UBoxComponent>("Box");
	SetRootComponent(Box);
	Box->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	Box->SetCollisionObjectType(ECC_WorldStatic);
	Box->SetCollisionResponseToAllChannels(ECR_Ignore);
	Box->SetCollisionResponseToChannel(ECC_Pawn,ECR_Overlap);
}

void AAuraEnemySpawnVolume::LoadActor_Implementation()
{
	if (bReached)
	{
		Destroy();
	}
}

// Called when the game starts or when spawned
void AAuraEnemySpawnVolume::BeginPlay()
{
	Super::BeginPlay();
	Box->OnComponentBeginOverlap.AddDynamic(this, &AAuraEnemySpawnVolume::OnSphereOverlap);
	
}

void AAuraEnemySpawnVolume::OnSphereOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
	UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if (!OtherActor->Implements<UPlayerInterface>()) return; //플레이어 아니면 바로 리턴
	bReached = true;
	for (AAuraEnemySpawnPoint* Point :SpawnPoints)
	{
		if (IsValid(Point))
		{
			Point->SpawnEnemy();
		}
	}

	//Destroy();->여기서 파괴해버리면 게임모드의 LoadWorldState 함수에서 IsValid()에 걸려 저장이 안 됨->그래서 그냥 닿지 않게만
	Box->SetCollisionEnabled(ECollisionEnabled::NoCollision);
}

