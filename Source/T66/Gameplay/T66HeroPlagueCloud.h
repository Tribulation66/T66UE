// Copyright Tribulation 66. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "T66HeroPlagueCloud.generated.h"

class USphereComponent;
class UStaticMeshComponent;

/**
 * Persistent poison cloud (Rogue ultimate). Spawned at hero location;
 * enemies inside take DOT every TickIntervalSeconds for DurationSeconds.
 */
UCLASS(Blueprintable)
class T66_API AT66HeroPlagueCloud : public AActor
{
	GENERATED_BODY()

public:
	AT66HeroPlagueCloud();

	/** Radius of the cloud (world units). */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "PlagueCloud")
	float Radius = 400.f;

	/** Total duration the cloud persists (seconds). */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "PlagueCloud")
	float DurationSeconds = 5.5f;

	/** Seconds between damage ticks to enemies inside. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "PlagueCloud")
	float TickIntervalSeconds = 0.5f;

	/** Total damage to deal per enemy over the full duration (split across ticks). Set before StartDamage() or via InitFromUltimate. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PlagueCloud")
	int32 TotalDamagePerTarget = 200;

	/** Call after spawn to apply ultimate damage and start ticking. */
	void InitFromUltimate(int32 UltimateDamage);

protected:
	virtual void BeginPlay() override;
	virtual void Tick(float DeltaSeconds) override;

private:
	UPROPERTY(VisibleAnywhere, Category = "Components")
	TObjectPtr<USphereComponent> DamageZone;

	UPROPERTY(VisibleAnywhere, Category = "Components")
	TObjectPtr<UStaticMeshComponent> CloudMesh;

	void ApplyTickDamage();
	void DestroySelf();

	FTimerHandle TickTimerHandle;
	FTimerHandle DestroyTimerHandle;
	float DamagePerTick = 0.f;
	int32 NumTicks = 0;
};
