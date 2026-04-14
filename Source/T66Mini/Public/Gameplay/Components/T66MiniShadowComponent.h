// Copyright Tribulation 66. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "T66MiniShadowComponent.generated.h"

class UStaticMeshComponent;
class USceneComponent;

UCLASS(ClassGroup=(Mini), meta=(BlueprintSpawnableComponent))
class T66MINI_API UT66MiniShadowComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UT66MiniShadowComponent();

	virtual void BeginPlay() override;

	void InitializeShadow(USceneComponent* InAttachParent, const FVector& InRelativeLocation, const FVector& InRelativeScale, const FLinearColor& InTint);

private:
	void EnsureShadowMesh();

	UPROPERTY()
	TObjectPtr<USceneComponent> AttachParent;

	UPROPERTY(Transient)
	TObjectPtr<UStaticMeshComponent> ShadowMesh;

	FVector RelativeLocation = FVector(0.f, 0.f, 2.f);
	FVector RelativeScale = FVector(0.24f, 0.24f, 1.f);
	FLinearColor Tint = FLinearColor(0.f, 0.f, 0.f, 0.24f);
};
