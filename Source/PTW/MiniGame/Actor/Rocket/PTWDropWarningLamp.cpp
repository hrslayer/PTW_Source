#include "PTWDropWarningLamp.h"

#include "Components/SpotLightComponent.h"


APTWDropWarningLamp::APTWDropWarningLamp()
{
	PrimaryActorTick.bCanEverTick = true;
	bReplicates = true;
	
	SceneComponent = CreateDefaultSubobject<USceneComponent>("SceneComponent");
	SetRootComponent(SceneComponent);
	
	LampMesh = CreateDefaultSubobject<UStaticMeshComponent>("LampMesh");
	LampMesh->SetupAttachment(SceneComponent);
	
	SpotLight = CreateDefaultSubobject<USpotLightComponent>("SpotLight");
	SpotLight->SetupAttachment(SceneComponent);
}

void APTWDropWarningLamp::StartDropWarningLamp()
{
	if (HasAuthority())
	{
		MulticastRPC_StartWarningLight();
	}
}

void APTWDropWarningLamp::StopDropWarningLamp()
{
	if (HasAuthority())
	{
		MulticastRPC_StopWarningLight();
	}
}

void APTWDropWarningLamp::MulticastRPC_StartWarningLight_Implementation()
{
	bIsWarning = true;
	SpotLight->SetVisibility(true);
	SpotLight->SetLightColor(FLinearColor::Red);
}

void APTWDropWarningLamp::MulticastRPC_StopWarningLight_Implementation()
{
	bIsWarning = false;
	SpotLight->SetVisibility(false);
}

void APTWDropWarningLamp::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);
	
	if (bIsWarning)
	{
		FRotator NewRotation = SpotLight->GetRelativeRotation();
		NewRotation.Yaw += RotationSpeed * DeltaSeconds;
		SpotLight->SetRelativeRotation(NewRotation);
	}
}

// Called when the game starts or when spawned
void APTWDropWarningLamp::BeginPlay()
{
	Super::BeginPlay();
	SpotLight->SetVisibility(false);
}

