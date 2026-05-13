// Copyright Tribulation 66. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"

class UWorld;

class T66_API FT66WidgetDumpTargets
{
public:
	static bool DumpTargetToJson(
		UWorld* World,
		const FString& TargetSpec,
		const FString& OutputPath,
		FString& OutError);

	static bool ParseAutomationSpec(
		const FString& AutomationSpec,
		FString& OutTargetSpec,
		FString& OutOutputPath,
		FString& OutError);
};
