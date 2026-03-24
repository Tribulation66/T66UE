// Copyright Tribulation 66. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "T66HeroOneAttackVFX.generated.h"

class UMaterialInstanceDynamic;
class UMaterialInterface;
class USceneComponent;
class UStaticMeshComponent;

/**
 * Arthur's hero-specific pierce attack VFX.
 * Uses a material-driven pixel streak + impact seal rather than the generic
 * Niagara line of square sprites.
 */
UCLASS(NotBlueprintable)
class T66_API AT66HeroOneAttackVFX : public AActor
{
	GENERATED_BODY()

public:
	AT66HeroOneAttackVFX();

	void InitEffect(const FVector& InStart, const FVector& InEnd, const FVector& InImpactLocation, const FLinearColor& InTint);

protected:
	virtual void BeginPlay() override;
	virtual void Tick(float DeltaSeconds) override;

private:
	void ApplyEffectTransforms();
	void UpdateMaterialParams(float Age01) const;

	UPROPERTY(VisibleAnywhere)
	TObjectPtr<USceneComponent> SceneRoot;

	UPROPERTY(VisibleAnywhere)
	TObjectPtr<UStaticMeshComponent> StreakMesh;

	UPROPERTY(VisibleAnywhere)
	TObjectPtr<UStaticMeshComponent> ImpactMesh;

	UPROPERTY()
	TObjectPtr<UMaterialInterface> StreakBaseMaterial = nullptr;

	UPROPERTY()
	TObjectPtr<UMaterialInterface> ImpactBaseMaterial = nullptr;

	UPROPERTY(Transient)
	TObjectPtr<UMaterialInstanceDynamic> StreakMID = nullptr;

	UPROPERTY(Transient)
	TObjectPtr<UMaterialInstanceDynamic> ImpactMID = nullptr;

	FVector StartLocation = FVector::ZeroVector;
	FVector EndLocation = FVector::ForwardVector * 100.f;
	FVector ImpactLocation = FVector::ForwardVector * 100.f;
	FLinearColor TintColor = FLinearColor(1.f, 0.98f, 0.88f, 1.f);

	float ElapsedSeconds = 0.f;
	float LifetimeSeconds = 0.16f;
	float TraceLength = 100.f;
	float ImpactDistance = 100.f;
	float BaseStreakWidthScale = 0.22f;
	float BaseImpactScale = 0.55f;
	bool bInitialized = false;
};
