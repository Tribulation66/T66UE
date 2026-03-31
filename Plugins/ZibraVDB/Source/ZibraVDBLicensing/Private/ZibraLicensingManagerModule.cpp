// Copyright Zibra AI Inc 2025-2026. All Rights Reserved.

#include "Modules/ModuleManager.h"
#include "UObject/Class.h"
#include "ZibraLicensingManager.h"
#include "ZibraLicensingSettings.h"

#if WITH_EDITOR
#include "ISettingsModule.h"
#endif

#define LOCTEXT_NAMESPACE "ZibraVDBForUnreal"

class FZibraVDBLicensingModule : public IModuleInterface
{
public:
	/** IModuleInterface implementation */
	virtual void StartupModule() noexcept override final;
	virtual void ShutdownModule() noexcept override final;

private:
	void RegisterSettings();
	void UnregisterSettings();
};

void FZibraVDBLicensingModule::StartupModule() noexcept
{
	RegisterSettings();

#if WITH_EDITOR
	FZibraLicensingManager::CreateInstance();
#endif
}

void FZibraVDBLicensingModule::ShutdownModule() noexcept
{
#if WITH_EDITOR
	FZibraLicensingManager::DestroyInstance();
#endif
}

void FZibraVDBLicensingModule::RegisterSettings()
{
#if WITH_EDITOR
	ISettingsModule* SettingsModule = FModuleManager::GetModulePtr<ISettingsModule>("Settings");
	if (SettingsModule)
	{
		SettingsModule->RegisterSettings("Project", "Plugins", "ZibraVDB Licensing",
			LOCTEXT("ZibraVDBLicenseSettingsName", "ZibraVDB License"),
			LOCTEXT("ZibraVDBLicenseSettingsDescription", "Set ZibraVDB License Key"),
			GetMutableDefault<UZibraLicensingSettings>());
	}
#endif
}

void FZibraVDBLicensingModule::UnregisterSettings()
{
#if WITH_EDITOR
	ISettingsModule* SettingsModule = FModuleManager::GetModulePtr<ISettingsModule>("Settings");
	if (SettingsModule)
	{
		SettingsModule->UnregisterSettings("Project", "Plugins", "ZibraVDB Licensing");
	}
#endif
}

IMPLEMENT_MODULE(FZibraVDBLicensingModule, ZibraVDBLicensing)

#undef LOCTEXT_NAMESPACE
