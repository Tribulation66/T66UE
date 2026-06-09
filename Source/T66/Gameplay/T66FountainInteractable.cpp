// Copyright Tribulation 66. All Rights Reserved.

#include "Gameplay/T66FountainInteractable.h"
#include "Core/T66RunStateSubsystem.h"
#include "Gameplay/T66VisualUtil.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/StaticMesh.h"
#include "Kismet/GameplayStatics.h"
#include "UObject/SoftObjectPath.h"

AT66FountainInteractable::AT66FountainInteractable()
{
	SingleMesh = TSoftObjectPtr<UStaticMesh>(FSoftObjectPath(TEXT("/Game/World/Interactables/Fountain/SM_Fountain_Pixal3D.SM_Fountain_Pixal3D")));

	if (UStaticMesh* Cylinder = FT66VisualUtil::GetBasicShapeCylinder())
	{
		VisualMesh->SetStaticMesh(Cylinder);
		VisualMesh->SetRelativeScale3D(FVector(0.45f, 0.45f, 2.0f));
		VisualMesh->SetRelativeLocation(FVector(0.f, 0.f, 100.f));
	}

	WaterMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("WaterMesh"));
	WaterMesh->SetupAttachment(RootComponent);
	WaterMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	if (UStaticMesh* Sphere = FT66VisualUtil::GetBasicShapeSphere())
	{
		WaterMesh->SetStaticMesh(Sphere);
		WaterMesh->SetRelativeScale3D(FVector(1.2f, 1.2f, 1.2f));
		WaterMesh->SetRelativeLocation(FVector(0.f, 0.f, 260.f));
	}

	ApplyRarityVisuals();
}

void AT66FountainInteractable::ApplyRarityVisuals()
{
	if (TryApplyImportedMesh())
	{
		if (WaterMesh)
		{
			WaterMesh->SetVisibility(false, true);
		}
		return;
	}

	const FLinearColor FountainColor(0.20f, 0.80f, 1.00f, 1.0f);
	FT66VisualUtil::ApplyT66Color(VisualMesh, this, FountainColor);
	FT66VisualUtil::ApplyT66Color(WaterMesh, this, FountainColor);
}

bool AT66FountainInteractable::Interact(APlayerController* PC)
{
	if (bConsumed)
	{
		return false;
	}

	UGameInstance* GI = UGameplayStatics::GetGameInstance(this);
	UT66RunStateSubsystem* RunState = GI ? GI->GetSubsystem<UT66RunStateSubsystem>() : nullptr;
	if (!RunState)
	{
		return false;
	}

	RunState->HealToFull();
	if (IsShowcaseReusable())
	{
		bConsumed = false;
		RefreshInteractionPrompt();
		return true;
	}

	bConsumed = true;
	Destroy();
	return true;
}
