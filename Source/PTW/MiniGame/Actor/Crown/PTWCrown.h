#pragma once
#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "PTWCrown.generated.h"


class UStaticMeshComponent;
class URotatingMovementComponent;
class APTWPlayerCharacter;

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FAttachmentChangedSignature);

static const FName CrownSocketName = FName("head_accessories");

UCLASS()
class PTW_API APTWCrown : public AActor
{
	GENERATED_BODY()

public:
	APTWCrown();
	
	UFUNCTION(BlueprintCallable, Category = "Crown|Attachment")
	void SetPreliminaryOwner(APTWPlayerCharacter* NewOwner);
	
	UFUNCTION(BlueprintNativeEvent, Category = "Crown|Attachment")
	void StartMovingToOwner();
	
	UFUNCTION(BlueprintCallable, Category="Crown|Movement")
	void MoveToOwner(float Speed);
	
	UFUNCTION(BlueprintCallable, Category="Crown|Status")
	bool HasReachedOwner();
	
	UFUNCTION(BlueprintCallable, Category = "Crown|Attachment")
	void AttachToOwnerHead();
	
	UFUNCTION(BlueprintCallable, Category = "Crown|Attachment")
	void DetachFromOwner();

protected:
	virtual void GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const override;
	virtual void BeginPlay() override;
	
protected:
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category="Components")
	TObjectPtr<UStaticMeshComponent> StaticMeshComponent;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category="Components")
	TObjectPtr<URotatingMovementComponent> RotatingMovementComponent;
	
	UPROPERTY(Replicated, VisibleAnywhere, BlueprintReadWrite, Category="Variables")
	TObjectPtr<APTWPlayerCharacter> OwnerCharacter;
	
	UPROPERTY(Replicated, VisibleAnywhere, BlueprintReadWrite, Category="Variables")
	TObjectPtr<APTWPlayerCharacter> PreliminaryOwner;
	
public:
	FAttachmentChangedSignature OnAttachToOwnerHead;
	FAttachmentChangedSignature OnDetachFromOwner;
	
};
