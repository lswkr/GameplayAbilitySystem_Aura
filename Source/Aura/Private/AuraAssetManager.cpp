// Copyright LeeSeungwon


#include "AuraAssetManager.h"
#include "AuraGameplayTags.h"
#include "AbilitySystemGlobals.h"

UAuraAssetManager& UAuraAssetManager::Get()
{
	check(GEngine);
	
	UAuraAssetManager* AuraAssetManager = Cast<UAuraAssetManager>(GEngine->AssetManager);
	return *AuraAssetManager;//포인터가 아닌 실제 AssetManager를 캐스트해서 반환
}

void UAuraAssetManager::StartInitialLoading()
{
	Super::StartInitialLoading();

	FAuraGameplayTags::InitializeNativeGameplayTags();
	//This is required to use TargetData.
	UAbilitySystemGlobals::Get().InitGlobalData();//이거 해줘야 서버, 클라이언트 간 연결 잘 된다는데 안 해도 일단 난 되긴 했음
	
}
