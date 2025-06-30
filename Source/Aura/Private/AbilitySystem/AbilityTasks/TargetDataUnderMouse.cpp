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
		const FGameplayAbilitySpecHandle SpecHandle = GetAbilitySpecHandle();
		const FPredictionKey ActivationPredictionKey = GetActivationPredictionKey();
		AbilitySystemComponent.Get()->AbilityTargetDataSetDelegate(SpecHandle, ActivationPredictionKey) //target데이터와 연관된 prediction key
		.AddUObject(this,&UTargetDataUnderMouse::OnTargetDataReplicatedCallback);
		const bool bCalledDelegate = AbilitySystemComponent.Get()->CallReplicatedTargetDataDelegatesIfSet(SpecHandle, ActivationPredictionKey);//delegate가 이미 set되었는지, 아직 listen중인지

		if (!bCalledDelegate)
		{
			SetWaitingOnRemotePlayerData(); //PlayerData를 기다르도록(아직 서버에 도달하지 못 했으므로)
		}
	}

	
}

void UTargetDataUnderMouse::SendMouseCursorData()//target data생성 및 전달
{
	FScopedPredictionWindow ScopedPrediction(AbilitySystemComponent.Get());//이 영역에서 이 줄 아래에 있는 것들은 다 Predicted된다.
	
	APlayerController* PC = Ability->GetCurrentActorInfo()->PlayerController.Get();
	FHitResult CursorHit;
	PC->GetHitResultUnderCursor(ECC_Visibility, false, CursorHit);

	FGameplayAbilityTargetDataHandle DataHandle;
	FGameplayAbilityTargetData_SingleTargetHit* Data = new FGameplayAbilityTargetData_SingleTargetHit();
	Data->HitResult = CursorHit;//커서 밑에 대한 데이터
	DataHandle.Add(Data);
	AbilitySystemComponent->ServerSetReplicatedTargetData(
		GetAbilitySpecHandle(),
		GetActivationPredictionKey(),
		DataHandle,
		FGameplayTag(),
		AbilitySystemComponent->ScopedPredictionKey
		);//AbilityTask는 멤버변수로 ASC를 갖고 있다.
	//original prediction key: 언제 이 Ability가 originally하게 activate되는지 알려준다.

	if (ShouldBroadcastAbilityTaskDelegates())//어빌리티가 Activate중인지 보장해주는 함수, 액티브 하지 않다면 브로드캐스팅을 막아주도록 하자.
	{
		ValidData.Broadcast(DataHandle);
	}
}

void UTargetDataUnderMouse::OnTargetDataReplicatedCallback(const FGameplayAbilityTargetDataHandle& DataHandle, //Activate함수의 Else에서 바인드 될 함수
	FGameplayTag ActivationTag)
{
	AbilitySystemComponent->ConsumeClientReplicatedTargetData(GetAbilitySpecHandle(), GetActivationPredictionKey());//TargetData를 받았으니 저장할 필요 없다는 것을 알려준다.(replicate된 데이터를 서버가 받았을 때 특정 자료구조에 저장하고 우리는 ASC에게 받았으니 저장할 필요가 없다고 말해줘야 한다. 받지 않았으면 저장해서 나중에 사용해야 하므로)
	if (ShouldBroadcastAbilityTaskDelegates())
	{
		ValidData.Broadcast(DataHandle);
	}
}
