// Copyright Tribulation 66. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "T66MiniHazardTrap.generated.h"

class UBillboardComponent;
class USceneComponent;
class UStaticMeshComponent;
class UMaterialInstanceDynamic;

UCLASS()
class T66MINI_API AT66MiniHazardTrap : public AActor
{
	GENERATED_BODY()

public:
	AT66MiniHazardTrap();

	virtual void Tick(float DeltaSeconds) override;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	void InitializeTrap(const FVector& SpawnLocation, float InRadius, float InDamagePerPulse, float InPulseInterval, float InWarmupSeconds, float InActiveSeconds, int32 InTrapVariant = 0);

	float GetRadius() const { return Radius; }
	float GetDamagePerPulse() const { return DamagePerPulse; }
	float GetPulseInterval() const { return PulseInterval; }
	float GetWarmupRemaining() const { return WarmupRemaining; }
	float GetActiveRemaining() const { return ActiveRemaining; }
	float GetLifetimeRemaining() const { return LifetimeRemaining; }
	int32 GetTrapVariant() const { return TrapVariant; }

protected:
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

private:
	void ApplyPulse();
	class AT66MiniPlayerPawn* FindClosestPlayerPawn(bool bRequireAlive = true) const;
	void UpdatePresentation() const;
	FLinearColor ResolveTrapTint() const;

	UFUNCTION()
	void OnRep_TrapPresentationState();

	UPROPERTY(VisibleAnywhere)
	TObjectPtr<USceneComponent> SceneRoot;

	UPROPERTY(VisibleAnywhere)
	TObjectPtr<UStaticMeshComponent> IndicatorMesh;

	UPROPERTY(VisibleAnywhere)
	TObjectPtr<UBillboardComponent> SpriteComponent;

	UPROPERTY()
	TObjectPtr<UMaterialInstanceDynamic> IndicatorMaterial;

	UPROPERTY(ReplicatedUsing = OnRep_TrapPresentationState)
	float Radius = 260.f;

	UPROPERTY(Replicated)
	float DamagePerPulse = 8.f;

	UPROPERTY(Replicated)
	float PulseInterval = 0.55f;

	float PulseAccumulator = 0.f;
	float WarmupRemaining = 1.0f;
	float ActiveRemaining = 4.5f;
	float LifetimeRemaining = 5.5f;

	UPROPERTY(ReplicatedUsing = OnRep_TrapPresentationState)
	int32 TrapVariant = 0;
};
