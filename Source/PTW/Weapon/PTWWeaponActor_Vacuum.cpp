// Fill out your copyright notice in the Description page of Project Settings.


#include "Weapon/PTWWeaponActor_Vacuum.h"

APTWWeaponActor_Vacuum::APTWWeaponActor_Vacuum()
{
	SuctionZone = CreateDefaultSubobject<UBoxComponent>(TEXT("SuctionZone"));
	SuctionZone->SetupAttachment(RootComponent);

	SuctionZone->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	SuctionZone->SetCollisionResponseToAllChannels(ECR_Ignore);
}

void APTWWeaponActor_Vacuum::BeginPlay()
{
	Super::BeginPlay();
	SuctionZone->OnComponentBeginOverlap.AddDynamic(this, &APTWWeaponActor_Vacuum::OnSuctionZoneOverlap);
}

void APTWWeaponActor_Vacuum::HandleReloadEvent(EReloadEventAction ActionType)
{
	// 부모의 재장전 로직을 오버라이드하여 아무 기능도 수행하지 않도록 비워둠
}

void APTWWeaponActor_Vacuum::OnSuctionZoneOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if (!OtherActor || OtherActor == GetOwner()) return;

	// [기능 검증용 로그] 물리 시뮬레이션 액터 도달 체크
	if (OtherComp && OtherComp->IsSimulatingPhysics())
	{
		//UE_LOG(LogTemp, Warning, TEXT("%s 가 청소기 흡입 영역(골인지점)에 도달했습니다!"), *OtherActor->GetName());
	}
}
