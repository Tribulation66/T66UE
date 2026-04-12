// Copyright Tribulation 66. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"

DECLARE_LOG_CATEGORY_EXTERN(LogT66RuntimeData, Log, All);

enum class ET66RuntimeDataSource : uint8
{
	None,
	LooseCsv,
	CookedDataTable
};

struct FT66RuntimeCsvLoadResult
{
	FString AbsolutePath;
	bool bFileFound = false;
	bool bLoaded = false;
	int32 LineCount = 0;
	int32 CharacterCount = 0;
};

// Centralizes legacy loose-file runtime data access so it can be audited and
// removed cleanly during the packaged-parity migration.
namespace T66LegacyRuntimeDataAccess
{
	const TCHAR* ToString(ET66RuntimeDataSource Source);

	FString MakeProjectContentPath(const TCHAR* RelativePath);

	bool TryLoadProjectContentCsvString(
		const TCHAR* RelativePath,
		FString& OutCsvContent,
		FT66RuntimeCsvLoadResult* OutResult = nullptr);

	bool TryLoadProjectContentCsvLines(
		const TCHAR* RelativePath,
		TArray<FString>& OutLines,
		FT66RuntimeCsvLoadResult* OutResult = nullptr);
}
