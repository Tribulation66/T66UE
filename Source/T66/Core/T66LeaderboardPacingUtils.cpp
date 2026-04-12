// Copyright Tribulation 66. All Rights Reserved.

#include "Core/T66LeaderboardPacingUtils.h"

#include "Containers/UnrealString.h"
#include "Misc/DefaultValueHelper.h"

namespace T66LeaderboardPacing
{
	const TCHAR* const StageMarkerPrefix = TEXT("PACE_STAGE|");

	FString MakeStageMarker(const int32 Stage, const int32 Score, const float ElapsedSeconds)
	{
		return FString::Printf(
			TEXT("%sStage=%d|Score=%d|TimeSeconds=%.3f"),
			StageMarkerPrefix,
			FMath::Max(1, Stage),
			FMath::Max(0, Score),
			FMath::Max(0.f, ElapsedSeconds));
	}

	bool IsStageMarker(const FString& Entry)
	{
		return Entry.StartsWith(StageMarkerPrefix, ESearchCase::CaseSensitive);
	}

	bool ParseStageMarker(const FString& Entry, FT66StagePacingPoint& OutPoint)
	{
		if (!IsStageMarker(Entry))
		{
			return false;
		}

		TArray<FString> Segments;
		Entry.ParseIntoArray(Segments, TEXT("|"), true);
		if (Segments.Num() < 4)
		{
			return false;
		}

		int32 ParsedStage = 0;
		int32 ParsedScore = 0;
		float ParsedTimeSeconds = 0.f;
		bool bHasStage = false;
		bool bHasScore = false;
		bool bHasTime = false;

		for (int32 SegmentIndex = 1; SegmentIndex < Segments.Num(); ++SegmentIndex)
		{
			FString Key;
			FString Value;
			if (!Segments[SegmentIndex].Split(TEXT("="), &Key, &Value))
			{
				continue;
			}

			if (Key == TEXT("Stage"))
			{
				bHasStage = LexTryParseString(ParsedStage, *Value);
			}
			else if (Key == TEXT("Score"))
			{
				bHasScore = LexTryParseString(ParsedScore, *Value);
			}
			else if (Key == TEXT("TimeSeconds"))
			{
				bHasTime = FDefaultValueHelper::ParseFloat(Value, ParsedTimeSeconds);
			}
		}

		if (!bHasStage || !bHasScore || !bHasTime)
		{
			return false;
		}

		OutPoint.Stage = FMath::Max(1, ParsedStage);
		OutPoint.Score = FMath::Max(0, ParsedScore);
		OutPoint.ElapsedSeconds = FMath::Max(0.f, ParsedTimeSeconds);
		return true;
	}

	void ExtractStageMarkers(const TArray<FString>& EventLog, TArray<FT66StagePacingPoint>& OutPoints)
	{
		OutPoints.Reset();

		for (const FString& Entry : EventLog)
		{
			FT66StagePacingPoint ParsedPoint;
			if (!ParseStageMarker(Entry, ParsedPoint))
			{
				continue;
			}

			const int32 ExistingIndex = OutPoints.IndexOfByPredicate([&ParsedPoint](const FT66StagePacingPoint& Candidate)
			{
				return Candidate.Stage == ParsedPoint.Stage;
			});

			if (ExistingIndex != INDEX_NONE)
			{
				OutPoints[ExistingIndex] = ParsedPoint;
			}
			else
			{
				OutPoints.Add(ParsedPoint);
			}
		}

		OutPoints.Sort([](const FT66StagePacingPoint& A, const FT66StagePacingPoint& B)
		{
			return A.Stage < B.Stage;
		});
	}
}
