// Copyright Tribulation 66. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "T66MiniArena.generated.h"

class UStaticMeshComponent;
class UTexture2D;

UCLASS()
class T66MINI_API AT66MiniArena : public AActor
{
	GENERATED_BODY()

public:
	AT66MiniArena();

	void InitializeArena(const FVector& InOrigin, float InHalfExtent, UTexture2D* InBackgroundTexture = nullptr);

	FVector GetArenaOrigin() const { return ArenaOrigin; }
	float GetArenaHalfExtent() const { return ArenaHalfExtent; }

private:
	UPROPERTY(VisibleAnywhere)
	TObjectPtr<USceneComponent> SceneRoot;

	UPROPERTY(VisibleAnywhere)
	TObjectPtr<UStaticMeshComponent> FloorMesh;

	UPROPERTY(VisibleAnywhere)
	TObjectPtr<UStaticMeshComponent> BackdropMesh;

	UPROPERTY(VisibleAnywhere)
	TArray<TObjectPtr<UStaticMeshComponent>> BorderMeshes;

	FVector ArenaOrigin = FVector::ZeroVector;
	float ArenaHalfExtent = 2200.f;
};
