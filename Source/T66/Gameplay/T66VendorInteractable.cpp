// Copyright Tribulation 66. All Rights Reserved.

#include "Gameplay/T66VendorInteractable.h"

#include "Components/BoxComponent.h"
#include "Core/T66InteractionPromptSubsystem.h"
#include "Engine/StaticMesh.h"
#include "Gameplay/T66PlayerController.h"
#include "Gameplay/T66SafeZoneComponent.h"
#include "Gameplay/T66VisualUtil.h"
#include "UObject/SoftObjectPath.h"

namespace
{
	static constexpr float T66VendorSafeZoneRadiusUU = 650.f;
}

AT66VendorInteractable::AT66VendorInteractable()
{
	SingleMesh = TSoftObjectPtr<UStaticMesh>(FSoftObjectPath(TEXT("/Game/Characters/NPCs/Gambler/QuadRetro/SM_Gambler_QuadRetro.SM_Gambler_QuadRetro")));

	if (TriggerBox)
	{
		TriggerBox->SetBoxExtent(FVector(340.f, 340.f, 260.f));
	}

	SafeZoneComponent = CreateDefaultSubobject<UT66SafeZoneComponent>(TEXT("VendorSafeZone"));
	if (SafeZoneComponent)
	{
		SafeZoneComponent->SetupAttachment(RootComponent);
		SafeZoneComponent->ConfigureSafeZone(T66VendorSafeZoneRadiusUU);
	}

	ApplyRarityVisuals();
}

bool AT66VendorInteractable::Interact(APlayerController* PC)
{
	AT66PlayerController* T66PC = Cast<AT66PlayerController>(PC);
	if (!T66PC)
	{
		return false;
	}

	return T66PC->OpenVendorInteractable(this);
}

void AT66VendorInteractable::ApplyRarityVisuals()
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
	FT66VisualUtil::ApplyT66Color(VisualMesh, this, FLinearColor(0.16f, 0.58f, 0.35f, 1.f));
	RefreshInteractionPrompt();
}

FText AT66VendorInteractable::BuildInteractionPromptText() const
{
	const UGameInstance* GI = GetGameInstance();
	const UT66InteractionPromptSubsystem* PromptSubsystem = GI ? GI->GetSubsystem<UT66InteractionPromptSubsystem>() : nullptr;
	const FText Verb = NSLOCTEXT("T66.VendorInteractable", "VendorInteractVerb", "shop");
	return PromptSubsystem ? PromptSubsystem->BuildCustomPromptText(Verb) : Verb;
}

FText AT66VendorInteractable::BuildInteractionPromptTargetName() const
{
	return NSLOCTEXT("T66.VendorInteractable", "VendorTargetName", "Vendor");
}

FVector AT66VendorInteractable::GetImportedVisualScale() const
{
	return FVector(0.82f, 0.82f, 0.82f);
}
