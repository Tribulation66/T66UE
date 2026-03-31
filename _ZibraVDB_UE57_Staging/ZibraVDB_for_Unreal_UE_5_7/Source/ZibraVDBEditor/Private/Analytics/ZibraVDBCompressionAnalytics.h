// Copyright Zibra AI Inc 2025-2026. All Rights Reserved.

#pragma once

#include "Containers/StaticArray.h"
#include "Dom/JsonValue.h"
#include "Misc/Timespan.h"

struct FSequenceInfo;
class UZibraVDBVolume4;
class UZibraVDBSequenceImporter;

class FZibraVDBCompressionAnalytics
{
public:
	static void TrackImport(UZibraVDBVolume4* CompressedVolume, bool IsReimport);
	static void TrackCompression(UZibraVDBVolume4* CompressedVolume, FTimespan CompressionTime, bool IsReimport);

private:
	static TArray<TSharedPtr<FJsonValue>> GetStringArrayAsJSONArray(const TArray<FString>& StringArray);
	static TArray<TSharedPtr<FJsonValue>> GetIntVectorAsJSONArray(const FIntVector& IntVector);
	static TSharedPtr<FJsonObject> GetPerChannelCompressionSettingsAsJSONObject(UZibraVDBVolume4* CompressedVolume);
};
