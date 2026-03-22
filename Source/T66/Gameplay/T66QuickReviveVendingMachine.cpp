// Copyright Tribulation 66. All Rights Reserved.

#include "Gameplay/T66QuickReviveVendingMachine.h"

#include "Core/T66RunStateSubsystem.h"
#include "Gameplay/T66VisualUtil.h"
#include "Gameplay/T66HeroBase.h"
#include "Components/BoxComponent.h"
#include "Components/StaticMeshComponent.h"
#include "UObject/SoftObjectPath.h"

AT66QuickReviveVendingMachine::AT66QuickReviveVendingMachine()
{
	SingleMesh = TSoftObjectPtr<UStaticMesh>(FSoftObjectPath(TEXT("/Game/World/Interactables/Vending/Vending.Vending")));

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

	if (UT66RunStateSubsystem* RunState = GetRunState())
	{
		RunState->QuickReviveChanged.AddDynamic(this, &AT66QuickReviveVendingMachine::HandleQuickReviveStateChanged);
	}

	RefreshInteractionPrompt();
}

void AT66QuickReviveVendingMachine::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	if (UT66RunStateSubsystem* RunState = GetRunState())
	{
		RunState->QuickReviveChanged.RemoveDynamic(this, &AT66QuickReviveVendingMachine::HandleQuickReviveStateChanged);
	}

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

bool AT66QuickReviveVendingMachine::ShouldShowInteractionPrompt(const AT66HeroBase* LocalHero) const
{
	if (!LocalHero || !IsLocalHeroOverlapping())
	{
		return false;
	}

	const UT66RunStateSubsystem* RunState = GetRunState();
	return RunState && !RunState->HasQuickReviveCharge() && !RunState->IsInQuickReviveDownedState();
}

bool AT66QuickReviveVendingMachine::Interact(APlayerController* PC)
{
	if (!PC)
	{
		return false;
	}

	UT66RunStateSubsystem* RunState = GetRunState();
	if (!RunState)
	{
		return false;
	}

	const bool bGranted = RunState->GrantQuickReviveCharge();
	if (bGranted)
	{
		RefreshInteractionPrompt();
	}
	return bGranted;
}

void AT66QuickReviveVendingMachine::HandleQuickReviveStateChanged()
{
	RefreshInteractionPrompt();
}

UT66RunStateSubsystem* AT66QuickReviveVendingMachine::GetRunState() const
{
	UGameInstance* GI = GetGameInstance();
	return GI ? GI->GetSubsystem<UT66RunStateSubsystem>() : nullptr;
}
