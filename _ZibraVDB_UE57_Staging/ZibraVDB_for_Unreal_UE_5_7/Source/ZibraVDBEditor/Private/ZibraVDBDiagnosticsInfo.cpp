// Copyright Zibra AI Inc 2025-2026. All Rights Reserved.

#include "ZibraVDBDiagnosticsInfo.h"

#include "JsonObjectConverter.h"
#include "Misc/EngineVersion.h"
#include "Windows/WindowsPlatformSurvey.h"
#include "ZibraLicensingManager.h"
#include "ZibraLicensingSettings.h"
#include "ZibraVDBEditor.h"
#include "ZibraVDBInfo.h"
#include "Runtime/Launch/Resources/Version.h"

#if (100 * ENGINE_MAJOR_VERSION + ENGINE_MINOR_VERSION) < 505
#include "Developer/Windows/WindowsTargetPlatform/Classes/WindowsTargetSettings.h"
#else
#include "Developer/Windows/WindowsTargetPlatformSettings/Classes/WindowsTargetSettings.h"
#endif

bool FZibraVDBDiagnosticsInfo::bIsCached = false;
FZibraVDBDiagnosticsInfo FZibraVDBDiagnosticsInfo::Cache = {};

FZibraVDBDiagnosticsInfo FZibraVDBDiagnosticsInfo::CollectInfo()
{
	if (bIsCached)
	{
		return Cache;
	}

	Cache.Version = FZibraVDBInfo::GetVersion();
	Cache.UnrealVersion = FEngineVersion::Current().ToString();

	FHardwareSurveyResults HardwareSurveyResults;
	FPlatformSurvey::GetSurveyResults(HardwareSurveyResults);
	Cache.OS = HardwareSurveyResults.Platform;
	Cache.OSVersion = HardwareSurveyResults.OSVersion;
	Cache.FreeDiskSpaceMB = HardwareSurveyResults.HardDriveFreeMB;
	Cache.PhysicalMemoryMB = HardwareSurveyResults.MemoryMB;
	Cache.GPU = HardwareSurveyResults.RHIAdapter.AdapterName;
	Cache.GPUDriverVersion = HardwareSurveyResults.RHIAdapter.AdapterUserDriverVersion;

	Cache.VRAMMB = HardwareSurveyResults.RHIAdapter.AdapterDedicatedMemoryMB;

	Cache.CurrentRHI = HardwareSurveyResults.RenderingAPI;

	UWindowsTargetSettings* WindowsSettings = UWindowsTargetSettings::StaticClass()->GetDefaultObject<UWindowsTargetSettings>();

	Cache.DefaultRHI = UEnum::GetDisplayValueAsText(WindowsSettings->DefaultGraphicsRHI).ToString();
	Cache.D3D12TargetShaderModels = WindowsSettings->D3D12TargetedShaderFormats;

	UZibraLicensingSettings* LicensingSettings =
		UZibraLicensingSettings::StaticClass()->GetDefaultObject<UZibraLicensingSettings>();
	Cache.IsLicenseKeySaved = !LicensingSettings->LicenseKey.IsEmpty();

	for (size_t i = 0; i < static_cast<size_t>(FZibraLicensingManager::Product::Count); ++i)
	{
		Cache.LicenseStatus[i] = UEnum::GetDisplayValueAsText(FZibraLicensingManager::GetInstance().GetStatus(
			static_cast<FZibraLicensingManager::Product>(i))).ToString();
	}

	bIsCached = true;
	return Cache;
}

FString FZibraVDBDiagnosticsInfo::CollectInfoString()
{
	FZibraVDBDiagnosticsInfo Info = CollectInfo();
	FString InfoJson;
	bool bWasSerializationSuccessful = FJsonObjectConverter::UStructToJsonObjectString(Info, InfoJson);
	if (!bWasSerializationSuccessful)
	{
		UE_LOG(LogZibraVDBEditor, Warning, TEXT("Failed to serialize diagnostics info to JSON."));
		return {};
	}
	return InfoJson;
}
