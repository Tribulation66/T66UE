// Copyright Tribulation 66. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Gameplay/Traps/T66TrapBase.h"
#include "T66TrapPressurePlate.generated.h"

class AT66TrapBase;
class UBoxComponent;
class USceneComponent;
class UStaticMeshComponent;
class UPrimitiveComponent;
struct FHitResult;

UCLASS(Blueprintable)
class T66_API AT66TrapPressurePlate : public AActor
{
	GENERATED_BODY()

public:
	AT66TrapPressurePlate();

	void AddLinkedTrap(AT66TrapBase* Trap);
	void SetTowerFloorNumber(int32 InTowerFloorNumber) { TowerFloorNumber = InTowerFloorNumber; }
	void SetTriggerTargetMode(ET66TrapTriggerTarget InTriggerTargetMode) { TriggerTargetMode = InTriggerTargetMode; }
	void SetIdleColor(const FLinearColor& InIdleColor) { IdleColor = InIdleColor; }
	void SetPressedColor(const FLinearColor& InPressedColor) { PressedColor = InPressedColor; }

protected:
	virtual void BeginPlay() override;
	virtual void OnConstruction(const FTransform& Transform) override;

private:
	UFUNCTION()
	void HandleTriggerZoneBeginOverlap(
		UPrimitiveComponent* OverlappedComponent,
		AActor* OtherActor,
		UPrimitiveComponent* OtherComp,
		int32 OtherBodyIndex,
		bool bFromSweep,
		const FHitResult& SweepResult);

	UFUNCTION()
	void HandleTriggerZoneEndOverlap(
		UPrimitiveComponent* OverlappedComponent,
		AActor* OtherActor,
		UPrimitiveComponent* OtherComp,
		int32 OtherBodyIndex);

	void UpdateVisuals();
	bool CanTriggerFromActor(const AActor* Actor) const;
	bool IsActorOnCompatibleTowerFloor(const AActor* Actor) const;
	void TriggerLinkedTraps(AActor* TriggeringActor);

	UPROPERTY(VisibleAnywhere, Category = "Components")
	TObjectPtr<USceneComponent> SceneRoot;

	UPROPERTY(VisibleAnywhere, Category = "Components")
	TObjectPtr<UBoxComponent> TriggerZone;

	UPROPERTY(VisibleAnywhere, Category = "Components")
	TObjectPtr<UStaticMeshComponent> PlateMesh;

	UPROPERTY(EditAnywhere, Category = "Trap")
	FVector TriggerExtents = FVector(95.f, 95.f, 65.f);

	UPROPERTY(EditAnywhere, Category = "Trap")
	float RearmDelaySeconds = 0.15f;

	UPROPERTY(EditAnywhere, Category = "Trap")
	ET66TrapTriggerTarget TriggerTargetMode = ET66TrapTriggerTarget::HeroesAndEnemies;

	UPROPERTY(EditAnywhere, Category = "Trap")
	FLinearColor IdleColor = FLinearColor(0.16f, 0.06f, 0.05f, 1.0f);

	UPROPERTY(EditAnywhere, Category = "Trap")
	FLinearColor PressedColor = FLinearColor(0.95f, 0.22f, 0.12f, 1.0f);

	UPROPERTY()
	TArray<TWeakObjectPtr<AT66TrapBase>> LinkedTraps;

	FTimerHandle RearmTimerHandle;
	int32 TowerFloorNumber = INDEX_NONE;
	bool bArmed = true;
	int32 OverlappingTriggerActorCount = 0;
};
