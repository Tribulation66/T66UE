// Copyright Zibra AI Inc 2025-2026. All Rights Reserved.

#include "ZibraVDBAssetDeprecatedEditor.h"

#include "DetailCategoryBuilder.h"
#include "DetailLayoutBuilder.h"
#include "DetailWidgetRow.h"
#include "GenericPlatform/GenericPlatformFile.h"
#include "Widgets/Images/SImage.h"
#include "Widgets/Input/SButton.h"
#include "Widgets/Layout/SBox.h"
#include "Widgets/Text/STextBlock.h"
#include "Widgets/ZibraVDBSequenceImporterWidget.h"
#include "ZibraVDBEditor.h"
#include "ZibraVDBVolumeDeprecated.h"

#define LOCTEXT_NAMESPACE "ZibraVDBForUnreal"

TSharedRef<IDetailCustomization> FZibraVDBAssetDeprecatedCustomization::MakeInstance()
{
	return MakeShared<FZibraVDBAssetDeprecatedCustomization>();
}

void FZibraVDBAssetDeprecatedCustomization::CustomizeDetails(IDetailLayoutBuilder& DetailBuilder)
{
	DetailBuilder.GetObjectsBeingCustomized(CustomizedObjects);

	// Create a category for the warning message (or you can add it to an existing category)
	IDetailCategoryBuilder& WarningCategory =
		DetailBuilder.EditCategory("Deprecation Warning", FText::GetEmpty(), ECategoryPriority::Important);

	// Optionally, you could also add a deprecation icon (e.g., a warning symbol)
	WarningCategory.AddCustomRow(LOCTEXT("DeprecationWarning", "Deprecation Warning"))
		.WholeRowContent()
			[SNew(SBox)
					.HAlign(HAlign_Center)
					.Padding(
						5)[SNew(SHorizontalBox) +
						   SHorizontalBox::Slot().AutoWidth().Padding(
							   5, 0, 5, 0)[SNew(SImage).Image(FAppStyle::GetBrush("Icons.Warning"))	   // Use warning icon
	] +
						   SHorizontalBox::Slot().FillWidth(1.0f)
							   [SNew(STextBlock)
									   .Text(FText::FromString(
										   "This asset was compressed using older ZibraVDB version. Please reimport this effect."))
									   .Font(FAppStyle::GetFontStyle("BoldFont"))
									   .ColorAndOpacity(FSlateColor(FLinearColor(1.0f, 0.5f, 0.5f)))]]];
	WarningCategory.AddCustomRow(LOCTEXT("DeprecationWarning", "Deprecation Warning"))
		.WholeRowContent()[SNew(SButton)
							   .HAlign(HAlign_Center)
							   .Text(LOCTEXT("OpenOpenVDBSequenceImportWindow", "Open OpenVDB Sequence Import Window"))
							   .OnClicked_Raw(this, &FZibraVDBAssetDeprecatedCustomization::OpenCompressionWindow)];
}
FReply FZibraVDBAssetDeprecatedCustomization::OpenCompressionWindow()
{
	if (!CustomizedObjects.IsEmpty())
	{
		auto VDBVolume = static_cast<UZibraVDBVolume*>(CustomizedObjects[0].Get());
		if (!VDBVolume->bIsLoadedFromZibraVDB)
		{
			// Search for .vdb file in SourceOpenVDBDirectory
			IPlatformFile::GetPlatformPhysical().IterateDirectory(*VDBVolume->SourceOpenVDBDirectory.Path,
				[VDBVolume](const TCHAR* FilenameOrDirectory, bool bIsDirectory) -> bool
				{
					// If .vdb file is found
					if (!bIsDirectory && FPaths::GetExtension(FilenameOrDirectory) == FString(TEXT("vdb")))
					{
						// Initialize input file in importer widget with it
						SZibraVDBSequenceImporterWidget::SetInputFile(FilenameOrDirectory);
						// And stop iteration
						return false;
					}
					// Continue iteration otherwise
					return true;
				});
		}
	}
	FZibraVDBEditorModule::OpenImportVDBSequenceWindow();
	return FReply::Handled();
}

#undef LOCTEXT_NAMESPACE
