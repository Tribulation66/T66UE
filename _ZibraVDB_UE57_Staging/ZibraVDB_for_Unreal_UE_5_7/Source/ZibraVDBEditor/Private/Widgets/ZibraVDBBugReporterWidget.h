// Copyright Zibra AI Inc 2025-2026. All Rights Reserved.

#pragma once

#include "IDetailCustomization.h"
#include "IDetailsView.h"
#include "HttpFwd.h"

#include "ZibraVDBBugReporterWidget.generated.h"

USTRUCT()
struct FBugReportRequest
{
	GENERATED_BODY()

	UPROPERTY()
	FString description;

	UPROPERTY()
	FString log;

	UPROPERTY()
	FString diagnostics_info;

	UPROPERTY()
	FString license_key;
};

UCLASS(BlueprintType, Config = Editor)
class ZIBRAVDBEDITOR_API UZibraVDBBugReporter final : public UObject
{
	GENERATED_UCLASS_BODY()
public:
	UPROPERTY()
	FString Description = "";

	UPROPERTY()
	bool bIncludeLog = true;

	void SubmitBugReport();

private:
	static const FString BugReportURL;
	static const size_t MaxLogSize;
	static const size_t MaxDescriptionSize;
	bool ValidateInput();
	void FillRequestBody(FHttpRequestRef Request);
	static FString GetOutputLog();

	static void GetBugReportResponse(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bWasSuccessful);
	FString GetDescription();
};

class FZibraVDBBugReporterCustomization final : public IDetailCustomization
{
public:
	static TSharedRef<IDetailCustomization> MakeInstance();
	void CustomizeDetails(IDetailLayoutBuilder& DetailBuilder) final;
};

class SZibraVDBBugReporterWidget : public SCompoundWidget
{
	SLATE_BEGIN_ARGS(SZibraVDBBugReporterWidget)
	{
	}
	SLATE_END_ARGS()

public:
	void Construct(const FArguments& InArgs);

	virtual ~SZibraVDBBugReporterWidget();

private:
	UZibraVDBBugReporter* ZibraVDBBugReporter;
	TSharedPtr<IDetailsView> DetailsView;
};

