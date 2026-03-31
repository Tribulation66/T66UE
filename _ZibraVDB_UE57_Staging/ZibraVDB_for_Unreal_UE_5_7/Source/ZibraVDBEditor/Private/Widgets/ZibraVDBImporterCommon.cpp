// Copyright Zibra AI Inc 2025-2026. All Rights Reserved.
#include "Widgets/ZibraVDBImporterCommon.h"

#include "AssetImportTask.h"
#include "AssetRegistry/AssetRegistryModule.h"
#include "AssetToolsModule.h"
#include "ContentBrowserModule.h"
#include "Editor.h"
#include "IContentBrowserSingleton.h"
#include "Misc/Char.h"
#include "ObjectTools.h"
#include "ZibraVDBEditor/Public/Importer/ZibraVDBImportFactory.h"
#include "ZibraVDBNotifications.h"
#include "ZibraVDBRuntime/Public/ZibraVDBVolume4.h"

#include <cctype>

#define LOCTEXT_NAMESPACE "ZibraVDBForUnreal"

bool FZibraVDBImporterCommon::IsDestinationAssetExists(const FString& DestinationPath, const FString& DestinationName)
{
	FAssetRegistryModule& AssetRegistryModule = FModuleManager::LoadModuleChecked<FAssetRegistryModule>("AssetRegistry");
	IAssetRegistry& AssetRegistry = AssetRegistryModule.Get();

	TArray<FAssetData> AssetsList;
	AssetRegistry.GetAssetsByPath(*DestinationPath, AssetsList);

	for (const FAssetData& CurAsset : AssetsList)
	{
		FString ExistingAssetNameWithoutExtension = FPaths::GetBaseFilename(CurAsset.AssetName.ToString());
		if (ExistingAssetNameWithoutExtension.Equals(DestinationName, ESearchCase::IgnoreCase))
		{
			return true;
		}
	}

	return false;
}

UObject* FZibraVDBImporterCommon::ImportCompressedFile(const FString& FilePath, const FString& AssetPath,
	const FString& DestinationName, bool bStripDestinationName /*= true*/,
	const TUniquePtr<FOpenVDBImportedDetails>& OpenVDBImportedDetails /*= nullptr*/, bool NotifyUser /*= true*/)
{
	if (!FPackageName::IsValidPath(AssetPath))
	{
		FZibraVDBNotifications::AddNotification(LOCTEXT("InvalidAssetPath", "Selected asset path is not valid."));
		return nullptr;
	}

	// Get the AssetTools module
	IAssetTools& AssetTools = FModuleManager::GetModuleChecked<FAssetToolsModule>("AssetTools").Get();
	// Generate a unique control asset name for this take if there are already assets of the same name
	IAssetRegistry& AssetRegistry = FModuleManager::LoadModuleChecked<FAssetRegistryModule>("AssetRegistry").Get();

	UZibraVDBImportFactory* ZibraVDBImportFactory = NewObject<UZibraVDBImportFactory>(UZibraVDBImportFactory::StaticClass());
	ZibraVDBImportFactory->AddToRoot();

	UAssetImportTask* Task = NewObject<UAssetImportTask>();
	Task->AddToRoot();
	Task->bAutomated = true;
	Task->bReplaceExisting = true;
	Task->DestinationPath = AssetPath;
	Task->bSave = false;
	auto BaseAssetName = bStripDestinationName ? StripDestinationName(DestinationName) : DestinationName;
	auto TargetName = ObjectTools::SanitizeInvalidChars(BaseAssetName, INVALID_LONGPACKAGE_CHARACTERS);
	int CopyCounter = 1;
	while (IsDestinationAssetExists(Task->DestinationPath, TargetName))
	{
		TargetName = BaseAssetName + TEXT("_") + FString::FromInt(CopyCounter++);
	}
	Task->DestinationName = TargetName;
	Task->Filename = FilePath;
	Task->Factory = ZibraVDBImportFactory;
	ZibraVDBImportFactory->SetAssetImportTask(Task);

	AssetTools.ImportAssetTasks({Task});

	const TArray<UObject*>& ImportedAssets = Task->GetObjects();
	SetCompressionDetails(ImportedAssets, OpenVDBImportedDetails);

	Task->RemoveFromRoot();
	ZibraVDBImportFactory->RemoveFromRoot();

	if (NotifyUser && !ImportedAssets.IsEmpty())
	{
		OpenContentDrawerAndFocusOnAsset(ImportedAssets[0]);
		auto StatusMessage = FText::Format(
			LOCTEXT("SuccessfullyImported", "The {0} has been successfully imported"), FText::FromString(DestinationName));
		FZibraVDBNotifications::AddNotification(StatusMessage);
	}

	return !ImportedAssets.IsEmpty() ? ImportedAssets[0] : nullptr;
}

void FZibraVDBImporterCommon::OpenContentDrawerAndFocusOnAsset(const UObject* Asset)
{
	if (GEditor)
	{
		FSoftObjectPath SoftObjectPath = FSoftObjectPath(Asset);
		FContentBrowserModule& ContentBrowserModule = FModuleManager::LoadModuleChecked<FContentBrowserModule>("ContentBrowser");
		FAssetRegistryModule& AssetRegistryModule = FModuleManager::LoadModuleChecked<FAssetRegistryModule>("AssetRegistry");
		FAssetData AssetData = AssetRegistryModule.Get().GetAssetByObjectPath(SoftObjectPath);
		if (AssetData.IsValid())
		{
			TArray<FAssetData> AssetDataList;
			AssetDataList.Add(AssetData);
			ContentBrowserModule.Get().SyncBrowserToAssets(AssetDataList);
			ContentBrowserModule.Get().FocusPrimaryContentBrowser(false);
		}
		else
		{
			UE_LOG(LogZibraVDBRuntime, Warning, TEXT("Asset not found: %s"), *SoftObjectPath.ToString());
		}
	}
}

FString FZibraVDBImporterCommon::StripDestinationName(const FString& DestinationName) noexcept
{
	const int BeforeFrameNumberIndex = DestinationName.FindLastCharByPredicate([](TCHAR C) { return !TChar<TCHAR>::IsDigit(C); });
	if (BeforeFrameNumberIndex == INDEX_NONE)
	{
		return DestinationName;
	}

	const int LastNameChar =
		DestinationName.FindLastCharByPredicate([](TCHAR C) { return !TChar<TCHAR>::IsPunct(C); }, BeforeFrameNumberIndex + 1);
	if (LastNameChar == INDEX_NONE)
	{
		return FString(BeforeFrameNumberIndex + 1, *DestinationName);
	}

	return FString(LastNameChar + 1, *DestinationName);
}

FString FZibraVDBImporterCommon::StripFrameNumber(const FString& DestinationName) noexcept
{
	const int BeforeFrameNumberIndex = DestinationName.FindLastCharByPredicate([](TCHAR C) { return !TChar<TCHAR>::IsDigit(C); });
	if (BeforeFrameNumberIndex == INDEX_NONE)
	{
		return DestinationName;
	}

	return FString(BeforeFrameNumberIndex + 1, *DestinationName);
}

void FZibraVDBImporterCommon::SetCompressionDetails(
	const TArray<UObject*>& ImportedAssets, const TUniquePtr<FOpenVDBImportedDetails>& OpenVDBImportedDetails)
{
	for (UObject* Asset : ImportedAssets)
	{
		UZibraVDBVolume4* ZibraVDBAsset = Cast<UZibraVDBVolume4>(Asset);
		if (ZibraVDBAsset == nullptr)
		{
			continue;
		}

		if (OpenVDBImportedDetails.IsValid())
		{
			ZibraVDBAsset->bIsLoadedFromZibraVDB = false;
			ZibraVDBAsset->SourceOpenVDBFilePath.FilePath = OpenVDBImportedDetails->InputFilePath;
			ZibraVDBAsset->Quality = OpenVDBImportedDetails->Quality;
			ZibraVDBAsset->bUsePerChannelCompressionSettings = OpenVDBImportedDetails->UsePerChannelCompressionSettings;
			ZibraVDBAsset->PerChannelCompressionSettings = OpenVDBImportedDetails->PerChannelCompressionSettings;
			ZibraVDBAsset->PerChannelReimportSettings = OpenVDBImportedDetails->PerChannelCompressionSettings;
		}
	}
}

#undef LOCTEXT_NAMESPACE
