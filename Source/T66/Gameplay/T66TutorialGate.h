// Copyright Tribulation 66. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "T66TutorialGate.generated.h"

class USphereComponent;
class UStaticMeshComponent;
class UStaticMesh;
class AT66PlayerController;

/**
 * Tutorial Gate: marks the tutorial complete and returns to the frontend.
 * Spawned by AT66TutorialManager at the end of the tutorial flow.
 */
UCLASS(Blueprintable)
class T66_API AT66TutorialGate : public AActor
{
	GENERATED_BODY()

public:
	AT66TutorialGate();

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Collision")
	TObjectPtr<USphereComponent> TriggerSphere;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Mesh")
	TObjectPtr<UStaticMeshComponent> GateMesh;

	/** Optional imported tutorial gate mesh. */
	UPROPERTY(EditDefaultsOnly, Category = "Mesh")
	TSoftObjectPtr<UStaticMesh> GateMeshOverride;

	/** Teleport target (Stage 1 start area). */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Tutorial")
	FVector TargetLocation = FVector(-27273.f, 0.f, 200.f);

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Tutorial")
	bool bDestroyOnUse = true;

	/** Interact (F) when nearby. Returns true if used. */
	UFUNCTION(BlueprintCallable, Category = "Tutorial")
	bool Interact(AT66PlayerController* PC);

protected:
	virtual void BeginPlay() override;
};
