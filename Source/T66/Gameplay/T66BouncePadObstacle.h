// Copyright Tribulation 66. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "T66BouncePadObstacle.generated.h"

class UBoxComponent;
class UMaterialInterface;
class UStaticMesh;
class UStaticMeshComponent;

/**
 * Fall Guys flipper pad (FALLGUYS_MAP_ANALYSIS.md C3): a striped disc that
 * launches the hero straight UP onto the deck/rim above — a deliberate
 * trampoline access route and pure slapstick. Launch only, never ragdoll or
 * disable (same capsule-launch contract as the obstacle traps). The disc mesh
 * carries its own exact convex collision (walk-on, below step height).
 */
UCLASS()
class T66_API AT66BouncePadObstacle : public AActor
{
	GENERATED_BODY()

public:
	AT66BouncePadObstacle();

	void InitPad(UStaticMesh* DiscMesh, UMaterialInterface* DiscMaterial, float DiscRadius);

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Visuals")
	TObjectPtr<UStaticMeshComponent> PadMesh;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Collision")
	TObjectPtr<UBoxComponent> LaunchZone;

	/** Disc thickness (top stays below the 45uu step height: boarding is a walk-on). */
	static constexpr float PadThickness = 40.0f;

protected:
	UFUNCTION()
	void OnLaunchZoneBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
		UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

private:
	float LastLaunchTimeSeconds = -100.0f;
};
