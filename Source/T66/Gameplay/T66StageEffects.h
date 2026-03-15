// Copyright Tribulation 66. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "T66StageEffects.generated.h"

class UBoxComponent;
class USphereComponent;
class UStaticMeshComponent;
class UNiagaraSystem;

/**
 * Shroom stage effect (Easy difficulty).
 * Jump on top: launches the hero dramatically upward.
 * Touch from the side: aggressive knockback in the direction you came from.
 * Both interactions spawn pixel particle bursts.
 */
UCLASS(Blueprintable)
class T66_API AT66Shroom : public AActor
{
	GENERATED_BODY()

public:
	AT66Shroom();

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Collision")
	TObjectPtr<UBoxComponent> SideTrigger;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Collision")
	TObjectPtr<USphereComponent> TopTrigger;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Visuals")
	TObjectPtr<UStaticMeshComponent> ShroomMesh;

	UPROPERTY(EditDefaultsOnly, Category = "Shroom")
	TSoftObjectPtr<UStaticMesh> ShroomMeshOverride;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Shroom|Tuning")
	float LaunchZVelocity = 2400.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Shroom|Tuning")
	float LaunchForwardVelocity = 1800.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Shroom|Tuning")
	float KnockbackForce = 2800.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Shroom|Tuning")
	float CooldownSeconds = 0.3f;

protected:
	virtual void BeginPlay() override;

	UFUNCTION()
	void OnSideTriggerOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
		UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

	UFUNCTION()
	void OnTopTriggerOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
		UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

private:
	void SpawnPixelBurst(const FVector& Location, const FLinearColor& Color) const;

	UPROPERTY()
	TObjectPtr<UNiagaraSystem> CachedPixelVFX = nullptr;

	float LastSideTriggerTime = -9999.f;
	float LastTopTriggerTime = -9999.f;
	TWeakObjectPtr<AActor> LastSideActor;
	TWeakObjectPtr<AActor> LastTopActor;
};
