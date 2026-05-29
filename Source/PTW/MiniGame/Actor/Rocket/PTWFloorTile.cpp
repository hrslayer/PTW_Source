
#include "PTWFloorTile.h"

#include "Components/TimelineComponent.h"
#include "MiniGame/GameMode/PTWRocketGameMode.h"

APTWFloorTile::APTWFloorTile()
{
	PrimaryActorTick.bCanEverTick = false;
	bReplicates = true;
	
	SceneComponent = CreateDefaultSubobject<USceneComponent>("SceneComponent");
	SetRootComponent(SceneComponent);
	
	SM = CreateDefaultSubobject<UStaticMeshComponent>("SM");
	SM->SetupAttachment(SceneComponent);
	
	FallTimeline = CreateDefaultSubobject<UTimelineComponent>(TEXT("TimeLineComp"));
	WarningTimeline = CreateDefaultSubobject<UTimelineComponent>(TEXT("WarningTimelineComp"));
}
void APTWFloorTile::BeginPlay()
{
	Super::BeginPlay();
	
	if (FallCurve)
	{
		FOnTimelineFloat UpdateDelegate;
		UpdateDelegate.BindUFunction(this, FName("HandleCollapseUpdate"));
		FallTimeline->AddInterpFloat(FallCurve, UpdateDelegate);
		
		FOnTimelineEvent FinishDelegate;
		FinishDelegate.BindUFunction(this, FName("HandleCollapseFinished"));
		FallTimeline->SetTimelineFinishedFunc(FinishDelegate);
	}
	
	if (WarningCurve)
	{
		FOnTimelineFloat UpdateDelegate;
		UpdateDelegate.BindUFunction(this, FName("HandleWarningFlashUpdate"));
		WarningTimeline->AddInterpFloat(WarningCurve, UpdateDelegate);
		
		FOnTimelineEvent FinishDelegate;
		FinishDelegate.BindUFunction(this, FName("HandleWarningFinished"));
		WarningTimeline->SetTimelineFinishedFunc(FinishDelegate);
	}
	
	DynamicMat = SM->CreateDynamicMaterialInstance(0);
	
}

void APTWFloorTile::StartCollapse()
{
	if (HasAuthority())
	{
		MulticastRPC_StartCollapse();
	}
	
	//GetWorldTimerManager().SetTimer(WarningTimerHandle, this, &APTWFloorTile::StartFalling, 3.0f, false);
}

void APTWFloorTile::HandleCollapseUpdate(float Value)
{
	float NewZ = FMath::Lerp(StartLocation.Z, StartLocation.Z - 2000.0f, Value);
    
	FVector NewLocation = StartLocation;
	NewLocation.Z = NewZ;
    
	SetActorLocation(NewLocation);
}

void APTWFloorTile::HandleCollapseFinished()
{
	if (HasAuthority())
	{
		Destroy();
	}
}

void APTWFloorTile::HandleWarningFlashUpdate(float Value)
{
	if (DynamicMat)
	{
		DynamicMat->SetScalarParameterValue(FName("FlashIntensity"), Value);
	}
}

void APTWFloorTile::HandleWarningFinished()
{
	StartFalling();
}

void APTWFloorTile::PlayWarningEffect()
{
	if (WarningCurve)
	{
		WarningTimeline->PlayFromStart();
	}
}

void APTWFloorTile::StartFalling()
{
	StartLocation = GetActorLocation();
	FallTimeline->PlayFromStart();
	
	CallWarningLampStop();
}

void APTWFloorTile::CallWarningLampStop()
{
	if (HasAuthority())
	{
		if (APTWRocketGameMode* RocketGameMode = Cast<APTWRocketGameMode>(GetWorld()->GetAuthGameMode()))
		{
			RocketGameMode->StopDropWarning();
		}
	}
}

void APTWFloorTile::MulticastRPC_StartCollapse_Implementation()
{
	PlayWarningEffect();
}


