// Copyright Tribulation 66. All Rights Reserved.

#include "Gameplay/T66ArcadeTruckInteractable.h"

#include "Core/T66GameInstance.h"
#include "Core/T66InteractionPromptSubsystem.h"
#include "Gameplay/T66VisualUtil.h"
#include "Components/StaticMeshComponent.h"
#include "UObject/SoftObjectPath.h"

namespace
{
	static FVector T66ResolveArcadeTruckScale(const FVector& InScale)
	{
		return FVector(
			FMath::Max(KINDA_SMALL_NUMBER, InScale.X),
			FMath::Max(KINDA_SMALL_NUMBER, InScale.Y),
			FMath::Max(KINDA_SMALL_NUMBER, InScale.Z));
	}

	static FVector T66ResolveImportedArcadeTruckLocation(const UStaticMeshComponent* MeshComponent, const FVector& Offset)
	{
		const float GroundedZ = MeshComponent ? MeshComponent->GetRelativeLocation().Z : 0.f;
		return FVector(Offset.X, Offset.Y, GroundedZ + Offset.Z);
	}

	static bool T66TryResolveArcadeTruckRowData(const AActor* Actor, const FName ArcadeRowID, FT66ArcadeInteractableData& OutData)
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
			if (const FT66ArcadeInteractableRow* FoundRow = DataTable->FindRow<FT66ArcadeInteractableRow>(ArcadeRowID, TEXT("ArcadeTruckResolve")))
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

AT66ArcadeTruckInteractable::AT66ArcadeTruckInteractable()
{
	ArcadeData.ArcadeID = FName(TEXT("Arcade_Truck"));
	ArcadeData.DisplayName = NSLOCTEXT("T66.Arcade", "TruckDisplayName", "Arcade Truck");
	ArcadeData.InteractionVerb = NSLOCTEXT("T66.Arcade", "TruckInteractVerb", "pilot truck");
	ArcadeData.ExitInteractionVerb = NSLOCTEXT("T66.Arcade", "TruckExitVerb", "exit truck");
	ArcadeData.ArcadeClass = ET66ArcadeInteractableClass::WorldMode;
	ArcadeData.PopupGameType = ET66ArcadePopupGameType::None;
	ArcadeData.DisplayMesh = TSoftObjectPtr<UStaticMesh>(FSoftObjectPath(TEXT("/Game/World/Props/Tractor.Tractor")));
	ArcadeData.DisplayMeshScale = FVector(1.f, 1.f, 1.f);
	ArcadeData.Tint = FLinearColor(0.94f, 0.53f, 0.17f, 1.f);
	ArcadeData.Modifiers.FloatValues.Add(T66ArcadeModifierKeys::VehicleDuration, 20.f);
	ArcadeData.Modifiers.FloatValues.Add(T66ArcadeModifierKeys::VehicleDriveSpeed, 2200.f);
	ArcadeData.Modifiers.FloatValues.Add(T66ArcadeModifierKeys::VehicleTurnSpeed, 620.f);
	ArcadeData.Modifiers.FloatValues.Add(T66ArcadeModifierKeys::VehicleMowRadius, 260.f);
	ArcadeData.Modifiers.FloatValues.Add(T66ArcadeModifierKeys::VehicleMowMinSpeed, 125.f);
	ArcadeRowID = ArcadeData.ArcadeID;

	ResolvedArcadeData = ArcadeData;
	ApplyArcadeTruckTuning();
	ApplyRarityVisuals();
}

void AT66ArcadeTruckInteractable::OnConstruction(const FTransform& Transform)
{
	Super::OnConstruction(Transform);

	RefreshResolvedArcadeData();
	ApplyArcadeTruckTuning();
	ApplyRarityVisuals();
	RefreshInteractionPrompt();
}

void AT66ArcadeTruckInteractable::BeginPlay()
{
	RefreshResolvedArcadeData();
	ApplyArcadeTruckTuning();
	Super::BeginPlay();
	ApplyRarityVisuals();
	RefreshInteractionPrompt();
}

void AT66ArcadeTruckInteractable::ApplyRarityVisuals()
{
	const FT66ArcadeInteractableData& Data = GetArcadeData();
	SingleMesh = Data.DisplayMesh;
	if (TryApplyImportedMesh())
	{
		if (VisualMesh)
		{
			VisualMesh->SetRelativeLocation(T66ResolveImportedArcadeTruckLocation(VisualMesh, Data.DisplayMeshOffset));
			FT66VisualUtil::ApplyT66Color(VisualMesh, this, Data.Tint);
		}

		RefreshInteractionPrompt();
		return;
	}

	if (VisualMesh)
	{
		VisualMesh->SetRelativeLocation(Data.DisplayMeshOffset);
		VisualMesh->SetRelativeScale3D(T66ResolveArcadeTruckScale(Data.DisplayMeshScale));
		FT66VisualUtil::ApplyT66Color(VisualMesh, this, Data.Tint);
	}

	RefreshInteractionPrompt();
}

FText AT66ArcadeTruckInteractable::BuildInteractionPromptText() const
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

FVector AT66ArcadeTruckInteractable::GetImportedVisualScale() const
{
	return T66ResolveArcadeTruckScale(GetArcadeData().DisplayMeshScale);
}

void AT66ArcadeTruckInteractable::RefreshResolvedArcadeData()
{
	ResolvedArcadeData = ArcadeData;

	const FName EffectiveRowID = !ArcadeRowID.IsNone() ? ArcadeRowID : ArcadeData.ArcadeID;
	FT66ArcadeInteractableData TableData;
	if (T66TryResolveArcadeTruckRowData(this, EffectiveRowID, TableData))
	{
		ResolvedArcadeData = TableData;
	}

	if (ResolvedArcadeData.ArcadeID.IsNone())
	{
		ResolvedArcadeData.ArcadeID = EffectiveRowID;
	}
	ResolvedArcadeData.ArcadeClass = ET66ArcadeInteractableClass::WorldMode;
	ResolvedArcadeData.PopupGameType = ET66ArcadePopupGameType::None;
}

void AT66ArcadeTruckInteractable::ApplyArcadeTruckTuning()
{
	const FT66ArcadeInteractableData& Data = GetArcadeData();
	SingleMesh = Data.DisplayMesh;
	ConfigurePilotableTractorTuning(
		Data.Modifiers.GetFloat(T66ArcadeModifierKeys::VehicleDuration, 20.f),
		Data.Modifiers.GetFloat(T66ArcadeModifierKeys::VehicleDriveSpeed, 2200.f),
		Data.Modifiers.GetFloat(T66ArcadeModifierKeys::VehicleTurnSpeed, 620.f),
		Data.Modifiers.GetFloat(T66ArcadeModifierKeys::VehicleMowRadius, 260.f),
		Data.Modifiers.GetFloat(T66ArcadeModifierKeys::VehicleMowMinSpeed, 125.f));
}

FText AT66ArcadeTruckInteractable::ResolvePromptVerb() const
{
	const FT66ArcadeInteractableData& Data = GetArcadeData();
	if (HasMountedHero())
	{
		return !Data.ExitInteractionVerb.IsEmpty()
			? Data.ExitInteractionVerb
			: NSLOCTEXT("T66.Arcade", "FallbackTruckExitVerb", "exit truck");
	}

	return !Data.InteractionVerb.IsEmpty()
		? Data.InteractionVerb
		: NSLOCTEXT("T66.Arcade", "FallbackTruckInteractVerb", "pilot truck");
}
