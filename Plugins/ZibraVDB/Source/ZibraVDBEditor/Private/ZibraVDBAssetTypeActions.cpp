// Copyright Zibra AI Inc 2025-2026. All Rights Reserved.

#include "ZibraVDBAssetTypeActions.h"

#include "ToolMenuSection.h"
#include "ZibraVDBRuntime/Public/ZibraVDBVolume4.h"
#include "ZibraVDBAssetEditor.h"
#include "ZibraModuleStyle.h"

#define LOCTEXT_NAMESPACE "ZibraVDBForUnreal"

FZibraVDBAssetTypeActions::FZibraVDBAssetTypeActions(EAssetTypeCategories::Type InAssetCategory)
	: ZibraVDBAssetCategory(InAssetCategory)
{
}

FText FZibraVDBAssetTypeActions::GetName() const
{
	return LOCTEXT("ZibraVDBAssetTypeActions", "ZibraVDB Asset");
}
FColor FZibraVDBAssetTypeActions::GetTypeColor() const
{
	return FColor(150, 200, 250);
}
UClass* FZibraVDBAssetTypeActions::GetSupportedClass() const
{
	return UZibraVDBVolume4::StaticClass();
}
uint32 FZibraVDBAssetTypeActions::GetCategories()
{
	return ZibraVDBAssetCategory;
}

void FZibraVDBAssetTypeActions::OpenAssetEditor(
	const TArray<UObject*>& InObjects, TSharedPtr<class IToolkitHost> EditWithinLevelEditor)
{
	EToolkitMode::Type Mode = EditWithinLevelEditor.IsValid() ? EToolkitMode::WorldCentric : EToolkitMode::Standalone;

	for (auto ObjIt = InObjects.CreateConstIterator(); ObjIt; ++ObjIt)
	{
		auto MyAsset = Cast<UZibraVDBVolume4>(*ObjIt);
		if (MyAsset != nullptr)
		{
			//  Create a new instance of your custom editor
			TSharedRef<FZibraVDBAssetEditor> NewEditor = MakeShared<FZibraVDBAssetEditor>();
			NewEditor->InitZibraVDBAssetEditor(Mode, EditWithinLevelEditor, MyAsset);
		}
	}
}

void FZibraVDBAssetTypeActions::GetActions(const TArray<UObject*>& InObjects, struct FToolMenuSection& Section)
{
	FText ButtonLabel = LOCTEXT("ExportZibraVDBFile", "Export .zibravdb file");
	FText ButtonToolTip = LOCTEXT("ExportZibraVDBFileTooltip", "Export asset to .zibravdb file");

	auto ZibraVolumeAssets = GetTypedWeakObjectPtrs<UZibraVDBVolume4>(InObjects);
	auto TDelegateExecuteAction = FExecuteAction::CreateSP(this, &FZibraVDBAssetTypeActions::ExportZibraVDB, ZibraVolumeAssets);
	auto UIAction = FUIAction(TDelegateExecuteAction);

	Section.AddMenuEntry(FName("ZibraVDBExport"), ButtonLabel, ButtonToolTip, FSlateIcon(FAppStyle::GetAppStyleSetName(), "DataTableEditor.Copy"), UIAction);
}

bool FZibraVDBAssetTypeActions::IsImportedAsset() const
{
	return true;
}

const FSlateBrush* FZibraVDBAssetTypeActions::GetIconBrush(const FAssetData&, FName) const
{
	return FZibraModuleStyle::Get().GetBrush("ClassIcon.ZibraVDBAsset");
}

const FSlateBrush* FZibraVDBAssetTypeActions::GetThumbnailBrush(const FAssetData&, FName) const
{
	return FZibraModuleStyle::Get().GetBrush("ClassThumbnail.ZibraVDBAsset");
}

void FZibraVDBAssetTypeActions::ExportZibraVDB(TArray<TWeakObjectPtr<UZibraVDBVolume4>> ZibraAssetsWeakPtrs)
{
	if (ZibraAssetsWeakPtrs.Num() > 0 && ZibraAssetsWeakPtrs[0].IsValid())
	{
		FZibraVDBAssetEditor::ExportZibraVDB(ZibraAssetsWeakPtrs[0]->GetZibraVDBFile());
	}
}

#undef LOCTEXT_NAMESPACE
