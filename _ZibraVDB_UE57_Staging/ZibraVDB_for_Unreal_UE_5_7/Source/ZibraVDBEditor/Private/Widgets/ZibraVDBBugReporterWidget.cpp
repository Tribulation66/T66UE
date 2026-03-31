// Copyright Zibra AI Inc 2025-2026. All Rights Reserved.
#include "ZibraVDBBugReporterWidget.h"

#include "DetailCategoryBuilder.h"
#include "DetailLayoutBuilder.h"
#include "DetailWidgetRow.h"
#include "GenericPlatform/GenericPlatformOutputDevices.h"
#include "HttpModule.h"
#include "JsonObjectConverter.h"
#include "Misc/FileHelper.h"
#include "Modules/ModuleManager.h"
#include "Widgets/Input/SButton.h"
#include "Widgets/Input/SEditableTextBox.h"
#include "Widgets/Input/SMultiLineEditableTextBox.h"
#include "Widgets/Text/STextBlock.h"
#include "ZibraLicensingManager.h"
#include "ZibraVDBDiagnosticsInfo.h"
#include "ZibraVDBNotifications.h"

#define LOCTEXT_NAMESPACE "ZibraVDBForUnreal"

const FString UZibraVDBBugReporter::BugReportURL = "https://analytics.zibra.ai/api/bugReport";
const size_t UZibraVDBBugReporter::MaxLogSize = 5 * 1024 * 1024;	// 5 MiB
const size_t UZibraVDBBugReporter::MaxDescriptionSize = 2000;

UZibraVDBBugReporter::UZibraVDBBugReporter(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer)
{
}

void UZibraVDBBugReporter::SubmitBugReport()
{
	if (!ValidateInput())
		return;

	FZibraVDBNotifications::AddNotification(LOCTEXT("SendingBugReport", "Sending Bug Report..."));

	FHttpRequestRef Request = FHttpModule::Get().CreateRequest();

	Request->SetVerb("POST");
	Request->SetURL(BugReportURL);
	Request->SetHeader(TEXT("User-Agent"), TEXT("X-UnrealEngine-Agent"));
	Request->SetHeader(TEXT("Content-Type"), TEXT("application/json"));
	Request->SetHeader(TEXT("Accepts"), TEXT("application/json"));
	FillRequestBody(Request);
	Request->OnProcessRequestComplete().BindStatic(&GetBugReportResponse);
	Request->ProcessRequest();
}

bool UZibraVDBBugReporter::ValidateInput()
{
	if (Description.IsEmpty())
	{
		FZibraVDBNotifications::AddNotification(
			LOCTEXT("PleaseEnterBugDescription", "Please enter bug description before submitting bug report."));
		return false;
	}
	return true;
}

void UZibraVDBBugReporter::FillRequestBody(FHttpRequestRef Request)
{
	TSharedPtr<FJsonObject> RequestBody = MakeShared<FJsonObject>();

	RequestBody->SetStringField("description", GetDescription());
	RequestBody->SetStringField("diagnostics_info", FZibraVDBDiagnosticsInfo::CollectInfoString());
	RequestBody->SetStringField("log", bIncludeLog ? GetOutputLog() : TEXT("User opted out of providing log"));
	if (FZibraLicensingManager::GetInstance().IsAnyLicenseVerified())
	{
		RequestBody->SetStringField("license_key", FString(FZibraLicensingManager::GetInstance().GetLicenseKey()));
	}

	FString RequestBodyString;
	TSharedRef<TJsonWriter<>> Writer = TJsonWriterFactory<>::Create(&RequestBodyString);
	FJsonSerializer::Serialize(RequestBody.ToSharedRef(), Writer);
	Request->SetContentAsString(RequestBodyString);
}

FString UZibraVDBBugReporter::GetOutputLog()
{
	FString LogFile = FGenericPlatformOutputDevices::GetAbsoluteLogFilename();
	if (LogFile.IsEmpty())
	{
		return {};
	}

	FOutputDevice* Log = FGenericPlatformOutputDevices::GetLog();
	if (!Log)
	{
		return {};
	}
	Log->Flush();

	FString LogContent;
	if (!FFileHelper::LoadFileToString(LogContent, *LogFile, FFileHelper::EHashOptions::None, 4))
	{
		return {};
	}

	if (LogContent.Len() > MaxLogSize)
	{
		LogContent = LogContent.Left(MaxLogSize);
	}

	return LogContent;
}

void UZibraVDBBugReporter::GetBugReportResponse(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bWasSuccessful)
{
	if (!bWasSuccessful || !Response.IsValid())
	{
		FZibraVDBNotifications::AddNotification(LOCTEXT("FailedToSendBugReportNetworkError",
			"Failed to send bug report. Network request failed. Please check your internet connection."));
		return;
	}

	if (Response->GetResponseCode() != 200)
	{
		FZibraVDBNotifications::AddNotification(
			FText::Format(LOCTEXT("FailedToSendBugReportServerError",
							  "Failed to send bug report. Server responded with an error code {0}. Response: {1}."),
				Response->GetResponseCode(), FText::FromString(Response->GetContentAsString())));
		return;
	}

	FZibraVDBNotifications::AddNotification(LOCTEXT("BugReportSentSuccessfully", "Bug report was sent successfully."));
}

FString UZibraVDBBugReporter::GetDescription()
{
	if (Description.Len() > MaxDescriptionSize)
	{
		Description = Description.Left(MaxDescriptionSize);
	}
	return Description;
}

TSharedRef<IDetailCustomization> FZibraVDBBugReporterCustomization::MakeInstance()
{
	return MakeShareable(new FZibraVDBBugReporterCustomization);
}

void FZibraVDBBugReporterCustomization::CustomizeDetails(IDetailLayoutBuilder& DetailBuilder)
{
	// Get importer instanse.
	UZibraVDBBugReporter* ZibraVDBBugReporter = nullptr;
	TArray<TWeakObjectPtr<UObject>> CustomizedObjects;
	DetailBuilder.GetObjectsBeingCustomized(CustomizedObjects);
	for (TWeakObjectPtr<UObject> Object : CustomizedObjects)
	{
		if (!Object.IsValid())
		{
			continue;
		}
		UZibraVDBBugReporter* Importer = Cast<UZibraVDBBugReporter>(Object);
		if (Importer)
		{
			ZibraVDBBugReporter = Importer;
			break;
		}
	}

	if (!ZibraVDBBugReporter)
	{
		return;
	}

	IDetailCategoryBuilder& BugReportCategory =
		DetailBuilder.EditCategory("Bug report", FText::GetEmpty(), ECategoryPriority::Uncommon);

	BugReportCategory.AddCustomRow(LOCTEXT("BugDescription", "Bug Description"))
		.NameContent()[SNew(STextBlock).Text(LOCTEXT("BugDescription", "Bug Description"))]
		.ValueContent()[SNew(SMultiLineEditableTextBox)
				.Text(LOCTEXT("BugDescriptionPlaceholder",
					"Please enter bug description here.\nMake sure to include all information relevant to reproducing the bug."))
				.SelectAllTextWhenFocused(true)
				.Margin(FMargin(0.0f, 4.0f, 0.0f, 0.0f))
				.OnTextChanged_Lambda(
					[ZibraVDBBugReporter](const FText& Text) { ZibraVDBBugReporter->Description = Text.ToString(); })];

	BugReportCategory.AddCustomRow(LOCTEXT("IncludeEditorLog", "Include editor log"))
		.NameContent()[SNew(STextBlock).Text(LOCTEXT("IncludeEditorLog", "Include editor log"))]
		.ValueContent()[SNew(SCheckBox).IsChecked(true).OnCheckStateChanged_Lambda([ZibraVDBBugReporter](ECheckBoxState newState)
			{ ZibraVDBBugReporter->bIncludeLog = newState == ECheckBoxState::Checked; })];

	BugReportCategory.AddCustomRow(LOCTEXT("", ""))
		.NameContent()[SNew(STextBlock).Text(LOCTEXT("", ""))]
		.ValueContent()[SNew(STextBlock)
				.Text(LOCTEXT("IncludeEditorLogNote", "Note that editor log may include certain information about your project."))];

	FTextBlockStyle ButtonTextStyle = FTextBlockStyle::GetDefault();
	ButtonTextStyle.SetColorAndOpacity(FLinearColor::White);
	ButtonTextStyle.SetFontSize(12.0f);

	BugReportCategory.AddCustomRow(LOCTEXT("Submit", "Submit"))
		.WholeRowWidget[SNew(SVerticalBox) + SVerticalBox::Slot().AutoHeight().Padding(5.0f)[SNew(SButton)
													 .TextStyle(&ButtonTextStyle)
													 .HAlign(HAlign_Center)
													 .Text(LOCTEXT("Submit", "Submit"))
													 .OnClicked_Lambda(
														 [ZibraVDBBugReporter]() -> FReply
														 {
															 ZibraVDBBugReporter->SubmitBugReport();
															 return FReply::Handled();
														 })]];
}

void SZibraVDBBugReporterWidget::Construct(const FArguments& InArgs)
{
	ZibraVDBBugReporter = NewObject<UZibraVDBBugReporter>();
	ZibraVDBBugReporter->AddToRoot();

	FDetailsViewArgs DetailsViewArgs;
	DetailsViewArgs.bAllowSearch = false;
	DetailsViewArgs.bHideSelectionTip = true;
	DetailsViewArgs.NameAreaSettings = FDetailsViewArgs::HideNameArea;
	DetailsViewArgs.bShowOptions = false;
	DetailsViewArgs.bShowModifiedPropertiesOption = false;
	DetailsViewArgs.bShowObjectLabel = false;

	DetailsView = FModuleManager::LoadModuleChecked<FPropertyEditorModule>("PropertyEditor").CreateDetailView(DetailsViewArgs);
	DetailsView->SetObject(ZibraVDBBugReporter);

	ChildSlot[DetailsView.ToSharedRef()];
}

SZibraVDBBugReporterWidget::~SZibraVDBBugReporterWidget()
{
	if (ZibraVDBBugReporter && ZibraVDBBugReporter->IsValidLowLevel())
	{
		ZibraVDBBugReporter->RemoveFromRoot();
	}
}

#undef LOCTEXT_NAMESPACE
