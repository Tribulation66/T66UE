// Copyright Tribulation 66. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "T66MiniGroundTelegraphActor.generated.h"

class UStaticMeshComponent;
class USceneComponent;

UCLASS()
class T66MINI_API AT66MiniGroundTelegraphActor : public AActor
{
	GENERATED_BODY()

public:
	AT66MiniGroundTelegraphActor();

	virtual void Tick(float DeltaSeconds) override;

	void InitializeTelegraph(const FVector& InWorldLocation, float InRadius, float InLifetimeSeconds, const FLinearColor& InTint);
	void DeactivateTelegraph();
	bool IsAvailableForReuse() const { return !bTelegraphActive; }

protected:
	virtual void BeginPlay() override;

private:
	UPROPERTY(VisibleAnywhere)
	TObjectPtr<USceneComponent> SceneRoot;

	UPROPERTY(VisibleAnywhere)
	TObjectPtr<UStaticMeshComponent> TelegraphMesh;

	FLinearColor Tint = FLinearColor(0.96f, 0.22f, 0.18f, 0.42f);
	float LifetimeSeconds = 1.0f;
	float LifetimeRemaining = 1.0f;
	float PulseAccumulator = 0.f;
	bool bTelegraphActive = false;
};
