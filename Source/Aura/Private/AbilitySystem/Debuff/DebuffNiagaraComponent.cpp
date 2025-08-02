// Copyright LeeSeungwon


#include "AbilitySystem/Debuff/DebuffNiagaraComponent.h"
#include "AbilitySystemBlueprintLibrary.h"
#include "AbilitySystemComponent.h"
#include "Interaction/CombatInterface.h"

UDebuffNiagaraComponent::UDebuffNiagaraComponent()
{
	bAutoActivate = false;

}

void UDebuffNiagaraComponent::BeginPlay()
{
	Super::BeginPlay();
	ICombatInterface* CombatInterface = Cast<ICombatInterface>((GetOwner()));

	UAbilitySystemComponent* ASC = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(GetOwner());
	if (ASC)//ASC가 있으면 ASC에 바로 바인딩
	{
		ASC->RegisterGameplayTagEvent(DebuffTag, EGameplayTagEventType::NewOrRemoved).AddUObject(this, &UDebuffNiagaraComponent::DebuffTagChanged);
	}
	else if (CombatInterface) //순간에 ASC는없고 CombatInterface구현해놓은 클래스라면 InitAbilityActorInfo에서 ASC등록할 때 OnASCRegistered를 브로드캐스팅해서RegisterGameplayTagEvent에 바인딩 할 수 있도록 OnASCRegistered델리게이트를 얻을 수 있는 함수 GetOnASCRegisteredDelegate에 바인딩시킨다.
	{	//Character, Enemy의InitAbilityActorInfo에서 브로드캐스팅
		CombatInterface->GetOnASCRegisteredDelegate().AddWeakLambda(this, [this](UAbilitySystemComponent * InASC)
		{
			InASC->RegisterGameplayTagEvent(DebuffTag, EGameplayTagEventType::NewOrRemoved).AddUObject(this, &UDebuffNiagaraComponent::DebuffTagChanged);
		}
		);//WeakLambda는 그냥Lambda보다 좀 더 안전-UObject를 인수로 가지는데 이 오브젝트를 weakpointer로 감싼다. 그리고 이를 추적해서 언제까지 Valid한지를 알 수 있다. 만약 Valid하지 않으면 Lambda는 더이상 Execute되지 않는다.
	}
	if (CombatInterface)
	{
		CombatInterface->GetOnDeathDelegate().AddDynamic(this, &UDebuffNiagaraComponent::OnOwnerDeath);//AAuraCharacterBase::MulticastHandleDeath_Implementation에서 브로드캐스트
	}
}

void UDebuffNiagaraComponent::DebuffTagChanged(const FGameplayTag CallbackTag, int32 NewCount)
{

	const bool bOwnerValid = IsValid(GetOwner());
	const bool bOwnerAlive = GetOwner()->Implements<UCombatInterface>() && !ICombatInterface::Execute_IsDead(GetOwner());

	
	if (NewCount > 0 && bOwnerValid && bOwnerAlive)
	{
		Activate();
	}
	else 
	{
		Deactivate();
	}
}

void UDebuffNiagaraComponent::OnOwnerDeath(AActor* DeadActor)
{
	Deactivate();
}
