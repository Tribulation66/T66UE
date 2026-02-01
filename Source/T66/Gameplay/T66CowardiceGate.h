// Copyright Tribulation 66. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "T66CowardiceGate.generated.h"

class UBoxComponent;
class UStaticMeshComponent;
class UStaticMesh;
class APlayerController;

/**
 * Cowardice Gate: prompts the player to skip the current stage boss.
 * If accepted: marks boss as owed, advances stage, and travels (may route to Coliseum).
 */
UCLASS(Blueprintable)
class T66_API AT66CowardiceGate : public AActor
{
	GENERATED_BODY()

public:
	AT66CowardiceGate();

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Collision")
	TObjectPtr<UBoxComponent> TriggerBox;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Mesh")
	TObjectPtr<UStaticMeshComponent> GateMesh;

	/** Optional imported gate mesh (if null/unloaded, uses cube). */
	UPROPERTY(EditDefaultsOnly, Category = "Mesh")
	TSoftObjectPtr<UStaticMesh> GateMeshOverride;

	/** Called by PlayerController when pressing F nearby. Returns true if handled. */
	bool Interact(APlayerController* PC);

	/** Called by the prompt widget when player confirms "Yes". */
	bool ConfirmCowardice();

protected:
	virtual void BeginPlay() override;
};

