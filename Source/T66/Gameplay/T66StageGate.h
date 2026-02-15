// Copyright Tribulation 66. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "T66StageGate.generated.h"

class UBoxComponent;
class UStaticMeshComponent;
class UStaticMesh;

/**
 * STAGE GATE: Big 3D rectangle. Interact (F) to advance to next stage (reload level,
 * keep bounty, gold, progress). Place on the far side of the map.
 */
UCLASS(Blueprintable)
class T66_API AT66StageGate : public AActor
{
	GENERATED_BODY()

public:
	AT66StageGate();

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Collision")
	TObjectPtr<UBoxComponent> TriggerBox;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Mesh")
	TObjectPtr<UStaticMeshComponent> GateMesh;

	/** Optional imported portal mesh (if null/unloaded, uses cube). */
	UPROPERTY(EditDefaultsOnly, Category = "Mesh")
	TSoftObjectPtr<UStaticMesh> GateMeshOverride;

	/** Called when player presses Interact (F) while overlapping. Returns true if advance was triggered. */
	UFUNCTION(BlueprintCallable, Category = "Stage")
	bool AdvanceToNextStage();

protected:
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
};
