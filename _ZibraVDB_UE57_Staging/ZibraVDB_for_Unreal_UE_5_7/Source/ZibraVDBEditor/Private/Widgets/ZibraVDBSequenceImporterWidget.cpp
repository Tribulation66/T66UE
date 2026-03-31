// Copyright Zibra AI Inc 2025-2026. All Rights Reserved.
#include "Widgets/ZibraVDBSequenceImporterWidget.h"

#include "Algo/Copy.h"
#include "Algo/Count.h"
#include "Analytics/ZibraVDBCompressionAnalytics.h"
#include "DetailBuilderTypes.h"
#include "DetailCategoryBuilder.h"
#include "DetailLayoutBuilder.h"
#include "DetailWidgetRow.h"
#include "Editor/UnrealEdEngine.h"
#include "EditorDirectories.h"
#include "EditorStyleSet.h"
#include "IDetailGroup.h"
#include "Misc/MessageDialog.h"
#include "Modules/ModuleManager.h"
#include "RenderingThread.h"
#include "Widgets/Input/SButton.h"
#include "Widgets/Input/SComboBox.h"
#include "Widgets/Input/SEditableTextBox.h"
#include "Widgets/Input/SFilePathPicker.h"
#include "Widgets/Layout/SBox.h"
#include "Widgets/Layout/SScrollBox.h"
#include "Widgets/Layout/SSeparator.h"
#include "Widgets/Notifications/SProgressBar.h"
#include "Widgets/Text/STextBlock.h"
#include "Widgets/ZibraVDBCompressorCommon.h"
#include "Widgets/ZibraVDBImporterCommon.h"
#include "ZibraLicensingManager.h"
#include "ZibraVDBAssetEditor.h"
#include "ZibraVDBRuntime/Public/ZibraVDBNotifications.h"
#include "ZibraVDBRuntime/Public/ZibraVDBVolume4.h"
#include "ZibraVDBSDKIntegration.h"
#include "ZibraVDBSDKIntegration/Public/ZibraVDBCompressorManager.h"

DEFINE_LOG_CATEGORY_STATIC(LogZibraVDBMetadata, Log, All);

#define LOCTEXT_NAMESPACE "ZibraVDBForUnreal"

FString SZibraVDBSequenceImporterWidget::InputFilePath;

TSharedRef<IDetailCustomization> FZibraVDBSequenceImporterCustomization::MakeInstance()
{
	return MakeShared<FZibraVDBSequenceImporterCustomization>();
}

void FZibraVDBSequenceImporterCustomization::CustomizeDetails(IDetailLayoutBuilder& DetailBuilder)
{
	// Get importer instanse.
	UZibraVDBSequenceImporter* ZibraVDBSequenceImporter = nullptr;
	TArray<TWeakObjectPtr<UObject>> CustomizedObjects;
	DetailBuilder.GetObjectsBeingCustomized(CustomizedObjects);
	for (TWeakObjectPtr<UObject> Object : CustomizedObjects)
	{
		if (!Object.IsValid())
		{
			continue;
		}
		UZibraVDBSequenceImporter* Importer = Cast<UZibraVDBSequenceImporter>(Object);
		if (Importer)
		{
			ZibraVDBSequenceImporter = Importer;
			break;
		}
	}

	if (!ZibraVDBSequenceImporter)
	{
		return;
	}

	ZibraVDBSequenceImporter->CachedDetailBuilder = &DetailBuilder;

	const auto& ChannelNames = ZibraVDBSequenceImporter->ChannelNames;

	ZibraVDBSequenceImporter->PerChannelCompressionSettings.Empty();
	for (const auto& ChannelName : ChannelNames)
	{
		ZibraVDBSequenceImporter->PerChannelCompressionSettings.Add({*ChannelName});
	}

	TSharedPtr<IPropertyHandle> PropertyHandle =
		DetailBuilder.GetProperty(GET_MEMBER_NAME_CHECKED(UZibraVDBSequenceImporter, PerChannelCompressionSettingsPlaceholder));

	TSharedRef<SHorizontalBox> AdditionalParameters = FZibraVDBCompressorCommon::ConstructPerChannelCompressionSettingsBox(
		ZibraVDBSequenceImporter->PerChannelCompressionSettings, ZibraVDBSequenceImporter->Quality);

	{
		DetailBuilder.EditDefaultProperty(PropertyHandle)
			->CustomWidget()
// [BUG] In UE 5.3 Visibility lambda does not work
#if (100 * ENGINE_MAJOR_VERSION + ENGINE_MINOR_VERSION) >= 504
			.Visibility(TAttribute<EVisibility>::CreateLambda(
				[ZibraVDBSequenceImporter]()
				{
					return ZibraVDBSequenceImporter->bUsePerChannelCompressionSettings &&
								   ZibraVDBSequenceImporter->NumberOfChannels > 0
							   ? EVisibility::Visible
							   : EVisibility::Collapsed;
				}))
#else
			.EditCondition(TAttribute<bool>::CreateLambda(
							   [ZibraVDBSequenceImporter]()
							   {
								   return ZibraVDBSequenceImporter->bUsePerChannelCompressionSettings &&
										  ZibraVDBSequenceImporter->NumberOfChannels;
							   }),
				nullptr)
#endif
			.WholeRowWidget[AdditionalParameters];
	}

	// Create a category so this is displayed early in the properties
	IDetailCategoryBuilder& CompressionCategory =
		DetailBuilder.EditCategory("Compression", FText::GetEmpty(), ECategoryPriority::Uncommon);

	constexpr float Padding = 5.0f;

	const TSharedRef<SVerticalBox> CompressionButton =
		(SNew(SVerticalBox) +
			SVerticalBox::Slot().AutoHeight().Padding(Padding)[SNew(SBox).Visibility_Lambda(
				[this, ZibraVDBSequenceImporter]
				{
					return (ZibraVDBSequenceImporter->CompressionState == EZibraCompressionState::Uninitialized)
							   ? EVisibility::Visible
							   : EVisibility::Collapsed;
				})[SNew(SButton)
					.HAlign(HAlign_Center)
					.ContentPadding(Padding)
					.Text(LOCTEXT("Compress", "Compress"))
					.OnClicked_UObject(ZibraVDBSequenceImporter, &UZibraVDBSequenceImporter::OnStartCompression)]] +
			SVerticalBox::Slot().AutoHeight().Padding(Padding)[SNew(SProgressBar)
					.Visibility_Lambda(
						[this, ZibraVDBSequenceImporter]
						{
							return (ZibraVDBSequenceImporter->CompressionState == EZibraCompressionState::Started)
									   ? EVisibility::Visible
									   : EVisibility::Collapsed;
						})
					.Percent_Lambda(
						[ZibraVDBSequenceImporter]() { return ZibraVDBSequenceImporter->GetCompressionProgressPercent(); })
					.ToolTipText_Lambda(
						[ZibraVDBSequenceImporter]() -> FText
						{
							auto PercentOptional = ZibraVDBSequenceImporter->GetCompressionProgressPercent();
							if (!PercentOptional.IsSet())
							{
								return FText::GetEmpty();
							}

							float Percent = PercentOptional.GetValue() * 100.0f;
							FString Message = FString::Printf(TEXT("%.2f%%"), Percent);
							return FText::FromString(Message);
						})
					.FillColorAndOpacity(FSlateColor(FLinearColor(0.0f, 1.0f, 0.0f, 1.0f)))]

		);

	CompressionCategory.AddCustomRow(LOCTEXT("Compress", "Compress")).WholeRowWidget[CompressionButton];

	const TSharedRef<SVerticalBox> CancelButton =
		(SNew(SVerticalBox) +
			SVerticalBox::Slot().AutoHeight().Padding(Padding)[SNew(SBox)[SNew(SButton)
					.HAlign(HAlign_Center)
					.ContentPadding(Padding)
					.Text(LOCTEXT("Cancel", "Cancel"))
					.OnClicked_UObject(ZibraVDBSequenceImporter, &UZibraVDBSequenceImporter::OnCancelCompression)]] +
			SVerticalBox::Slot().AutoHeight().Padding(Padding));

	CompressionCategory.AddCustomRow(LOCTEXT("Cancel", "Cancel"))
		.Visibility(TAttribute<EVisibility>::Create(TAttribute<EVisibility>::FGetter::CreateLambda(
			[ZibraVDBSequenceImporter]() -> EVisibility
			{
				return ZibraVDBSequenceImporter->CompressionState == EZibraCompressionState::Started ? EVisibility::Visible
																									 : EVisibility::Collapsed;
			})))
		.WholeRowWidget[CancelButton];
}

void SZibraVDBSequenceImporterWidget::Construct(const FArguments& InArgs)
{
	// Create the details view
	FDetailsViewArgs DetailsViewArgs;
	DetailsViewArgs.bHideSelectionTip = true;
	DetailsViewArgs.NameAreaSettings = FDetailsViewArgs::HideNameArea;
	DetailsViewArgs.bAllowSearch = false;
	DetailsViewArgs.bUpdatesFromSelection = false;
	DetailsViewArgs.bShowOptions = false;

	DetailsView = FModuleManager::LoadModuleChecked<FPropertyEditorModule>("PropertyEditor").CreateDetailView(DetailsViewArgs);

	ZibraVDBSequenceImporter = NewObject<UZibraVDBSequenceImporter>();
	ZibraVDBSequenceImporter->AddToRoot();

	if (!InputFilePath.IsEmpty())
	{
		ZibraVDBSequenceImporter->VDBFilePath.FilePath = InputFilePath;
		ZibraVDBSequenceImporter->OnCompressionFilePathPicked();
		InputFilePath.Empty();
	}

	DetailsView->SetObject(ZibraVDBSequenceImporter);
	DetailsView->OnFinishedChangingProperties().AddLambda(
		[this](const FPropertyChangedEvent& PropertyChangedEvent)
		{
			FName PropertyName =
				(PropertyChangedEvent.MemberProperty != nullptr) ? PropertyChangedEvent.MemberProperty->GetFName() : NAME_None;

			if (PropertyName == GET_MEMBER_NAME_CHECKED(UZibraVDBSequenceImporter, VDBFilePath))
			{
				ZibraVDBSequenceImporter->OnCompressionFilePathPicked();
				DetailsView->ForceRefresh();
			}

			if (PropertyName == GET_MEMBER_NAME_CHECKED(UZibraVDBSequenceImporter, Quality))
			{
				ZibraVDBSequenceImporter->OnDefaultQualityChanged();
			}
			if (PropertyName == GET_MEMBER_NAME_CHECKED(UZibraVDBSequenceImporter, bUsePerChannelCompressionSettings))
			{
				ZibraVDBSequenceImporter->OnUsePerChannelCompressionSettingsChanged();
			}
		});

	ChildSlot[DetailsView.ToSharedRef()];
}

SZibraVDBSequenceImporterWidget::~SZibraVDBSequenceImporterWidget()
{
	if (ZibraVDBSequenceImporter && ZibraVDBSequenceImporter->IsValidLowLevel())
	{
		ZibraVDBSequenceImporter->RemoveFromRoot();
	}
}

void SZibraVDBSequenceImporterWidget::SetInputFile(const FString& InFilePath)
{
	InputFilePath = InFilePath;
}

void LogCompressionStart(const FString& InputFileMask, float Quality)
{
	UE_LOG(LogZibraVDBCompression, Display, TEXT("Compressing %s with Quality of %f"), *InputFileMask, Quality);
}

void LogCompressionEnd(const FTimespan& CompressionTime)
{
	UE_LOG(LogZibraVDBCompression, Display, TEXT("Compression took %.3f seconds."), CompressionTime.GetTotalSeconds());
}

void UZibraVDBSequenceImporter::Tick(float DeltaTime)
{
	switch (CompressionState)
	{
		case EZibraCompressionState::Uninitialized:
			break;
		case EZibraCompressionState::Started:
			break;
		case EZibraCompressionState::Finished:
		{
			auto OpenVDBImportedDetails = MakeUnique<FZibraVDBImporterCommon::FOpenVDBImportedDetails>();
			OpenVDBImportedDetails->InputFilePath = VDBFilePath.FilePath;
			OpenVDBImportedDetails->InputFileMask = InputFileMask;
			OpenVDBImportedDetails->Quality = Quality;
			OpenVDBImportedDetails->UsePerChannelCompressionSettings = bUsePerChannelCompressionSettings;
			OpenVDBImportedDetails->PerChannelCompressionSettings = PerChannelCompressionSettings.FilterByPredicate(
				[](const FCompressionSettings& CompressionSettings) -> bool { return CompressionSettings.bActive; });
			Algo::Transform(ChannelNames, OpenVDBImportedDetails->AvailableChannels,
				[](const TSharedPtr<FString>& Name) noexcept -> FString { return *Name; });

			UObject* ImportedFile = FZibraVDBImporterCommon::ImportCompressedFile(
				FString(OutputFile), AssetPath.Path, FPaths::GetBaseFilename(VDBFilePath.FilePath), true, OpenVDBImportedDetails);

			ZibraCompressionWorker.Reset();
			CompressorManager->Release();
			CompressorManager.Reset();

			FZibraVDBCompressorCommon::DeleteTempCompressedFile(FString(OutputFile));

			CompressionEnd = FDateTime::UtcNow();
			LogCompressionEnd(CompressionEnd - CompressionStart);

			UZibraVDBVolume4* CompressedVolume = Cast<UZibraVDBVolume4>(ImportedFile);
			if (CompressedVolume)
			{
				FZibraVDBCompressionAnalytics::TrackCompression(CompressedVolume, CompressionEnd - CompressionStart, false);
			}

			CompressionState = EZibraCompressionState::Uninitialized;

			if (CachedDetailBuilder)
				CachedDetailBuilder->ForceRefreshDetails();

			break;
		}
	}
}
bool UZibraVDBSequenceImporter::IsTickableInEditor() const
{
	return true;
}

TStatId UZibraVDBSequenceImporter::GetStatId() const
{
	return TStatId();
}

UWorld* UZibraVDBSequenceImporter::GetWorld() const
{
	return GetOuter()->GetWorld();
}

void UZibraVDBSequenceImporter::BeginDestroy()
{
	Super::BeginDestroy();
	ClearMetadata();
}

UZibraVDBSequenceImporter::UZibraVDBSequenceImporter(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer)
{
}

void UZibraVDBSequenceImporter::ClearMetadata() noexcept
{
	if (SequenceInfo)
	{
		FZibraVDBSDKIntegration::ReleaseSequenceInfo(SequenceInfo);
		SequenceInfo = nullptr;
	}
}

void LogSequenceInfo(
	const Zibra::OpenVDBHelper::SequenceInfo* SequenceInfo, const FString& ChannelNamesString, const FTimespan& MetadataReadingTime)
{
	UE_LOG(LogZibraVDBMetadata, Display, TEXT(" \n\
		Originl side: %d						\n\
		Number of frames: %d					\n\
		Number of channels: %d					\n\
		Original channels: %s					\n\
		Metadata reading time: %.3f seconds		"),
		SequenceInfo->originalSize, SequenceInfo->frameCount, SequenceInfo->channelCount, *ChannelNamesString,
		MetadataReadingTime.GetTotalSeconds());
}

#define ZIB_INT_DIVIDE_CEIL(a, b) ((a + b - 1) / b)
#define ZIB_CEIL_TO_POWER_OF_TWO(intValue, powerOfTwo) ((intValue + (powerOfTwo - 1)) & (~(powerOfTwo - 1)))

uint32_t FindTheMostFrequentChannelOccurances(char** channelNames, uint32_t channelNumber)
{
	TMap<FString, uint32> Counter;

	for (uint32 i = 0; i < channelNumber; ++i)
	{
		FString channelName = FString(channelNames[i]);
		Counter.FindOrAdd(channelName, 0);
		if (!channelName.IsEmpty())
		{
			Counter[channelName]++;
		}
	}

	uint32 MostFrequent = 0;
	for (const auto& occurance : Counter)
	{
		if (occurance.Value > MostFrequent)
			MostFrequent = occurance.Value;
	}

	return MostFrequent;
}

void UZibraVDBSequenceImporter::OnCompressionFilePathPicked()
{
	MetadataReadingStart = FDateTime::UtcNow();

	PerChannelCompressionSettings.Empty();

	const FString SourceFileMask = FZibraVDBAssetEditor::GetInputFileMask(VDBFilePath);

	FZibraVDBCompressorCommon::ReadSequenceInfo(
		SourceFileMask,
		[this](Zibra::OpenVDBHelper::SequenceInfo* InSequenceInfo)
		{
			SequenceInfo = InSequenceInfo;
			OriginalVDBSize = SequenceInfo->originalSize / 1024.0f / 1024.0f;
			NumberOfChannels = SequenceInfo->channelCount;
			FramesNumber = SequenceInfo->frameCount;

			ChannelNamesString = "";
			ChannelNames.Empty();
			for (int Index = 0; Index < int(SequenceInfo->channelCount); Index++)
			{
				const FString ChannelName = SequenceInfo->channelNames[Index];

				ChannelNames.Add(MakeShared<FString>(ChannelName));

				ChannelNamesString += ChannelName;

				if (Index != (SequenceInfo->channelCount - 1))
				{
					ChannelNamesString += ", ";
				}
			}

			ChannelNamesUI = ChannelNames;
			ChannelNamesUI.Add(MakeShared<FString>("None"));

			MetadataReadingEnd = FDateTime::UtcNow();
			LogSequenceInfo(SequenceInfo, ChannelNamesString, MetadataReadingEnd - MetadataReadingStart);
		},
		[this]() noexcept { VDBFilePath.FilePath = ""; });
}

void UZibraVDBSequenceImporter::OnDefaultQualityChanged()
{
	if (bUsePerChannelCompressionSettings)
	{
		return;
	}

	for (auto& ChannelCompressionSettings : PerChannelCompressionSettings)
	{
		ChannelCompressionSettings.Quality = Quality;
	}
}

void UZibraVDBSequenceImporter::OnUsePerChannelCompressionSettingsChanged()
{
	for (auto& ChannelCompressionSettings : PerChannelCompressionSettings)
	{
		ChannelCompressionSettings.Quality = Quality;
	}
}

FReply UZibraVDBSequenceImporter::OnStartCompression()
{
	if (!FZibraLicensingManager::GetInstance().IsLicenseVerified(FZibraLicensingManager::Product::Compression))
	{
		FText ErrorText;
		static const FText ErrorTitle =
			LOCTEXT("ZibraVDBLicenseErrorTitle", "ZibraVDB License Error");
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
		return FReply::Handled();
	}

	if (!ValidateCompressionParameters())
	{
		return FReply::Handled();
	}

	CompressionStart = FDateTime::UtcNow();

	CompressionState = EZibraCompressionState::Started;

	FString SourceFileMask = FZibraVDBAssetEditor::GetInputFileMask(VDBFilePath);

	LogCompressionStart(SourceFileMask, Quality);

	InputFileMask = SourceFileMask;

	const FString IntermedateDir = FPaths::ConvertRelativePathToFull(FPaths::ProjectIntermediateDir());
	OutputFile = FPaths::CreateTempFilename(*IntermedateDir, TEXT("ZibraVDBTemp"), TEXT(".zibravdb"));

	auto IsChannelActivePredicate = [](const FCompressionSettings& Item) -> bool { return Item.bActive; };
	auto GetSettigsPairTransformFunc = [](const FCompressionSettings& Item) -> TPair<FString, float>
	{ return {Item.Name, Item.Quality}; };

	TArray<TPair<FString, float>> ActiveCompressionSettings;
	Algo::TransformIf(
		PerChannelCompressionSettings, ActiveCompressionSettings, IsChannelActivePredicate, GetSettigsPairTransformFunc);

	CompressorManager = MakeShared<FZibraVDBCompressorManager>();
	Zibra::CE::ReturnCode status = CompressorManager->Initialize(Quality, *SequenceInfo, ActiveCompressionSettings);
	if (status != Zibra::CE::ZCE_SUCCESS)
	{
		UE_LOG(LogZibraVDBCompression, Error, TEXT("Failed to initialize compressor manager."));
		return FReply::Handled();
	}

	CompressorManager->SetOutputFile(OutputFile);
	ZibraCompressionWorker = MakeUnique<FZibraCompressionWorker>(
		CompressorManager, InputFileMask, [this]() noexcept { CompressionState = EZibraCompressionState::Finished; });

	return FReply::Handled();
}

FReply UZibraVDBSequenceImporter::OnCancelCompression()
{
	// Calls ~ZibraCompressionWorker()->FZibraCompressionWorker::Stop()->FZibraVDBBridge::AbortCompression()
	ZibraCompressionWorker.Reset();

	CompressorManager->Release();

	CompressionState = EZibraCompressionState::Uninitialized;

	if (CachedDetailBuilder)
		CachedDetailBuilder->ForceRefreshDetails();

	return FReply::Handled();
}

bool UZibraVDBSequenceImporter::ValidateCompressionParameters()
{
	if (!FPaths::FileExists(VDBFilePath.FilePath))
	{
		FZibraVDBNotifications::AddNotification(LOCTEXT("FileDoesntExist", "File does not exist."));
		return false;
	}

	if (!FPackageName::IsValidPath(AssetPath.Path))
	{
		FZibraVDBNotifications::AddNotification(LOCTEXT("InvalidAssetPath", "Selected asset path is not valid."));
		return false;
	}

	const int ActiveChannelCount = Algo::CountIf(
		PerChannelCompressionSettings, [](const FCompressionSettings& CompressionSettings) { return CompressionSettings.bActive; });

	if (bUsePerChannelCompressionSettings && ActiveChannelCount == 0)
	{
		FZibraVDBNotifications::AddNotification(
			LOCTEXT("SelectAtLeastOneChannelToCompress", "Please, select at least one channel to compress"));
		return false;
	}

	return true;
}

TOptional<float> UZibraVDBSequenceImporter::GetCompressionProgressPercent()
{
	switch (CompressionState)
	{
		case EZibraCompressionState::Uninitialized:
			return TOptional<float>();
		case EZibraCompressionState::Started:
			return CompressorManager->GetCompressionProgress();
		case EZibraCompressionState::Finished:
			return 1.0f;
		default:
			return TOptional<float>();
	}
}

#undef LOCTEXT_NAMESPACE
