// Copyright Tribulation 66. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "T66TowerDescentHole.generated.h"

class UBoxComponent;
class UPrimitiveComponent;

UCLASS(Blueprintable)
class T66_API AT66TowerDescentHole : public AActor
{
	GENERATED_BODY()

public:
	AT66TowerDescentHole();

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Collision")
	TObjectPtr<UBoxComponent> TriggerBox;

	void InitializeHole(int32 InFromFloorNumber, int32 InToFloorNumber, const FVector& InTriggerExtent);

protected:
	virtual void BeginPlay() override;

	UFUNCTION()
	void OnTriggerBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
		UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

	UFUNCTION()
	void OnTriggerEndOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
		UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);

private:
	int32 FromFloorNumber = INDEX_NONE;
	int32 ToFloorNumber = INDEX_NONE;
	TSet<TWeakObjectPtr<AActor>> ActiveActors;
};
