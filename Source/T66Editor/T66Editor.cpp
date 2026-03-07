// Copyright Tribulation 66. All Rights Reserved.

#include "T66Editor.h"
#include "T66UISetupSubsystem.h"
#include "Core/T66GameInstance.h"
#include "Editor.h"
#include "Engine/World.h"
#include "Kismet/GameplayStatics.h"
#include "ToolMenus.h"
#include "LevelEditor.h"
#include "Framework/MultiBox/MultiBoxBuilder.h"
#include "Math/UnrealMathUtility.h"
#include "Misc/CoreDelegates.h"

DEFINE_LOG_CATEGORY(LogT66Editor);

IMPLEMENT_MODULE(FT66EditorModule, T66Editor)

void FT66EditorModule::StartupModule()
{
	UE_LOG(LogT66Editor, Log, TEXT("T66Editor module started"));
	RegisterT66ToolsMenu();
	PostLoadMapHandle = FCoreUObjectDelegates::PostLoadMapWithWorld.AddStatic(&FT66EditorModule::OnGameplayLevelLoaded);
}

void FT66EditorModule::ShutdownModule()
{
	if (PostLoadMapHandle.IsValid())
	{
		FCoreUObjectDelegates::PostLoadMapWithWorld.Remove(PostLoadMapHandle);
		PostLoadMapHandle.Reset();
	}
	UnregisterT66ToolsMenu();
	UE_LOG(LogT66Editor, Log, TEXT("T66Editor module shutdown"));
}

void FT66EditorModule::OnGameplayLevelLoaded(UWorld* LoadedWorld)
{
	if (!LoadedWorld || LoadedWorld->WorldType != EWorldType::PIE)
	{
		return;
	}
	const FString LevelName = UGameplayStatics::GetCurrentLevelName(LoadedWorld, true);
	if (!LevelName.Equals(TEXT("GameplayLevel"), ESearchCase::IgnoreCase))
	{
		return;
	}
	// Landscape auto-regen disabled: GameplayLevel now uses LowPolyNature terrain tiles + mesh hills instead of Landscape.
	UE_LOG(LogT66Editor, Log, TEXT("[MAP] GameplayLevel PIE loaded — landscape auto-regen skipped (LowPolyNature environment active)"));
}

void FT66EditorModule::RegisterT66ToolsMenu()
{
	UToolMenus* ToolMenus = UToolMenus::Get();
	if (!ToolMenus)
	{
		UE_LOG(LogT66Editor, Error, TEXT("[MAP] RegisterT66ToolsMenu: UToolMenus::Get() is null"));
		return;
	}

	T66ToolsMenuName = FName(TEXT("LevelEditor.MainMenu.Window"));
	UToolMenu* WindowMenu = ToolMenus->ExtendMenu(T66ToolsMenuName);
	if (!WindowMenu)
	{
		UE_LOG(LogT66Editor, Error, TEXT("[MAP] RegisterT66ToolsMenu: failed to extend Window menu"));
		return;
	}

	FToolMenuSection& Section = WindowMenu->FindOrAddSection(TEXT("T66Tools"));
	Section.Label = NSLOCTEXT("T66Editor", "T66ToolsSection", "T66 Tools");
	Section.AddSubMenu(
		TEXT("T66ToolsSubMenu"),
		NSLOCTEXT("T66Editor", "T66Tools", "T66 Tools"),
		NSLOCTEXT("T66Editor", "T66ToolsTooltip", "T66 editor tools"),
		FNewToolMenuDelegate::CreateLambda([this](UToolMenu* SubMenu)
		{
			FToolMenuSection& SubSection = SubMenu->AddSection(TEXT("T66ProceduralLandscape"));
			SubSection.AddMenuEntry(
				TEXT("CreateLabLevel"),
				NSLOCTEXT("T66Editor", "CreateLabLevel", "Create Lab Level"),
				NSLOCTEXT("T66Editor", "CreateLabLevelTooltip", "Create The Lab level (Content/Maps/LabLevel) with PlayerStart, lighting, and GameMode. Replaces current level."),
				FSlateIcon(),
				FUIAction(FExecuteAction::CreateLambda([]()
				{
					if (GEditor)
					{
						if (UT66UISetupSubsystem* Sub = GEditor->GetEditorSubsystem<UT66UISetupSubsystem>())
						{
							Sub->CreateLabLevel();
						}
					}
				}))
			);
			SubSection.AddMenuEntry(
				TEXT("SetupDemoMap"),
				NSLOCTEXT("T66Editor", "SetupDemoMap", "Setup Demo Map"),
				NSLOCTEXT("T66Editor", "SetupDemoMapTooltip", "Configure Map_Summer for gameplay: set GameMode, add PlayerStart if missing, save. Run once when using the demo map switch."),
				FSlateIcon(),
				FUIAction(FExecuteAction::CreateLambda([]()
				{
					if (GEditor)
					{
						if (UT66UISetupSubsystem* Sub = GEditor->GetEditorSubsystem<UT66UISetupSubsystem>())
						{
							Sub->ConfigureDemoMapLevel();
						}
					}
				}))
			);
		})
	);
}

void FT66EditorModule::UnregisterT66ToolsMenu()
{
	UToolMenus* ToolMenus = UToolMenus::Get();
	if (ToolMenus && T66ToolsMenuName != NAME_None)
	{
		ToolMenus->RemoveEntry(T66ToolsMenuName, TEXT("T66ToolsSection"), TEXT("T66ToolsSubMenu"));
	}
	T66ToolsMenuName = NAME_None;
}
