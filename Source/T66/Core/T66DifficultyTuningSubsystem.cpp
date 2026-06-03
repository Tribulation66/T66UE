// Copyright Tribulation 66. All Rights Reserved.

#include "Core/T66DifficultyTuningSubsystem.h"

#include "Engine/AssetManager.h"
#include "Engine/StreamableManager.h"

DEFINE_LOG_CATEGORY_STATIC(LogT66DifficultyTuning, Log, All);

namespace
{
	static constexpr const TCHAR* T66DifficultyTuningDataTablePath = TEXT("/Game/Data/DT_DifficultyTuning.DT_DifficultyTuning");

	static bool LoadDifficultyRow(const UDataTable* DataTable, const FName RowName, FT66DifficultyTuningRow& OutTuning)
	{
		if (!DataTable)
		{
			return false;
		}

		if (const FT66DifficultyTuningRow* Row = DataTable->FindRow<FT66DifficultyTuningRow>(RowName, TEXT("DifficultyTuningDataTable")))
		{
			OutTuning = *Row;
			return true;
		}

		UE_LOG(LogT66DifficultyTuning, Error, TEXT("DifficultyTuning DataTable '%s' is missing row '%s'."), *DataTable->GetPathName(), *RowName.ToString());
		return false;
	}

	static int32 ClampStartStage(const FT66DifficultyTuningRow& Tuning)
	{
		return FMath::Clamp(Tuning.StartStage, 1, 20);
	}
}

FT66DifficultyTuningTable::FT66DifficultyTuningTable()
{
	Easy.StartStage = 1;
	Easy.EndStage = 4;
	Easy.bUsesFinalSequence = false;
	Easy.SkullColorBandSize = 4;

	Medium.StartStage = 5;
	Medium.EndStage = 8;
	Medium.bUsesFinalSequence = false;
	Medium.SkullColorBandSize = 4;

	Hard.StartStage = 9;
	Hard.EndStage = 12;
	Hard.bUsesFinalSequence = false;
	Hard.SkullColorBandSize = 4;

	VeryHard.StartStage = 13;
	VeryHard.EndStage = 16;
	VeryHard.bUsesFinalSequence = false;
	VeryHard.SkullColorBandSize = 4;

	Impossible.StartStage = 17;
	Impossible.EndStage = 20;
	Impossible.bUsesFinalSequence = true;
	Impossible.SkullColorBandSize = 4;
}

bool FT66DifficultyTuningTable::LoadFromDataTable(const UDataTable* DataTable)
{
	bool bLoadedAllRows = true;
	bLoadedAllRows &= LoadDifficultyRow(DataTable, TEXT("Easy"), Easy);
	bLoadedAllRows &= LoadDifficultyRow(DataTable, TEXT("Medium"), Medium);
	bLoadedAllRows &= LoadDifficultyRow(DataTable, TEXT("Hard"), Hard);
	bLoadedAllRows &= LoadDifficultyRow(DataTable, TEXT("VeryHard"), VeryHard);
	bLoadedAllRows &= LoadDifficultyRow(DataTable, TEXT("Impossible"), Impossible);
	return bLoadedAllRows;
}

const FT66DifficultyTuningRow& FT66DifficultyTuningTable::Get(const ET66Difficulty Difficulty) const
{
	switch (Difficulty)
	{
	case ET66Difficulty::Easy: return Easy;
	case ET66Difficulty::Medium: return Medium;
	case ET66Difficulty::Hard: return Hard;
	case ET66Difficulty::VeryHard: return VeryHard;
	case ET66Difficulty::Impossible: return Impossible;
	default: return Easy;
	}
}

void UT66DifficultyTuningSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);
	QueueTuningDataTableLoad();
}

void UT66DifficultyTuningSubsystem::QueueTuningDataTableLoad()
{
	const FSoftObjectPath TuningTablePath(T66DifficultyTuningDataTablePath);
	if (UDataTable* ResidentTable = Cast<UDataTable>(TuningTablePath.ResolveObject()))
	{
		if (ResidentTable->GetRowStruct() != FT66DifficultyTuningRow::StaticStruct())
		{
			UE_LOG(LogT66DifficultyTuning, Error, TEXT("DifficultyTuning DataTable '%s' has row struct '%s', expected '%s'."),
				*ResidentTable->GetPathName(),
				ResidentTable->GetRowStruct() ? *ResidentTable->GetRowStruct()->GetName() : TEXT("<null>"),
				*FT66DifficultyTuningRow::StaticStruct()->GetName());
			return;
		}

		bTuningLoaded = CachedTuning.LoadFromDataTable(ResidentTable);
		return;
	}

	TuningDataTableLoadHandle = UAssetManager::GetStreamableManager().RequestAsyncLoad(
		TArray<FSoftObjectPath>{ TuningTablePath },
		FStreamableDelegate::CreateUObject(this, &UT66DifficultyTuningSubsystem::HandleTuningDataTableLoaded));
	if (!TuningDataTableLoadHandle.IsValid())
	{
		UE_LOG(LogT66DifficultyTuning, Warning, TEXT("Failed to queue async load for difficulty tuning DataTable at '%s'; using built-in defaults."), T66DifficultyTuningDataTablePath);
	}
}

void UT66DifficultyTuningSubsystem::HandleTuningDataTableLoaded()
{
	TuningDataTableLoadHandle.Reset();

	const FSoftObjectPath TuningTablePath(T66DifficultyTuningDataTablePath);
	UDataTable* DifficultyTuningDataTable = Cast<UDataTable>(TuningTablePath.ResolveObject());
	if (!DifficultyTuningDataTable)
	{
		UE_LOG(LogT66DifficultyTuning, Warning, TEXT("Failed to resolve difficulty tuning DataTable after async load at '%s'; using built-in defaults."), T66DifficultyTuningDataTablePath);
		return;
	}

	if (DifficultyTuningDataTable->GetRowStruct() != FT66DifficultyTuningRow::StaticStruct())
	{
		UE_LOG(LogT66DifficultyTuning, Error, TEXT("DifficultyTuning DataTable '%s' has row struct '%s', expected '%s'."),
			*DifficultyTuningDataTable->GetPathName(),
			DifficultyTuningDataTable->GetRowStruct() ? *DifficultyTuningDataTable->GetRowStruct()->GetName() : TEXT("<null>"),
			*FT66DifficultyTuningRow::StaticStruct()->GetName());
		return;
	}

	bTuningLoaded = CachedTuning.LoadFromDataTable(DifficultyTuningDataTable);
}

bool UT66DifficultyTuningSubsystem::IsTuningReady(const TCHAR* Caller) const
{
	if (bTuningLoaded)
	{
		return true;
	}

	if (!bWarnedTuningUnavailable)
	{
		bWarnedTuningUnavailable = true;
		UE_LOG(LogT66DifficultyTuning, Warning, TEXT("Difficulty tuning requested by %s before DataTable '%s' was available; using built-in defaults."), Caller ? Caller : TEXT("<unknown>"), T66DifficultyTuningDataTablePath);
	}
	return false;
}

const FT66DifficultyTuningRow& UT66DifficultyTuningSubsystem::GetDifficultyTuning(const ET66Difficulty Difficulty) const
{
	IsTuningReady(TEXT("GetDifficultyTuning"));
	return CachedTuning.Get(Difficulty);
}

int32 UT66DifficultyTuningSubsystem::GetDifficultyStartStage(const ET66Difficulty Difficulty) const
{
	return ClampStartStage(GetDifficultyTuning(Difficulty));
}

int32 UT66DifficultyTuningSubsystem::GetDifficultyEndStage(const ET66Difficulty Difficulty) const
{
	const FT66DifficultyTuningRow& Tuning = GetDifficultyTuning(Difficulty);
	const int32 StartStage = ClampStartStage(Tuning);
	return FMath::Clamp(Tuning.EndStage, StartStage, 20);
}

int32 UT66DifficultyTuningSubsystem::GetDifficultyLocalStage(const ET66Difficulty Difficulty, const int32 AbsoluteStage) const
{
	return FMath::Clamp(AbsoluteStage - GetDifficultyStartStage(Difficulty) + 1, 1, 4);
}

ET66ItemRarity UT66DifficultyTuningSubsystem::GetLocalStageIdolRarity(const int32 LocalStageNumber) const
{
	switch (FMath::Clamp(LocalStageNumber, 1, 4))
	{
	case 1:  return ET66ItemRarity::Black;
	case 2:  return ET66ItemRarity::Red;
	case 3:  return ET66ItemRarity::Yellow;
	case 4:
	default: return ET66ItemRarity::White;
	}
}

ET66WeaponRarity UT66DifficultyTuningSubsystem::GetLocalStageWeaponRarity(const int32 LocalStageNumber) const
{
	switch (FMath::Clamp(LocalStageNumber, 1, 4))
	{
	case 1:  return ET66WeaponRarity::Black;
	case 2:  return ET66WeaponRarity::Red;
	case 3:  return ET66WeaponRarity::Yellow;
	case 4:
	default: return ET66WeaponRarity::White;
	}
}

bool UT66DifficultyTuningSubsystem::DoesDifficultyUseFinalSequence(const ET66Difficulty Difficulty) const
{
	return GetDifficultyTuning(Difficulty).bUsesFinalSequence;
}

int32 UT66DifficultyTuningSubsystem::GetDifficultySkullColorBandSize(const ET66Difficulty Difficulty) const
{
	return FMath::Max(1, GetDifficultyTuning(Difficulty).SkullColorBandSize);
}
