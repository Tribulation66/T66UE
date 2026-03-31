// Copyright Zibra AI Inc 2025-2026. All Rights Reserved.

#include "ZibraVDBAssetEditor.h"

#include "Analytics/ZibraVDBCompressionAnalytics.h"
#include "DesktopPlatformModule.h"
#include "DetailCategoryBuilder.h"
#include "DetailLayoutBuilder.h"
#include "DetailWidgetRow.h"
#include "Dom/JsonObject.h"
#include "EditorReimportHandler.h"
#include "Framework/MultiBox/MultiBoxBuilder.h"
#include "HAL/FileManager.h"
#include "HAL/PlatformFileManager.h"
#include "Misc/MessageDialog.h"
#include "Misc/ScopedSlowTask.h"
#include "Modules/ModuleManager.h"
#include "PropertyEditorModule.h"
#include "RenderingThread.h"
#include "Widgets/Docking/SDockTab.h"
#include "Widgets/ZibraVDBCompressorCommon.h"
#include "Widgets/ZibraVDBImporterCommon.h"
#include "ZibraCompressionWorker.h"
#include "ZibraVDBAssetEditorCommands.h"
#include "ZibraVDBLicensing/Public/ZibraLicensingManager.h"
#include "ZibraVDBRuntime/Public/ZibraVDBNotifications.h"
#include "ZibraVDBRuntime/Public/ZibraVDBVolume4.h"
#include "ZibraVDBSDKIntegration.h"

#define LOCTEXT_NAMESPACE "ZibraVDBForUnreal"

FZibraVDBAssetEditor::~FZibraVDBAssetEditor()
{
	if (SequenceInfo)
	{
		FZibraVDBSDKIntegration::ReleaseSequenceInfo(SequenceInfo);
	}
}

void FZibraVDBAssetEditor::RegisterTabSpawners(const TSharedRef<FTabManager>& InTabManager)
{
	WorkspaceMenuCategory =
		InTabManager->AddLocalWorkspaceMenuCategory(LOCTEXT("ZibraVDBAssetEditorName", "ZibraVDB Asset Editor"));
	auto WorkspaceMenuCategoryRef = WorkspaceMenuCategory.ToSharedRef();

	FAssetEditorToolkit::RegisterTabSpawners(InTabManager);

	InTabManager
		->RegisterTabSpawner(
			FZibraVDBAssetEditor::ZibraVDBAssetDetailsID, FOnSpawnTab::CreateSP(this, &FZibraVDBAssetEditor::SpawnTab_Details))
		.SetDisplayName(LOCTEXT("Details", "Details"))
		.SetGroup(WorkspaceMenuCategoryRef)
		.SetIcon(FSlateIcon(FAppStyle::GetAppStyleSetName(), "LevelEditor.Tabs.Details"));
}

void FZibraVDBAssetEditor::UnregisterTabSpawners(const TSharedRef<FTabManager>& InTabManager)
{
	FAssetEditorToolkit::UnregisterTabSpawners(InTabManager);

	InTabManager->UnregisterTabSpawner(FZibraVDBAssetEditor::ZibraVDBAssetDetailsID);
}

void FZibraVDBAssetEditor::CreateInternalWidgets()
{
	FPropertyEditorModule& PropertyEditorModule = FModuleManager::GetModuleChecked<FPropertyEditorModule>("PropertyEditor");
	FDetailsViewArgs DetailsViewArgs;

	CurrentZibraVDBAsset->PerChannelReimportSettings = CurrentZibraVDBAsset->PerChannelCompressionSettings;

	DetailsViewArgs.NameAreaSettings = FDetailsViewArgs::HideNameArea;
	DetailsView = PropertyEditorModule.CreateDetailView(DetailsViewArgs);
	DetailsView->SetObject(CurrentZibraVDBAsset);
	DetailsView->OnFinishedChangingProperties().AddLambda(
		[this](const FPropertyChangedEvent& PropertyChangedEvent)
		{
			FName PropertyName =
				(PropertyChangedEvent.MemberProperty != nullptr) ? PropertyChangedEvent.MemberProperty->GetFName() : NAME_None;

			if (PropertyName == GET_MEMBER_NAME_CHECKED(UZibraVDBVolume4, SourceOpenVDBFilePath))
			{
				OnSourceOpenVDBFilePathPicked();
			}
			if (PropertyName == GET_MEMBER_NAME_CHECKED(UZibraVDBVolume4, Quality))
			{
				OnDefaultQualityChanged();
			}
			if (PropertyName == GET_MEMBER_NAME_CHECKED(UZibraVDBVolume4, bUsePerChannelCompressionSettings))
			{
				OnUsePerChannelCompressionSettingsChanged();
			}
		});
}

void FZibraVDBAssetEditor::ReadSequenceInfo()
{
	if (CompressionState != EZibraCompressionState::Uninitialized)
	{
		return;
	}

	const FString SourceFileMask = GetInputFileMask(CurrentZibraVDBAsset->SourceOpenVDBFilePath);

	TArray<FText> ErrorMessages{};

	FZibraVDBCompressorCommon::ReadSequenceInfo(
		SourceFileMask,
		[&, this](Zibra::OpenVDBHelper::SequenceInfo* InSequenceInfo) noexcept
		{
			if (InSequenceInfo->frameCount == 0)
			{
				ErrorMessages.Add(FText::Format(LOCTEXT("NoOpenVDBFiles", "Selected path \"{0}\" has no OpenVDB files."),
					FText::FromString(CurrentZibraVDBAsset->SourceOpenVDBFilePath.FilePath)));

				return;
			}

			if (InSequenceInfo->channelCount == 0)
			{
				ErrorMessages.Add(LOCTEXT("OpenVDBHasNoChannels", "Selected OpenVDB files have no valid channels."));

				return;
			}

			SequenceInfo = InSequenceInfo;
		},
		[this]() noexcept { CurrentZibraVDBAsset->SourceOpenVDBFilePath.FilePath = ""; });

	for (const FText& ErrorMessage : ErrorMessages)
	{
		FZibraVDBNotifications::AddNotification(ErrorMessage);
	}
}

void FZibraVDBAssetEditor::OnSourceOpenVDBFilePathPicked()
{
	ReadSequenceInfo();

	CurrentZibraVDBAsset->PerChannelReimportSettings.Empty();
	for (uint32_t ChannelIndex = 0; ChannelIndex < SequenceInfo->channelCount; ++ChannelIndex)
	{
		CurrentZibraVDBAsset->PerChannelReimportSettings.Add({SequenceInfo->channelNames[ChannelIndex]});
	}
	DetailsView->ForceRefresh();
}

TSharedRef<SDockTab> FZibraVDBAssetEditor::SpawnTab_Details(const FSpawnTabArgs& Args)
{
	check(Args.GetTabId() == FZibraVDBAssetEditor::ZibraVDBAssetDetailsID);

	CreateInternalWidgets();

	TSharedRef<SDockTab> SpawnedTab = SNew(SDockTab)
										  .Label(LOCTEXT("Details", "Details"))
										  .CanEverClose(false)[DetailsView.ToSharedRef()];

	return SpawnedTab;
}

void FZibraVDBAssetEditor::InitZibraVDBAssetEditor(const EToolkitMode::Type Mode, const TSharedPtr<IToolkitHost>& InitToolkitHost,
	TObjectPtr<UZibraVDBVolume4> ZibraVDBVolumeAsset)
{
	GEditor->GetEditorSubsystem<UAssetEditorSubsystem>()->CloseOtherEditors(ZibraVDBVolumeAsset, this);

	FZibraVDBAssetEditorCommands::Register();

	BindCommands();

	CurrentZibraVDBAsset = ZibraVDBVolumeAsset;

	const TSharedRef<FTabManager::FLayout> StandaloneZibraVDBEditorLayout =
		FTabManager::NewLayout("Standalone_ZibraVDBEditor_Layout")
			->AddArea(FTabManager::NewPrimaryArea()
					->SetOrientation(Orient_Vertical)
					->Split(FTabManager::NewStack()->AddTab(ZibraVDBAssetDetailsID, ETabState::OpenedTab)));
	InitAssetEditor(
		Mode, InitToolkitHost, ZibraVDBAssetEditorName, StandaloneZibraVDBEditorLayout, true, true, ZibraVDBVolumeAsset);

	ExtendToolbar();
	RegenerateMenusAndToolbars();
}

// FAssetEditorToolkit overrides
FName FZibraVDBAssetEditor::GetToolkitFName() const
{
	return FName("ZibraVDBAssetEditor");
}
FText FZibraVDBAssetEditor::GetBaseToolkitName() const
{
	return LOCTEXT("ZibraVDBAssetEditorName", "ZibraVDB Asset Editor");
}
FText FZibraVDBAssetEditor::GetToolkitName() const
{
	return FText::FromString(CurrentZibraVDBAsset->GetName());
}
FText FZibraVDBAssetEditor::GetToolkitToolTipText() const
{
	return FText::Format(
		LOCTEXT("ZibraVDBAssetEditorToolTipText", "{0} Editor"), FText::FromString(CurrentZibraVDBAsset->GetName()));
}
FLinearColor FZibraVDBAssetEditor::GetWorldCentricTabColorScale() const
{
	return FLinearColor::White;
}
FString FZibraVDBAssetEditor::GetWorldCentricTabPrefix() const
{
	return TEXT("ZibraVDBAssetEditor");
}

// FGCObject overrides
void FZibraVDBAssetEditor::AddReferencedObjects(FReferenceCollector& Collector)
{
	// Add any objects that you want to keep alive here
	Collector.AddReferencedObject(CurrentZibraVDBAsset);
}

FString FZibraVDBAssetEditor::GetReferencerName() const
{
	return TEXT("FZibraVDBAssetEditor");
}

void FZibraVDBAssetEditor::BindCommands()
{
	const FZibraVDBAssetEditorCommands& Commands = FZibraVDBAssetEditorCommands::Get();

	ToolkitCommands->MapAction(
		Commands.ReimportCommand, FExecuteAction::CreateRaw(this, &FZibraVDBAssetEditor::ReimportAsset), FCanExecuteAction());
	ToolkitCommands->MapAction(
		Commands.ExportCommand, FExecuteAction::CreateRaw(this, &FZibraVDBAssetEditor::ExportZibraVDB), FCanExecuteAction());
}

void FZibraVDBAssetEditor::ExtendToolbar()
{
	struct Local
	{
		static void FillToolbar(FToolBarBuilder& ToolbarBuilder)
		{
			ToolbarBuilder.BeginSection("ZibraVDBAsset");
			{
				ToolbarBuilder.AddToolBarButton(FZibraVDBAssetEditorCommands::Get().ReimportCommand, NAME_None, TAttribute<FText>(),
					TAttribute<FText>(), FSlateIcon(FAppStyle::GetAppStyleSetName(), "Persona.ReimportAsset"));
				ToolbarBuilder.AddToolBarButton(FZibraVDBAssetEditorCommands::Get().ExportCommand, NAME_None, TAttribute<FText>(),
					TAttribute<FText>(), FSlateIcon(FAppStyle::GetAppStyleSetName(), "DataTableEditor.Copy"));
			}
			ToolbarBuilder.EndSection();
		}
	};

	TSharedPtr<FExtender> ToolbarExtender = MakeShared<FExtender>();
	ToolbarExtender->AddToolBarExtension(
		"Asset", EExtensionHook::After, ToolkitCommands, FToolBarExtensionDelegate::CreateStatic(&Local::FillToolbar));

	AddToolbarExtender(ToolbarExtender);
}

void FZibraVDBAssetEditor::ReimportAsset()
{
	if (!CurrentZibraVDBAsset)
	{
		return;
	}
	if (CurrentZibraVDBAsset->bIsLoadedFromZibraVDB)
	{
		ReimportZibraVDB();
	}
	else
	{
		ReimportOpenVDB();
	}
}

void FZibraVDBAssetEditor::ExportZibraVDB()
{
	if (!CurrentZibraVDBAsset)
	{
		return;
	}
	ExportZibraVDB(CurrentZibraVDBAsset->GetZibraVDBFile());
}

void FZibraVDBAssetEditor::ExportZibraVDB(const ZibraVDB::FZibraVDBFile& ZibraVDBFile)
{
	TArray<FString> FilePath;
	bool bOpened = FDesktopPlatformModule::Get()->SaveFileDialog(nullptr, TEXT("Export ZibraVDB File"), FPaths::ProjectDir(),
		TEXT(""), TEXT("ZibraVDB File (*.zibravdb)|*.zibravdb"), EFileDialogFlags::None, FilePath);

	if (bOpened)
	{
		ZibraVDBFile.Export(FilePath[0]);
	}
}

void FZibraVDBAssetEditor::ReimportZibraVDB()
{
	FReimportManager::Instance()->UpdateReimportPaths(CurrentZibraVDBAsset, {CurrentZibraVDBAsset->SourceZibraVDBFile.FilePath});
	FReimportManager::Instance()->Reimport(
		CurrentZibraVDBAsset, false, true, CurrentZibraVDBAsset->SourceZibraVDBFile.FilePath, nullptr, INDEX_NONE, false, true);

	FZibraVDBCompressionAnalytics::TrackImport(CurrentZibraVDBAsset, true);
}

void FZibraVDBAssetEditor::ReimportOpenVDB()
{
	if (!FZibraLicensingManager::GetInstance().IsLicenseVerified(FZibraLicensingManager::Product::Compression))
	{
		FText ErrorText;
		static const FText ErrorTitle = LOCTEXT("ZibraVDBLicenseErrorTitle", "ZibraVDB License Error");
		if (FZibraLicensingManager::GetInstance().IsAnyLicenseVerified())
		{
			static const FText NoCompressionErrorText = LOCTEXT("TriedToCompressButLicenseNoCompression",
				"Your license does not include Compression functionality. You can change your license via Project Settings. You can purchase new ZibraVDB license on https://zibra.ai");
			ErrorText = NoCompressionErrorText;
		}
		else
		{
			static const FText NoLicenseErrorText = LOCTEXT("TriedToCompressButLicenseNotVerified",
				"Compression requires active license. You can enter your license details via Project Settings. You can purchase ZibraVDB license on https://zibra.ai");
			ErrorText = NoLicenseErrorText;
		}
		UE_LOG(LogZibraVDBLicensing, Error, TEXT("%s"), *ErrorText.ToString());
		FMessageDialog::Open(EAppMsgType::Ok, ErrorText, ErrorTitle);
		return;
	}

	FString SourceFileMask = GetInputFileMask(CurrentZibraVDBAsset->SourceOpenVDBFilePath);

	TArray<FCompressionSettings> ActiveChannelsToCompress = CurrentZibraVDBAsset->PerChannelReimportSettings.FilterByPredicate(
		[](const FCompressionSettings& CompressionSettings) -> bool { return CompressionSettings.bActive; });

	if (ActiveChannelsToCompress.IsEmpty())
	{
		FZibraVDBNotifications::AddNotification(LOCTEXT("SelectAtLeastOneChannelToCompress", "Please, select at least one channel to compress"));
		return;
	}

	if (!SequenceInfo)
	{
		ReadSequenceInfo();

		// In case, failed to read metadata, or if it is empty (only empty frames in vdb).
		if (!SequenceInfo)
		{
			return;
		}
	}

	FDateTime CompressionStart = FDateTime::UtcNow();

	CompressionState = EZibraCompressionState::Started;

	CurrentZibraVDBAsset->PerChannelReimportSettings = ActiveChannelsToCompress;
	CurrentZibraVDBAsset->PerChannelCompressionSettings = ActiveChannelsToCompress;

	TArray<TPair<FString, float>> PerChannelCompressionSettings;
	for (const auto& Channel : ActiveChannelsToCompress)
	{
		PerChannelCompressionSettings.Add({Channel.Name, Channel.Quality});
	}

	const FString IntermedateDir = FPaths::ConvertRelativePathToFull(FPaths::ProjectIntermediateDir());
	const FString OuputFilePath = FPaths::CreateTempFilename(*IntermedateDir, TEXT("ZibraVDBTemp"), TEXT(".zibravdb"));

	TSharedPtr<FZibraVDBCompressorManager> CompressorManager = MakeShared<FZibraVDBCompressorManager>();
	auto ReturnCode = CompressorManager->Initialize(CurrentZibraVDBAsset->Quality, *SequenceInfo, PerChannelCompressionSettings);
	if (ReturnCode != Zibra::CE::ZCE_SUCCESS)
	{
		FZibraVDBNotifications::AddNotification(LOCTEXT("CompressorInitializationFailed", "Failed to initialize compressor."));
		return;
	}
	CompressorManager->SetOutputFile(OuputFilePath);
	ZibraCompressionWorker = MakeUnique<FZibraCompressionWorker>(
		CompressorManager, SourceFileMask, [this]() noexcept { CompressionState = EZibraCompressionState::Finished; });

	FScopedSlowTask SlowTask(1.0f, LOCTEXT("ReimportingAsset", "Reimporting Asset..."));
	SlowTask.MakeDialog();

	// Spin-lock with thread sleeping and progress bar
	float PreviousDeltaTime = 0.0;
	while (CompressionState != EZibraCompressionState::Finished)
	{
		const auto CompressionProgress = CompressorManager->GetCompressionProgress();
		const auto DeltaTime = FMath::Clamp(CompressionProgress - SlowTask.CompletedWork - PreviousDeltaTime, 0.0f, 1.0f);
		SlowTask.EnterProgressFrame(DeltaTime);

		PreviousDeltaTime = DeltaTime;

		FPlatformProcess::Sleep(0.1f);
	}

	auto OpenVDBImportedDetails = MakeUnique<FZibraVDBImporterCommon::FOpenVDBImportedDetails>();
	OpenVDBImportedDetails->InputFilePath = CurrentZibraVDBAsset->SourceOpenVDBFilePath.FilePath;
	OpenVDBImportedDetails->InputFileMask = SourceFileMask;
	OpenVDBImportedDetails->Quality = CurrentZibraVDBAsset->Quality;
	OpenVDBImportedDetails->UsePerChannelCompressionSettings = CurrentZibraVDBAsset->bUsePerChannelCompressionSettings;
	OpenVDBImportedDetails->PerChannelCompressionSettings = CurrentZibraVDBAsset->PerChannelCompressionSettings;

	// Import asset from ZibraVDB
	FReimportManager::Instance()->UpdateReimportPaths(CurrentZibraVDBAsset, {OuputFilePath});
	FReimportManager::Instance()->Reimport(
		CurrentZibraVDBAsset, false, true, FString(OuputFilePath), nullptr, INDEX_NONE, false, true);
	FZibraVDBImporterCommon::SetCompressionDetails({CurrentZibraVDBAsset}, OpenVDBImportedDetails);

	// Delete temp ZibraVDB file
	FZibraVDBCompressorCommon::DeleteTempCompressedFile(FString(OuputFilePath));

	FDateTime CompressionEnd = FDateTime::UtcNow();

	FZibraVDBCompressionAnalytics::TrackCompression(CurrentZibraVDBAsset, CompressionEnd - CompressionStart, true);

	CompressorManager->Release();

	CompressionState = EZibraCompressionState::Uninitialized;
	DetailsView->ForceRefresh();
}

FString FZibraVDBAssetEditor::GetInputFileMask(const FFilePath& FilePath)
{
	FString FileMask = FilePath.FilePath;
	FileMask.RemoveFromEnd(".vdb");
	FileMask = FZibraVDBImporterCommon::StripFrameNumber(FileMask);
	return FileMask;
}

void FZibraVDBAssetEditor::OnDefaultQualityChanged()
{
	if (CurrentZibraVDBAsset->bUsePerChannelCompressionSettings)
	{
		return;
	}

	for (auto& PerChannelCompressionSettingsPair : CurrentZibraVDBAsset->PerChannelReimportSettings)
	{
		PerChannelCompressionSettingsPair.Quality = CurrentZibraVDBAsset->Quality;
	}
}

void FZibraVDBAssetEditor::OnUsePerChannelCompressionSettingsChanged()
{
	for (auto& PerChannelCompressionSettingsPair : CurrentZibraVDBAsset->PerChannelReimportSettings)
	{
		PerChannelCompressionSettingsPair.Quality = CurrentZibraVDBAsset->Quality;
	}
}

TSharedRef<IDetailCustomization> FZibraVDBAssetEditorCustomization::MakeInstance()
{
	return MakeShared<FZibraVDBAssetEditorCustomization>();
}

void FZibraVDBAssetEditorCustomization::CustomizeDetails(IDetailLayoutBuilder& DetailBuilder)
{
	UZibraVDBVolume4* ZibraVDBVolume = nullptr;
	TArray<TWeakObjectPtr<UObject>> CustomizedObjects;
	DetailBuilder.GetObjectsBeingCustomized(CustomizedObjects);
	for (TWeakObjectPtr<UObject> Object : CustomizedObjects)
	{
		ZibraVDBVolume = Cast<UZibraVDBVolume4>(Object);
		if (ZibraVDBVolume)
		{
			break;
		}
	}

	if (!ZibraVDBVolume)
	{
		return;
	}

	TSharedPtr<IPropertyHandle> PropertyHandle =
		DetailBuilder.GetProperty(GET_MEMBER_NAME_CHECKED(UZibraVDBVolume4, PerChannelCompressionSettingsPlaceholder));

	TSharedRef<SHorizontalBox> AdditionalParameters = FZibraVDBCompressorCommon::ConstructPerChannelCompressionSettingsBox(
		ZibraVDBVolume->PerChannelReimportSettings, ZibraVDBVolume->Quality);

	DetailBuilder.EditDefaultProperty(PropertyHandle)
		->CustomWidget()
		.Visibility(TAttribute<EVisibility>::Create(TAttribute<EVisibility>::FGetter::CreateLambda([ZibraVDBVolume]() -> EVisibility
			{ return ZibraVDBVolume->bUsePerChannelCompressionSettings ? EVisibility::Visible : EVisibility::Collapsed; })))
		.WholeRowWidget[AdditionalParameters];
}

#undef LOCTEXT_NAMESPACE
