// Fill out your copyright notice in the Description page of Project Settings.


#include "Gameplay/Customize/PTWPreviewActor.h"
#include "Components/SkeletalMeshComponent.h"

APTWPreviewActor::APTWPreviewActor()
{
	PrimaryActorTick.bCanEverTick = false;

	BaseMesh = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("BaseMesh"));
	RootComponent = BaseMesh;

	BaseMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	SK_UpperAddon = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("SK_UpperAddon"));
	SK_UpperAddon->SetupAttachment(BaseMesh);

	SK_BackAddon = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("SK_BackAddon"));
	SK_BackAddon->SetupAttachment(BaseMesh);

	SK_Eyewear = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("SK_Eyewear"));
	SK_Eyewear->SetupAttachment(BaseMesh);

	SK_Gloves = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("SK_Gloves"));
	SK_Gloves->SetupAttachment(BaseMesh);

	SK_Hair = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("SK_Hair"));
	SK_Hair->SetupAttachment(BaseMesh);

	SK_Hat = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("SK_Hat"));
	SK_Hat->SetupAttachment(BaseMesh);

	SK_Head = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("SK_Head"));
	SK_Head->SetupAttachment(BaseMesh);

	SK_Lower = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("SK_Lower"));
	SK_Lower->SetupAttachment(BaseMesh);

	SK_Shoes = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("SK_Shoes"));
	SK_Shoes->SetupAttachment(BaseMesh);
}

void APTWPreviewActor::BeginPlay()
{
	Super::BeginPlay();

	SK_UpperAddon->SetLeaderPoseComponent(BaseMesh);
	SK_BackAddon->SetLeaderPoseComponent(BaseMesh);
	SK_Eyewear->SetLeaderPoseComponent(BaseMesh);
	SK_Gloves->SetLeaderPoseComponent(BaseMesh);
	SK_Hair->SetLeaderPoseComponent(BaseMesh);
	SK_Hat->SetLeaderPoseComponent(BaseMesh);
	SK_Head->SetLeaderPoseComponent(BaseMesh);
	SK_Lower->SetLeaderPoseComponent(BaseMesh);
	SK_Shoes->SetLeaderPoseComponent(BaseMesh);
}
