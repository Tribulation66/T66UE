// Copyright Tribulation 66. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Gameplay/Traps/T66TrapBase.h"
#include "T66FloorSpikePatchTrap.generated.h"

class UInstancedStaticMeshComponent;
class UMaterialInstanceDynamic;
class USphereComponent;
class UStaticMeshComponent;

UCLASS(Blueprintable)
class T66_API AT66FloorSpikePatchTrap : public AT66TrapBase
{
	GENERATED_BODY()

public:
	AT66FloorSpikePatchTrap();

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Trap")
	float Radius = 285.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Trap")
	float WarningDurationSeconds = 0.9f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Trap")
	float RiseDurationSeconds = 0.25f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Trap")
	float RaisedDurationSeconds = 1.05f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Trap")
	float RetractDurationSeconds = 0.25f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Trap")
	float CooldownDurationSeconds = 2.85f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Trap")
	float DamageIntervalSeconds = 0.35f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Trap")
	float InitialCycleDelaySeconds = 1.15f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Trap")
	float SpikeHeight = 165.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Trap")
	int32 SpikeCount = 10;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Trap")
	int32 DamageHP = 11;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Trap|Visual")
	FLinearColor IdleMarkerColor = FLinearColor(0.10f, 0.08f, 0.08f, 1.f);

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Trap|Visual")
	FLinearColor WarningColor = FLinearColor(1.f, 0.20f, 0.20f, 0.92f);

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Trap|Visual")
	FLinearColor SpikeColor = FLinearColor(0.85f, 0.85f, 0.85f, 1.f);

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Trap|Visual")
	FLinearColor BurstColor = FLinearColor(1.f, 0.42f, 0.22f, 0.95f);

protected:
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
	virtual void Tick(float DeltaSeconds) override;
	virtual void OnConstruction(const FTransform& Transform) override;
	virtual void HandleTrapEnabledChanged() override;
	virtual bool CanAcceptExternalTrigger() const override;
	virtual bool HandleTrapTriggered(AActor* TriggeringActor) override;

private:
	void BeginWarningCycle();
	void BeginRiseCycle();
	void ActivatePatch();
	void BeginRetractCycle();
	void FinishRetractCycle();
	void ApplyDamagePulse();
	void ScheduleNextCycle(float DelaySeconds);
	void RebuildSpikes();
	void UpdateMarkerVisuals();
	void UpdateSpikeTransforms(float RaisedAlpha);
	void SpawnRiseBurst() const;
	bool ShouldTickForAnimation() const;

	UPROPERTY(VisibleAnywhere, Category = "Components")
	TObjectPtr<USphereComponent> DamageZone;

	UPROPERTY(VisibleAnywhere, Category = "Components")
	TObjectPtr<UStaticMeshComponent> MarkerMesh;

	UPROPERTY(VisibleAnywhere, Category = "Components")
	TObjectPtr<UInstancedStaticMeshComponent> SpikeInstances;

	UPROPERTY(Transient)
	TArray<TObjectPtr<UMaterialInstanceDynamic>> SpikeDynamicMaterials;

	FTimerHandle WarningTimerHandle;
	FTimerHandle RaiseTimerHandle;
	FTimerHandle ActiveTimerHandle;
	FTimerHandle DamageTimerHandle;

	bool bWarningActive = false;
	bool bRising = false;
	bool bRaised = false;
	bool bRetracting = false;
	float PhaseElapsed = 0.f;
	float WarningVFXAccum = 0.f;
};
