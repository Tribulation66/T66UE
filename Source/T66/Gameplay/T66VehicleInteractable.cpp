// Copyright Tribulation 66. All Rights Reserved.

#include "Gameplay/T66VehicleInteractable.h"

#include "Core/T66GameInstance.h"
#include "Core/T66InteractionPromptSubsystem.h"
#include "Gameplay/T66VisualUtil.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/DataTable.h"

namespace
{
	static FVector T66ResolveVehicleScale(const FVector& InScale)
	{
		return FVector(
			FMath::Max(KINDA_SMALL_NUMBER, InScale.X),
			FMath::Max(KINDA_SMALL_NUMBER, InScale.Y),
			FMath::Max(KINDA_SMALL_NUMBER, InScale.Z));
	}

	static FVector T66ResolveImportedVehicleLocation(const UStaticMeshComponent* MeshComponent, const FVector& Offset)
	{
		const float GroundedZ = MeshComponent ? MeshComponent->GetRelativeLocation().Z : 0.f;
		return FVector(Offset.X, Offset.Y, GroundedZ + Offset.Z);
	}

	static FName T66GetVehicleDifficultyID(const ET66Difficulty Difficulty)
	{
		switch (Difficulty)
		{
		case ET66Difficulty::Medium:
			return FName(TEXT("Medium"));
		case ET66Difficulty::Hard:
			return FName(TEXT("Hard"));
		case ET66Difficulty::VeryHard:
			return FName(TEXT("VeryHard"));
		case ET66Difficulty::Impossible:
			return FName(TEXT("Impossible"));
		case ET66Difficulty::Easy:
		default:
			return FName(TEXT("Easy"));
		}
	}

	static FName T66GetSelectedVehicleDifficultyID(const AActor* Actor)
	{
		const UWorld* World = Actor ? Actor->GetWorld() : nullptr;
		const UT66GameInstance* T66GI = World ? Cast<UT66GameInstance>(World->GetGameInstance()) : nullptr;
		return T66GetVehicleDifficultyID(T66GI ? T66GI->SelectedDifficulty : ET66Difficulty::Easy);
	}

	static FName T66BuildVehicleDifficultyRowID(const FName VehicleID, const FName DifficultyID)
	{
		if (VehicleID.IsNone() || DifficultyID.IsNone())
		{
			return NAME_None;
		}

		return FName(*FString::Printf(TEXT("%s_%s"), *VehicleID.ToString(), *DifficultyID.ToString()));
	}

	static bool T66FindVehicleRowData(UDataTable* DataTable, const FName RowID, FT66VehicleInteractableData& OutData)
	{
		if (!DataTable || RowID.IsNone())
		{
			return false;
		}

		if (const FT66VehicleInteractableRow* FoundRow = DataTable->FindRow<FT66VehicleInteractableRow>(RowID, TEXT("VehicleInteractableResolve")))
		{
			OutData = FoundRow->VehicleData;
			if (OutData.VehicleID.IsNone())
			{
				OutData.VehicleID = RowID;
			}
			return true;
		}

		return false;
	}

	static bool T66TryResolveVehicleRowData(const AActor* Actor, const FName VehicleRowID, FT66VehicleInteractableData& OutData)
	{
		if (VehicleRowID.IsNone())
		{
			return false;
		}

		UDataTable* DataTable = LoadObject<UDataTable>(nullptr, TEXT("/Game/Data/DT_VehicleInteractables.DT_VehicleInteractables"));
		if (!DataTable)
		{
			return false;
		}

		const FString VehicleRowString = VehicleRowID.ToString();
		const bool bAlreadyDifficultySpecific = VehicleRowString.StartsWith(TEXT("Vehicle_"));
		const FName DifficultyRowID = bAlreadyDifficultySpecific
			? NAME_None
			: T66BuildVehicleDifficultyRowID(VehicleRowID, T66GetSelectedVehicleDifficultyID(Actor));

		if (T66FindVehicleRowData(DataTable, DifficultyRowID, OutData))
		{
			if (OutData.DifficultyID.IsNone())
			{
				OutData.DifficultyID = T66GetSelectedVehicleDifficultyID(Actor);
			}
			return true;
		}

		return T66FindVehicleRowData(DataTable, VehicleRowID, OutData);
	}
}

AT66VehicleInteractable::AT66VehicleInteractable()
{
	VehicleData.VehicleID = FName(TEXT("Vehicle"));
	VehicleData.DifficultyID = FName(TEXT("Shared"));
	VehicleData.DisplayName = NSLOCTEXT("T66.Vehicle", "VehicleDisplayName", "Vehicle");
	VehicleData.InteractionVerb = NSLOCTEXT("T66.Vehicle", "VehicleInteractVerb", "pilot vehicle");
	VehicleData.ExitInteractionVerb = NSLOCTEXT("T66.Vehicle", "VehicleExitVerb", "exit vehicle");
	VehicleData.DisplayMeshScale = FVector(1.f, 1.f, 1.f);
	VehicleData.Tint = FLinearColor(0.94f, 0.53f, 0.17f, 1.f);
	VehicleData.PilotSeconds = 20.f;
	VehicleData.DriveSpeed = 2200.f;
	VehicleData.TurnSpeedDegreesPerSecond = 620.f;
	VehicleData.MowKillRadius = 260.f;
	VehicleData.MowMinSpeed = 125.f;
	VehicleRowID = VehicleData.VehicleID;

	ResolvedVehicleData = VehicleData;
	ApplyVehicleTuning();
	ApplyRarityVisuals();
}

void AT66VehicleInteractable::SetVehicleRowID(const FName InVehicleRowID)
{
	VehicleRowID = InVehicleRowID;
	RefreshResolvedVehicleData();
	ApplyVehicleTuning();
	ApplyRarityVisuals();
	RefreshInteractionPrompt();
}

void AT66VehicleInteractable::OnConstruction(const FTransform& Transform)
{
	Super::OnConstruction(Transform);

	RefreshResolvedVehicleData();
	ApplyVehicleTuning();
	ApplyRarityVisuals();
	RefreshInteractionPrompt();
}

void AT66VehicleInteractable::BeginPlay()
{
	RefreshResolvedVehicleData();
	ApplyVehicleTuning();
	Super::BeginPlay();
	ApplyRarityVisuals();
	RefreshInteractionPrompt();
}

void AT66VehicleInteractable::ApplyRarityVisuals()
{
	const FT66VehicleInteractableData& Data = GetVehicleData();
	SingleMesh = Data.DisplayMesh;
	if (TryApplyImportedMesh())
	{
		if (VisualMesh)
		{
			VisualMesh->SetRelativeLocation(T66ResolveImportedVehicleLocation(VisualMesh, Data.DisplayMeshOffset));
		}

		RefreshInteractionPrompt();
		return;
	}

	if (VisualMesh)
	{
		VisualMesh->SetRelativeLocation(Data.DisplayMeshOffset);
		VisualMesh->SetRelativeScale3D(T66ResolveVehicleScale(Data.DisplayMeshScale));
		FT66VisualUtil::ApplyT66Color(VisualMesh, this, Data.Tint);
	}

	RefreshInteractionPrompt();
}

FText AT66VehicleInteractable::BuildInteractionPromptText() const
{
	if (GetRemainingPilotSeconds() <= KINDA_SMALL_NUMBER)
	{
		return FText::GetEmpty();
	}

	const UGameInstance* GI = GetGameInstance();
	const UT66InteractionPromptSubsystem* PromptSubsystem = GI ? GI->GetSubsystem<UT66InteractionPromptSubsystem>() : nullptr;
	const FText PromptVerb = ResolvePromptVerb();
	if (PromptSubsystem)
	{
		return PromptSubsystem->BuildCustomPromptTextWithSeconds(PromptVerb, FMath::CeilToInt(GetRemainingPilotSeconds()));
	}

	return PromptVerb;
}

FText AT66VehicleInteractable::BuildInteractionPromptTargetName() const
{
	return GetVehicleData().DisplayName.IsEmpty()
		? AT66WorldInteractableBase::BuildInteractionPromptTargetName()
		: GetVehicleData().DisplayName;
}

FVector AT66VehicleInteractable::GetImportedVisualScale() const
{
	return T66ResolveVehicleScale(GetVehicleData().DisplayMeshScale);
}

void AT66VehicleInteractable::RefreshResolvedVehicleData()
{
	ResolvedVehicleData = VehicleData;

	const FName EffectiveRowID = !VehicleRowID.IsNone() ? VehicleRowID : VehicleData.VehicleID;
	FT66VehicleInteractableData TableData;
	if (T66TryResolveVehicleRowData(this, EffectiveRowID, TableData))
	{
		ResolvedVehicleData = TableData;
	}

	if (ResolvedVehicleData.VehicleID.IsNone())
	{
		ResolvedVehicleData.VehicleID = EffectiveRowID;
	}
}

void AT66VehicleInteractable::ApplyVehicleTuning()
{
	const FT66VehicleInteractableData& Data = GetVehicleData();
	SingleMesh = Data.DisplayMesh;
	ConfigurePilotableTractorTuning(
		Data.PilotSeconds,
		Data.DriveSpeed,
		Data.TurnSpeedDegreesPerSecond,
		Data.MowKillRadius,
		Data.MowMinSpeed);
}

FText AT66VehicleInteractable::ResolvePromptVerb() const
{
	const FT66VehicleInteractableData& Data = GetVehicleData();
	if (HasMountedHero())
	{
		return !Data.ExitInteractionVerb.IsEmpty()
			? Data.ExitInteractionVerb
			: NSLOCTEXT("T66.Vehicle", "FallbackVehicleExitVerb", "exit vehicle");
	}

	return !Data.InteractionVerb.IsEmpty()
		? Data.InteractionVerb
		: NSLOCTEXT("T66.Vehicle", "FallbackVehicleInteractVerb", "pilot vehicle");
}