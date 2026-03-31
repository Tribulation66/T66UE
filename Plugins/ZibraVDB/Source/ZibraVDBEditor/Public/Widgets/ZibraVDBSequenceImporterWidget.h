// Copyright Zibra AI Inc 2025-2026. All Rights Reserved.

#pragma once

#include "IDetailCustomization.h"
#include "IDetailsView.h"
#include "Tickable.h"
#include "ZibraCompressionWorker.h"
#include "ZibraVDBCompressorCommon.h"
#include "ZibraVDBEditor/Public/ZibraEditorSettings.h"

#include "ZibraVDBSequenceImporterWidget.generated.h"
class IDetailLayoutBuilder;

UCLASS(BlueprintType, Config = Editor)
class ZIBRAVDBEDITOR_API UZibraVDBSequenceImporter final : public UObject, public FTickableGameObject
{
	GENERATED_UCLASS_BODY()
public:
	UPROPERTY(VisibleAnywhere, Category = "Importing", DisplayName = "File Path",
		meta = (FilePathFilter = "OpenVDB File (*.vdb)|*.vdb", NoResetToDefault))
	FFilePath VDBFilePath;

	UPROPERTY(EditAnywhere, Category = "Importing", DisplayName = "Asset Path", meta = (ContentDir))
	FDirectoryPath AssetPath = GetDefault<UZibraEditorSettings>()->DefaultAssetPath;

	UPROPERTY(VisibleAnywhere, Category = "OpenVDB Sequence Attributes", DisplayName = "Size Of Original VDB (MiB)",
		meta = (NoResetToDefault))
	float OriginalVDBSize = 0.0f;

	UPROPERTY(VisibleAnywhere, Category = "OpenVDB Sequence Attributes", meta = (NoResetToDefault))
	int NumberOfChannels = 0;
	UPROPERTY(VisibleAnywhere, Category = "OpenVDB Sequence Attributes", meta = (NoResetToDefault))
	int FramesNumber = 0;
	UPROPERTY(VisibleAnywhere, Category = "OpenVDB Sequence Attributes", DisplayName = "Channel Names", meta = (NoResetToDefault))
	FString ChannelNamesString = "";

	/** Sets compression quality. Larger values result in larger file size, but more accurate compression result. */
	UPROPERTY(EditAnywhere, Category = "Compression Settings",
		meta = (NoResetToDefault, ClampMin = 0.0f, ClampMax = 1.0f, UIMin = 0.0f, UIMax = 1.0f))
	float Quality = 0.6f;

	UPROPERTY(EditAnywhere, Category = "Compression Settings", meta = (NoResetToDefault))
	bool bUsePerChannelCompressionSettings = false;

	UPROPERTY(EditAnywhere, Category = "Compression Settings", meta = (NoResetToDefault))
	bool PerChannelCompressionSettingsPlaceholder;	// Placeholder

public:
	void Tick(float DeltaTime) final;
	bool IsTickableInEditor() const final;
	TStatId GetStatId() const final;
	UWorld* GetWorld() const final;
	void BeginDestroy() final;

private:
	FReply OnStartCompression();
	FReply OnCancelCompression();

	bool ValidateCompressionParameters();
	TOptional<float> GetCompressionProgressPercent();
	void OnCompressionFilePathPicked();
	void OnDefaultQualityChanged();
	void OnUsePerChannelCompressionSettingsChanged();
	void ClearMetadata() noexcept;

	friend class FZibraVDBSequenceImporterCustomization;
	friend class SZibraVDBSequenceImporterWidget;
	TArray<TSharedPtr<FString>> ChannelNames;
	TArray<TSharedPtr<FString>> ChannelNamesUI;

	FDateTime MetadataReadingStart;
	FDateTime MetadataReadingEnd;
	FDateTime CompressionStart;
	FDateTime CompressionEnd;

	EZibraCompressionState CompressionState = EZibraCompressionState::Uninitialized;
	TUniquePtr<FZibraCompressionWorker> ZibraCompressionWorker;
	Zibra::OpenVDBHelper::SequenceInfo* SequenceInfo;
	FString InputFileMask;
	FString OutputFile;
	TSharedPtr<FZibraVDBCompressorManager> CompressorManager;
	TArray<FCompressionSettings> PerChannelCompressionSettings;

	IDetailLayoutBuilder* CachedDetailBuilder;
};

class FZibraVDBSequenceImporterCustomization final : public IDetailCustomization
{
public:
	static TSharedRef<IDetailCustomization> MakeInstance();
	void CustomizeDetails(IDetailLayoutBuilder& DetailLayout) final;
};

class SZibraVDBSequenceImporterWidget : public SCompoundWidget
{
	SLATE_BEGIN_ARGS(SZibraVDBSequenceImporterWidget)
	{
	}
	SLATE_END_ARGS()

public:
	void Construct(const FArguments& InArgs);

	virtual ~SZibraVDBSequenceImporterWidget();

	// Can be used to open the import window with specified file pre-selected.
	// After opening window, automatically resets
	static void SetInputFile(const FString& InFilePath);

private:
	UZibraVDBSequenceImporter* ZibraVDBSequenceImporter;
	TSharedPtr<IDetailsView> DetailsView;
	static FString InputFilePath;
};
