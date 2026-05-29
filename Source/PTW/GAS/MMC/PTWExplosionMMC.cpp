
#include "PTWExplosionMMC.h"

#include "PTWGameplayTag/GameplayTags.h"


UPTWExplosionMMC::UPTWExplosionMMC()
{
	RelevantAttributesToCapture.Add(FPTWDamageStatics::DamageStatics().WeaponDamageDef);
	RelevantAttributesToCapture.Add(FPTWDamageStatics::DamageStatics().DefenseDef);
}

float UPTWExplosionMMC::CalculateBaseMagnitude_Implementation(const FGameplayEffectSpec& Spec) const
{
	float RawDamage = FMath::Abs(Spec.GetSetByCallerMagnitude(GameplayTags::Data::Damage, false, 0.0f));
	
	float DamageAfterDefense = GetDamageAfterDefense(RawDamage, Spec);
	
	float Distance = Spec.GetSetByCallerMagnitude(GameplayTags::Data::Distance, false, 0.0f);
	float Radius = Spec.GetSetByCallerMagnitude(GameplayTags::Data::Radius, false, 1.0f);

	float FalloffMultiplier = FMath::Clamp(1.0f - (Distance / Radius), 0.0f, 1.0f);
    
	return FMath::Max<float>(DamageAfterDefense * FalloffMultiplier, 1.0f);
}
