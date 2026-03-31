// Copyright Zibra AI Inc 2025-2026. All Rights Reserved.

#pragma once

#include "UObject/ObjectMacros.h"

#include "ZibraEditorSettings.generated.h"

UCLASS(config = EditorPerProjectUserSettings)
class ZIBRAVDBEDITOR_API UZibraEditorSettings : public UObject
{
	GENERATED_BODY()

public:
	UPROPERTY(config, EditAnywhere, Category = "Notifications")
	bool CheckUpdatesOnStartup = true;

	UPROPERTY(config, EditAnywhere, Category = "Import", meta = (ContentDir))
	FDirectoryPath DefaultAssetPath = "/Game";
};
