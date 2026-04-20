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

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Trap|Visual")
	FVector TrapVisualScale = FVector(0.32f);

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Trap|Visual")
	FLinearColor TrapTint = FLinearColor(0.95f, 0.78f, 0.20f, 1.f);

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Trap|Visual")
	FLinearColor WindupColor = FLinearColor(1.f, 0.80f, 0.25f, 1.f);

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Trap|Visual")
	FLinearColor ProjectileTint = FLinearColor(1.f, 0.88f, 0.30f, 1.f);

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Trap|Visual")
	FLinearColor ProjectileTrailColor = FLinearColor(1.f, 0.78f, 0.25f, 0.95f);

protected:
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
	virtual void OnConstruction(const FTransform& Transform) override;
	virtual void HandleTrapEnabledChanged() override;

private:
	void UpdateTrapVisuals();
	void HandleFireCycleStart();
	void FireProjectile();
	void ScheduleNextFireCycle(float DelaySeconds);
	AT66HeroBase* ResolveTargetHero(FVector& OutTargetLocation) const;
	FVector GetMuzzleLocation() const;
	void SpawnWindupBurst(const FVector& Direction) const;

	UPROPERTY(VisibleAnywhere, Category = "Components")
	TObjectPtr<UStaticMeshComponent> HousingMesh;

	UPROPERTY(VisibleAnywhere, Category = "Components")
	TObjectPtr<UStaticMeshComponent> TrapMesh;

	UPROPERTY(VisibleAnywhere, Category = "Components")
	TObjectPtr<USceneComponent> ProjectileSpawnPoint;

	FTimerHandle FireCycleTimerHandle;
	TWeakObjectPtr<AT66HeroBase> PendingTargetHero;
	FVector PendingAimDirection = FVector::ForwardVector;
};
