// Copyright Zibra AI Inc 2025-2026. All Rights Reserved.

#include "Widgets/ZibraVDBCompressorCommon.h"

#include "DetailLayoutBuilder.h"
#include "HAL/FileManager.h"
#include "Misc/Paths.h"
#include "Misc/ScopedSlowTask.h"
#include "Widgets/Input/SSpinBox.h"
#include "Widgets/ZibraVDBImporterCommon.h"
#include "Windows/WindowsPlatformProcess.h"
#include "ZibraReadMetadataWorker.h"
#include "ZibraVDBRuntime/Public/ZibraVDBNotifications.h"
#include "ZibraVDBSDKIntegration.h"

#define LOCTEXT_NAMESPACE "ZibraVDBForUnreal"

void FZibraVDBCompressorCommon::DeleteTempCompressedFile(const FString& FilePath)
{
	IFileManager::Get().Delete(*FilePath);
}

EMetadataReadingState FZibraVDBCompressorCommon::ReadSequenceInfo(const FString& InputFileMask,
	TFunction<void(Zibra::OpenVDBHelper::SequenceInfo*)> InCompletionCallback, TFunction<void()> InErrorCallback)
{
	EMetadataReadingState MetadataReadingState = EMetadataReadingState::Running;

	std::vector<std::filesystem::path> FilePaths = FZibraVDBSDKIntegration::CalculateFileList(InputFileMask);

	TUniquePtr<FZibraReadMetadataWorker> ZibraReadMetadataWorker = MakeUnique<FZibraReadMetadataWorker>(FilePaths,
		[&](Zibra::OpenVDBHelper::SequenceInfo* SequenceInfo)
		{
			if (SequenceInfo == nullptr)
			{
				InErrorCallback();
				MetadataReadingState = EMetadataReadingState::Error;
				return;
			}
			InCompletionCallback(SequenceInfo);
			MetadataReadingState = EMetadataReadingState::Finished;
		});

	FScopedSlowTask SlowTask(1.0f, LOCTEXT("ReadingMetadata", "Reading metadata..."));
	SlowTask.MakeDialog();

	float PreviousProgress = 0.0;
	while (MetadataReadingState == EMetadataReadingState::Running)
	{
		const auto Progress = ZibraReadMetadataWorker->Progress;
		const auto DeltaProgress = FMath::Clamp(Progress - PreviousProgress, 0.0f, 1.0f);
		SlowTask.EnterProgressFrame(DeltaProgress);

		PreviousProgress = Progress;

		FPlatformProcess::Sleep(0.1);
	}

	if (MetadataReadingState == EMetadataReadingState::Error)
	{
		FZibraVDBNotifications::AddNotification(FText::Format(
			LOCTEXT("OpenVDBMetadataReadingFailure", "Error reading metadata from OpenVDB sequence {0}"), FText::FromString(InputFileMask)));
	}

	return MetadataReadingState;
}

TSharedRef<SHorizontalBox> FZibraVDBCompressorCommon::ConstructPerChannelCompressionSettingsBox(
	TArray<FCompressionSettings>& PerChannelCompressionSettings, float DefaultQuality)
{
	TSharedRef<SHorizontalBox> AdditionalParameters = SNew(SHorizontalBox);

	TSharedPtr<SVerticalBox> ChannelNamesBox, NeedToCompressBox, QualityBox;

	AdditionalParameters->AddSlot().MaxWidth(
		70)[SNew(SVerticalBox) + SVerticalBox::Slot()[SAssignNew(ChannelNamesBox, SVerticalBox)]];
	AdditionalParameters->AddSlot().MaxWidth(
		70)[SNew(SVerticalBox) + SVerticalBox::Slot()[SAssignNew(NeedToCompressBox, SVerticalBox)]];
	AdditionalParameters->AddSlot().MaxWidth(100)[SNew(SVerticalBox) + SVerticalBox::Slot()[SAssignNew(QualityBox, SVerticalBox)]];

	ChannelNamesBox->AddSlot().FillHeight(25).Padding(
		0, 5)[SNew(STextBlock).Font(IDetailLayoutBuilder::GetDetailFont()).Text(LOCTEXT("Channel", "Channel"))];
	NeedToCompressBox->AddSlot().FillHeight(25).Padding(5).HAlign(EHorizontalAlignment::HAlign_Center)
		[SNew(STextBlock).Font(IDetailLayoutBuilder::GetDetailFont()).Text(LOCTEXT("Import", "Import"))];
	QualityBox->AddSlot().FillHeight(25).Padding(5).HAlign(EHorizontalAlignment::HAlign_Center)
		[SNew(STextBlock).Font(IDetailLayoutBuilder::GetDetailFont()).Text(LOCTEXT("Quality", "Quality"))];

	for (int ChannelIndex = 0; ChannelIndex < PerChannelCompressionSettings.Num(); ++ChannelIndex)
	{
		ChannelNamesBox->AddSlot().FillHeight(25).Padding(
			0, 5)[SNew(STextBlock)
					  .Font(IDetailLayoutBuilder::GetDetailFont())
					  .Text(FText::FromString(PerChannelCompressionSettings[ChannelIndex].Name))];
		NeedToCompressBox->AddSlot().FillHeight(25).Padding(5).HAlign(
			EHorizontalAlignment::HAlign_Center)[SNew(SCheckBox)
													 .IsChecked(PerChannelCompressionSettings[ChannelIndex].bActive)
													 .OnCheckStateChanged_Lambda(
														 [ChannelIndex, &PerChannelCompressionSettings](ECheckBoxState NewState) {
															 PerChannelCompressionSettings[ChannelIndex].bActive =
																 NewState == ECheckBoxState::Checked;
														 })];
		QualityBox->AddSlot().FillHeight(40).Padding(
			5)[SNew(SSpinBox<float>)
				   .MinValue(0.0f)
				   .MaxValue(1.0f)
				   .Font(IDetailLayoutBuilder::GetDetailFont())
				   .Value(PerChannelCompressionSettings[ChannelIndex].Quality)
				   .OnValueCommitted_Lambda([ChannelIndex, &PerChannelCompressionSettings](float Value, ETextCommit::Type)
					   { PerChannelCompressionSettings[ChannelIndex].Quality = Value; })
				   .IsEnabled_Lambda(
					   [ChannelIndex, &PerChannelCompressionSettings]() {
						   return ChannelIndex < PerChannelCompressionSettings.Num()
									  ? PerChannelCompressionSettings[ChannelIndex].bActive
									  : false;
					   })];
	}
	return AdditionalParameters;
}

#undef LOCTEXT_NAMESPACE
