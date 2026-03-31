// Copyright Zibra AI Inc 2025-2026. All Rights Reserved.

#pragma once

#include "Containers/Ticker.h"
#include "Modules/ModuleManager.h"
#include "PropertyEditorModule.h"
#include "IAssetTypeActions.h"
#include "Slate/SlateGameResources.h"

DECLARE_LOG_CATEGORY_EXTERN(LogZibraVDBEditor, Log, All);

class FZibraVDBEditorModule : public IModuleInterface
{
public:
	/** IModuleInterface implementation */
	virtual void StartupModule() noexcept override final;
	virtual void ShutdownModule() noexcept override final;

	static void OpenImportZibraVDBFileWindow();
	static void OpenImportVDBSequenceWindow();
	static void OpenBugReporterWindow();

private:
	void RegisterMenus();
	void RegisterSettings();
	void UnregisterSettings();

	void RegisterInterfaceCustomizations();
	void UnregisterInterfaceCustomizations();

	void RegisterAssetTypeActions();
	void UnregisterAssetTypeActions();

	void RegisterSequencerTrackEditors();
	void UnregisterSequencerTrackEditors();

	TSharedRef<class SDockTab> OpenImportZibraVDBFileTab(const class FSpawnTabArgs& SpawnTabArgs);
	TSharedRef<class SDockTab> OpenImportVDBSequenceWindowTab(const class FSpawnTabArgs& SpawnTabArgs);
	TSharedRef<class SDockTab> OpenBugReporterWindowTab(const class FSpawnTabArgs& SpawnTabArgs);

	void DisplayConfigurationNotification();
	void RunAutomaticUpdateCheck();

	void CopyDiagnosticsInfo();
	void CheckForUpdates();
	void GoToDocumentation();

	TSharedPtr<class FUICommandList> PluginCommands;
	TSharedPtr<IAssetTypeActions> ZibraVDBAssetActions;
	TSharedPtr<FSlateGameResources> ZibraStyleSet;
	FSlateBrush ZibraIconBrush;
	FDelegateHandle ZibraVDBTrackEditorCreateHandle;
};
