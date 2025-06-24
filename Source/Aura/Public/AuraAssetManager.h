// Copyright LeeSeungwon

#pragma once

#include "CoreMinimal.h"
#include "Engine/AssetManager.h"
#include "AuraAssetManager.generated.h"

/**
 * 
 */
UCLASS()
class AURA_API UAuraAssetManager : public UAssetManager
{
	GENERATED_BODY()

public:
	static UAuraAssetManager& Get();

protected:
	virtual void StartInitialLoading() override; //게임의 에셋을 로딩할 때 호출(매우 일찍 호출됨.) InitializeNativeTags함수 호출하기 위한 함수
};
