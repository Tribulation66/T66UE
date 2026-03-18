// Copyright Tribulation 66. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Gameplay/T66WorldInteractableBase.h"
#include "T66PilotableTractor.generated.h"

class AT66HeroBase;
class UBoxComponent;
class USceneComponent;
class UTextRenderComponent;

UCLASS(Blueprintable)
class T66_API AT66PilotableTractor : public AT66WorldInteractableBase
{
	GENERATED_BODY()

public:
	AT66PilotableTractor();

	virtual bool Interact(APlayerController* PC) override;

	bool IsMountedByHero(const AT66HeroBase* Hero) const;
	void SetDriveForwardInput(float Value);
	void SetDriveRightInput(float Value);

protected:
	virtual void BeginPlay() override;
	virtual void Tick(float DeltaSeconds) override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
	virtual void ApplyRarityVisuals() override;

private:
	bool MountHero(AT66HeroBase* Hero);
	void DismountHero(bool bDestroyAfterExit);
	void ExpireTractor();
	void TickMountedDriving(float DeltaSeconds);
	void HandleMountedEnemyMow(float DeltaSeconds);
	void UpdatePrompt();
	void UpdatePromptFacing() const;
	void UpdatePromptPlacement();
	FString BuildPromptString() const;

	UPROPERTY(VisibleAnywhere, Category = "Tractor")
	TObjectPtr<USceneComponent> TractorRoot;

	UPROPERTY(VisibleAnywhere, Category = "Tractor")
	TObjectPtr<UBoxComponent> BodyCollision;

	UPROPERTY(VisibleAnywhere, Category = "Tractor")
	TObjectPtr<UTextRenderComponent> PromptText;

	UPROPERTY(EditDefaultsOnly, Category = "Tractor|Pilot", meta = (ClampMin = "1.0"))
	float TotalPilotSeconds = 30.f;

	UPROPERTY(VisibleAnywhere, Category = "Tractor|Pilot")
	float RemainingPilotSeconds = 30.f;

	UPROPERTY(EditDefaultsOnly, Category = "Tractor|Prompt", meta = (ClampMin = "50.0"))
	float PromptVisibleDistance = 650.f;

	UPROPERTY(EditDefaultsOnly, Category = "Tractor|Combat", meta = (ClampMin = "50.0"))
	float MowKillRadius = 220.f;

	UPROPERTY(EditDefaultsOnly, Category = "Tractor|Combat", meta = (ClampMin = "0.0"))
	float MowMinSpeed = 150.f;

	UPROPERTY(EditDefaultsOnly, Category = "Tractor|Combat", meta = (ClampMin = "0.01"))
	float MowCheckInterval = 0.05f;

	UPROPERTY(EditDefaultsOnly, Category = "Tractor|Visual")
	FVector MountedHeroVisualOffset = FVector::ZeroVector;

	UPROPERTY(EditDefaultsOnly, Category = "Tractor|Visual")
	FRotator MountedHeroVisualRotation = FRotator::ZeroRotator;

	UPROPERTY(EditDefaultsOnly, Category = "Tractor|Visual")
	FVector MountedHeroSeatOffset = FVector(-10.f, 0.f, 245.f);

	UPROPERTY(EditDefaultsOnly, Category = "Tractor|Movement", meta = (ClampMin = "100.0"))
	float DriveSpeed = 1800.f;

	UPROPERTY(EditDefaultsOnly, Category = "Tractor|Movement", meta = (ClampMin = "30.0"))
	float TurnSpeedDegreesPerSecond = 540.f;

	UPROPERTY(Transient)
	TWeakObjectPtr<AT66HeroBase> MountedHero;

	float MowCheckAccumSeconds = 0.f;
	float ForwardDriveInput = 0.f;
	float RightDriveInput = 0.f;
	FVector CurrentVelocity = FVector::ZeroVector;
	FVector LastFrameLocation = FVector::ZeroVector;
};
