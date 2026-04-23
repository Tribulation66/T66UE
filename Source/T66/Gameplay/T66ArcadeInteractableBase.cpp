// Copyright Tribulation 66. All Rights Reserved.

#include "Gameplay/T66ArcadeInteractableBase.h"

#include "Core/T66GameInstance.h"
#include "Core/T66InteractionPromptSubsystem.h"
#include "Gameplay/T66PlayerController.h"
#include "Gameplay/T66VisualUtil.h"
#include "Components/StaticMeshComponent.h"

namespace
{
	static FVector T66ResolveArcadeScale(const FVector& InScale)
	{
		return FVector(
			FMath::Max(KINDA_SMALL_NUMBER, InScale.X),
			FMath::Max(KINDA_SMALL_NUMBER, InScale.Y),
			FMath::Max(KINDA_SMALL_NUMBER, InScale.Z));
	}

	static FVector T66ResolveImportedArcadeLocation(const UStaticMeshComponent* MeshComponent, const FVector& Offset)
	{
		const float GroundedZ = MeshComponent ? MeshComponent->GetRelativeLocation().Z : 0.f;
		return FVector(Offset.X, Offset.Y, GroundedZ + Offset.Z);
	}

	static bool T66TryResolveArcadeRowData(const AActor* Actor, const FName ArcadeRowID, FT66ArcadeInteractableData& OutData)
	{
		if (ArcadeRowID.IsNone())
		{
			return false;
		}

		if (const UWorld* World = Actor ? Actor->GetWorld() : nullptr)
		{
			if (UT66GameInstance* T66GI = Cast<UT66GameInstance>(World->GetGameInstance()))
			{
				if (T66GI->GetArcadeInteractableData(ArcadeRowID, OutData))
				{
					return true;
				}
			}
		}

		if (UDataTable* DataTable = LoadObject<UDataTable>(nullptr, TEXT("/Game/Data/DT_ArcadeInteractables.DT_ArcadeInteractables")))
		{
			if (const FT66ArcadeInteractableRow* FoundRow = DataTable->FindRow<FT66ArcadeInteractableRow>(ArcadeRowID, TEXT("ArcadeInteractableBaseResolve")))
			{
				OutData = FoundRow->ArcadeData;
				if (OutData.ArcadeID.IsNone())
				{
					OutData.ArcadeID = ArcadeRowID;
				}
				return true;
			}
		}

		return false;
	}
}

AT66ArcadeInteractableBase::AT66ArcadeInteractableBase()
{
	ArcadeData.DisplayName = NSLOCTEXT("T66.Arcade", "DefaultArcadeDisplayName", "Arcade");
	ArcadeData.InteractionVerb = NSLOCTEXT("T66.Arcade", "DefaultArcadeVerb", "play arcade");
	ArcadeData.ArcadeClass = ET66ArcadeInteractableClass::PopupMinigame;
	ArcadeData.PopupGameType = ET66ArcadePopupGameType::None;
	ArcadeData.DisplayMeshScale = FVector(1.f, 1.f, 1.f);
	ArcadeData.Tint = FLinearColor(0.18f, 0.56f, 0.34f, 1.f);

	ResolvedArcadeData = ArcadeData;
	ApplyRarityVisuals();
}

void AT66ArcadeInteractableBase::OnConstruction(const FTransform& Transform)
{
	Super::OnConstruction(Transform);

	RefreshResolvedArcadeData();
	ApplyRarityVisuals();
	RefreshInteractionPrompt();
}

bool AT66ArcadeInteractableBase::Interact(APlayerController* PC)
{
	const FT66ArcadeInteractableData& Data = GetArcadeData();
	if (bConsumed
		|| bArcadeSessionActive
		|| Data.ArcadeClass != ET66ArcadeInteractableClass::PopupMinigame)
	{
		return false;
	}

	AT66PlayerController* T66PC = Cast<AT66PlayerController>(PC);
	if (!T66PC)
	{
		return false;
	}

	if (!T66PC->OpenArcadePopup(Data, this))
	{
		return false;
	}

	bArcadeSessionActive = true;
	RefreshInteractionPrompt();
	return true;
}

void AT66ArcadeInteractableBase::HandleArcadePopupClosed(const bool bSucceeded)
{
	bArcadeSessionActive = false;

	const FT66ArcadeInteractableData& Data = GetArcadeData();
	const bool bShouldConsume = bSucceeded ? Data.bConsumeOnSuccess : Data.bConsumeOnFailure;
	if (bShouldConsume)
	{
		bConsumed = true;
		Destroy();
		return;
	}

	RefreshInteractionPrompt();
}

void AT66ArcadeInteractableBase::ApplyRarityVisuals()
{
	if (!VisualMesh)
	{
		return;
	}

	const FT66ArcadeInteractableData& Data = GetArcadeData();
	SingleMesh = Data.DisplayMesh;
	if (TryApplyImportedMesh())
	{
		VisualMesh->SetRelativeLocation(T66ResolveImportedArcadeLocation(VisualMesh, Data.DisplayMeshOffset));
		FT66VisualUtil::ApplyT66Color(VisualMesh, this, Data.Tint);
		RefreshInteractionPrompt();
		return;
	}

	if (UStaticMesh* Cube = FT66VisualUtil::GetBasicShapeCube())
	{
		VisualMesh->SetStaticMesh(Cube);
	}

	VisualMesh->SetRelativeLocation(Data.DisplayMeshOffset);
	VisualMesh->SetRelativeScale3D(T66ResolveArcadeScale(Data.DisplayMeshScale));
	FT66VisualUtil::ApplyT66Color(VisualMesh, this, Data.Tint);
	RefreshInteractionPrompt();
}

bool AT66ArcadeInteractableBase::ShouldShowInteractionPrompt(const AT66HeroBase* LocalHero) const
{
	return !bArcadeSessionActive && AT66WorldInteractableBase::ShouldShowInteractionPrompt(LocalHero);
}

FText AT66ArcadeInteractableBase::BuildInteractionPromptText() const
{
	const UGameInstance* GI = GetGameInstance();
	const UT66InteractionPromptSubsystem* PromptSubsystem = GI ? GI->GetSubsystem<UT66InteractionPromptSubsystem>() : nullptr;
	return PromptSubsystem
		? PromptSubsystem->BuildCustomPromptText(ResolveInteractionVerb())
		: ResolveInteractionVerb();
}

FVector AT66ArcadeInteractableBase::GetImportedVisualScale() const
{
	return T66ResolveArcadeScale(GetArcadeData().DisplayMeshScale);
}

void AT66ArcadeInteractableBase::RefreshResolvedArcadeData()
{
	ResolvedArcadeData = ArcadeData;

	const FName EffectiveRowID = !ArcadeRowID.IsNone() ? ArcadeRowID : ArcadeData.ArcadeID;
	FT66ArcadeInteractableData TableData;
	if (T66TryResolveArcadeRowData(this, EffectiveRowID, TableData))
	{
		ResolvedArcadeData = TableData;
	}

	if (ResolvedArcadeData.ArcadeID.IsNone())
	{
		ResolvedArcadeData.ArcadeID = EffectiveRowID;
	}
	ResolvedArcadeData.ArcadeClass = ET66ArcadeInteractableClass::PopupMinigame;
}

FText AT66ArcadeInteractableBase::ResolveInteractionVerb() const
{
	const FT66ArcadeInteractableData& Data = GetArcadeData();
	if (!Data.InteractionVerb.IsEmpty())
	{
		return Data.InteractionVerb;
	}

	return NSLOCTEXT("T66.Arcade", "FallbackArcadeVerb", "play arcade");
}
