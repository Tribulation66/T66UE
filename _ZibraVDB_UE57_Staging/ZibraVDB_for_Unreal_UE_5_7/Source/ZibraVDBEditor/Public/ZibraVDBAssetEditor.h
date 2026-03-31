// Copyright Zibra AI Inc 2025-2026. All Rights Reserved.

#pragma once

#include "EditorStyleSet.h"
#include "IDetailCustomization.h"
#include "Toolkits/AssetEditorToolkit.h"
#include "Toolkits/IToolkitHost.h"
#include "Widgets/ZibraVDBCompressorCommon.h"
#include "ZibraCompressionWorker.h"
#include "ZibraVDBFile.h"
#include "ZibraVDBVolume4.h"

struct FSequenceInfo
{
	uint64_t OriginalSize;
	uint32_t FrameCount;
	uint32_t ChannelCount;
	char** ChannelNames;
};

class FZibraVDBAssetEditor final : public FAssetEditorToolkit, public FGCObject
{
public:
	inline static FName ZibraVDBAssetDetailsID = TEXT("ZibraVDB Details");
	inline static FName ZibraVDBAssetEditorName = TEXT("ZibraVDBAssetEditor");

	~FZibraVDBAssetEditor();

	//~ Begin FAssetEditorToolkit Interface.

	void RegisterTabSpawners(const TSharedRef<class FTabManager>& InTabManager) final;
	void UnregisterTabSpawners(const TSharedRef<class FTabManager>& InTabManager) final;

	//~ End FAssetEditorToolkit Interface.

	void CreateInternalWidgets();
	TSharedRef<SDockTab> SpawnTab_Details(const FSpawnTabArgs& Args);

	// This function will initialize the editor
	void InitZibraVDBAssetEditor(const EToolkitMode::Type Mode, const TSharedPtr<IToolkitHost>& InitToolkitHost,
		TObjectPtr<UZibraVDBVolume4> ZibraVDBVolumeAsset);

	// FAssetEditorToolkit overrides
	FName GetToolkitFName() const final;
	FText GetBaseToolkitName() const final;
	FText GetToolkitName() const final;
	FText GetToolkitToolTipText() const final;
	FLinearColor GetWorldCentricTabColorScale() const final;
	FString GetWorldCentricTabPrefix() const final;

	// FGCObject overrides
	void AddReferencedObjects(FReferenceCollector& Collector) final;
	FString GetReferencerName() const final;

	void BindCommands();
	void ExtendToolbar();

	void ReimportAsset();
	void ReimportZibraVDB();
	void ReimportOpenVDB();
	void ExportZibraVDB();
	static void ExportZibraVDB(const ZibraVDB::FZibraVDBFile& ZibraVDBFile);

	static FString GetInputFileMask(const FFilePath& FilePath);

private:
	void ReadSequenceInfo();
	void OnSourceOpenVDBFilePathPicked();
	void OnDefaultQualityChanged();
	void OnUsePerChannelCompressionSettingsChanged();

	TObjectPtr<class UZibraVDBVolume4> CurrentZibraVDBAsset;
	Zibra::OpenVDBHelper::SequenceInfo* SequenceInfo;
	TSharedPtr<IDetailsView> DetailsView;
	EZibraCompressionState CompressionState = EZibraCompressionState::Uninitialized;
	TUniquePtr<FZibraCompressionWorker> ZibraCompressionWorker;
};

class FZibraVDBAssetEditorCustomization : public IDetailCustomization
{
public:
	static TSharedRef<IDetailCustomization> MakeInstance();
	void CustomizeDetails(IDetailLayoutBuilder& DetailBuilder) final;
};
