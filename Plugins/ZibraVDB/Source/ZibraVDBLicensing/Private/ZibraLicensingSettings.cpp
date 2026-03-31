// Copyright Zibra AI Inc 2025-2026. All Rights Reserved.

#include "ZibraLicensingSettings.h"

#include "Logging/LogMacros.h"
#include "Misc/AssertionMacros.h"
#include "Misc/ConfigCacheIni.h"
#include "Misc/MessageDialog.h"
#include "UObject/UnrealType.h"
#include "ZibraLicensingManager.h"

#define LOCTEXT_NAMESPACE "ZibraVDBForUnreal"

#if WITH_EDITOR

void UZibraLicensingSettings::PostInitProperties()
{
	Super::PostInitProperties();

	LoadSettings();
}

void UZibraLicensingSettings::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
	FName PropertyName = (PropertyChangedEvent.Property != NULL) ? PropertyChangedEvent.Property->GetFName() : NAME_None;
	FZibraLicensingManager& licensingManager = FZibraLicensingManager::GetInstance();

	SaveSettings();

	if (PropertyName == GET_MEMBER_NAME_CHECKED(UZibraLicensingSettings, OfflineActivation))
	{
		licensingManager.RemoveLicense();

		OfflineActivation.TrimStartAndEndInline();

		if (OfflineActivation == "")
		{
			return;
		}

		licensingManager.SetManualActivationFlag();
		licensingManager.ValidateLicenseOffline(OfflineActivation);
	}
	else if (PropertyName == GET_MEMBER_NAME_CHECKED(UZibraLicensingSettings, LicenseServerAddress))
	{
		licensingManager.RemoveLicense();

		LicenseServerAddress.TrimStartAndEndInline();

		if (LicenseServerAddress == "")
		{
			return;
		}

		licensingManager.SetManualActivationFlag();
		licensingManager.ValidateLicenseLicenseServer(LicenseServerAddress);
	}
	else if (PropertyName == GET_MEMBER_NAME_CHECKED(UZibraLicensingSettings, LicenseKey))
	{
		licensingManager.RemoveLicense();

		LicenseKey.TrimStartAndEndInline();

		if (LicenseKey == "")
		{
			return;
		}

		bool bIsFormatValid = licensingManager.ValidateKeyFormat(LicenseKey);
		if (!bIsFormatValid)
		{
			UE_LOG(LogZibraVDBLicensing, Warning, TEXT("Invalid license key format"));
			static const FText errorText = LOCTEXT("InvalidLicenseKeyFormat", "Invalid license key format");
			FMessageDialog::Open(EAppMsgType::Ok, errorText);
		}
		else
		{
			licensingManager.SetManualActivationFlag();
			licensingManager.ValidateLicenseKey(LicenseKey);
		}
	}
	else if (PropertyName == GET_MEMBER_NAME_CHECKED(UZibraLicensingSettings, SaveLocation))
	{
		RemoveUnusedConfig();
	}
}

void UZibraLicensingSettings::LoadSettings()
{
	{
		FConfigFile TempConfig;
		TempConfig.Read(FPaths::ProjectConfigDir() / TEXT("DefaultEditor.ini"));
		FString SaveLocationString;
		TempConfig.GetString(TEXT("/Script/ZibraVDBLicensing.ZibraLicensingSettings"), TEXT("SaveLocation"), SaveLocationString);
		SaveLocation = SaveLocationString == "ProjectSettings" ? EZibraLicenseSettingsSaveLocation ::ProjectSettings
															   : EZibraLicenseSettingsSaveLocation ::UserSettings;
	}

	switch (SaveLocation)
	{
		case EZibraLicenseSettingsSaveLocation::ProjectSettings:
		{
			FConfigFile TempConfig;
			TempConfig.Read(FPaths::ProjectConfigDir() / TEXT("DefaultEditor.ini"));
			TempConfig.GetString(TEXT("/Script/ZibraVDBLicensing.ZibraLicensingSettings"), TEXT("LicenseKey"), LicenseKey);
			TempConfig.GetString(
				TEXT("/Script/ZibraVDBLicensing.ZibraLicensingSettings"), TEXT("LicenseServerAddress"), LicenseServerAddress);
			TempConfig.GetString(
				TEXT("/Script/ZibraVDBLicensing.ZibraLicensingSettings"), TEXT("OfflineActivation"), OfflineActivation);
		}
		break;
		default:
			checkNoEntry();
			[[fallthrough]];
		case EZibraLicenseSettingsSaveLocation::UserSettings:
		{
			FConfigFile* ConfigFile = GConfig->Find(GEditorPerProjectIni);
			if (ConfigFile == nullptr)
			{
				checkNoEntry();
				return;
			}

			ConfigFile->GetString(TEXT("/Script/ZibraVDBLicensing.ZibraLicensingSettings"), TEXT("LicenseKey"), LicenseKey);
			ConfigFile->GetString(
				TEXT("/Script/ZibraVDBLicensing.ZibraLicensingSettings"), TEXT("LicenseServerAddress"), LicenseServerAddress);
			ConfigFile->GetString(
				TEXT("/Script/ZibraVDBLicensing.ZibraLicensingSettings"), TEXT("OfflineActivation"), OfflineActivation);
		}
		break;
	}
}

void UZibraLicensingSettings::SaveSettings()
{
	switch (SaveLocation)
	{
		case EZibraLicenseSettingsSaveLocation::ProjectSettings:
		{
			FConfigFile TempConfig;
			TempConfig.Read(FPaths::ProjectConfigDir() / TEXT("DefaultEditor.ini"));
			TempConfig.SetString(
				TEXT("/Script/ZibraVDBLicensing.ZibraLicensingSettings"), TEXT("SaveLocation"), TEXT("ProjectSettings"));
			TempConfig.SetString(TEXT("/Script/ZibraVDBLicensing.ZibraLicensingSettings"), TEXT("LicenseKey"), *LicenseKey);
			TempConfig.SetString(
				TEXT("/Script/ZibraVDBLicensing.ZibraLicensingSettings"), TEXT("LicenseServerAddress"), *LicenseServerAddress);
			TempConfig.SetString(
				TEXT("/Script/ZibraVDBLicensing.ZibraLicensingSettings"), TEXT("OfflineActivation"), *OfflineActivation);
			TempConfig.Write(FPaths::ProjectConfigDir() / TEXT("DefaultEditor.ini"));
		}
		break;
		default:
			checkNoEntry();
			[[fallthrough]];
		case EZibraLicenseSettingsSaveLocation::UserSettings:
		{
			FConfigFile* ConfigFile = GConfig->Find(GEditorPerProjectIni);
			if (ConfigFile == nullptr)
			{
				checkNoEntry();
				return;
			}

			ConfigFile->SetString(TEXT("/Script/ZibraVDBLicensing.ZibraLicensingSettings"), TEXT("LicenseKey"), *LicenseKey);
			ConfigFile->SetString(
				TEXT("/Script/ZibraVDBLicensing.ZibraLicensingSettings"), TEXT("LicenseServerAddress"), *LicenseServerAddress);
			ConfigFile->SetString(
				TEXT("/Script/ZibraVDBLicensing.ZibraLicensingSettings"), TEXT("OfflineActivation"), *OfflineActivation);
			GConfig->Flush(false, GEditorPerProjectIni);
		}
		break;
	}
}

void UZibraLicensingSettings::RemoveUnusedConfig()
{
	// This is a new value
	// We need to cleanup config that is NOT SaveLocation
	switch (SaveLocation)
	{
		case EZibraLicenseSettingsSaveLocation::ProjectSettings:
		{
			FConfigFile* ConfigFile = GConfig->Find(GEditorPerProjectIni);
			if (ConfigFile == nullptr)
			{
				checkNoEntry();
				return;
			}

			ConfigFile->Remove(TEXT("/Script/ZibraVDBLicensing.ZibraLicensingSettings"));
			ConfigFile->Dirty = true;

			GConfig->Flush(false, GEditorPerProjectIni);
		}
		break;
		default:
			checkNoEntry();
			[[fallthrough]];
		case EZibraLicenseSettingsSaveLocation::UserSettings:
		{
			FConfigFile TempConfig;
			TempConfig.Read(FPaths::ProjectConfigDir() / TEXT("DefaultEditor.ini"));
			TempConfig.Remove(TEXT("/Script/ZibraVDBLicensing.ZibraLicensingSettings"));
			TempConfig.Dirty = true;
			TempConfig.Write(FPaths::ProjectConfigDir() / TEXT("DefaultEditor.ini"));
		}
		break;
	}
}

#endif

#undef LOCTEXT_NAMESPACE
