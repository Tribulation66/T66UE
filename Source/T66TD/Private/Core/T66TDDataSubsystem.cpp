// Copyright Tribulation 66. All Rights Reserved.

#include "Core/T66TDDataSubsystem.h"

#include "Dom/JsonObject.h"
#include "Serialization/JsonReader.h"
#include "Serialization/JsonSerializer.h"
#include "Misc/FileHelper.h"
#include "Misc/Paths.h"

namespace
{
	FString T66TDTrimCell(const FString& InValue)
	{
		FString Result = InValue;
		Result.TrimStartAndEndInline();
		return Result;
	}

	TArray<TArray<FString>> T66TDParseCsv(const FString& RawText)
	{
		FString Text = RawText;
		Text.ReplaceInline(TEXT("\r\n"), TEXT("\n"));
		Text.ReplaceInline(TEXT("\r"), TEXT("\n"));

		TArray<TArray<FString>> Rows;
		TArray<FString> CurrentRow;
		FString CurrentCell;
		bool bInQuotes = false;

		for (int32 Index = 0; Index < Text.Len(); ++Index)
		{
			const TCHAR Character = Text[Index];
			if (Character == TEXT('"'))
			{
				const bool bEscapedQuote = bInQuotes && Index + 1 < Text.Len() && Text[Index + 1] == TEXT('"');
				if (bEscapedQuote)
				{
					CurrentCell.AppendChar(TEXT('"'));
					++Index;
				}
				else
				{
					bInQuotes = !bInQuotes;
				}
				continue;
			}

			if (!bInQuotes && Character == TEXT(','))
			{
				CurrentRow.Add(T66TDTrimCell(CurrentCell));
				CurrentCell.Reset();
				continue;
			}

			if (!bInQuotes && Character == TEXT('\n'))
			{
				CurrentRow.Add(T66TDTrimCell(CurrentCell));
				CurrentCell.Reset();

				bool bHasValue = false;
				for (const FString& Cell : CurrentRow)
				{
					if (!Cell.IsEmpty())
					{
						bHasValue = true;
						break;
					}
				}

				if (bHasValue)
				{
					Rows.Add(CurrentRow);
				}

				CurrentRow.Reset();
				continue;
			}

			CurrentCell.AppendChar(Character);
		}

		if (!CurrentCell.IsEmpty() || CurrentRow.Num() > 0)
		{
			CurrentRow.Add(T66TDTrimCell(CurrentCell));
			Rows.Add(CurrentRow);
		}

		return Rows;
	}

	TArray<TMap<FString, FString>> T66TDLoadCsvRows(const FString& AbsolutePath)
	{
		FString RawText;
		if (!FFileHelper::LoadFileToString(RawText, *AbsolutePath))
		{
			return {};
		}

		const TArray<TArray<FString>> ParsedRows = T66TDParseCsv(RawText);
		if (ParsedRows.Num() < 2)
		{
			return {};
		}

		const TArray<FString>& Headers = ParsedRows[0];
		TArray<TMap<FString, FString>> OutputRows;
		for (int32 RowIndex = 1; RowIndex < ParsedRows.Num(); ++RowIndex)
		{
			const TArray<FString>& Row = ParsedRows[RowIndex];
			TMap<FString, FString> RowMap;
			for (int32 ColumnIndex = 0; ColumnIndex < Headers.Num(); ++ColumnIndex)
			{
				RowMap.Add(Headers[ColumnIndex], Row.IsValidIndex(ColumnIndex) ? Row[ColumnIndex] : FString());
			}
			OutputRows.Add(MoveTemp(RowMap));
		}

		return OutputRows;
	}

	FString T66TDGetValue(const TMap<FString, FString>& Row, const TCHAR* Key)
	{
		if (const FString* Found = Row.Find(Key))
		{
			return *Found;
		}
		return FString();
	}

	float T66TDToFloat(const FString& Value, const float DefaultValue = 0.f)
	{
		float Parsed = DefaultValue;
		return LexTryParseString(Parsed, *Value) ? Parsed : DefaultValue;
	}

	int32 T66TDToInt(const FString& Value, const int32 DefaultValue = 0)
	{
		int32 Parsed = DefaultValue;
		return LexTryParseString(Parsed, *Value) ? Parsed : DefaultValue;
	}

	FLinearColor T66TDToColor(const FString& Value, const FLinearColor& DefaultColor)
	{
		FString Sanitized = Value;
		Sanitized.ReplaceInline(TEXT("("), TEXT(""));
		Sanitized.ReplaceInline(TEXT(")"), TEXT(""));
		FLinearColor ParsedColor;
		return ParsedColor.InitFromString(Sanitized) ? ParsedColor : DefaultColor;
	}

	TArray<FName> T66TDSplitNameList(const FString& Value)
	{
		TArray<FString> Parts;
		Value.ParseIntoArray(Parts, TEXT("|"), true);

		TArray<FName> Names;
		Names.Reserve(Parts.Num());
		for (const FString& Part : Parts)
		{
			const FString Trimmed = T66TDTrimCell(Part);
			if (!Trimmed.IsEmpty())
			{
				Names.Add(FName(*Trimmed));
			}
		}

		return Names;
	}

	FString T66TDDataPath(const TCHAR* FileName)
	{
		return FPaths::ConvertRelativePathToFull(FPaths::ProjectContentDir() / TEXT("TD/Data") / FileName);
	}
}

void UT66TDDataSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);
	ReloadData();
}

void UT66TDDataSubsystem::ReloadData()
{
	LoadHeroes();
	LoadDifficulties();
	LoadMaps();
	LoadLayouts();
}

const FT66TDHeroDefinition* UT66TDDataSubsystem::FindHero(const FName HeroID) const
{
	return Heroes.FindByPredicate([HeroID](const FT66TDHeroDefinition& Definition)
	{
		return Definition.HeroID == HeroID;
	});
}

const FT66TDDifficultyDefinition* UT66TDDataSubsystem::FindDifficulty(const FName DifficultyID) const
{
	return Difficulties.FindByPredicate([DifficultyID](const FT66TDDifficultyDefinition& Definition)
	{
		return Definition.DifficultyID == DifficultyID;
	});
}

const FT66TDMapDefinition* UT66TDDataSubsystem::FindMap(const FName MapID) const
{
	return Maps.FindByPredicate([MapID](const FT66TDMapDefinition& Definition)
	{
		return Definition.MapID == MapID;
	});
}

const FT66TDMapLayoutDefinition* UT66TDDataSubsystem::FindLayout(const FName MapID) const
{
	return Layouts.Find(MapID);
}

TArray<const FT66TDMapDefinition*> UT66TDDataSubsystem::GetMapsForDifficulty(const FName DifficultyID) const
{
	TArray<const FT66TDMapDefinition*> MatchingMaps;
	for (const FT66TDMapDefinition& Map : Maps)
	{
		if (Map.DifficultyID == DifficultyID)
		{
			MatchingMaps.Add(&Map);
		}
	}
	return MatchingMaps;
}

void UT66TDDataSubsystem::LoadHeroes()
{
	Heroes.Reset();

	for (const TMap<FString, FString>& Row : T66TDLoadCsvRows(T66TDDataPath(TEXT("T66TD_Heroes.csv"))))
	{
		FT66TDHeroDefinition Definition;
		Definition.HeroID = FName(*T66TDGetValue(Row, TEXT("HeroID")));
		Definition.DisplayName = T66TDGetValue(Row, TEXT("DisplayName"));
		Definition.Description = T66TDGetValue(Row, TEXT("Description"));
		Definition.RoleLabel = T66TDGetValue(Row, TEXT("RoleLabel"));
		Definition.Signature = T66TDGetValue(Row, TEXT("Signature"));
		Definition.UltimateLabel = T66TDGetValue(Row, TEXT("UltimateLabel"));
		Definition.PassiveLabel = T66TDGetValue(Row, TEXT("PassiveLabel"));
		Definition.PlaceholderColor = T66TDToColor(T66TDGetValue(Row, TEXT("PlaceholderColor")), FLinearColor(0.26f, 0.30f, 0.36f, 1.0f));

		if (Definition.HeroID != NAME_None)
		{
			Heroes.Add(MoveTemp(Definition));
		}
	}
}

void UT66TDDataSubsystem::LoadDifficulties()
{
	Difficulties.Reset();

	for (const TMap<FString, FString>& Row : T66TDLoadCsvRows(T66TDDataPath(TEXT("T66TD_Difficulties.csv"))))
	{
		FT66TDDifficultyDefinition Definition;
		Definition.DifficultyID = FName(*T66TDGetValue(Row, TEXT("DifficultyID")));
		Definition.DisplayName = T66TDGetValue(Row, TEXT("DisplayName"));
		Definition.Description = T66TDGetValue(Row, TEXT("Description"));
		Definition.AccentColor = T66TDToColor(T66TDGetValue(Row, TEXT("AccentColor")), FLinearColor(0.32f, 0.44f, 0.68f, 1.0f));
		Definition.MapCount = T66TDToInt(T66TDGetValue(Row, TEXT("MapCount")), 4);
		Definition.EnemyHealthScalar = T66TDToFloat(T66TDGetValue(Row, TEXT("EnemyHealthScalar")), 1.0f);
		Definition.EnemySpeedScalar = T66TDToFloat(T66TDGetValue(Row, TEXT("EnemySpeedScalar")), 1.0f);
		Definition.BossScalar = T66TDToFloat(T66TDGetValue(Row, TEXT("BossScalar")), 1.0f);
		Definition.RewardScalar = T66TDToFloat(T66TDGetValue(Row, TEXT("RewardScalar")), 1.0f);

		if (Definition.DifficultyID != NAME_None)
		{
			Difficulties.Add(MoveTemp(Definition));
		}
	}
}

void UT66TDDataSubsystem::LoadMaps()
{
	Maps.Reset();

	for (const TMap<FString, FString>& Row : T66TDLoadCsvRows(T66TDDataPath(TEXT("T66TD_Maps.csv"))))
	{
		FT66TDMapDefinition Definition;
		Definition.MapID = FName(*T66TDGetValue(Row, TEXT("MapID")));
		Definition.DifficultyID = FName(*T66TDGetValue(Row, TEXT("DifficultyID")));
		Definition.DisplayName = T66TDGetValue(Row, TEXT("DisplayName"));
		Definition.ThemeLabel = T66TDGetValue(Row, TEXT("ThemeLabel"));
		Definition.Description = T66TDGetValue(Row, TEXT("Description"));
		Definition.BackgroundRelativePath = T66TDGetValue(Row, TEXT("BackgroundRelativePath"));
		Definition.LaneCount = T66TDToInt(T66TDGetValue(Row, TEXT("LaneCount")), 1);
		Definition.BuildPadCount = T66TDToInt(T66TDGetValue(Row, TEXT("BuildPadCount")), 12);
		Definition.BossWave = T66TDToInt(T66TDGetValue(Row, TEXT("BossWave")), 15);
		Definition.EnemyNotes = T66TDGetValue(Row, TEXT("EnemyNotes"));
		Definition.FeaturedHeroIDs = T66TDSplitNameList(T66TDGetValue(Row, TEXT("FeaturedHeroIDs")));

		if (Definition.MapID != NAME_None && Definition.DifficultyID != NAME_None)
		{
			Maps.Add(MoveTemp(Definition));
		}
	}
}

void UT66TDDataSubsystem::LoadLayouts()
{
	Layouts.Reset();

	FString RawText;
	if (!FFileHelper::LoadFileToString(RawText, *T66TDDataPath(TEXT("T66TD_Layouts.json"))))
	{
		return;
	}

	TSharedPtr<FJsonObject> RootObject;
	const TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory<>::Create(RawText);
	if (!FJsonSerializer::Deserialize(Reader, RootObject) || !RootObject.IsValid())
	{
		return;
	}

	for (const TPair<FString, TSharedPtr<FJsonValue>>& Pair : RootObject->Values)
	{
		const TSharedPtr<FJsonObject>* LayoutObject = nullptr;
		if (!Pair.Value.IsValid() || !Pair.Value->TryGetObject(LayoutObject) || !LayoutObject || !LayoutObject->IsValid())
		{
			continue;
		}

		FT66TDMapLayoutDefinition Layout;
		Layout.MapID = FName(*Pair.Key);

		const TArray<TSharedPtr<FJsonValue>>* PathsArray = nullptr;
		if ((*LayoutObject)->TryGetArrayField(TEXT("paths"), PathsArray) && PathsArray)
		{
			for (const TSharedPtr<FJsonValue>& PathValue : *PathsArray)
			{
				const TArray<TSharedPtr<FJsonValue>>* PathPointsArray = nullptr;
				if (!PathValue.IsValid() || !PathValue->TryGetArray(PathPointsArray) || !PathPointsArray)
				{
					continue;
				}

				TArray<FVector2D> PathPoints;
				for (const TSharedPtr<FJsonValue>& PointValue : *PathPointsArray)
				{
					const TSharedPtr<FJsonObject>* PointObject = nullptr;
					if (!PointValue.IsValid() || !PointValue->TryGetObject(PointObject) || !PointObject || !PointObject->IsValid())
					{
						continue;
					}

					PathPoints.Add(FVector2D(
						(*PointObject)->GetNumberField(TEXT("x")),
						(*PointObject)->GetNumberField(TEXT("y"))));
				}

				if (PathPoints.Num() >= 2)
				{
					Layout.Paths.Add(MoveTemp(PathPoints));
				}
			}
		}

		const TArray<TSharedPtr<FJsonValue>>* PadsArray = nullptr;
		if ((*LayoutObject)->TryGetArrayField(TEXT("pads"), PadsArray) && PadsArray)
		{
			for (const TSharedPtr<FJsonValue>& PadValue : *PadsArray)
			{
				const TSharedPtr<FJsonObject>* PadObject = nullptr;
				if (!PadValue.IsValid() || !PadValue->TryGetObject(PadObject) || !PadObject || !PadObject->IsValid())
				{
					continue;
				}

				FT66TDMapPadDefinition Pad;
				Pad.PositionNormalized = FVector2D(
					(*PadObject)->GetNumberField(TEXT("x")),
					(*PadObject)->GetNumberField(TEXT("y")));
				Pad.RadiusNormalized = (*PadObject)->GetNumberField(TEXT("radius"));
				Layout.Pads.Add(Pad);
			}
		}

		if (Layout.MapID != NAME_None && Layout.Paths.Num() > 0 && Layout.Pads.Num() > 0)
		{
			Layouts.Add(Layout.MapID, MoveTemp(Layout));
		}
	}
}
