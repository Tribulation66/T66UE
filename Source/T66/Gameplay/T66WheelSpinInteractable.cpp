// Copyright Tribulation 66. All Rights Reserved.

#include "Gameplay/T66WheelSpinInteractable.h"
#include "Gameplay/T66PlayerController.h"
#include "Gameplay/T66VisualUtil.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/StaticMesh.h"
#include "UObject/SoftObjectPath.h"

AT66WheelSpinInteractable::AT66WheelSpinInteractable()
{
	// Replace base cube with an invisible root visual (we'll use the sphere + wheel).
	if (VisualMesh)
	{
		VisualMesh->SetVisibility(false);
	}

	SphereBase = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("SphereBase"));
	SphereBase->SetupAttachment(RootComponent);
	SphereBase->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	WheelMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("WheelMesh"));
	WheelMesh->SetupAttachment(RootComponent);
	WheelMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	if (UStaticMesh* Sphere = FT66VisualUtil::GetBasicShapeSphere())
	{
		SphereBase->SetStaticMesh(Sphere);
		SphereBase->SetRelativeScale3D(FVector(0.85f, 0.85f, 0.85f));
		// Grounded: sphere radius ~42.5 => center at 42.5.
		SphereBase->SetRelativeLocation(FVector(0.f, 0.f, 42.5f));
	}
	if (UStaticMesh* Cylinder = FT66VisualUtil::GetBasicShapeCylinder())
	{
		WheelMesh->SetStaticMesh(Cylinder);
		WheelMesh->SetRelativeScale3D(FVector(0.65f, 0.65f, 0.15f));
		WheelMesh->SetRelativeRotation(FRotator(0.f, 0.f, 90.f));
		WheelMesh->SetRelativeLocation(FVector(0.f, 0.f, 95.f));
	}

	// Default expected import locations (safe if missing).
	MeshBlack = TSoftObjectPtr<UStaticMesh>(FSoftObjectPath(TEXT("/Game/World/Interactables/Wheels/SM_Wheel_Black.SM_Wheel_Black")));
	MeshRed = TSoftObjectPtr<UStaticMesh>(FSoftObjectPath(TEXT("/Game/World/Interactables/Wheels/SM_Wheel_Red.SM_Wheel_Red")));
	MeshYellow = TSoftObjectPtr<UStaticMesh>(FSoftObjectPath(TEXT("/Game/World/Interactables/Wheels/SM_Wheel_Yellow.SM_Wheel_Yellow")));
	MeshWhite = TSoftObjectPtr<UStaticMesh>(FSoftObjectPath(TEXT("/Game/World/Interactables/Wheels/SM_Wheel_White.SM_Wheel_White")));

	ApplyRarityVisuals();
}

void AT66WheelSpinInteractable::ApplyRarityVisuals()
{
	auto PickMesh = [&]() -> const TSoftObjectPtr<UStaticMesh>&
	{
		switch (Rarity)
		{
			case ET66Rarity::Black: return MeshBlack;
			case ET66Rarity::Red: return MeshRed;
			case ET66Rarity::Yellow: return MeshYellow;
			case ET66Rarity::White: return MeshWhite;
			default: return MeshBlack;
		}
	};

	UStaticMesh* M = nullptr;
	const TSoftObjectPtr<UStaticMesh>& Ptr = PickMesh();
	if (!Ptr.IsNull())
	{
		M = Ptr.Get();
		if (!M)
		{
			M = Ptr.LoadSynchronous();
		}
	}

	if (M && WheelMesh)
	{
		// Imported mesh: use as-authored and hide the sphere base.
		WheelMesh->SetStaticMesh(M);
		WheelMesh->SetRelativeScale3D(FVector(1.f, 1.f, 1.f));
		WheelMesh->SetRelativeRotation(FRotator::ZeroRotator);
		const float HalfHeight = FMath::Max(1.f, M->GetBounds().BoxExtent.Z);
		WheelMesh->SetRelativeLocation(FVector(0.f, 0.f, HalfHeight));

		if (SphereBase)
		{
			SphereBase->SetVisibility(false, true);
		}
		return;
	}

	// Fallback: primitives + rarity tint.
	const FLinearColor R = FT66RarityUtil::GetRarityColor(Rarity);
	FT66VisualUtil::ApplyT66Color(SphereBase, this, FLinearColor(0.12f, 0.12f, 0.14f, 1.f));
	FT66VisualUtil::ApplyT66Color(WheelMesh, this, R);
}

bool AT66WheelSpinInteractable::Interact(APlayerController* PC)
{
	if (!PC || bConsumed) return false;

	if (AT66PlayerController* T66PC = Cast<AT66PlayerController>(PC))
	{
		T66PC->StartWheelSpinHUD(Rarity);
	}

	bConsumed = true;
	Destroy();
	return true;
}

