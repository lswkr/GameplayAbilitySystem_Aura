// Copyright LeeSeungwon


#include "Actor/AuraEffectActor.h"
#include "AbilitySystemBlueprintLibrary.h"
#include "AbilitySystem/AuraAbilitySystemComponent.h"


AAuraEffectActor::AAuraEffectActor()
{
	PrimaryActorTick.bCanEverTick = false;
	SetRootComponent(CreateDefaultSubobject<USceneComponent> ("SceneRoot")); //최대한의 유연성을 위해 Scenecomponent만 하나 생성해서 RootComponent로 설정
	
}


void AAuraEffectActor::BeginPlay()
{
	Super::BeginPlay();
}

void AAuraEffectActor::ApplyEffectToTarget(AActor* TargetActor, TSubclassOf<UGameplayEffect> GameplayEffectClass)
{

	UAbilitySystemComponent* TargetASC = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(TargetActor);

	if (TargetASC==nullptr) return;

	check(GameplayEffectClass);
	
	FGameplayEffectContextHandle EffectContextHandle = TargetASC->MakeEffectContext();
	EffectContextHandle.AddSourceObject(this);
	//어떤 ASC든 GameplayEffectClass만 있으면 GameplayEffectSpec을 만들 수 있다.
	const FGameplayEffectSpecHandle EffectSpecHandle = TargetASC->MakeOutgoingSpec(GameplayEffectClass, ActorLevel, EffectContextHandle);//gameplay effect는 각자의 레벨을 가지고 있다. 이 함수는 SpecHandle을 반환한다.
	//Spec EffectSpec - 보통 Handle을 줄여서 저렇게 명명한다.
	const FActiveGameplayEffectHandle ActiveEffectHandle = TargetASC->ApplyGameplayEffectSpecToSelf(*EffectSpecHandle.Data.Get());//Data는 스마트 포인터(TSharedPtr)이다. 그래서 진짜의 RawPointer를 얻으려면 Get()함수를 써야한다. Get함수가 포인터를 반환하는데 지금 함수는 값을 원하므로 *까지 붙여주면 끝
	//ApplyGameplayEffectSpecToSelf는 GE적용하면 GE는 Active상태가 되고 FGameplayEffectHandle반환- infinite일 경우 이 핸들과 ASC를 저장해서 이후 처리에 활용->Map에 저장

	const bool bIsInfinite = EffectSpecHandle.Data.Get()->Def.Get()->DurationPolicy == EGameplayEffectDurationType::Infinite;//Def-GE그 자체의 ObjectPtr

	if (bIsInfinite && InfiniteEffectRemovalPolicy == EEffectRemovalPolicy::RemoveOnEndOverlap)//infinite 중 Remove해야할 것들만 저장하기 위해
	{
		ActiveEffectHandles.Add(ActiveEffectHandle, TargetASC);
	}
}

void AAuraEffectActor::OnOverlap(AActor* TargetActor)
{
	if (InstantEffectApplicationPolicy == EEffectApplicationPolicy::ApplyOnOverlap)
	{
		ApplyEffectToTarget(TargetActor, InstantGameplayEffectClass);
	}

	if (DurationEffectApplicationPolicy == EEffectApplicationPolicy::ApplyOnOverlap)
	{
		ApplyEffectToTarget(TargetActor, DurationGameplayEffectClass);
	}
	if (InfiniteEffectApplicationPolicy == EEffectApplicationPolicy::ApplyOnOverlap)
	{
		ApplyEffectToTarget(TargetActor, InfiniteGameplayEffectClass);
	}
}

void AAuraEffectActor::OnEndOverlap(AActor* TargetActor)
{
	if (InstantEffectApplicationPolicy == EEffectApplicationPolicy::ApplyOnEndOverlap)
	{
		ApplyEffectToTarget(TargetActor, InstantGameplayEffectClass);
	}

	if (DurationEffectApplicationPolicy == EEffectApplicationPolicy::ApplyOnEndOverlap)
	{
		ApplyEffectToTarget(TargetActor, DurationGameplayEffectClass);
	}

	if (InfiniteEffectApplicationPolicy == EEffectApplicationPolicy::ApplyOnEndOverlap)
	{
		ApplyEffectToTarget(TargetActor, InfiniteGameplayEffectClass);
	}

	if (InfiniteEffectRemovalPolicy == EEffectRemovalPolicy::RemoveOnEndOverlap)
	{
		UAbilitySystemComponent* TargetASC = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(TargetActor);
		if (!IsValid(TargetASC)) return;

		//Effect를 찾아서 삭제하는데 반복 중에 이를 삭제하는 것은 크래시 일으킴->따로 모아서 삭제하도록 Array만들기
		TArray <FActiveGameplayEffectHandle> HandlesToRemove;
		for (TTuple<FActiveGameplayEffectHandle, UAbilitySystemComponent*> HandlePair : ActiveEffectHandles)
		{
			if (TargetASC == HandlePair.Value)
			{
				TargetASC->RemoveActiveGameplayEffect(HandlePair.Key, 1);//stack제거하는 개수도 설정 가능, 디폴트 값이 -1이다. 설정 안 해주면 겹쳐져 들어갔다가 하나에서라도 나오면 안에 들어가 있더라도 이펙트가 다 사라져버림
				HandlesToRemove.Add(HandlePair.Key);
			}
		}
		for (FActiveGameplayEffectHandle& Handle: HandlesToRemove)
		{
			ActiveEffectHandles.FindAndRemoveChecked(Handle);//Effect핸들 삭제
		}
	
	}
	
}

