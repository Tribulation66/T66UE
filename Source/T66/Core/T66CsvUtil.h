// Copyright Tribulation 66. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Misc/FileHelper.h"
#include "Misc/Paths.h"

// Shared CSV / data-cell parsing helpers used by every per-module DataSubsystem.
// All helpers are inline so this header has no .cpp; including modules just need to depend on T66.
namespace T66CsvUtil
{
	inline FString TrimCell(const FString& InValue)
	{
		FString Result = InValue;
		Result.TrimStartAndEndInline();
		return Result;
	}

	inline TArray<TArray<FString>> ParseCsv(const FString& RawText)
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
				CurrentRow.Add(TrimCell(CurrentCell));
				CurrentCell.Reset();
				continue;
			}

			if (!bInQuotes && Character == TEXT('\n'))
			{
				CurrentRow.Add(TrimCell(CurrentCell));
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
			CurrentRow.Add(TrimCell(CurrentCell));
			Rows.Add(CurrentRow);
		}

		return Rows;
	}

	// Pass a non-null LogContext to emit a Warning to LogTemp when the CSV file fails to load (matches legacy Deck/Idle behavior).
	inline TArray<TMap<FString, FString>> LoadCsvRows(const FString& AbsolutePath, const TCHAR* LogContext = nullptr)
	{
		FString RawText;
		if (!FFileHelper::LoadFileToString(RawText, *AbsolutePath))
		{
			if (LogContext)
			{
				UE_LOG(LogTemp, Warning, TEXT("%s: failed to load '%s'."), LogContext, *AbsolutePath);
			}
			return {};
		}

		const TArray<TArray<FString>> ParsedRows = ParseCsv(RawText);
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

	inline FString GetValue(const TMap<FString, FString>& Row, const TCHAR* Key)
	{
		if (const FString* Found = Row.Find(Key))
		{
			return *Found;
		}
		return FString();
	}

	inline int32 ToInt(const FString& Value, const int32 DefaultValue = 0)
	{
		int32 Parsed = DefaultValue;
		return LexTryParseString(Parsed, *Value) ? Parsed : DefaultValue;
	}

	inline float ToFloat(const FString& Value, const float DefaultValue = 0.f)
	{
		float Parsed = DefaultValue;
		return LexTryParseString(Parsed, *Value) ? Parsed : DefaultValue;
	}

	inline double ToDouble(const FString& Value, const double DefaultValue = 0.0)
	{
		double Parsed = DefaultValue;
		return LexTryParseString(Parsed, *Value) ? Parsed : DefaultValue;
	}

	inline bool ToBool(const FString& Value, const bool bDefaultValue = false)
	{
		if (Value.IsEmpty())
		{
			return bDefaultValue;
		}
		return Value.Equals(TEXT("true"), ESearchCase::IgnoreCase)
			|| Value.Equals(TEXT("1"), ESearchCase::IgnoreCase)
			|| Value.Equals(TEXT("yes"), ESearchCase::IgnoreCase);
	}

	inline FName ToName(const TMap<FString, FString>& Row, const TCHAR* Key)
	{
		return FName(*GetValue(Row, Key));
	}

	inline FLinearColor ToColor(const FString& Value, const FLinearColor& DefaultColor)
	{
		FString Sanitized = Value;
		Sanitized.ReplaceInline(TEXT("("), TEXT(""));
		Sanitized.ReplaceInline(TEXT(")"), TEXT(""));
		FLinearColor ParsedColor;
		return ParsedColor.InitFromString(Sanitized) ? ParsedColor : DefaultColor;
	}

	inline TArray<FName> SplitNameList(const FString& Value)
	{
		TArray<FString> Parts;
		Value.ParseIntoArray(Parts, TEXT("|"), true);

		TArray<FName> Names;
		Names.Reserve(Parts.Num());
		for (const FString& Part : Parts)
		{
			const FString Trimmed = TrimCell(Part);
			if (!Trimmed.IsEmpty())
			{
				Names.Add(FName(*Trimmed));
			}
		}
		return Names;
	}

	// Resolves an absolute path into ProjectContent/<Subdir>/<FileName>.
	inline FString ProjectContentPath(const TCHAR* Subdir, const TCHAR* FileName)
	{
		return FPaths::ConvertRelativePathToFull(FPaths::ProjectContentDir() / Subdir / FileName);
	}
}
