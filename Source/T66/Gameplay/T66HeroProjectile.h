// Copyright Tribulation 66. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "T66HeroProjectile.generated.h"

class USphereComponent;
class UStaticMeshComponent;
class UProjectileMovementComponent;

UCLASS(Blueprintable)
class T66_API AT66HeroProjectile : public AActor
{
	GENERATED_BODY()

public:
	AT66HeroProjectile();

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Collision")
	TObjectPtr<USphereComponent> CollisionSphere;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Visuals")
	TObjectPtr<UStaticMeshComponent> VisualMesh;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Movement")
	TObjectPtr<UProjectileMovementComponent> ProjectileMovement;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Combat")
	int32 Damage = 20;

	void SetTargetLocation(const FVector& TargetLoc);
	void SetScaleMultiplier(float InScaleMultiplier);

protected:
	virtual void BeginPlay() override;

	UFUNCTION()
	void OnSphereOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
		UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

	float ScaleMultiplier = 1.f;
	float BaseVisualScale = 0.15f;
};
