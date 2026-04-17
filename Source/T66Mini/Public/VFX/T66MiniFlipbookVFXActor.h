// Copyright Tribulation 66. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "T66MiniFlipbookVFXActor.generated.h"

class UStaticMeshComponent;
class USceneComponent;
class UTexture;

UCLASS()
class T66MINI_API AT66MiniFlipbookVFXActor : public AActor
{
	GENERATED_BODY()

public:
	AT66MiniFlipbookVFXActor();

	virtual void Tick(float DeltaSeconds) override;

	void InitializeVfx(
		const FVector& InWorldLocation,
		const FVector& InRelativeScale,
		const float InLifetimeSeconds,
		const FLinearColor& InTint,
		UTexture* InTexture = nullptr,
		const float InGrowthFactor = 0.65f);
	void DeactivateVfx();
	bool IsAvailableForReuse() const { return !bVfxActive; }

protected:
	virtual void BeginPlay() override;

private:
	UPROPERTY(VisibleAnywhere)
	TObjectPtr<USceneComponent> SceneRoot;

	UPROPERTY(VisibleAnywhere)
	TObjectPtr<UStaticMeshComponent> VfxMesh;

	FVector BaseScale = FVector(0.20f, 0.20f, 1.f);
	FLinearColor Tint = FLinearColor(1.f, 1.f, 1.f, 0.42f);
	float LifetimeSeconds = 0.18f;
	float LifetimeRemaining = 0.18f;
	float GrowthFactor = 0.65f;
	bool bVfxActive = false;
};
