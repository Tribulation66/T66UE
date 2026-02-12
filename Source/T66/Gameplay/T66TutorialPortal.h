// Copyright Tribulation 66. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "T66TutorialPortal.generated.h"

class USphereComponent;
class UStaticMeshComponent;
class AT66PlayerController;

/**
 * Tutorial Portal: teleports the player to the main Stage 1 start area (no level load).
 * Spawned by AT66TutorialManager at the end of the tutorial wave.
 */
UCLASS(Blueprintable)
class T66_API AT66TutorialPortal : public AActor
{
	GENERATED_BODY()

public:
	AT66TutorialPortal();

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Collision")
	TObjectPtr<USphereComponent> TriggerSphere;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Mesh")
	TObjectPtr<UStaticMeshComponent> PortalMesh;

	/** Teleport target (Stage 1 start area). */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Tutorial")
	FVector TargetLocation = FVector(-27273.f, 0.f, 200.f);

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Tutorial")
	bool bDestroyOnUse = true;

	/** Interact (F) when nearby. Returns true if used. */
	UFUNCTION(BlueprintCallable, Category = "Tutorial")
	bool Interact(AT66PlayerController* PC);
};

