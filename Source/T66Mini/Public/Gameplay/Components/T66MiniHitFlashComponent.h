// Copyright Tribulation 66. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "T66MiniHitFlashComponent.generated.h"

class UStaticMeshComponent;
class USceneComponent;

UCLASS(ClassGroup=(Mini), meta=(BlueprintSpawnableComponent))
class T66MINI_API UT66MiniHitFlashComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UT66MiniHitFlashComponent();

	virtual void BeginPlay() override;
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	void InitializeFlash(USceneComponent* InAttachParent, const FVector& InRelativeLocation, const FVector& InRelativeScale, const FLinearColor& InTint);
	void TriggerHitFlash(const FLinearColor& InTint, float InDuration = 0.10f);

private:
	void EnsureFlashMesh();
	void ApplyVisuals(float NormalizedRemaining);

	UPROPERTY()
	TObjectPtr<USceneComponent> AttachParent;

	UPROPERTY(Transient)
	TObjectPtr<UStaticMeshComponent> FlashMesh;

	FVector RelativeLocation = FVector(0.f, 0.f, 6.f);
	FVector RelativeScale = FVector(0.22f, 0.22f, 1.f);
	FLinearColor Tint = FLinearColor(1.f, 0.8f, 0.4f, 0.42f);
	float FlashDuration = 0.f;
	float FlashRemaining = 0.f;
};
