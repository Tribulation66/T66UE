// Copyright Tribulation 66. All Rights Reserved.

#include "T66Editor.h"
#include "T66ProceduralLandscapeEditorTool.h"
#include "Gameplay/T66ProceduralLandscapeParams.h"
#include "Editor.h"
#include "Engine/World.h"
#include "ToolMenus.h"
#include "LevelEditor.h"
#include "Framework/MultiBox/MultiBoxBuilder.h"
#include "Math/UnrealMathUtility.h"

DEFINE_LOG_CATEGORY(LogT66Editor);

IMPLEMENT_MODULE(FT66EditorModule, T66Editor)

void FT66EditorModule::StartupModule()
{
	UE_LOG(LogT66Editor, Log, TEXT("T66Editor module started"));
	RegisterT66ToolsMenu();
}

void FT66EditorModule::ShutdownModule()
{
	UnregisterT66ToolsMenu();
	UE_LOG(LogT66Editor, Log, TEXT("T66Editor module shutdown"));
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
				TEXT("GenerateProceduralHillsLandscape"),
				NSLOCTEXT("T66Editor", "GenerateProceduralHills", "Generate Procedural Hills Landscape (Dev)"),
				NSLOCTEXT("T66Editor", "GenerateProceduralHillsTooltip", "Create or update the Landscape in the current level with procedural rolling hills."),
				FSlateIcon(),
				FUIAction(FExecuteAction::CreateLambda([]()
				{
					UWorld* World = GEditor ? GEditor->GetEditorWorldContext().World() : nullptr;
					if (!World)
					{
						UE_LOG(LogT66Editor, Error, TEXT("[MAP] Generate Procedural Hills: no editor world"));
						return;
					}
					FT66ProceduralLandscapeParams Params;
					Params.Seed = FMath::Rand();
					const bool bOk = T66ProceduralLandscapeEditor::GenerateProceduralHillsLandscape(World, Params);
					if (!bOk)
					{
						UE_LOG(LogT66Editor, Error, TEXT("[MAP] Generate Procedural Hills: GenerateProceduralHillsLandscape returned false"));
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
