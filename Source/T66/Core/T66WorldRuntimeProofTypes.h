// Copyright Tribulation 66. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"

struct FT66WorldRuntimeDebugSnapshot
{
	FString SystemName;
	TMap<FString, int64> Counters;
	TMap<FString, bool> Flags;
	TMap<FString, FString> Evidence;
	TArray<FString> MeasurementGaps;
	TArray<FString> Notes;

	void AddCounter(const TCHAR* Key, const int64 Value)
	{
		Counters.Add(Key ? FString(Key) : FString(), Value);
	}

	void AddFlag(const TCHAR* Key, const bool bValue)
	{
		Flags.Add(Key ? FString(Key) : FString(), bValue);
	}

	void AddEvidence(const TCHAR* Key, const FString& Value)
	{
		Evidence.Add(Key ? FString(Key) : FString(), Value);
	}

	void AddMeasurementGap(const FString& Value)
	{
		if (!Value.IsEmpty())
		{
			MeasurementGaps.Add(Value);
		}
	}

	void AddNote(const FString& Value)
	{
		if (!Value.IsEmpty())
		{
			Notes.Add(Value);
		}
	}
};
