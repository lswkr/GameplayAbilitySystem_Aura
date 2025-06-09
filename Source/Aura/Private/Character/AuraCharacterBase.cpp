// Copyright LeeSeungwon


#include "Character/AuraCharacterBase.h"

AAuraCharacterBase::AAuraCharacterBase()
{
 	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = false;

}


void AAuraCharacterBase::BeginPlay()
{
	Super::BeginPlay();
	
}