// Fill out your copyright notice in the Description page of Project Settings.


#include "PTWGA_KnockBackProjectileFire.h"

#include "CoreFramework/PTWPlayerCharacter.h"

void UPTWGA_KnockBackProjectileFire::PerformFireAction(const FPTWFireConext Context)
{
	Super::PerformFireAction(Context);
	ApplyKnockBack(Context);
}

void UPTWGA_KnockBackProjectileFire::ApplyKnockBack(const FPTWFireConext& Context)
{
	if (Context.PC)
	{
		FVector LaunchDir = -Context.PC->GetControlRotation().Vector();
		LaunchDir.Z = FMath::Clamp(LaunchDir.Z + 0.6f, 0.5f, 1.2f);
		
		Context.PC->LaunchCharacter(LaunchDir * LaunchDirStrength, true, true);
	}
}
