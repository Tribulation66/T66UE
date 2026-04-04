// Copyright Tribulation 66. All Rights Reserved.

#include "Gameplay/T66TreeOfLifeInteractable.h"
#include "Core/T66RunStateSubsystem.h"
#include "Gameplay/T66VisualUtil.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/StaticMesh.h"
#include "Kismet/GameplayStatics.h"
#include "UObject/SoftObjectPath.h"

AT66TreeOfLifeInteractable::AT66TreeOfLifeInteractable()
{
	SingleMesh = TSoftObjectPtr<UStaticMesh>(FSoftObjectPath(TEXT("/Game/World/Interactables/Fountain/Fountain.Fountain")));

	// Trunk: cylinder
	if (UStaticMesh* Cylinder = FT66VisualUtil::GetBasicShapeCylinder())
	{
		VisualMesh->SetStaticMesh(Cylinder);
		VisualMesh->SetRelativeScale3D(FVector(0.45f, 0.45f, 2.0f));
		VisualMesh->SetRelativeLocation(FVector(0.f, 0.f, 100.f));
	}

	// Crown: sphere
	CrownMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("CrownMesh"));
	CrownMesh->SetupAttachment(RootComponent);
	CrownMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	if (UStaticMesh* Sphere = FT66VisualUtil::GetBasicShapeSphere())
	{
		CrownMesh->SetStaticMesh(Sphere);
		CrownMesh->SetRelativeScale3D(FVector(1.2f, 1.2f, 1.2f));
		CrownMesh->SetRelativeLocation(FVector(0.f, 0.f, 260.f));
	}

	ApplyRarityVisuals();
}

void AT66TreeOfLifeInteractable::ApplyRarityVisuals()
{
	if (TryApplyImportedMesh())
	{
		if (CrownMesh) CrownMesh->SetVisibility(false, true);
		return;
	}

	const FLinearColor FountainColor(0.20f, 0.80f, 1.00f, 1.0f);
	FT66VisualUtil::ApplyT66Color(VisualMesh, this, FountainColor);
	FT66VisualUtil::ApplyT66Color(CrownMesh, this, FountainColor);
}

bool AT66TreeOfLifeInteractable::Interact(APlayerController* PC)
{
	if (bConsumed) return false;

	UGameInstance* GI = UGameplayStatics::GetGameInstance(this);
	UT66RunStateSubsystem* RunState = GI ? GI->GetSubsystem<UT66RunStateSubsystem>() : nullptr;
	if (!RunState) return false;

	RunState->AddMaxHearts(1);
	RunState->HealToFull();
	bConsumed = true;
	Destroy();
	return true;
}

