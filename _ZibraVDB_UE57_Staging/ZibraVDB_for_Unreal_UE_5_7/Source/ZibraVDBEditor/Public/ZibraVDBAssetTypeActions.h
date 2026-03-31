// Copyright Zibra AI Inc 2025-2026. All Rights Reserved.

#pragma once

#include "AssetTypeActions_Base.h"
#include "Subsystems/AssetEditorSubsystem.h"
#include "ZibraVDBVolume4.h"

class FZibraVDBAssetTypeActions final : public FAssetTypeActions_Base
{
public:
	FZibraVDBAssetTypeActions(EAssetTypeCategories::Type InAssetCategory);

	FText GetName() const final;
	FColor GetTypeColor() const final;
	UClass* GetSupportedClass() const final;
	uint32 GetCategories() final;
	void OpenAssetEditor(
		const TArray<UObject*>& InObjects, TSharedPtr<class IToolkitHost> EditWithinLevelEditor = TSharedPtr<IToolkitHost>()) final;
	void GetActions(const TArray<UObject*>& InObjects, struct FToolMenuSection& Section) final;
	bool IsImportedAsset() const final;
	const FSlateBrush* GetIconBrush(const FAssetData&, FName) const final;
	const FSlateBrush* GetThumbnailBrush(const FAssetData&, FName) const final;

private:
	void ExportZibraVDB(TArray<TWeakObjectPtr<UZibraVDBVolume4>> ZibraAssetsWeakPtrs);
	EAssetTypeCategories::Type ZibraVDBAssetCategory;
};
