// Copyright Zibra AI Inc 2025-2026. All Rights Reserved.

#include "ZibraVDBCompressionAnalytics.h"

#include "Dom/JsonObject.h"
#include "Widgets/ZibraVDBSequenceImporterWidget.h"
#include "ZibraVDBAnalyticsManager.h"
#include "ZibraVDBVolume4.h"

void FZibraVDBCompressionAnalytics::TrackImport(UZibraVDBVolume4* CompressedVolume, bool IsReimport)
{
	ensure(CompressedVolume);

	FAnalyticsEvent Event;
	Event.EventType = "Import";
	Event.Data.JsonObject->SetNumberField("FrameNumber", CompressedVolume->GetZibraVDBFile().GetHeader().FrameCount);
	Event.Data.JsonObject->SetNumberField("ChannelNumber", CompressedVolume->GetZibraVDBFile().GetHeader().ChannelNames.Num());
	Event.Data.JsonObject->SetArrayField(
		"AvailableChannels", GetStringArrayAsJSONArray(CompressedVolume->GetZibraVDBFile().GetHeader().ChannelNames));
	Event.Data.JsonObject->SetArrayField("VDBResolution", GetIntVectorAsJSONArray(CompressedVolume->VDBSize));
	Event.Data.JsonObject->SetNumberField("OriginalVDBSize", CompressedVolume->OriginalVDBSize);
	Event.Data.JsonObject->SetNumberField("CompressionRate", CompressedVolume->CompressionRate);
	Event.Data.JsonObject->SetNumberField("Quality", CompressedVolume->Quality);
	Event.Data.JsonObject->SetBoolField("UsePerChannelCompressionSettings", CompressedVolume->bUsePerChannelCompressionSettings);
	Event.Data.JsonObject->SetObjectField(
		"PerChannelCompressionSettings", GetPerChannelCompressionSettingsAsJSONObject(CompressedVolume));
	Event.Data.JsonObject->SetStringField("UEAssetID", CompressedVolume->GetUEAssetID());
	Event.Data.JsonObject->SetStringField("AssetUUID", CompressedVolume->GetCompressedFileUUID());
	Event.Data.JsonObject->SetBoolField("Reimport", IsReimport);

	FZibraVDBAnalyticsManager::SubmitEvent(Event);
}

void FZibraVDBCompressionAnalytics::TrackCompression(UZibraVDBVolume4* CompressedVolume, FTimespan CompressionTime, bool IsReimport)
{
	ensure(CompressedVolume);

	FAnalyticsEvent Event;
	Event.EventType = "Compression";
	Event.Data.JsonObject->SetNumberField("FrameNumber", CompressedVolume->GetZibraVDBFile().GetHeader().FrameCount);
	Event.Data.JsonObject->SetNumberField("ChannelNumber", CompressedVolume->GetZibraVDBFile().GetHeader().ChannelNames.Num());
	Event.Data.JsonObject->SetArrayField(
		"AvailableChannels", GetStringArrayAsJSONArray(CompressedVolume->GetZibraVDBFile().GetHeader().ChannelNames));
	Event.Data.JsonObject->SetArrayField("VDBResolution", GetIntVectorAsJSONArray(CompressedVolume->VDBSize));
	Event.Data.JsonObject->SetNumberField("OriginalVDBSize", CompressedVolume->OriginalVDBSize);
	Event.Data.JsonObject->SetNumberField("CompressionRate", CompressedVolume->CompressionRate);
	Event.Data.JsonObject->SetNumberField("Quality", CompressedVolume->Quality);
	Event.Data.JsonObject->SetBoolField("UsePerChannelCompressionSettings", CompressedVolume->bUsePerChannelCompressionSettings);
	Event.Data.JsonObject->SetObjectField(
		"PerChannelCompressionSettings", GetPerChannelCompressionSettingsAsJSONObject(CompressedVolume));
	Event.Data.JsonObject->SetNumberField("CompressionTime", CompressionTime.GetTotalSeconds());
	Event.Data.JsonObject->SetStringField("UEAssetID", CompressedVolume->GetUEAssetID());
	Event.Data.JsonObject->SetStringField("AssetUUID", CompressedVolume->GetCompressedFileUUID());
	Event.Data.JsonObject->SetBoolField("Reimport", IsReimport);

	FZibraVDBAnalyticsManager::SubmitEvent(Event);
}

TArray<TSharedPtr<FJsonValue>> FZibraVDBCompressionAnalytics::GetStringArrayAsJSONArray(const TArray<FString>& StringArray)
{
	TArray<TSharedPtr<FJsonValue>> ChannelNames;
	ChannelNames.Reserve(StringArray.Num());
	for (int i = 0; i < StringArray.Num(); ++i)
	{
		ChannelNames.Add(MakeShared<FJsonValueString>(StringArray[i]));
	}

	return ChannelNames;
}

TArray<TSharedPtr<FJsonValue>> FZibraVDBCompressionAnalytics::GetIntVectorAsJSONArray(const FIntVector& IntVector)
{
	TArray<TSharedPtr<FJsonValue>> IntVectorArray;
	IntVectorArray.Reserve(3);
	IntVectorArray.Add(MakeShared<FJsonValueNumber>(IntVector.X));
	IntVectorArray.Add(MakeShared<FJsonValueNumber>(IntVector.Y));
	IntVectorArray.Add(MakeShared<FJsonValueNumber>(IntVector.Z));

	return IntVectorArray;
}

TSharedPtr<FJsonObject> FZibraVDBCompressionAnalytics::GetPerChannelCompressionSettingsAsJSONObject(
	UZibraVDBVolume4* CompressedVolume)
{
	TSharedPtr<FJsonObject> PerChannelCompressionSettings = MakeShared<FJsonObject>();
	if (!CompressedVolume->bUsePerChannelCompressionSettings)
	{
		return PerChannelCompressionSettings;
	}

	for (const auto& ChannelCompressionSetting : CompressedVolume->PerChannelCompressionSettings)
	{
		TSharedPtr<FJsonObject> ChannelCompressionSettingObject = MakeShared<FJsonObject>();
		ChannelCompressionSettingObject->SetNumberField("Quality", ChannelCompressionSetting.Quality);

		PerChannelCompressionSettings->SetObjectField(ChannelCompressionSetting.Name, ChannelCompressionSettingObject);
	}

	return PerChannelCompressionSettings;
}
