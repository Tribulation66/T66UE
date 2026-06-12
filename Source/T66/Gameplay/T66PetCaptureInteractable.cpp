// Copyright Tribulation 66. All Rights Reserved.

#include "Gameplay/T66PetCaptureInteractable.h"

#include "Core/T66AchievementsSubsystem.h"
#include "Core/T66GameInstance.h"
#include "Core/T66ShelvedFeatureGate.h"
#include "Gameplay/T66VisualUtil.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/StaticMesh.h"
#include "GameFramework/PlayerController.h"

AT66PetCaptureInteractable::AT66PetCaptureInteractable()
{
	Rarity = ET66Rarity::Yellow;
	if (VisualMesh)
	{
		if (UStaticMesh* Sphere = FT66VisualUtil::GetBasicShapeSphere())
		{
			VisualMesh->SetStaticMesh(Sphere);
			VisualMesh->SetRelativeScale3D(FVector(0.32f));
			VisualMesh->SetRelativeLocation(FVector(0.f, 0.f, 32.f));
		}
	}
}

void AT66PetCaptureInteractable::InitializePetCapture(const FPetData& InPetData)
{
	PetData = InPetData;
	PetID = PetData.PetID.IsNone() ? PetData.SourceBossID : PetData.PetID;
	SourceBossID = PetData.SourceBossID.IsNone() ? PetID : PetData.SourceBossID;
	SingleMesh = PetData.CaptureVisualMesh;
	ApplyPetCaptureVisual();
}

void AT66PetCaptureInteractable::ApplyPetCaptureVisual()
{
	if (!VisualMesh)
	{
		return;
	}

	if (!TryApplyImportedMesh())
	{
		if (UStaticMesh* Sphere = FT66VisualUtil::GetBasicShapeSphere())
		{
			VisualMesh->SetStaticMesh(Sphere);
			VisualMesh->SetRelativeScale3D(FVector(0.32f));
			VisualMesh->SetRelativeLocation(FVector(0.f, 0.f, 32.f));
		}
	}
	FT66VisualUtil::ApplyT66Color(VisualMesh, this, PetData.PlaceholderColor);
}

bool AT66PetCaptureInteractable::Interact(APlayerController* PC)
{
	if (!FT66ShelvedFeatureGate::IsPetsEnabled())
	{
		return false;
	}

	if (bConsumed || PetID.IsNone())
	{
		return false;
	}

	UGameInstance* GIBase = GetGameInstance();
	UT66AchievementsSubsystem* Achievements = GIBase ? GIBase->GetSubsystem<UT66AchievementsSubsystem>() : nullptr;
	if (!Achievements)
	{
		return false;
	}

	bool bAutoEquipped = false;
	const bool bNewCapture = Achievements->CapturePet(PetID, bAutoEquipped);
	if (UT66GameInstance* T66GI = Cast<UT66GameInstance>(GIBase))
	{
		T66GI->SelectedPetID = Achievements->GetActivePetID();
	}

	bConsumed = true;
	UE_LOG(LogTemp, Log, TEXT("[Pets] Captured pet %s from boss %s (new=%d autoEquip=%d)"),
		*PetID.ToString(),
		*SourceBossID.ToString(),
		bNewCapture ? 1 : 0,
		bAutoEquipped ? 1 : 0);
	Destroy();
	static_cast<void>(PC);
	return true;
}

FText AT66PetCaptureInteractable::BuildInteractionPromptTargetName() const
{
	if (!FT66ShelvedFeatureGate::IsPetsEnabled())
	{
		return FText::FromString(FT66ShelvedFeatureGate::GetShelvedReason(ET66ShelvedFeature::Pets));
	}

	const FText PetName = PetData.DisplayName.IsEmpty() ? FText::FromName(PetID) : PetData.DisplayName;
	return FText::Format(
		NSLOCTEXT("T66.Pets", "CapturePetPromptTarget", "Capture {0}"),
		PetName);
}
