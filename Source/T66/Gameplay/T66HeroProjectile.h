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
	void SetTargetActor(AActor* InTargetActor);
	void SetScaleMultiplier(float InScaleMultiplier);
	void SetTintColor(const FLinearColor& InColor);

protected:
	virtual void BeginPlay() override;
	virtual void Tick(float DeltaSeconds) override;

	UFUNCTION()
	void OnSphereOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
		UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

	float ScaleMultiplier = 1.f;
	float BaseVisualScale = 0.15f;

private:
	/** Intended target for this shot. If set, we only apply damage to this target. */
	TWeakObjectPtr<AActor> TargetActor;

	UPROPERTY(Transient)
	TObjectPtr<UMaterialInstanceDynamic> VisualMID = nullptr;

	FLinearColor TintColor = FLinearColor(1.f, 0.2f, 0.2f, 1.f);

	bool IsTargetAlive() const;
	void ApplyDamageToTarget(AActor* Target);
};
