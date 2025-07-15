// Copyright LeeSeungwon


#include "AbilitySystem/AbilityTasks/TargetDataUnderMouse.h"
#include "AbilitySystemComponent.h"

UTargetDataUnderMouse* UTargetDataUnderMouse::CreateTargetDataUnderMouse(UGameplayAbility* OwningAbility)
{
	UTargetDataUnderMouse* MyObj = NewAbilityTask<UTargetDataUnderMouse> (OwningAbility);
	return MyObj;//노드의 Async Task로 리턴된다.
}

void UTargetDataUnderMouse::Activate()
{
	const bool bIsLocallyControlled = Ability->GetCurrentActorInfo()->IsLocallyControlled();
	if (bIsLocallyControlled)
	{
		SendMouseCursorData();
	}
	else
	{
		//TODO: We are on the server, so listen for target data.
		const FGameplayAbilitySpecHandle SpecHandle = GetAbilitySpecHandle(); //현 Ability의 스펙핸들
		const FPredictionKey ActivationPredictionKey = GetActivationPredictionKey(); // 현 Ability의 프리딕션 키
		AbilitySystemComponent.Get()->AbilityTargetDataSetDelegate(SpecHandle, ActivationPredictionKey) //target데이터와 연관된 prediction key(/** Returns TargetDataSet delegate for a given Ability/PredictionKey pair */)
		.AddUObject(this,&UTargetDataUnderMouse::OnTargetDataReplicatedCallback);//해당 ability와 predictionkey에 해당하는 델리게이트에 OnTargetDataReplicatedCallback를 바인딩. 서버가 클라이언트에서 Replicate된 데이터를 받은 경우 그 데이터를 쓰도록 하는 함수.
		const bool bCalledDelegate = AbilitySystemComponent.Get()->CallReplicatedTargetDataDelegatesIfSet(SpecHandle, ActivationPredictionKey);//delegate가 이미 set되었는지, 아직 listen중인지

		if (!bCalledDelegate)
		{
			SetWaitingOnRemotePlayerData(); //PlayerData(TargetData)를 기다리도록(아직 서버에 도달하지 못 했으므로)
		}
	}

	
}

void UTargetDataUnderMouse::SendMouseCursorData()//target data생성 및 전달
{
	FScopedPredictionWindow ScopedPrediction(AbilitySystemComponent.Get());//이 영역에서 이 줄 아래에 있는 것들은 다 Predicted된다.
	
	APlayerController* PC = Ability->GetCurrentActorInfo()->PlayerController.Get();
	FHitResult CursorHit; //커서에 닿은 것
	PC->GetHitResultUnderCursor(ECC_Visibility, false, CursorHit);

	FGameplayAbilityTargetDataHandle DataHandle;
	FGameplayAbilityTargetData_SingleTargetHit* Data = new FGameplayAbilityTargetData_SingleTargetHit();

	Data->HitResult = CursorHit;//커서에 닿은 HitResult를 Data에 저장
	DataHandle.Add(Data); //DataHandle에 Data저장

	AbilitySystemComponent->ServerSetReplicatedTargetData( //Replicates targeting data to the server
		GetAbilitySpecHandle(),
		GetActivationPredictionKey(),
		DataHandle,
		FGameplayTag(),
		AbilitySystemComponent->ScopedPredictionKey
		);//AbilityTask는 멤버변수로 ASC를 갖고 있다.
	//original prediction key: 언제 이 Ability가 originally하게 activate되는지 알려준다.

	if (ShouldBroadcastAbilityTaskDelegates())//어빌리티가 Activate중인지 보장해주는 함수, 액티브 하지 않다면 브로드캐스팅을 막아주도록 한다.
	{
		ValidData.Broadcast(DataHandle);
	}
}

void UTargetDataUnderMouse::OnTargetDataReplicatedCallback(const FGameplayAbilityTargetDataHandle& DataHandle, //Activate함수의 Else에서 바인드 될 함수
	FGameplayTag ActivationTag)
{
	AbilitySystemComponent->ConsumeClientReplicatedTargetData(GetAbilitySpecHandle(), GetActivationPredictionKey());//TargetData를 받았으니 저장할 필요 없다는 것을 알려준다.(replicate된 데이터를 서버가 받았을 때 특정 자료구조에 저장하고 우리는 ASC에게 데이터를 받았으니 저장할 필요가 없다고 말해줘야 한다. 받지 않았으면 저장해서 나중에 사용해야 하므로)
	/* ConsumeClientReplicatedTargetData: 클라이언트에서 복제된 타겟 데이터만 소비합니다. */
	if (ShouldBroadcastAbilityTaskDelegates())
	{
		ValidData.Broadcast(DataHandle);
	}
}
