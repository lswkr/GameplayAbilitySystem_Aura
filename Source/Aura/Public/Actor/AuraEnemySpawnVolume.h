// Copyright LeeSeungwon

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Interaction/SaveInterface.h"
#include "AuraEnemySpawnVolume.generated.h"

class UBoxComponent;
class AAuraEnemySpawnPoint;

UCLASS()
class AURA_API AAuraEnemySpawnVolume : public AActor, public ISaveInterface/*한 번 스폰되면 그 이후로 스폰 안 되도록*/
{
	GENERATED_BODY()
	
public:	
	AAuraEnemySpawnVolume();

	/* Save Interface */ 
	virtual void LoadActor_Implementation() override; 
	/* End Save Interface */

	UPROPERTY(BlueprintReadOnly, SaveGame)
	bool bReached = false;
	
protected:
	virtual void BeginPlay() override;

	UFUNCTION()
	virtual void OnSphereOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult & SweepResult);

	UPROPERTY(EditAnywhere)
	TArray<AAuraEnemySpawnPoint*> SpawnPoints;
private:
	UPROPERTY(VisibleAnywhere)
	TObjectPtr<UBoxComponent> Box;

};
