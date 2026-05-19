// Copyright Tribulation 66. All Rights Reserved.

#include "Gameplay/T66WorldVisualProp.h"

#include "Core/T66GameInstance.h"
#include "Gameplay/T66VisualUtil.h"
#include "Components/SceneComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/DataTable.h"
#include "Engine/StaticMesh.h"

namespace
{
	static FVector T66ResolvePropScale(const FVector& InScale)
	{
		return FVector(
			FMath::Max(KINDA_SMALL_NUMBER, InScale.X),
			FMath::Max(KINDA_SMALL_NUMBER, InScale.Y),
			FMath::Max(KINDA_SMALL_NUMBER, InScale.Z));
	}

	static FName T66GetVisualPropDifficultyID(const ET66Difficulty Difficulty)
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

	static FName T66GetSelectedVisualPropDifficultyID(const AActor* Actor)
	{
		const UWorld* World = Actor ? Actor->GetWorld() : nullptr;
		const UT66GameInstance* T66GI = World ? Cast<UT66GameInstance>(World->GetGameInstance()) : nullptr;
		return T66GetVisualPropDifficultyID(T66GI ? T66GI->SelectedDifficulty : ET66Difficulty::Easy);
	}

	static FName T66BuildVisualPropDifficultyRowID(const FName PropID, const FName DifficultyID)
	{
		if (PropID.IsNone() || DifficultyID.IsNone())
		{
			return NAME_None;
		}

		return FName(*FString::Printf(TEXT("%s_%s"), *PropID.ToString(), *DifficultyID.ToString()));
	}

	static bool T66IsDifficultySpecificVisualPropRow(const FName RowID)
	{
		const FString RowString = RowID.ToString();
		return RowString.EndsWith(TEXT("_Easy"))
			|| RowString.EndsWith(TEXT("_Medium"))
			|| RowString.EndsWith(TEXT("_Hard"))
			|| RowString.EndsWith(TEXT("_VeryHard"))
			|| RowString.EndsWith(TEXT("_Impossible"));
	}

	static bool T66FindVisualPropRowData(UDataTable* DataTable, const FName RowID, FT66WorldVisualPropData& OutData)
	{
		if (!DataTable || RowID.IsNone())
		{
			return false;
		}

		if (const FT66WorldVisualPropRow* FoundRow = DataTable->FindRow<FT66WorldVisualPropRow>(RowID, TEXT("WorldVisualPropResolve")))
		{
			OutData = FoundRow->PropData;
			if (OutData.PropID.IsNone())
			{
				OutData.PropID = RowID;
			}
			return true;
		}

		return false;
	}

	static bool T66TryResolveVisualPropRowData(UDataTable* DataTable, const AActor* Actor, const FName PropRowID, const bool bUseDifficultySpecificRow, FT66WorldVisualPropData& OutData)
	{
		if (PropRowID.IsNone())
		{
			return false;
		}

		if (!DataTable)
		{
			return false;
		}

		const FName DifficultyID = T66GetSelectedVisualPropDifficultyID(Actor);
		if (bUseDifficultySpecificRow && !T66IsDifficultySpecificVisualPropRow(PropRowID))
		{
			const FName DifficultyRowID = T66BuildVisualPropDifficultyRowID(PropRowID, DifficultyID);
			if (T66FindVisualPropRowData(DataTable, DifficultyRowID, OutData))
			{
				if (OutData.DifficultyID.IsNone())
				{
					OutData.DifficultyID = DifficultyID;
				}
				return true;
			}
		}

		return T66FindVisualPropRowData(DataTable, PropRowID, OutData);
	}

	static UStaticMesh* T66ResolveFallbackVisualPropMesh(const FName PropID)
	{
		const FString PropString = PropID.ToString();
		if (PropString.Contains(TEXT("Skull")) || PropString.Contains(TEXT("Lamp")))
		{
			return FT66VisualUtil::GetBasicShapeSphere();
		}
		if (PropString.Contains(TEXT("Torch")))
		{
			return FT66VisualUtil::GetBasicShapeCone();
		}
		if (PropString.Contains(TEXT("Vase")))
		{
			return FT66VisualUtil::GetBasicShapeCylinder();
		}
		return FT66VisualUtil::GetBasicShapeCube();
	}
}

AT66WorldVisualProp::AT66WorldVisualProp()
{
	PrimaryActorTick.bCanEverTick = false;

	SceneRoot = CreateDefaultSubobject<USceneComponent>(TEXT("SceneRoot"));
	SetRootComponent(SceneRoot);

	VisualMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("VisualMesh"));
	VisualMesh->SetupAttachment(SceneRoot);
	VisualMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	VisualMesh->SetGenerateOverlapEvents(false);
	VisualMesh->SetCanEverAffectNavigation(false);

	PropDataTable = TSoftObjectPtr<UDataTable>(FSoftObjectPath(TEXT("/Game/Data/DT_WorldVisualProps.DT_WorldVisualProps")));

	DefaultPropData.PropID = FName(TEXT("WallLamp"));
	DefaultPropData.DifficultyID = FName(TEXT("Easy"));
	DefaultPropData.DisplayName = NSLOCTEXT("T66.WorldVisualProp", "WallLampDisplayName", "Wall Lamp");
	DefaultPropData.DisplayMeshScale = FVector(0.7f, 0.25f, 0.7f);
	DefaultPropData.DisplayMeshOffset = FVector(0.f, 0.f, 180.f);
	DefaultPropData.Tint = FLinearColor(0.86f, 0.74f, 0.46f, 1.f);
	DefaultPropData.bAutoGroundToActorOrigin = false;
	DefaultPropData.bSnapToGround = false;
	ResolvedPropData = DefaultPropData;
}

void AT66WorldVisualProp::SetPropRowID(FName InPropRowID)
{
	PropRowID = InPropRowID;
	RefreshVisuals();
}

void AT66WorldVisualProp::OnConstruction(const FTransform& Transform)
{
	Super::OnConstruction(Transform);
	RefreshVisuals();
}

void AT66WorldVisualProp::BeginPlay()
{
	Super::BeginPlay();
	RefreshVisuals();
}

void AT66WorldVisualProp::RefreshVisuals()
{
	RefreshResolvedPropData();
	ApplyVisualData();
}

void AT66WorldVisualProp::RefreshResolvedPropData()
{
	ResolvedPropData = DefaultPropData;

	const FName EffectiveRowID = !PropRowID.IsNone() ? PropRowID : DefaultPropData.PropID;
	FT66WorldVisualPropData TableData;
	UDataTable* DataTable = PropDataTable.IsNull()
		? LoadObject<UDataTable>(nullptr, TEXT("/Game/Data/DT_WorldVisualProps.DT_WorldVisualProps"))
		: PropDataTable.LoadSynchronous();
	if (T66TryResolveVisualPropRowData(DataTable, this, EffectiveRowID, bUseDifficultySpecificRow, TableData))
	{
		ResolvedPropData = TableData;
	}

	if (ResolvedPropData.PropID.IsNone())
	{
		ResolvedPropData.PropID = EffectiveRowID;
	}
	if (ResolvedPropData.DifficultyID.IsNone())
	{
		ResolvedPropData.DifficultyID = T66GetSelectedVisualPropDifficultyID(this);
	}
}

void AT66WorldVisualProp::ApplyVisualData()
{
	if (!VisualMesh)
	{
		return;
	}

	const FT66WorldVisualPropData& Data = ResolvedPropData;
	UStaticMesh* Mesh = Data.DisplayMesh.IsNull() ? nullptr : Data.DisplayMesh.LoadSynchronous();
	if (!Mesh)
	{
		Mesh = T66ResolveFallbackVisualPropMesh(Data.PropID);
	}

	if (Mesh)
	{
		VisualMesh->SetStaticMesh(Mesh);
	}

	VisualMesh->SetRelativeScale3D(T66ResolvePropScale(Data.DisplayMeshScale));
	VisualMesh->SetRelativeRotation(Data.DisplayMeshRotation);
	VisualMesh->SetRelativeLocation(Data.DisplayMeshOffset);

	FT66VisualUtil::ApplyT66Color(VisualMesh, this, Data.Tint);

	if (Data.bAutoGroundToActorOrigin && Mesh)
	{
		FT66VisualUtil::GroundMeshToActorOrigin(VisualMesh, Mesh);
		VisualMesh->AddLocalOffset(Data.DisplayMeshOffset);
	}

	if (Data.bSnapToGround)
	{
		FT66VisualUtil::SnapToGround(this, GetWorld());
	}
}
