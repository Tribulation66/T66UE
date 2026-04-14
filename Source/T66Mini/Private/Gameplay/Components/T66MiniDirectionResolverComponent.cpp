// Copyright Tribulation 66. All Rights Reserved.

#include "Gameplay/Components/T66MiniDirectionResolverComponent.h"

#include "Gameplay/Components/T66MiniSpritePresentationComponent.h"
#include "Camera/PlayerCameraManager.h"
#include "GameFramework/Actor.h"
#include "GameFramework/PlayerController.h"

namespace
{
	FVector T66MiniResolveCameraRight2D(const UWorld* World)
	{
		const APlayerController* PlayerController = World ? World->GetFirstPlayerController() : nullptr;
		const APlayerCameraManager* CameraManager = PlayerController ? PlayerController->PlayerCameraManager : nullptr;
		FVector CameraRight = CameraManager ? CameraManager->GetActorRightVector() : FVector::RightVector;
		CameraRight.Z = 0.f;
		if (CameraRight.IsNearlyZero())
		{
			return FVector::RightVector;
		}

		return CameraRight.GetSafeNormal2D();
	}
}

UT66MiniDirectionResolverComponent::UT66MiniDirectionResolverComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
}

void UT66MiniDirectionResolverComponent::BeginPlay()
{
	Super::BeginPlay();

	if (const AActor* Owner = GetOwner())
	{
		LastOwnerLocation = Owner->GetActorLocation();
		bHasLastOwnerLocation = true;
	}
}

void UT66MiniDirectionResolverComponent::TickComponent(
	const float DeltaTime,
	const ELevelTick TickType,
	FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	AActor* Owner = GetOwner();
	if (!Owner || !Presentation)
	{
		return;
	}

	const FVector CurrentLocation = Owner->GetActorLocation();
	if (!bHasLastOwnerLocation)
	{
		LastOwnerLocation = CurrentLocation;
		bHasLastOwnerLocation = true;
		return;
	}

	const FVector Delta = CurrentLocation - LastOwnerLocation;
	LastOwnerLocation = CurrentLocation;

	float FacingSign = 1.f;
	const FVector CameraRight2D = T66MiniResolveCameraRight2D(GetWorld());
	if (bHasFacingWorldTarget)
	{
		const FVector ToTarget = FacingWorldTarget - CurrentLocation;
		if (ToTarget.SizeSquared2D() <= 4.f)
		{
			return;
		}

		FacingSign = FVector::DotProduct(ToTarget.GetSafeNormal2D(), CameraRight2D) >= 0.f ? 1.f : -1.f;
	}
	else
	{
		if (Delta.SizeSquared2D() <= 9.f)
		{
			return;
		}

		FacingSign = FVector::DotProduct(Delta.GetSafeNormal2D(), CameraRight2D) >= 0.f ? 1.f : -1.f;
	}

	Presentation->ApplyFacingSign(FacingSign);
}

void UT66MiniDirectionResolverComponent::BindPresentation(UT66MiniSpritePresentationComponent* InPresentation)
{
	Presentation = InPresentation;
}

void UT66MiniDirectionResolverComponent::SetFacingWorldTarget(const FVector& InTargetLocation)
{
	FacingWorldTarget = InTargetLocation;
	bHasFacingWorldTarget = true;
}

void UT66MiniDirectionResolverComponent::ClearFacingWorldTarget()
{
	bHasFacingWorldTarget = false;
}
