// Copyright Tribulation 66. All Rights Reserved.

#include "Gameplay/Components/T66MiniHitFlashComponent.h"

#include "Components/StaticMeshComponent.h"
#include "GameFramework/Actor.h"
#include "VFX/T66MiniVfxShared.h"

UT66MiniHitFlashComponent::UT66MiniHitFlashComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
}

void UT66MiniHitFlashComponent::BeginPlay()
{
	Super::BeginPlay();
	EnsureFlashMesh();
}

void UT66MiniHitFlashComponent::TickComponent(
	const float DeltaTime,
	const ELevelTick TickType,
	FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	if (FlashRemaining <= 0.f)
	{
		if (FlashMesh)
		{
			FlashMesh->SetVisibility(false, true);
		}
		return;
	}

	FlashRemaining = FMath::Max(0.f, FlashRemaining - DeltaTime);
	const float NormalizedRemaining = FlashDuration > 0.f ? (FlashRemaining / FlashDuration) : 0.f;
	ApplyVisuals(NormalizedRemaining);
}

void UT66MiniHitFlashComponent::InitializeFlash(
	USceneComponent* InAttachParent,
	const FVector& InRelativeLocation,
	const FVector& InRelativeScale,
	const FLinearColor& InTint)
{
	AttachParent = InAttachParent;
	RelativeLocation = InRelativeLocation;
	RelativeScale = InRelativeScale;
	Tint = InTint;
	if (GetOwner() && GetOwner()->HasActorBegunPlay())
	{
		EnsureFlashMesh();
	}
}

void UT66MiniHitFlashComponent::TriggerHitFlash(const FLinearColor& InTint, const float InDuration)
{
	Tint = InTint;
	FlashDuration = FMath::Max(0.01f, InDuration);
	FlashRemaining = FlashDuration;
	EnsureFlashMesh();
	ApplyVisuals(1.f);
}

void UT66MiniHitFlashComponent::EnsureFlashMesh()
{
	AActor* Owner = GetOwner();
	if (!Owner)
	{
		return;
	}

	if (!FlashMesh)
	{
		FlashMesh = NewObject<UStaticMeshComponent>(Owner, TEXT("MiniHitFlashMesh"));
		if (!FlashMesh)
		{
			return;
		}

		Owner->AddInstanceComponent(FlashMesh);
		FlashMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		FlashMesh->SetCastShadow(false);
		FlashMesh->SetStaticMesh(T66MiniVfx::LoadPlaneMesh());
		FlashMesh->RegisterComponent();
		USceneComponent* ParentComponent = AttachParent.Get() ? AttachParent.Get() : Owner->GetRootComponent();
		FlashMesh->AttachToComponent(ParentComponent, FAttachmentTransformRules::KeepRelativeTransform);
		FlashMesh->SetRelativeRotation(FRotator(-90.f, 0.f, 0.f));
		FlashMesh->SetVisibility(false, true);
	}
}

void UT66MiniHitFlashComponent::ApplyVisuals(const float NormalizedRemaining)
{
	if (!FlashMesh)
	{
		return;
	}

	const float Progress = 1.f - FMath::Clamp(NormalizedRemaining, 0.f, 1.f);
	const float PulseScale = 1.f + (Progress * 0.8f);
	const FLinearColor CurrentTint = FLinearColor(Tint.R, Tint.G, Tint.B, Tint.A * NormalizedRemaining);

	FlashMesh->SetVisibility(CurrentTint.A > 0.01f, true);
	FlashMesh->SetRelativeLocation(RelativeLocation);
	FlashMesh->SetRelativeScale3D(RelativeScale * PulseScale);
	T66MiniVfx::ApplyTintedMaterial(FlashMesh, this, nullptr, CurrentTint);
}
