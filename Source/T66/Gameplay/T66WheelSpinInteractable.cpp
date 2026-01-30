// Copyright Tribulation 66. All Rights Reserved.

#include "Gameplay/T66WheelSpinInteractable.h"
#include "UI/T66WheelOverlayWidget.h"
#include "Gameplay/T66PlayerController.h"
#include "Gameplay/T66VisualUtil.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/StaticMesh.h"

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

	ApplyRarityVisuals();
}

void AT66WheelSpinInteractable::ApplyRarityVisuals()
{
	const FLinearColor R = FT66RarityUtil::GetRarityColor(Rarity);
	FT66VisualUtil::ApplyT66Color(SphereBase, this, FLinearColor(0.12f, 0.12f, 0.14f, 1.f));
	FT66VisualUtil::ApplyT66Color(WheelMesh, this, R);
}

bool AT66WheelSpinInteractable::Interact(APlayerController* PC)
{
	if (!PC || bConsumed) return false;

	UT66WheelOverlayWidget* W = CreateWidget<UT66WheelOverlayWidget>(PC, UT66WheelOverlayWidget::StaticClass());
	if (!W) return false;

	W->SetWheelRarity(Rarity);
	W->AddToViewport(120);

	FInputModeGameAndUI InputMode;
	InputMode.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);
	InputMode.SetHideCursorDuringCapture(false);
	PC->SetInputMode(InputMode);
	PC->bShowMouseCursor = true;

	bConsumed = true;
	Destroy();
	return true;
}

