// Copyright LeeSeungwon


#include "Character/AuraEnemy.h"

#include "AbilitySystem/AuraAbilitySystemComponent.h"
#include "AbilitySystem/AuraAbilitySystemLibrary.h"
#include "AbilitySystem/AuraAttributeSet.h"
#include "Components/WidgetComponent.h"
#include "Aura/Aura.h"
#include "UI/Widget/AuraUserWidget.h"
#include "AuraGameplayTags.h"
#include "AI/AuraAIController.h"
#include "BehaviorTree/BehaviorTree.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "GameFramework/CharacterMovementComponent.h"

AAuraEnemy::AAuraEnemy()
{
	GetMesh()->SetCollisionResponseToChannel(ECC_Visibility,ECR_Block);

	//Enemy는 직접적으로 ASC,AS를 가지게 했다. Aura Character는 가지지 않음
	AbilitySystemComponent = CreateDefaultSubobject<UAuraAbilitySystemComponent>("AbilitySystemComponent");
	AbilitySystemComponent->SetIsReplicated(true);
	AbilitySystemComponent->SetReplicationMode(EGameplayEffectReplicationMode::Minimal);

	bUseControllerRotationPitch = false;
	bUseControllerRotationYaw = false;
	bUseControllerRotationRoll = false;

	GetCharacterMovement()->bUseControllerDesiredRotation = true;
	
	AttributeSet = CreateDefaultSubobject<UAuraAttributeSet>("AttributeSet");

	HealthBar = CreateDefaultSubobject<UWidgetComponent>("HealthBar");
	HealthBar->SetupAttachment(GetRootComponent());
}

void AAuraEnemy::PossessedBy(AController* NewController)
{
	Super::PossessedBy(NewController);

	if (!HasAuthority()) return; //AI는 서버에서만 신경쓰면 되므로 HasAuthority
	
	AuraAIController = Cast<AAuraAIController> (NewController);
	AuraAIController->GetBlackboardComponent()->InitializeBlackboard(*BehaviorTree->BlackboardAsset);//AIController에 있는 블랙보드에 우리가 설정해놓은 비헤이비어 트리를 가져와 거기에 설정된 블랙보드 애셋으로 블랙보드를 초기화한다. 
	AuraAIController->RunBehaviorTree(BehaviorTree);
	AuraAIController->GetBlackboardComponent()->SetValueAsBool(FName("HitReacting"), false); //HitReacting부울 변수를 처음에 false로 초기화하기 위해
	AuraAIController->GetBlackboardComponent()->SetValueAsBool(FName("RangedAttacker"), CharacterClass != ECharacterClass::Warrior);

}

void AAuraEnemy::HighlightActor()
{
	GetMesh()->SetRenderCustomDepth(true);
	GetMesh()->SetCustomDepthStencilValue(CUSTOM_DEPTH_RED);
	Weapon->SetRenderCustomDepth(true); //어차피 캐릭터에 딸려 있는 놈이므로 함께 null일테고 함께 valid할 것이므로 같이 해도 된다.
	Weapon->SetCustomDepthStencilValue(CUSTOM_DEPTH_RED);
}

void AAuraEnemy::UnHighlightActor()
{
	GetMesh()->SetRenderCustomDepth(false);
	Weapon->SetRenderCustomDepth(false);
}

void AAuraEnemy::SetCombatTarget_Implementation(AActor* InCombatTarget)
{
	CombatTarget = InCombatTarget;
}

AActor* AAuraEnemy::GetCombatTarget_Implementation()
{
	return CombatTarget;
}

int32 AAuraEnemy::GetPlayerLevel_Implementation()
{
	return Level;
}

void AAuraEnemy::Die(const FVector& DeathImpulse)
{
	SetLifeSpan(LifeSpan);

	if (AuraAIController) AuraAIController->GetBlackboardComponent()->SetValueAsBool(FName("Dead"), true);
	
	Super::Die(DeathImpulse);
}


void AAuraEnemy::BeginPlay()
{
	Super::BeginPlay();
	GetCharacterMovement()->MaxWalkSpeed = BaseWalkSpeed; //설정된 BaseWalkSpeed값으로 MaxWalkSpeed를 조정하기 위해 BeginPlay에서 설정.
	InitAbilityActorInfo();
	if (HasAuthority()) //서버에서 어빌리티들 넣어주기
	{
		UAuraAbilitySystemLibrary::GiveStartupAbilities(this, AbilitySystemComponent, CharacterClass);//da에 있는 공통 어빌리티 초기화
	}
	if (UAuraUserWidget* AuraUserWidget = Cast<UAuraUserWidget>(HealthBar->GetUserWidgetObject()))
	{
		AuraUserWidget->SetWidgetController(this); //위젯 컨트롤러 set. Enemy는 머리 위에 HealthBar를 띄워 놓을 것이므로 컨트롤러를 Enemy자체로 설정한다.
	} //바인딩 할 컨트롤러 Set
	
	
	//값 초기화 된 후 여기서 위젯 바인딩 시키기, OverlayWidgetController의 BindCallbacksToDependencies역할
	if (const UAuraAttributeSet* AuraAS = Cast<UAuraAttributeSet>(AttributeSet))
	{
		AbilitySystemComponent->GetGameplayAttributeValueChangeDelegate(AuraAS->GetHealthAttribute()).AddLambda(
			[this](const FOnAttributeChangeData& Data)
			{
				OnHealthChanged.Broadcast(Data.NewValue);
			}
		);
		AbilitySystemComponent->GetGameplayAttributeValueChangeDelegate(AuraAS->GetMaxHealthAttribute()).AddLambda(
			[this](const FOnAttributeChangeData& Data)
			{
				OnMaxHealthChanged.Broadcast(Data.NewValue);
			}
		);
		
		AbilitySystemComponent->RegisterGameplayTagEvent(FAuraGameplayTags::Get().Effects_HitReact, EGameplayTagEventType::NewOrRemoved).AddUObject( //특정 태그가 Added 되거나 Removed될 때 이벤트를 등록시켜주는 함수. 델리게이트를 반환한다.(Gameplaytag, int32 tagcount)
			this,
			&AAuraEnemy::HitReactTagChanged
		);
		//Attribute변할 때의 델리게이트에 바인딩 시킨 후 첫 값 브로드캐스팅, AuraCharacter의 BroadcastInitialValue역할
		OnHealthChanged.Broadcast(AuraAS->GetHealth());
		OnMaxHealthChanged.Broadcast(AuraAS->GetMaxHealth());
	}
	
}

void AAuraEnemy::HitReactTagChanged(const FGameplayTag CallbackTag, int32 NewCount)
{
	bHitReacting = NewCount > 0;
	GetCharacterMovement()->MaxWalkSpeed =  bHitReacting ? 0.f : BaseWalkSpeed;// 한 방 맞으면 0.f 아니면 평소 속도
	if (AuraAIController && AuraAIController->GetBlackboardComponent())//AIController가 클라이언트에 없으므로 null 오류 발생 가능하기에 if문 추가.
	{
		AuraAIController->GetBlackboardComponent()->SetValueAsBool(FName("HitReacting"), bHitReacting);

	}
}

void AAuraEnemy::InitAbilityActorInfo()
{
	AbilitySystemComponent->InitAbilityActorInfo(this,this);
	Cast<UAuraAbilitySystemComponent>(AbilitySystemComponent)->AbilityActorInfoSet();

	if (HasAuthority())
	{
		InitializeDefaultAttributes(); //이 함수 돌리기 전에 블루프린트에서 어트리뷰트를 다 정해놓아야 함
	}
	OnASCRegistered.Broadcast(AbilitySystemComponent);//CharacterBase에 넣을 수도 있으나 Super잘못하면 망가질 수 있어서 그냥 이렇게 Enemy, Aura 둘 다 따로 구현
}

void AAuraEnemy::InitializeDefaultAttributes() const
{
	UAuraAbilitySystemLibrary::InitializeDefaultAttribute(this, CharacterClass, Level, AbilitySystemComponent);
}
