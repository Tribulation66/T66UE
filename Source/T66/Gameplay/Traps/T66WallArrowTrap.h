// Copyright Tribulation 66. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Gameplay/Traps/T66TrapBase.h"
#include "T66WallArrowTrap.generated.h"

class AT66HeroBase;
class UStaticMeshComponent;
class USceneComponent;

UCLASS(Blueprintable)
class T66_API AT66WallArrowTrap : public AT66TrapBase
{
	GENERATED_BODY()

public:
	AT66WallArrowTrap();

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Trap")
	float FireIntervalSeconds = 2.6f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Trap")
	float WindupDurationSeconds = 0.4f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Trap")
	float InitialFireDelaySeconds = 0.8f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Trap")
	float DetectionRange = 5200.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Trap")
	float ProjectileSpeed = 2400.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Trap")
	int32 DamageHP = 12;

protected:
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
	virtual void OnConstruction(const FTransform& Transform) override;

private:
	void HandleFireCycleStart();
	void FireProjectile();
	void ScheduleNextFireCycle(float DelaySeconds);
	AT66HeroBase* ResolveTargetHero(FVector& OutTargetLocation) const;
	FVector GetMuzzleLocation() const;
	void SpawnWindupBurst(const FVector& Direction) const;

	UPROPERTY(VisibleAnywhere, Category = "Components")
	TObjectPtr<UStaticMeshComponent> TrapMesh;

	UPROPERTY(VisibleAnywhere, Category = "Components")
	TObjectPtr<USceneComponent> ProjectileSpawnPoint;

	FTimerHandle FireCycleTimerHandle;
	TWeakObjectPtr<AT66HeroBase> PendingTargetHero;
	FVector PendingAimDirection = FVector::ForwardVector;
};
