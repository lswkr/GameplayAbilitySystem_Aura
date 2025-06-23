// Copyright LeeSeungwon


#include "AbilitySystem/ModMagCalc/MMC_MaxHealth.h"

#include "AbilitySystem/AuraAttributeSet.h"
#include "Interaction/CombatInterface.h"

UMMC_MaxHealth::UMMC_MaxHealth()
{
	//GE에서 본 것들과 같은 행동들
	VigorDef.AttributeToCapture = UAuraAttributeSet::GetVigorAttribute(); //매크로로 나온 함수
	VigorDef.AttributeSource = EGameplayEffectAttributeCaptureSource::Target;//source, Target 모두 Aura이므로 상관 없음
	VigorDef.bSnapshot = false; //GE Effect Spec이 만들어졌을 때 캡처할 것인지, 적용되었을 때 캡처할 것인지에 대한 부울 변수

	RelevantAttributesToCapture.Add(VigorDef); //위에서 VigorDef를 정의한 다음 RelevantAttributesToCapture어레이에 집어넣는 행동

	//모디파이어가 실행되고 이펙트가 적용되면 application이 될 때 Vigor이 캡처되고 우리는 CalculateBaseMagnitude_Implementation안에서 접근할 수 있게 된다.
}

float UMMC_MaxHealth::CalculateBaseMagnitude_Implementation(const FGameplayEffectSpec& Spec) const
{
	//Gather Tags from Source and Target
	const FGameplayTagContainer* SourceTags = Spec.CapturedSourceTags.GetAggregatedTags();//GESpec이 source와 Target에서 캡처된 태그를 가지고 있고 그 태그에 이렇게 접근한다.
	const FGameplayTagContainer* TargetTags = Spec.CapturedTargetTags.GetAggregatedTags();

	FAggregatorEvaluateParameters EvaluationParameters;
	EvaluationParameters.SourceTags = SourceTags;
	EvaluationParameters.TargetTags = TargetTags;

	float Vigor = 0.f;
	GetCapturedAttributeMagnitude(VigorDef, Spec, EvaluationParameters, Vigor); //Def로 캡처된 값 가져오기
	Vigor = FMath::Max<float>(Vigor, 0.f);

	ICombatInterface* CombatInterface = Cast<ICombatInterface>(Spec.GetContext().GetSourceObject());//SourceObject는 현재 Aura이다.
	const int32 PlayerLevel = CombatInterface->GetPlayerLevel();

	return 80.f + 2.5f * Vigor + 10.f * PlayerLevel;
}
