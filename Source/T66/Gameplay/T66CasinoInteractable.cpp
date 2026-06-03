// Copyright Tribulation 66. All Rights Reserved.

#include "Gameplay/T66CasinoInteractable.h"

#include "Components/BoxComponent.h"
#include "Core/T66InteractionPromptSubsystem.h"
#include "Engine/StaticMesh.h"
#include "Gameplay/T66PlayerController.h"
#include "Gameplay/T66SafeZoneComponent.h"
#include "Gameplay/T66VisualUtil.h"
#include "UObject/SoftObjectPath.h"

namespace
{
	static constexpr float T66CasinoSafeZoneRadiusUU = 650.f;
}

AT66CasinoInteractable::AT66CasinoInteractable()
{
	SingleMesh = TSoftObjectPtr<UStaticMesh>(FSoftObjectPath(TEXT("/Game/Characters/NPCs/Gambler/GamblerDemonStand/GamblerDemonStand.GamblerDemonStand")));

	if (TriggerBox)
	{
		TriggerBox->SetBoxExtent(FVector(340.f, 340.f, 260.f));
	}

	SafeZoneComponent = CreateDefaultSubobject<UT66SafeZoneComponent>(TEXT("CasinoSafeZone"));
	if (SafeZoneComponent)
	{
		SafeZoneComponent->SetupAttachment(RootComponent);
		SafeZoneComponent->ConfigureSafeZone(T66CasinoSafeZoneRadiusUU);
	}

	ApplyRarityVisuals();
}

bool AT66CasinoInteractable::Interact(APlayerController* PC)
{
	if (bConsumed)
	{
		return false;
	}

	AT66PlayerController* T66PC = Cast<AT66PlayerController>(PC);
	if (!T66PC)
	{
		return false;
	}

	return T66PC->OpenCasinoGamblerInteractable(this);
}

void AT66CasinoInteractable::HandleCasinoGambleCompleted()
{
	bConsumed = true;
	RefreshInteractionPrompt();
}

void AT66CasinoInteractable::ApplyRarityVisuals()
{
	if (!VisualMesh)
	{
		return;
	}

	if (TryApplyImportedMesh())
	{
		FT66VisualUtil::GroundMeshToActorOrigin(VisualMesh);
		RefreshInteractionPrompt();
		return;
	}

	if (UStaticMesh* Cube = FT66VisualUtil::GetBasicShapeCube())
	{
		VisualMesh->SetStaticMesh(Cube);
	}

	VisualMesh->SetRelativeScale3D(GetImportedVisualScale());
	FT66VisualUtil::ApplyT66Color(VisualMesh, this, FLinearColor(0.72f, 0.11f, 0.16f, 1.f));
	RefreshInteractionPrompt();
}

FText AT66CasinoInteractable::BuildInteractionPromptText() const
{
	const UGameInstance* GI = GetGameInstance();
	const UT66InteractionPromptSubsystem* PromptSubsystem = GI ? GI->GetSubsystem<UT66InteractionPromptSubsystem>() : nullptr;
	const FText Verb = NSLOCTEXT("T66.CasinoInteractable", "CasinoInteractVerb", "gamble");
	return PromptSubsystem ? PromptSubsystem->BuildCustomPromptText(Verb) : Verb;
}

FText AT66CasinoInteractable::BuildInteractionPromptTargetName() const
{
	return NSLOCTEXT("T66.CasinoInteractable", "CasinoTargetName", "Casino");
}

FVector AT66CasinoInteractable::GetImportedVisualScale() const
{
	return FVector(0.75f, 0.75f, 0.75f);
}
