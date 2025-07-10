// Copyright LeeSeungwon


#include "AI/AuraAIController.h"

#include "BehaviorTree/BehaviorTreeComponent.h"
#include "BehaviorTree/BlackboardComponent.h"

AAuraAIController::AAuraAIController()
{
	Blackboard = CreateDefaultSubobject<UBlackboardComponent> ("BlackboardComponent"); // blackboard라는 변수는 이미 AIController에 있으므로 따로 변수 만들 필요 없이 여기에서 이렇게 CreateDefaultSubobject해주면 됨.
	check(Blackboard);
	
	BehaviorTreeComponent= CreateDefaultSubobject<UBehaviorTreeComponent>("BehaviorTreeComponent");
	check(BehaviorTreeComponent);
}
