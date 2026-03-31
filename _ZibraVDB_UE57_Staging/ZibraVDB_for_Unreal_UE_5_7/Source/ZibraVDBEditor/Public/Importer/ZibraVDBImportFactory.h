// Copyright Zibra AI Inc 2025-2026. All Rights Reserved.

#pragma once

#include "Factories/Factory.h"
#include "EditorReimportHandler.h"

#include "ZibraVDBImportFactory.generated.h"

#if WITH_EDITORONLY_DATA

UCLASS(hidecategories = Object)
class UZibraVDBImportFactory final : public UFactory, public FReimportHandler
{
	GENERATED_UCLASS_BODY()

public:
	// UFactory interface
	UObject* FactoryCreateFile(UClass* InClass, UObject* InParent, FName InName, EObjectFlags Flags, const FString& Filename,
		const TCHAR* Parms, FFeedbackContext* Warn, bool& bOutOperationCanceled) final;
	bool FactoryCanImport(const FString& Filename) final;
	bool DoesSupportClass(UClass* Class) final;
	UClass* ResolveSupportedClass() final;
	// End of UFactory interface
	
	// FReimportHandler interface
	virtual bool CanReimport(UObject* Obj, TArray<FString>& OutFilenames) override;
	virtual void SetReimportPaths(UObject* Obj, const TArray<FString>& NewReimportPaths) override;
	virtual EReimportResult::Type Reimport(UObject* Obj) override;
	virtual int32 GetPriority() const override;
	// End of FReimportHandler interface
};

#endif
