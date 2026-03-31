// Copyright Zibra AI Inc 2025-2026. All Rights Reserved.

#pragma once

#include "IDetailCustomization.h"
#include "IDetailsView.h"
#include "UObject/ObjectMacros.h"
#include "ZibraVDBEditor/Public/ZibraEditorSettings.h"

#include "ZibraVDBCompressedImporterWidget.generated.h"

class IDetailLayoutBuilder;

UCLASS(BlueprintType, Config = Editor)
class ZIBRAVDBEDITOR_API UZibraVDBCompressedImporter final : public UObject
{
	GENERATED_UCLASS_BODY()
public:
	UPROPERTY(VisibleAnywhere, Category = "Importing", DisplayName = "File Path",
		meta = (NoResetToDefault, FilePathFilter = "ZibraVDB File(*.zibravdb) | *.zibravdb"))
	FFilePath ZibraVDBFilePath;

	UPROPERTY(EditAnywhere, Category = "Importing", DisplayName = "Asset Path", meta = (ContentDir, NoResetToDefault))
	FDirectoryPath AssetPath = GetDefault<UZibraEditorSettings>()->DefaultAssetPath;

	UPROPERTY(VisibleAnywhere, Category = Volume, DisplayName = "Compressed ZibraVDB size (in MBs)", meta = (NoResetToDefault))
	int ZibraVDBFileSize = 0;

	UPROPERTY(VisibleAnywhere, Category = Volume, DisplayName = "Frames Number", meta = (NoResetToDefault))
	int FramesNumber = 0;

	FReply OnImportButtonClicked();
	void OnZibraVDBFilePathPicked();
	bool ValidateImportParameters();
};

class FZibraVDBCompressedImporterCustomization final : public IDetailCustomization
{
public:
	static TSharedRef<IDetailCustomization> MakeInstance();
	void CustomizeDetails(IDetailLayoutBuilder& DetailLayout) final;
};

class SZibraVDBCompressedImporterWidget : public SCompoundWidget
{
	SLATE_BEGIN_ARGS(SZibraVDBCompressedImporterWidget)
	{
	}
	SLATE_END_ARGS()

	// Construct the widget
	void Construct(const FArguments& InArgs);

	virtual ~SZibraVDBCompressedImporterWidget();

private:
	UZibraVDBCompressedImporter* ZibraVDBCompressedImporter;

	TSharedPtr<IDetailsView> DetailsView;
};
