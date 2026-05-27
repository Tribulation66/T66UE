// Copyright Tribulation 66. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "T66TrapArrowProjectile.generated.h"

class UBoxComponent;
class UStaticMeshComponent;
class UProjectileMovementComponent;
class UPrimitiveComponent;
struct FHitResult;

UCLASS(Blueprintable)
class T66_API AT66TrapArrowProjectile : public AActor
{
	GENERATED_BODY()

public:
	AT66TrapArrowProjectile();

	static int32 GetActiveTrapProjectileCount();

	void InitializeProjectile(
		const FVector& Direction,
		int32 InDamageHP,
		float InProjectileSpeed,
		const FLinearColor& InProjectileTint,
		const FLinearColor& InTrailColor);

protected:
	virtual void BeginPlay() override;
	virtual void EndPlay(EEndPlayReason::Type EndPlayReason) override;
	virtual void Tick(float DeltaSeconds) override;

private:
	void UpdateVisuals();

	UFUNCTION()
	void OnDamageBoxOverlap(
		UPrimitiveComponent* OverlappedComponent,
		AActor* OtherActor,
		UPrimitiveComponent* OtherComp,
		int32 OtherBodyIndex,
		bool bFromSweep,
		const FHitResult& SweepResult);

	UPROPERTY(VisibleAnywhere, Category = "Components")
	TObjectPtr<UBoxComponent> DamageBox;

	UPROPERTY(VisibleAnywhere, Category = "Components")
	TObjectPtr<UStaticMeshComponent> VisualMesh;

	UPROPERTY(VisibleAnywhere, Category = "Components")
	TObjectPtr<UStaticMeshComponent> AccentMesh;

	UPROPERTY(VisibleAnywhere, Category = "Components")
	TObjectPtr<UProjectileMovementComponent> ProjectileMovement;

	UPROPERTY(EditAnywhere, Category = "Trap")
	int32 DamageHP = 10;

	UPROPERTY(EditAnywhere, Category = "Trap")
	FLinearColor ProjectileTint = FLinearColor(1.f, 0.04f, 0.02f, 1.f);

	UPROPERTY(EditAnywhere, Category = "Trap")
	FLinearColor TrailColor = FLinearColor(1.f, 0.04f, 0.02f, 1.f);

	float ProjectileSpeed = 2400.f;
	bool bCountedAsActiveProjectile = false;
};
