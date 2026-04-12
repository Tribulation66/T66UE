// Copyright Tribulation 66. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "T66ArthurUltimateSword.generated.h"

class USceneComponent;
class UStaticMeshComponent;

UCLASS(NotBlueprintable)
class T66_API AT66ArthurUltimateSword : public AActor
{
	GENERATED_BODY()

public:
	AT66ArthurUltimateSword();

	void InitSwordFlight(const FVector& InStart, const FVector& InEnd);

protected:
	virtual void BeginPlay() override;
	virtual void Tick(float DeltaSeconds) override;

private:
	void UpdateSwordTransform(float Alpha);

	UPROPERTY(VisibleAnywhere)
	TObjectPtr<USceneComponent> SceneRoot;

	UPROPERTY(VisibleAnywhere)
	TObjectPtr<UStaticMeshComponent> SwordMesh;

	FVector StartLocation = FVector::ZeroVector;
	FVector EndLocation = FVector::ForwardVector * 100.f;
	FRotator SwordRotation = FRotator::ZeroRotator;
	float ElapsedSeconds = 0.f;
	float TravelDurationSeconds = 0.25f;
	bool bInitialized = false;

	static constexpr float BaseSwordScale = 0.60f;
	static constexpr float UltimateSwordScaleMultiplier = 20.f;
	static constexpr float SwordTravelSpeed = 5200.f;
};
