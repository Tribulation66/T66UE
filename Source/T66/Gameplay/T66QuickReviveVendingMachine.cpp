// Copyright Tribulation 66. All Rights Reserved.

#include "Gameplay/T66QuickReviveVendingMachine.h"

#include "Gameplay/T66VisualUtil.h"
#include "Gameplay/T66HeroBase.h"
#include "Components/BoxComponent.h"
#include "Components/StaticMeshComponent.h"
#include "UObject/SoftObjectPath.h"

AT66QuickReviveVendingMachine::AT66QuickReviveVendingMachine()
{
	SingleMesh = TSoftObjectPtr<UStaticMesh>(FSoftObjectPath(TEXT("/Game/World/Interactables/Vending/SM_QuickReviveVending_Pixal3D.SM_QuickReviveVending_Pixal3D")));

	if (VisualMesh)
	{
		VisualMesh->SetRelativeScale3D(FVector(1.0f, 0.9f, 2.2f));
		VisualMesh->SetRelativeLocation(FVector(0.f, 0.f, 110.f));
	}

	if (TriggerBox)
	{
		TriggerBox->SetBoxExtent(FVector(380.f, 320.f, 240.f));
	}

	ApplyRarityVisuals();
}

void AT66QuickReviveVendingMachine::BeginPlay()
{
	Super::BeginPlay();

	RefreshInteractionPrompt();
}

void AT66QuickReviveVendingMachine::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	Super::EndPlay(EndPlayReason);
}

void AT66QuickReviveVendingMachine::ApplyRarityVisuals()
{
	if (TryApplyImportedMesh())
	{
		return;
	}

	if (!VisualMesh)
	{
		return;
	}

	FT66VisualUtil::ApplyT66Color(VisualMesh, this, FLinearColor(0.70f, 0.12f, 0.12f, 1.f));
}

bool AT66QuickReviveVendingMachine::ShouldShowInteractionPrompt(const AT66HeroBase* /*LocalHero*/) const
{
	return false;
}

bool AT66QuickReviveVendingMachine::Interact(APlayerController* /*PC*/)
{
	return false;
}
