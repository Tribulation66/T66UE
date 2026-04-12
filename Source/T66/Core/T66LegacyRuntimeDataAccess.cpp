// Copyright Tribulation 66. All Rights Reserved.

#include "Core/T66LegacyRuntimeDataAccess.h"

#include "HAL/FileManager.h"
#include "Misc/FileHelper.h"
#include "Misc/Paths.h"

DEFINE_LOG_CATEGORY(LogT66RuntimeData);

namespace
{
	void PopulateResult(
		const FString& AbsolutePath,
		bool bLoaded,
		int32 LineCount,
		int32 CharacterCount,
		FT66RuntimeCsvLoadResult* OutResult)
	{
		if (!OutResult)
		{
			return;
		}

		OutResult->AbsolutePath = AbsolutePath;
		OutResult->bFileFound = IFileManager::Get().FileExists(*AbsolutePath);
		OutResult->bLoaded = bLoaded;
		OutResult->LineCount = LineCount;
		OutResult->CharacterCount = CharacterCount;
	}
}

namespace T66LegacyRuntimeDataAccess
{
	const TCHAR* ToString(const ET66RuntimeDataSource Source)
	{
		switch (Source)
		{
		case ET66RuntimeDataSource::LooseCsv:
			return TEXT("LooseCsv");
		case ET66RuntimeDataSource::CookedDataTable:
			return TEXT("CookedDataTable");
		default:
			return TEXT("None");
		}
	}

	FString MakeProjectContentPath(const TCHAR* RelativePath)
	{
		return FPaths::ConvertRelativePathToFull(FPaths::ProjectContentDir() / RelativePath);
	}

	bool TryLoadProjectContentCsvString(
		const TCHAR* RelativePath,
		FString& OutCsvContent,
		FT66RuntimeCsvLoadResult* OutResult)
	{
		OutCsvContent.Reset();

		const FString AbsolutePath = MakeProjectContentPath(RelativePath);
		const bool bLoaded = FFileHelper::LoadFileToString(OutCsvContent, *AbsolutePath) && !OutCsvContent.IsEmpty();
		PopulateResult(AbsolutePath, bLoaded, 0, OutCsvContent.Len(), OutResult);
		return bLoaded;
	}

	bool TryLoadProjectContentCsvLines(
		const TCHAR* RelativePath,
		TArray<FString>& OutLines,
		FT66RuntimeCsvLoadResult* OutResult)
	{
		OutLines.Reset();

		const FString AbsolutePath = MakeProjectContentPath(RelativePath);
		const bool bLoaded = FFileHelper::LoadFileToStringArray(OutLines, *AbsolutePath) && OutLines.Num() > 0;
		PopulateResult(AbsolutePath, bLoaded, OutLines.Num(), 0, OutResult);
		return bLoaded;
	}
}
