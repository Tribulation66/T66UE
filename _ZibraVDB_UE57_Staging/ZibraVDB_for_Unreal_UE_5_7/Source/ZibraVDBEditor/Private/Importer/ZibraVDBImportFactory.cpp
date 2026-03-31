// Copyright Zibra AI Inc 2025-2026. All Rights Reserved.

#include "Importer/ZibraVDBImportFactory.h"

#include "AssetImportTask.h"
#include "Editor/UnrealEd/Public/Editor.h"
#include "EditorFramework/AssetImportData.h"
#include "EngineUtils.h"
#include "HAL/FileManager.h"
#include "Interfaces/IMainFrameModule.h"
#include "Misc/FeedbackContext.h"
#include "Misc/Paths.h"
#include "Misc/ScopedSlowTask.h"
#include "Modules/ModuleManager.h"
#include "Subsystems/ImportSubsystem.h"
#include "UObject/StrongObjectPtr.h"
#include "UObject/UObjectGlobals.h"
#include "ZibraVDBRuntime/Public/ZibraVDBActor.h"
#include "ZibraVDBRuntime/Public/ZibraVDBAssetComponent.h"
#include "ZibraVDBRuntime/Public/ZibraVDBCommon.h"
#include "ZibraVDBRuntime/Public/ZibraVDBFile.h"
#include "ZibraVDBRuntime/Public/ZibraVDBVolume4.h"

DEFINE_LOG_CATEGORY_STATIC(LogZibraVDBImporter, Log, All);

#define LOCTEXT_NAMESPACE "ZibraVDBForUnreal"

#if WITH_EDITORONLY_DATA

UZibraVDBImportFactory::UZibraVDBImportFactory(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer)
{
	bCreateNew = false;
	bEditAfterNew = false;
	bEditorImport = true;
	bText = false;

	SupportedClass = UZibraVDBVolume4::StaticClass();

	Formats.Add(TEXT("zibravdb;ZibraVDB format"));
}

bool UZibraVDBImportFactory::DoesSupportClass(UClass* Class)
{
	return Class == UZibraVDBVolume4::StaticClass();
}

UClass* UZibraVDBImportFactory::ResolveSupportedClass()
{
	return UZibraVDBVolume4::StaticClass();
}

UObject* UZibraVDBImportFactory::FactoryCreateFile(UClass* InClass, UObject* InParent, FName InName, EObjectFlags Flags,
	const FString& Filename, const TCHAR* Parms, FFeedbackContext* Warn, bool& bOutOperationCanceled)
{
	GEditor->GetEditorSubsystem<UImportSubsystem>()->BroadcastAssetPreImport(this, InClass, InParent, InName, Parms);
	FString Extension = FPaths::GetExtension(Filename, false);
	if (Extension == "zibravdb")
	{
		UZibraVDBVolume4* Vol = NewObject<UZibraVDBVolume4>(InParent, UZibraVDBVolume4::StaticClass(), InName, Flags);
		ZibraVDB::FZibraVDBFile File{};
		bool result = File.ReadFromFile(Filename);
		if (!result)
		{
			return nullptr;
		}

		Vol->Import(MoveTemp(File));
		return Vol;
	}
	return nullptr;
}

bool UZibraVDBImportFactory::FactoryCanImport(const FString& Filename)
{
	const FString Extension = FPaths::GetExtension(Filename);
	return Extension == TEXT("zibravdb");
}

bool UZibraVDBImportFactory::CanReimport(UObject* Obj, TArray<FString>& OutFilenames)
{
	if (UZibraVDBVolume4* ZibraVDBVolume = Cast<UZibraVDBVolume4>(Obj))
	{
		if (ZibraVDBVolume->AssetImportData != nullptr)
		{
			ZibraVDBVolume->AssetImportData->ExtractFilenames(OutFilenames);
			return true;
		}
	}
	return false;
}

void UZibraVDBImportFactory::SetReimportPaths(UObject* Obj, const TArray<FString>& NewReimportPaths)
{
	UZibraVDBVolume4* ZibraVDBVolume = Cast<UZibraVDBVolume4>(Obj);
	if (ZibraVDBVolume && ZibraVDBVolume->AssetImportData && !NewReimportPaths.IsEmpty())
	{
		ZibraVDBVolume->AssetImportData->UpdateFilenameOnly(NewReimportPaths[0]);
	}
}

EReimportResult::Type UZibraVDBImportFactory::Reimport(UObject* Obj)
{
	UZibraVDBVolume4* ZibraVDBVolume = Cast<UZibraVDBVolume4>(Obj);
	if (!ZibraVDBVolume || !ZibraVDBVolume->AssetImportData)
	{
		return EReimportResult::Failed;
	}

	// Make sure file is valid and exists
	const FString Filename = ZibraVDBVolume->AssetImportData->GetFirstFilename();
	if (!Filename.Len() || IFileManager::Get().FileSize(*Filename) == INDEX_NONE)
	{
		return EReimportResult::Failed;
	}

	// Run the import again
	EReimportResult::Type Result = EReimportResult::Failed;
	bool OutCanceled = false;

	UObject* ReimportedObject = ImportObject(ZibraVDBVolume->GetClass(), ZibraVDBVolume->GetOuter(), *ZibraVDBVolume->GetName(),
		RF_Public | RF_Standalone, Filename, nullptr, OutCanceled);
	UZibraVDBVolume4* ReimportedZibraVDBVolume = Cast<UZibraVDBVolume4>(ReimportedObject);

	if (ReimportedObject && ReimportedZibraVDBVolume)
	{
		UE_LOG(LogZibraVDBImporter, Log, TEXT("ZibraVDBVolume imported successfully"));

		ZibraVDBVolume->AssetImportData->Update(Filename);

		// Try to find the outer package so we can dirty it up
		if (ZibraVDBVolume->GetOuter())
		{
			ZibraVDBVolume->GetOuter()->MarkPackageDirty();
		}
		else
		{
			ZibraVDBVolume->MarkPackageDirty();
		}

		// Refresh all the component that depends on this asset.
		for (TActorIterator<AZibraVDBActor> It(GWorld); It; ++It)
		{
			AZibraVDBActor* Actor = *It;
			if (Actor && Actor->GetAssetComponent()->GetZibraVDBVolume() == ZibraVDBVolume)
			{
				Actor->UnregisterAllComponents();
				Actor->GetAssetComponent()->OnZibraVDBVolumeReimport(ReimportedZibraVDBVolume);
				Actor->RegisterAllComponents();
			}
		}

		Result = EReimportResult::Succeeded;
	}
	else
	{
		if (OutCanceled)
		{
			UE_LOG(LogZibraVDBImporter, Warning, TEXT("ZibraVDBVolume import canceled"));
			Result = EReimportResult::Cancelled;
		}
		else
		{
			UE_LOG(LogZibraVDBImporter, Warning, TEXT("ZibraVDBVolume import failed"));
			Result = EReimportResult::Failed;
		}
	}

	return Result;
}

int32 UZibraVDBImportFactory::GetPriority() const
{
	return ImportPriority;
}

#endif

#undef LOCTEXT_NAMESPACE
