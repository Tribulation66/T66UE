// Copyright Zibra AI Inc 2025-2026. All Rights Reserved.

#include "ZibraVDBEditor.h"

#include "Analytics/ZibraVDBUsageAnalytics.h"
#include "AssetToolsModule.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/Texture2D.h"
#include "Framework/Docking/TabManager.h"
#include "IAssetTools.h"
#include "ISequencerModule.h"
#include "ISettingsModule.h"
#include "ImageUtils.h"
#include "LevelEditor.h"
#include "Misc/MessageDialog.h"
#include "PropertyCustomizationHelpers.h"
#include "Runtime/ApplicationCore/Public/Windows/WindowsPlatformApplicationMisc.h"
#include "Runtime/JsonUtilities/Public/JsonObjectConverter.h"
#include "Sequencer/ZibraVDBTrackEditor.h"
#include "Slate/SlateGameResources.h"
#include "Styling/SlateStyleRegistry.h"
#include "ToolMenus.h"
#include "UObject/UObjectBase.h"
#include "Widgets/Docking/SDockTab.h"
#include "Widgets/ZibraVDBBugReporterWidget.h"
#include "Widgets/ZibraVDBCompressedImporterWidget.h"
#include "Widgets/ZibraVDBSequenceImporterWidget.h"
#include "ZibraEditorSettings.h"
#include "ZibraLicensingManager.h"
#include "ZibraModuleStyle.h"
#include "ZibraVDBAssetDeprecatedEditor.h"
#include "ZibraVDBAssetEditor.h"
#include "ZibraVDBAssetTypeActions.h"
#include "ZibraVDBDiagnosticsInfo.h"
#include "ZibraVDBInfo.h"
#include "ZibraVDBNotifications.h"
#include "ZibraVDBUpdateChecker.h"
#include "ZibraVDBVolumeDeprecated.h"

DEFINE_LOG_CATEGORY(LogZibraVDBEditor);

#define LOCTEXT_NAMESPACE "ZibraVDBForUnreal"

static const FName ZibraVDBImportZibraVDBTabName("Import .zibravdb File");
static const FName ZibraVDBImportVDBSequenceTabName("Import OpenVDB Sequence");
static const FName ZibraVDBBugReporterTabName("Bug Reporter");

static const TCHAR* DocumentationLink = TEXT("https://zibra.notion.site/ZibraVDB-Documentation-4cb92e96487f4f31893469eb407ba097");

void FZibraVDBEditorModule::StartupModule() noexcept
{
	FZibraModuleStyle::Initialize();
	FZibraModuleStyle::ReloadTextures();

	FZibraVDBUsageAnalytics::Initialize();

	RegisterSettings();
	RegisterInterfaceCustomizations();
	RegisterAssetTypeActions();
	RegisterSequencerTrackEditors();

	DisplayConfigurationNotification();
	RunAutomaticUpdateCheck();

	UE_LOG(LogZibraVDBEditor, Display, TEXT("ZibraVDB version: %s"), *FZibraVDBInfo::GetVersion());
}

void FZibraVDBEditorModule::RegisterInterfaceCustomizations()
{
	FPropertyEditorModule& PropertyModule = FModuleManager::LoadModuleChecked<FPropertyEditorModule>("PropertyEditor");
	PropertyModule.RegisterCustomClassLayout(UZibraVDBCompressedImporter::StaticClass()->GetFName(),
		FOnGetDetailCustomizationInstance::CreateStatic(&FZibraVDBCompressedImporterCustomization::MakeInstance));
	PropertyModule.RegisterCustomClassLayout(UZibraVDBSequenceImporter::StaticClass()->GetFName(),
		FOnGetDetailCustomizationInstance::CreateStatic(&FZibraVDBSequenceImporterCustomization::MakeInstance));
	PropertyModule.RegisterCustomClassLayout(UZibraVDBBugReporter::StaticClass()->GetFName(),
		FOnGetDetailCustomizationInstance::CreateStatic(&FZibraVDBBugReporterCustomization::MakeInstance));
	PropertyModule.RegisterCustomClassLayout(UZibraVDBVolume4::StaticClass()->GetFName(),
		FOnGetDetailCustomizationInstance::CreateStatic(&FZibraVDBAssetEditorCustomization::MakeInstance));
	PropertyModule.RegisterCustomClassLayout(UZibraVDBVolume::StaticClass()->GetFName(),
		FOnGetDetailCustomizationInstance::CreateStatic(&FZibraVDBAssetDeprecatedCustomization::MakeInstance));
	PropertyModule.NotifyCustomizationModuleChanged();

	FGlobalTabmanager::Get()
		->RegisterNomadTabSpawner(
			ZibraVDBImportZibraVDBTabName, FOnSpawnTab::CreateRaw(this, &FZibraVDBEditorModule::OpenImportZibraVDBFileTab))
		.SetDisplayName(LOCTEXT("ImportZibraVDBFile", "Import .zibravdb File"))
		.SetMenuType(ETabSpawnerMenuType::Hidden);
	FGlobalTabmanager::Get()
		->RegisterNomadTabSpawner(
			ZibraVDBImportVDBSequenceTabName, FOnSpawnTab::CreateRaw(this, &FZibraVDBEditorModule::OpenImportVDBSequenceWindowTab))
		.SetDisplayName(LOCTEXT("ImportOpenVDBSequence", "Import OpenVDB Sequence"))
		.SetMenuType(ETabSpawnerMenuType::Hidden);
	FGlobalTabmanager::Get()
		->RegisterNomadTabSpawner(
			ZibraVDBBugReporterTabName, FOnSpawnTab::CreateRaw(this, &FZibraVDBEditorModule::OpenBugReporterWindowTab))
		.SetDisplayName(LOCTEXT("BugReporter", "Bug Reporter"))
		.SetMenuType(ETabSpawnerMenuType::Hidden);

	UToolMenus::RegisterStartupCallback(
		FSimpleMulticastDelegate::FDelegate::CreateRaw(this, &FZibraVDBEditorModule::RegisterMenus));
}

void FZibraVDBEditorModule::RegisterAssetTypeActions()
{
	if (FModuleManager::Get().IsModuleLoaded("AssetTools"))
	{
		IAssetTools& AssetTools = FModuleManager::LoadModuleChecked<FAssetToolsModule>("AssetTools").Get();
		EAssetTypeCategories::Type ZibraVDBAssetCategory =
			AssetTools.RegisterAdvancedAssetCategory(FName(TEXT("ZibraVDB Assets")), LOCTEXT("ZibraVDBAssets", "ZibraVDB Assets"));

		ZibraVDBAssetActions = MakeShared<FZibraVDBAssetTypeActions>(ZibraVDBAssetCategory);
		AssetTools.RegisterAssetTypeActions(ZibraVDBAssetActions.ToSharedRef());
	}
}

void FZibraVDBEditorModule::DisplayConfigurationNotification()
{
#if ZIBRAVDB_DEBUG
	FZibraVDBNotifications::AddNotification(
		FText::FromString("Using DEBUG version of Native Plugin. This is only for ZibraAI's internal usage."), 30);
#elif ZIBRAVDB_PROFILE
	FZibraVDBNotifications::AddNotification(
		FText::FromString("Using PROFILE version of Native Plugin. This is only for ZibraAI's internal usage."), 30);
#elif ZIBRAVDB_RELEASENOCHECK
	FZibraVDBNotifications::AddNotification(
		FText::FromString("Using RELEASENOCHECK version of Native Plugin. This is only for ZibraAI's internal usage."), 30);
#endif
	FString AssetPath = TEXT("/ZibraVDB/Icons/compression_icon.compression_icon");
	UTexture2D* MyTexture = Cast<UTexture2D>(StaticLoadObject(UTexture2D::StaticClass(), nullptr, *AssetPath));
	ZibraIconBrush.SetResourceObject(MyTexture);
	ZibraIconBrush.ImageSize = FVector2D(MyTexture->GetSizeX(), MyTexture->GetSizeY());

	ZibraStyleSet = FSlateGameResources::New(FName("ZibraStyleSet"), "/Game/UI/Styles", "/Game/UI/Styles");
	ZibraStyleSet->Set("LowResIcon", &ZibraIconBrush);

	FSlateStyleRegistry::RegisterSlateStyle(*ZibraStyleSet);
}

void FZibraVDBEditorModule::RunAutomaticUpdateCheck()
{
	if (GetMutableDefault<UZibraEditorSettings>()->CheckUpdatesOnStartup)
	{
		FZibraVDBUpdateChecker::CheckForUpdates(true);
	}
}

void FZibraVDBEditorModule::ShutdownModule() noexcept
{
	FZibraModuleStyle::Shutdown();
	FZibraVDBUsageAnalytics::Shutdown();

	UnregisterSettings();
	UnregisterInterfaceCustomizations();
	UnregisterAssetTypeActions();
	UnregisterSequencerTrackEditors();
}

void FZibraVDBEditorModule::UnregisterAssetTypeActions()
{
	if (FModuleManager::Get().IsModuleLoaded("AssetTools"))
	{
		IAssetTools& AssetTools = FModuleManager::LoadModuleChecked<FAssetToolsModule>("AssetTools").Get();
		AssetTools.UnregisterAssetTypeActions(ZibraVDBAssetActions.ToSharedRef());
	}
}

void FZibraVDBEditorModule::UnregisterInterfaceCustomizations()
{
	UToolMenus::UnRegisterStartupCallback(this);

	UToolMenus::UnregisterOwner(this);

	FGlobalTabmanager::Get()->UnregisterNomadTabSpawner(ZibraVDBImportZibraVDBTabName);
	FGlobalTabmanager::Get()->UnregisterNomadTabSpawner(ZibraVDBImportVDBSequenceTabName);
	FGlobalTabmanager::Get()->UnregisterNomadTabSpawner(ZibraVDBBugReporterTabName);
}

void FZibraVDBEditorModule::RegisterSequencerTrackEditors()
{
	ISequencerModule& SequencerModule = FModuleManager::LoadModuleChecked<ISequencerModule>("Sequencer");
	ZibraVDBTrackEditorCreateHandle = SequencerModule.RegisterTrackEditor(
		FOnCreateTrackEditor::CreateStatic(&FZibraVDBTrackEditor::CreateTrackEditor)
	);
}

void FZibraVDBEditorModule::UnregisterSequencerTrackEditors()
{
	ISequencerModule* SequencerModule = FModuleManager::GetModulePtr<ISequencerModule>("Sequencer");
	if (SequencerModule)
	{
		SequencerModule->UnRegisterTrackEditor(ZibraVDBTrackEditorCreateHandle);
	}
}

TSharedRef<SDockTab> FZibraVDBEditorModule::OpenImportZibraVDBFileTab(const FSpawnTabArgs& SpawnTabArgs)
{
	return SNew(SDockTab).TabRole(ETabRole::NomadTab)[SNew(SZibraVDBCompressedImporterWidget)];
}

TSharedRef<SDockTab> FZibraVDBEditorModule::OpenImportVDBSequenceWindowTab(const FSpawnTabArgs& SpawnTabArgs)
{
	return SNew(SDockTab).TabRole(ETabRole::NomadTab)[SNew(SZibraVDBSequenceImporterWidget)];
}

TSharedRef<SDockTab> FZibraVDBEditorModule::OpenBugReporterWindowTab(const FSpawnTabArgs& SpawnTabArgs)
{
	return SNew(SDockTab).TabRole(ETabRole::NomadTab)[SNew(SZibraVDBBugReporterWidget)];
}

void FZibraVDBEditorModule::OpenImportZibraVDBFileWindow()
{
	FGlobalTabmanager::Get()->TryInvokeTab(ZibraVDBImportZibraVDBTabName);
}

void FZibraVDBEditorModule::OpenImportVDBSequenceWindow()
{
	FGlobalTabmanager::Get()->TryInvokeTab(ZibraVDBImportVDBSequenceTabName);
}

void FZibraVDBEditorModule::OpenBugReporterWindow()
{
	FGlobalTabmanager::Get()->TryInvokeTab(ZibraVDBBugReporterTabName);
}

void FZibraVDBEditorModule::CopyDiagnosticsInfo()
{
	FString Info = FZibraVDBDiagnosticsInfo::CollectInfoString();
	Info = "```json\nZibraVDB for Unreal Diagnostic Information Begin\n" + Info +
		   "\nZibraVDB for Unreal Diagnostic Information End\n```\n";
	FPlatformApplicationMisc::ClipboardCopy(*Info);
	UE_LOG(LogZibraVDBEditor, Log, TEXT("Diagnostics info is copied to the clipboard:\n%s"), *Info);
	FZibraVDBNotifications::AddNotification(
		LOCTEXT("DiagnosticsInfoCopiedToClipboard", "Diagnostics info is copied to the clipboard."));
}

void FZibraVDBEditorModule::CheckForUpdates()
{
	FZibraVDBUpdateChecker::CheckForUpdates(false);
}

void FZibraVDBEditorModule::GoToDocumentation()
{
	FPlatformProcess::LaunchURL(DocumentationLink, nullptr, nullptr);
}

void FZibraVDBEditorModule::RegisterMenus()
{
	// Owner will be used for cleanup in call to UToolMenus::UnregisterOwner
	FToolMenuOwnerScoped OwnerScoped(this);

	{
		UToolMenu* Menu = UToolMenus::Get()->ExtendMenu("LevelEditor.MainMenu.Tools");
		{
			FToolMenuSection& Section = Menu->AddSection("Plugins", LOCTEXT("PluginsSection", "Plugins"));

			Section.AddSubMenu("ZibraVDB", LOCTEXT("ZibraVDBMenuName", "ZibraVDB"),
				LOCTEXT("ZibraVDBMenuTooltip", "ZibraVDB Plugin Features"), FNewToolMenuChoice(), false,
				FSlateIcon("ZibraStyleSet", "LowResIcon"));

			UToolMenu* ZibraLiquidSubMenu = UToolMenus::Get()->ExtendMenu("LevelEditor.MainMenu.Tools.ZibraVDB");

			FToolMenuSection& ZibraVDBImportSection = ZibraLiquidSubMenu->AddSection("Import", LOCTEXT("Import", "Import"));

			ZibraVDBImportSection.AddMenuEntry(ZibraVDBImportZibraVDBTabName,
				LOCTEXT("ImportZibraVDBFile", "Import .zibravdb File"), LOCTEXT("ImportZibraVDBFile", "Import .zibravdb File"),
				TAttribute<FSlateIcon>(),
				FUIAction(FExecuteAction::CreateStatic(&FZibraVDBEditorModule::OpenImportZibraVDBFileWindow)));
			ZibraVDBImportSection.AddMenuEntry(ZibraVDBImportVDBSequenceTabName,
				LOCTEXT("ImportVDBSequence", "Import OpenVDB Sequence"), LOCTEXT("ImportOpenVDBSequence", "Import OpenVDB Sequence"),
				TAttribute<FSlateIcon>(),
				FUIAction(FExecuteAction::CreateStatic(&FZibraVDBEditorModule::OpenImportVDBSequenceWindow)));

			FToolMenuSection& ZibraVDBDiagnosticsSection =
				ZibraLiquidSubMenu->AddSection("Diagnostics", LOCTEXT("ZibraVDBDiagnosticsSection", "Diagnostics"));

			ZibraVDBDiagnosticsSection.AddMenuEntry(FName(TEXT("Copy diagnostics info")),
				LOCTEXT("CopyDiagnosticsInfo", "Copy diagnostics info"),
				LOCTEXT("CopyDiagnosticsInfoDescription", "Copy diagnostics information about system and project to the clipboard"),
				TAttribute<FSlateIcon>(), FUIAction(FExecuteAction::CreateRaw(this, &FZibraVDBEditorModule::CopyDiagnosticsInfo)));
			ZibraVDBDiagnosticsSection.AddMenuEntry(ZibraVDBBugReporterTabName, LOCTEXT("BugReporter", "Bug Reporter"),
				LOCTEXT("OpenBugReporterWindow", "Open Bug Reporter Window"), TAttribute<FSlateIcon>(),
				FUIAction(FExecuteAction::CreateStatic(&FZibraVDBEditorModule::OpenBugReporterWindow)));

			FToolMenuSection& ZibraVDBAboutSection =
				ZibraLiquidSubMenu->AddSection("About", LOCTEXT("ZibraVDBAboutSection", "About"));

			ZibraVDBAboutSection.AddMenuEntry(FName(TEXT("Check for updates")), LOCTEXT("RunUpdateCheck", "Check for updates"),
				LOCTEXT("RunUpdateCheckDescription", "Check whether there is newer ZibraVDB version available"),
				TAttribute<FSlateIcon>(), FUIAction(FExecuteAction::CreateRaw(this, &FZibraVDBEditorModule::CheckForUpdates)));
			ZibraVDBAboutSection.AddMenuEntry(FName(TEXT("Documentation")), LOCTEXT("OpenDocumentation", "Open Documentation"),
				LOCTEXT("OpenDocumentationDescription", "Open website with plugin documentation"), TAttribute<FSlateIcon>(),
				FUIAction(FExecuteAction::CreateRaw(this, &FZibraVDBEditorModule::GoToDocumentation)));
		}
	}
}

void FZibraVDBEditorModule::RegisterSettings()
{
	ISettingsModule* SettingsModule = FModuleManager::GetModulePtr<ISettingsModule>("Settings");
	if (SettingsModule)
	{
		SettingsModule->RegisterSettings("Project", "Plugins", "ZibraVDB Editor",
			LOCTEXT("ZibraVDBEditorSettingsName", "ZibraVDB Editor"),
			LOCTEXT("ZibraVDBEditorSettingsDescription", "ZibraVDB Editor Settings"), GetMutableDefault<UZibraEditorSettings>());
	}
}

void FZibraVDBEditorModule::UnregisterSettings()
{
	ISettingsModule* SettingsModule = FModuleManager::GetModulePtr<ISettingsModule>("Settings");
	if (SettingsModule)
	{
		SettingsModule->UnregisterSettings("Project", "Plugins", "ZibraVDB Editor");
	}
}

#undef LOCTEXT_NAMESPACE

IMPLEMENT_MODULE(FZibraVDBEditorModule, ZibraVDBEditor)
