// Copyright Zibra AI Inc 2025-2026. All Rights Reserved.

#pragma once

#include "UObject/ObjectMacros.h"

#include "ZibraLicensingSettings.generated.h"

UENUM()
enum class EZibraLicenseSettingsSaveLocation : uint8
{
	UserSettings = 0,	   // EditorPerProjectUserSettings
	ProjectSettings = 1	   // DefaultEditor
};

UCLASS()
class ZIBRAVDBLICENSING_API UZibraLicensingSettings : public UObject
{
	GENERATED_BODY()

public:
#if WITH_EDITOR
	//~ Begin UObject Interface.
	void PostInitProperties() final;
	void PostEditChangeProperty(FPropertyChangedEvent& e) final;
	//~ End UObject Interface.
#endif

	UPROPERTY(EditAnywhere, Category = "ZibraVDBSettings")
	EZibraLicenseSettingsSaveLocation SaveLocation = EZibraLicenseSettingsSaveLocation::UserSettings;

	UPROPERTY(EditAnywhere, Category = "ZibraVDBSettings")
	FString LicenseKey;

	UPROPERTY(EditAnywhere, Category = "ZibraVDBSettings")
	FString LicenseServerAddress;

	UPROPERTY(EditAnywhere, Category = "ZibraVDBSettings")
	FString OfflineActivation;

#if WITH_EDITOR
private:
	void LoadSettings();
	void SaveSettings();
	void RemoveUnusedConfig();
#endif
};
