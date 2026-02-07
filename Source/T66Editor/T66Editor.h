// Copyright Tribulation 66. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Modules/ModuleInterface.h"
#include "Modules/ModuleManager.h"

DECLARE_LOG_CATEGORY_EXTERN(LogT66Editor, Log, All);

class FT66EditorModule : public IModuleInterface
{
public:
	virtual void StartupModule() override;
	virtual void ShutdownModule() override;

private:
	void RegisterT66ToolsMenu();
	void UnregisterT66ToolsMenu();
	static void OnGameplayLevelLoaded(UWorld* LoadedWorld);

	FName T66ToolsMenuName;
	FDelegateHandle PostLoadMapHandle;
};
