// Copyright Tribulation 66. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Gameplay/T66CompanionBase.h"
#include "T66TutorialGuideCompanion.generated.h"

class AActor;

/**
 * Tutorial-only guide companion that follows explicit authored waypoints instead of the hero.
 */
UCLASS(Blueprintable)
class T66_API AT66TutorialGuideCompanion : public AT66CompanionBase
{
	GENERATED_BODY()

public:
	AT66TutorialGuideCompanion();

	void MoveToGuidePoint(const FVector& InTargetLocation, float InAcceptanceRadius = 120.f);
	void StopGuideMovement();
	bool HasReachedGuidePoint() const;
	bool IsGuideMoving() const { return bHasMoveTarget; }

	void FaceLocation(const FVector& InWorldLocation);
	void SetFacingActor(AActor* InActor);

protected:
	virtual void Tick(float DeltaTime) override;

private:
	void UpdateGuideAnimation(bool bIsMoving);
	void UpdateGroundLocation(float DeltaTime, FVector& InOutLocation);
	void UpdateFacing(float DeltaTime, bool bIsMoving);

	UPROPERTY()
	TWeakObjectPtr<AActor> FacingActor;

	FVector DesiredFacingLocation = FVector::ZeroVector;
	bool bUseFacingLocation = false;
	FVector GuideTargetLocation = FVector::ZeroVector;
	float GuideAcceptanceRadius = 120.f;

	UPROPERTY(EditDefaultsOnly, Category = "Tutorial")
	float GuideMoveSpeedUnitsPerSecond = 430.f;

	UPROPERTY(EditDefaultsOnly, Category = "Tutorial")
	float GuideTurnSpeedDegreesPerSecond = 540.f;

	bool bHasMoveTarget = false;
};
