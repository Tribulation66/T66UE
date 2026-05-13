// Copyright Tribulation 66. All Rights Reserved.

#include "UI/T66ScreenBase.h"

#include "Engine/World.h"
#include "Gameplay/T66PlayerController.h"
#include "HAL/IConsoleManager.h"
#include "Misc/Paths.h"
#include "UI/T66UIManager.h"
#include "UI/T66WidgetDumpTargets.h"

DEFINE_LOG_CATEGORY_STATIC(LogT66UIDumpCommands, Log, All);

namespace
{
	FString ResolveDumpPathFromArgs(const TArray<FString>& Args)
	{
		for (const FString& Arg : Args)
		{
			FString Key;
			FString Value;
			if (Arg.Split(TEXT("="), &Key, &Value) && Key.Equals(TEXT("Path"), ESearchCase::IgnoreCase))
			{
				return Value.TrimQuotes();
			}
		}

		for (const FString& Arg : Args)
		{
			if (!Arg.StartsWith(TEXT("-")) && !Arg.IsEmpty())
			{
				return Arg.TrimQuotes();
			}
		}

		return FPaths::Combine(FPaths::ProjectSavedDir(), TEXT("UI"), TEXT("DumpScreen.json"));
	}

	FString ResolveArgValue(const TArray<FString>& Args, const TCHAR* RequestedKey)
	{
		for (const FString& Arg : Args)
		{
			FString Key;
			FString Value;
			if (Arg.Split(TEXT("="), &Key, &Value) && Key.Equals(RequestedKey, ESearchCase::IgnoreCase))
			{
				return Value.TrimQuotes();
			}
		}

		return FString();
	}

	bool ResolveDumpWidgetArgs(const TArray<FString>& Args, FString& OutTargetSpec, FString& OutOutputPath, FString& OutError)
	{
		OutTargetSpec = ResolveArgValue(Args, TEXT("Target"));
		if (OutTargetSpec.IsEmpty())
		{
			for (const TCHAR* TargetKey : { TEXT("Class"), TEXT("Tag"), TEXT("ViewportIndex"), TEXT("Actor") })
			{
				const FString TargetValue = ResolveArgValue(Args, TargetKey);
				if (!TargetValue.IsEmpty())
				{
					OutTargetSpec = FString::Printf(TEXT("%s=%s"), TargetKey, *TargetValue);
					break;
				}
			}
		}

		OutOutputPath = ResolveArgValue(Args, TEXT("Path"));
		if (OutOutputPath.IsEmpty())
		{
			OutOutputPath = FPaths::Combine(FPaths::ProjectSavedDir(), TEXT("UI"), TEXT("DumpWidget.json"));
		}

		if (OutTargetSpec.IsEmpty())
		{
			OutError = TEXT("No widget target supplied. Usage: T66.UI.DumpWidget Class=<UClassOrSlateType> Path=<output_path.json>");
			return false;
		}

		return true;
	}

	UT66ScreenBase* FindActiveScreen(UWorld* World)
	{
		if (!World)
		{
			return nullptr;
		}

		AT66PlayerController* PlayerController = Cast<AT66PlayerController>(World->GetFirstPlayerController());
		UT66UIManager* UIManager = PlayerController ? PlayerController->GetUIManager() : nullptr;
		if (!UIManager)
		{
			return nullptr;
		}

		if (UIManager->IsModalActive())
		{
			return UIManager->GetCurrentModal();
		}

		return UIManager->GetCurrentScreen();
	}

	void DumpCurrentScreen(const TArray<FString>& Args, UWorld* World)
	{
		const FString OutputPath = ResolveDumpPathFromArgs(Args);
		UT66ScreenBase* ActiveScreen = FindActiveScreen(World);
		if (!ActiveScreen)
		{
			UE_LOG(LogT66UIDumpCommands, Warning, TEXT("T66.UI.DumpScreen failed: no active screen."));
			return;
		}

		const bool bDumped = ActiveScreen->DumpToJson(OutputPath);
		if (bDumped)
		{
			UE_LOG(LogT66UIDumpCommands, Display, TEXT("T66.UI.DumpScreen wrote: %s"), *FPaths::ConvertRelativePathToFull(OutputPath));
		}
		else
		{
			UE_LOG(LogT66UIDumpCommands, Warning, TEXT("T66.UI.DumpScreen failed: %s"), *FPaths::ConvertRelativePathToFull(OutputPath));
		}
	}

	void DumpTargetWidget(const TArray<FString>& Args, UWorld* World)
	{
		FString TargetSpec;
		FString OutputPath;
		FString Error;
		if (!ResolveDumpWidgetArgs(Args, TargetSpec, OutputPath, Error))
		{
			UE_LOG(LogT66UIDumpCommands, Warning, TEXT("T66.UI.DumpWidget failed: %s"), *Error);
			return;
		}

		const bool bDumped = FT66WidgetDumpTargets::DumpTargetToJson(World, TargetSpec, OutputPath, Error);
		if (bDumped)
		{
			UE_LOG(
				LogT66UIDumpCommands,
				Display,
				TEXT("T66.UI.DumpWidget wrote: Target=%s Path=%s"),
				*TargetSpec,
				*FPaths::ConvertRelativePathToFull(OutputPath));
		}
		else
		{
			UE_LOG(
				LogT66UIDumpCommands,
				Warning,
				TEXT("T66.UI.DumpWidget failed: Target=%s Path=%s Error=%s"),
				*TargetSpec,
				*FPaths::ConvertRelativePathToFull(OutputPath),
				*Error);
		}
	}

	static FAutoConsoleCommandWithWorldAndArgs T66UIDumpScreenCommand(
		TEXT("T66.UI.DumpScreen"),
		TEXT("Dumps the active T66 Slate screen to JSON. Usage: T66.UI.DumpScreen Path=<output_path.json>"),
		FConsoleCommandWithWorldAndArgsDelegate::CreateStatic(&DumpCurrentScreen));

	static FAutoConsoleCommandWithWorldAndArgs T66UIDumpWidgetCommand(
		TEXT("T66.UI.DumpWidget"),
		TEXT("Dumps a non-screen UI widget to JSON. Usage: T66.UI.DumpWidget Class=<UClassOrSlateType>|Tag=<Tag>|ViewportIndex=<n>|Actor=<ActorName> Path=<output_path.json>"),
		FConsoleCommandWithWorldAndArgsDelegate::CreateStatic(&DumpTargetWidget));
}
