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

void AAuraEffectActor::ApplyEffectToTarget(AActor* Target, TSubclassOf<UGameplayEffect> GameplayEffectClass)
{

	UAbilitySystemComponent* TargetASC = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(Target);

	if (TargetASC==nullptr) return;

	check(GameplayEffectClass);
	
	FGameplayEffectContextHandle EffectContextHandle = TargetASC->MakeEffectContext();
	EffectContextHandle.AddSourceObject(this);
	//어떤 ASC든 GameplayEffectClass만 있으면 GameplayEffectSpec을 만들 수 있다.
	const FGameplayEffectSpecHandle EffectSpecHandle = TargetASC->MakeOutgoingSpec(GameplayEffectClass, 1.f, EffectContextHandle);//gameplay effect는 각자의 레벨을 가지고 있다. 이 함수는 SpecHandle을 반환한다.
	//Spec EffectSpec - 보통 Handle을 줄여서 저렇게 명명한다.
	TargetASC->ApplyGameplayEffectSpecToSelf(*EffectSpecHandle.Data.Get());//Data는 스마트 포인터(TSharedPtr)이다. 그래서 진짜의 RawPointer를 얻으려면 Get()함수를 써야한다. Get함수가 포인터를 반환하는데 지금 함수는 값을 원하므로 *까지 붙여주면 끝
	
}

