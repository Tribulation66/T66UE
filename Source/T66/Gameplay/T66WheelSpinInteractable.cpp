// Copyright Tribulation 66. All Rights Reserved.

#include "Gameplay/T66WheelSpinInteractable.h"
#include "UI/T66WheelOverlayWidget.h"
#include "Gameplay/T66PlayerController.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/StaticMesh.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "Materials/MaterialInterface.h"

static void ApplyT66Color(UStaticMeshComponent* Mesh, UObject* Outer, const FLinearColor& Color)
{
	if (!Mesh) return;
	if (UMaterialInterface* ColorMat = LoadObject<UMaterialInterface>(nullptr, TEXT("/Game/Materials/M_PlaceholderColor.M_PlaceholderColor")))
	{
		if (UMaterialInstanceDynamic* Mat = UMaterialInstanceDynamic::Create(ColorMat, Outer ? Outer : Mesh))
		{
			Mat->SetVectorParameterValue(FName("Color"), Color);
			Mesh->SetMaterial(0, Mat);
			return;
		}
	}
	if (UMaterialInstanceDynamic* Mat = Mesh->CreateAndSetMaterialInstanceDynamic(0))
	{
		Mat->SetVectorParameterValue(FName("Color"), Color);
		Mat->SetVectorParameterValue(TEXT("BaseColor"), Color);
	}
}

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

	if (UStaticMesh* Sphere = LoadObject<UStaticMesh>(nullptr, TEXT("/Engine/BasicShapes/Sphere.Sphere")))
	{
		SphereBase->SetStaticMesh(Sphere);
		SphereBase->SetRelativeScale3D(FVector(0.85f, 0.85f, 0.85f));
		// Grounded: sphere radius ~42.5 => center at 42.5.
		SphereBase->SetRelativeLocation(FVector(0.f, 0.f, 42.5f));
	}
	if (UStaticMesh* Cylinder = LoadObject<UStaticMesh>(nullptr, TEXT("/Engine/BasicShapes/Cylinder.Cylinder")))
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
	ApplyT66Color(SphereBase, this, FLinearColor(0.12f, 0.12f, 0.14f, 1.f));
	ApplyT66Color(WheelMesh, this, R);
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

