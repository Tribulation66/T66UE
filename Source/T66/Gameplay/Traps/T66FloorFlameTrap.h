// Copyright Tribulation 66. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Gameplay/Traps/T66TrapBase.h"
#include "T66FloorFlameTrap.generated.h"

class UNiagaraComponent;
class UNiagaraSystem;
class USphereComponent;
class UStaticMeshComponent;

UCLASS(Blueprintable)
class T66_API AT66FloorFlameTrap : public AT66TrapBase
{
	GENERATED_BODY()

public:
	AT66FloorFlameTrap();

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Trap")
	float Radius = 260.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Trap")
	float WarningDurationSeconds = 0.8f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Trap")
	float ActiveDurationSeconds = 1.15f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Trap")
	float CooldownDurationSeconds = 3.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Trap")
	float DamageIntervalSeconds = 0.35f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Trap")
	float InitialCycleDelaySeconds = 1.2f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Trap")
	int32 DamageHP = 10;

protected:
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
	virtual void Tick(float DeltaSeconds) override;
	virtual void OnConstruction(const FTransform& Transform) override;

private:
	void BeginWarningCycle();
	void ActivateFlames();
	void DeactivateFlames();
	void ApplyDamagePulse();
	void ScheduleNextCycle(float DelaySeconds);
	void SpawnActivationBurst() const;
	void UpdateMarkerVisuals();

	UPROPERTY(VisibleAnywhere, Category = "Components")
	TObjectPtr<USphereComponent> DamageZone;

	UPROPERTY(VisibleAnywhere, Category = "Components")
	TObjectPtr<UStaticMeshComponent> MarkerMesh;

	UPROPERTY(Transient)
	TObjectPtr<UNiagaraSystem> CachedFireSystem = nullptr;

	UPROPERTY(Transient)
	TObjectPtr<UNiagaraComponent> ActiveFireComponent = nullptr;

	FTimerHandle WarningTimerHandle;
	FTimerHandle ActiveTimerHandle;
	FTimerHandle DamageTimerHandle;

	bool bWarningActive = false;
	bool bFlamesActive = false;
	float WarningElapsed = 0.f;
	float WarningVFXAccum = 0.f;
};
