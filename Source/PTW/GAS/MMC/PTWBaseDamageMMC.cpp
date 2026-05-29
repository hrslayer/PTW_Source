#include "PTWBaseDamageMMC.h"

float UPTWBaseDamageMMC::CalculateBaseMagnitude_Implementation(const FGameplayEffectSpec& Spec) const
{
	return Super::CalculateBaseMagnitude_Implementation(Spec);
}

float UPTWBaseDamageMMC::GetDamageAfterDefense(float RawDamage, const FGameplayEffectSpec& Spec) const
{
	const FGameplayTagContainer* SourceTags = Spec.CapturedSourceTags.GetAggregatedTags();
	const FGameplayTagContainer* TargetTags = Spec.CapturedTargetTags.GetAggregatedTags();
    
	FAggregatorEvaluateParameters EvaluationParams;
	EvaluationParams.SourceTags = SourceTags;
	EvaluationParams.TargetTags = TargetTags;

	float DefensePoint = 0.0f;
	GetCapturedAttributeMagnitude(FPTWDamageStatics::DamageStatics().DefenseDef, Spec, EvaluationParams, DefensePoint);
    
	DefensePoint = FMath::Max<float>(DefensePoint, 0.0f);
	const float DamageReduction = DefensePoint / (DefensePoint + 100.0f);
    
	return RawDamage * (1.0f - DamageReduction);
}
