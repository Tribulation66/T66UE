// Copyright Tribulation 66. All Rights Reserved.

#include "Gameplay/T66TutorialGuideCompanion.h"

#include "Components/SkeletalMeshComponent.h"
#include "Engine/World.h"
#include "GameFramework/Actor.h"

AT66TutorialGuideCompanion::AT66TutorialGuideCompanion()
{
	PrimaryActorTick.bCanEverTick = true;
	FollowSpeed = 0.f;
	GroundFollowSpeed = 12.f;
}

void AT66TutorialGuideCompanion::MoveToGuidePoint(const FVector& InTargetLocation, const float InAcceptanceRadius)
{
	GuideTargetLocation = InTargetLocation;
	GuideAcceptanceRadius = FMath::Max(24.f, InAcceptanceRadius);
	bHasMoveTarget = true;
}

void AT66TutorialGuideCompanion::StopGuideMovement()
{
	bHasMoveTarget = false;
}

bool AT66TutorialGuideCompanion::HasReachedGuidePoint() const
{
	if (!bHasMoveTarget)
	{
		return true;
	}

	return FVector::Dist2D(GetActorLocation(), GuideTargetLocation) <= GuideAcceptanceRadius;
}

void AT66TutorialGuideCompanion::FaceLocation(const FVector& InWorldLocation)
{
	FacingActor.Reset();
	DesiredFacingLocation = InWorldLocation;
	bUseFacingLocation = true;
}

void AT66TutorialGuideCompanion::SetFacingActor(AActor* InActor)
{
	FacingActor = InActor;
	bUseFacingLocation = false;
}

void AT66TutorialGuideCompanion::Tick(float DeltaTime)
{
	if (bIsPreviewMode)
	{
		return;
	}

	FVector NewLocation = GetActorLocation();
	bool bIsMoving = false;

	if (bHasMoveTarget)
	{
		const FVector CurrentPlanar(NewLocation.X, NewLocation.Y, 0.f);
		const FVector TargetPlanar(GuideTargetLocation.X, GuideTargetLocation.Y, 0.f);
		const FVector Delta = TargetPlanar - CurrentPlanar;
		const float DistanceToTarget = Delta.Size();

		if (DistanceToTarget <= GuideAcceptanceRadius)
		{
			bHasMoveTarget = false;
		}
		else
		{
			const FVector Direction = Delta.GetSafeNormal();
			const float StepDistance = GuideMoveSpeedUnitsPerSecond * DeltaTime;
			const float MoveDistance = FMath::Min(DistanceToTarget, StepDistance);
			NewLocation.X += Direction.X * MoveDistance;
			NewLocation.Y += Direction.Y * MoveDistance;
			bIsMoving = true;
		}
	}

	UpdateGroundLocation(DeltaTime, NewLocation);
	SetActorLocation(NewLocation, false, nullptr, ETeleportType::TeleportPhysics);
	UpdateFacing(DeltaTime, bIsMoving);
	UpdateGuideAnimation(bIsMoving);
}

void AT66TutorialGuideCompanion::UpdateGuideAnimation(const bool bIsMoving)
{
	if (!bUsingCharacterVisual || !SkeletalMesh || !SkeletalMesh->IsVisible())
	{
		return;
	}

	const uint8 DesiredState = bIsMoving ? 2 : 0;
	if (DesiredState == LastMovementAnimState)
	{
		return;
	}

	LastMovementAnimState = DesiredState;
	UAnimationAsset* DesiredAnimation = bIsMoving
		? (CachedRunAnim ? CachedRunAnim : CachedWalkAnim)
		: CachedAlertAnim;
	if (DesiredAnimation)
	{
		SkeletalMesh->PlayAnimation(DesiredAnimation, true);
	}
}

void AT66TutorialGuideCompanion::UpdateGroundLocation(float DeltaTime, FVector& InOutLocation)
{
	UWorld* World = GetWorld();
	if (!World)
	{
		return;
	}

	++GroundTraceTickCounter;
	if (!bHasCachedGroundZ || GroundTraceTickCounter % GroundTraceEveryNTicks == 0)
	{
		FHitResult Hit;
		const FVector TraceOrigin(InOutLocation.X, InOutLocation.Y, GetActorLocation().Z);
		const FVector Start = TraceOrigin + FVector(0.f, 0.f, 2000.f);
		const FVector End = TraceOrigin - FVector(0.f, 0.f, 9000.f);
		if (World->LineTraceSingleByChannel(Hit, Start, End, ECC_WorldStatic)
			|| World->LineTraceSingleByChannel(Hit, Start, End, ECC_Visibility))
		{
			CachedGroundZ = Hit.ImpactPoint.Z;
			bHasCachedGroundZ = true;
		}
	}

	if (!bHasCachedGroundZ)
	{
		return;
	}

	const float CurrentZ = GetActorLocation().Z;
	InOutLocation.Z = FMath::FInterpTo(CurrentZ, CachedGroundZ, DeltaTime, GroundFollowSpeed);
	if (FMath::IsNearlyEqual(InOutLocation.Z, CachedGroundZ, 0.5f))
	{
		InOutLocation.Z = CachedGroundZ;
	}
}

void AT66TutorialGuideCompanion::UpdateFacing(float DeltaTime, const bool bIsMoving)
{
	FVector FacingDirection = FVector::ZeroVector;

	if (bIsMoving && bHasMoveTarget)
	{
		FacingDirection = GuideTargetLocation - GetActorLocation();
	}
	else if (AActor* FacingTarget = FacingActor.Get())
	{
		FacingDirection = FacingTarget->GetActorLocation() - GetActorLocation();
	}
	else if (bUseFacingLocation)
	{
		FacingDirection = DesiredFacingLocation - GetActorLocation();
	}

	FacingDirection.Z = 0.f;
	if (FacingDirection.IsNearlyZero())
	{
		return;
	}

	FRotator DesiredRotation = FacingDirection.Rotation();
	DesiredRotation.Pitch = 0.f;
	DesiredRotation.Roll = 0.f;

	const FRotator CurrentRotation = GetActorRotation();
	const FRotator NewRotation = FMath::RInterpConstantTo(CurrentRotation, DesiredRotation, DeltaTime, GuideTurnSpeedDegreesPerSecond);
	SetActorRotation(NewRotation);
}
