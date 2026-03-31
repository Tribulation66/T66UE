// Copyright Zibra AI Inc 2025-2026. All Rights Reserved.
#include "Widgets/ZibraVDBCompressedImporterWidget.h"

#include "Analytics/ZibraVDBCompressionAnalytics.h"
#include "AssetRegistry/AssetRegistryModule.h"
#include "AssetToolsModule.h"
#include "DetailCategoryBuilder.h"
#include "DetailLayoutBuilder.h"
#include "DetailWidgetRow.h"
#include "Editor/UnrealEdEngine.h"
#include "EditorDirectories.h"
#include "EditorStyleSet.h"
#include "HAL/FileManager.h"
#include "Misc/FileHelper.h"
#include "Widgets/Docking/SDockTab.h"
#include "Widgets/Input/SButton.h"
#include "Widgets/Input/SEditableTextBox.h"
#include "Widgets/Input/SFilePathPicker.h"
#include "Widgets/Input/SNumericEntryBox.h"
#include "Widgets/Layout/SBox.h"
#include "Widgets/Layout/SScrollBox.h"
#include "Widgets/Layout/SSeparator.h"
#include "Widgets/Notifications/SProgressBar.h"
#include "Widgets/Text/STextBlock.h"
#include "Widgets/ZibraVDBImporterCommon.h"
#include "ZibraVDBRuntime/Public/ZibraVDBFile.h"
#include "ZibraVDBRuntime/Public/ZibraVDBNotifications.h"
#include "ZibraVDBSDKIntegration.h"
#include "ZibraVDBSDKIntegration/Public/ZibraVDBDecompressorManager.h"

#define LOCTEXT_NAMESPACE "ZibraVDBForUnreal"

UZibraVDBCompressedImporter::UZibraVDBCompressedImporter(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer)
{
}

FReply UZibraVDBCompressedImporter::OnImportButtonClicked()
{
	if (!ValidateImportParameters())
	{
		return FReply::Handled();
	}

	UObject* CompressedFile = FZibraVDBImporterCommon::ImportCompressedFile(
		ZibraVDBFilePath.FilePath, AssetPath.Path, FPaths::GetBaseFilename(ZibraVDBFilePath.FilePath), false);

	UZibraVDBVolume4* ZibraVDBVolume = Cast<UZibraVDBVolume4>(CompressedFile);

	if (ZibraVDBVolume)
	{
		FZibraVDBCompressionAnalytics::TrackImport(ZibraVDBVolume, false);
	}

	return FReply::Handled();
}

void UZibraVDBCompressedImporter::OnZibraVDBFilePathPicked()
{
	const FString& FilePath = ZibraVDBFilePath.FilePath;

	if (FilePath.IsEmpty())
	{
		FramesNumber = 0;
		ZibraVDBFileSize = 0;
		return;
	}

	// TODO: refactor to read only header, not the whole file.
	TArray64<uint8> BinaryData;
	const bool bLoadSuccessful = FFileHelper::LoadFileToArray(BinaryData, *FilePath);

	if (!bLoadSuccessful)
	{
		FZibraVDBNotifications::AddNotification(LOCTEXT("LoadFileToArrayFailed", "Failed to load data from file."));
		return;
	}

	FZibraVDBDecompressorManager DecompressorManager;

	auto ReturnCode = DecompressorManager.Initialize(BinaryData);
	if (ReturnCode != Zibra::CE::ZCE_SUCCESS)
	{
		FZibraVDBNotifications::AddNotification(
			FText::Format(LOCTEXT("DecompressorInitializationFailure", "Failed to initialize decompressor: {0}"),
				FText::FromString(FZibraVDBSDKIntegration::ErrorToString(ReturnCode))));
		return;
	}

	Zibra::CE::Decompression::PlaybackInfo PlaybackInfo = DecompressorManager.GetPlaybackInfo();

	DecompressorManager.Release(); 

	FramesNumber = PlaybackInfo.frameCount;

	ZibraVDBFileSize = IFileManager::Get().FileSize(*FilePath) / 1024 / 1024;
	ZibraVDBFilePath.FilePath = FPaths::ConvertRelativePathToFull(FilePath);
}

bool UZibraVDBCompressedImporter::ValidateImportParameters()
{
	if (!FPaths::FileExists(ZibraVDBFilePath.FilePath))
	{
		FZibraVDBNotifications::AddNotification(LOCTEXT("FileDoesntExist", "File does not exist."));
		return false;
	}

	if (!FPackageName::IsValidPath(AssetPath.Path))
	{
		FZibraVDBNotifications::AddNotification(LOCTEXT("InvalidAssetPath", "Selected asset path is not valid."));
		return false;
	}
	return true;
}

TSharedRef<IDetailCustomization> FZibraVDBCompressedImporterCustomization::MakeInstance()
{
	return MakeShared<FZibraVDBCompressedImporterCustomization>();
}

void FZibraVDBCompressedImporterCustomization::CustomizeDetails(IDetailLayoutBuilder& DetailBuilder)
{
	// Create a category so this is displayed early in the properties
	IDetailCategoryBuilder& Category = DetailBuilder.EditCategory("Import", FText::GetEmpty(), ECategoryPriority::Uncommon);

	// Get importer instanse.
	UZibraVDBCompressedImporter* ZibraVDBCompressedImporter = nullptr;
	TArray<TWeakObjectPtr<UObject>> CustomizedObjects;
	DetailBuilder.GetObjectsBeingCustomized(CustomizedObjects);
	for (TWeakObjectPtr<UObject> Object : CustomizedObjects)
	{
		if (!Object.IsValid())
		{
			continue;
		}
		UZibraVDBCompressedImporter* Importer = Cast<UZibraVDBCompressedImporter>(Object);
		if (Importer)
		{
			ZibraVDBCompressedImporter = Importer;
			break;
		}
	}

	if (!ZibraVDBCompressedImporter)
	{
		return;
	}

	// Add import button.
	const TSharedRef<SButton> ImportButton =
		SNew(SButton)
			.HAlign(HAlign_Center)
			.ContentPadding(5.0f)
			.Text(LOCTEXT("Import", "Import"))
			.OnClicked_UObject(ZibraVDBCompressedImporter, &UZibraVDBCompressedImporter::OnImportButtonClicked);

	Category.AddCustomRow(LOCTEXT("Import", "Import")).WholeRowContent()[ImportButton];
}

void SZibraVDBCompressedImporterWidget::Construct(const FArguments& InArgs)
{
	// Create the details view
	FDetailsViewArgs DetailsViewArgs;
	DetailsViewArgs.bHideSelectionTip = true;
	DetailsViewArgs.NameAreaSettings = FDetailsViewArgs::HideNameArea;
	DetailsViewArgs.bAllowSearch = false;
	DetailsViewArgs.bUpdatesFromSelection = false;
	DetailsViewArgs.bShowOptions = false;

	DetailsView = FModuleManager::LoadModuleChecked<FPropertyEditorModule>("PropertyEditor").CreateDetailView(DetailsViewArgs);

	ZibraVDBCompressedImporter = NewObject<UZibraVDBCompressedImporter>();
	ZibraVDBCompressedImporter->AddToRoot();
	DetailsView->SetObject(ZibraVDBCompressedImporter);
	DetailsView->OnFinishedChangingProperties().AddLambda(
		[this](const FPropertyChangedEvent& PropertyChangedEvent)
		{
			FName PropertyName =
				(PropertyChangedEvent.MemberProperty != nullptr) ? PropertyChangedEvent.MemberProperty->GetFName() : NAME_None;

			if (PropertyName == GET_MEMBER_NAME_CHECKED(UZibraVDBCompressedImporter, ZibraVDBFilePath))
			{
				ZibraVDBCompressedImporter->OnZibraVDBFilePathPicked();
			}
		});

	ChildSlot[DetailsView.ToSharedRef()];
}

SZibraVDBCompressedImporterWidget::~SZibraVDBCompressedImporterWidget()
{
	if (ZibraVDBCompressedImporter && ZibraVDBCompressedImporter->IsValidLowLevel())
	{
		ZibraVDBCompressedImporter->RemoveFromRoot();
	}
}

#undef LOCTEXT_NAMESPACE
