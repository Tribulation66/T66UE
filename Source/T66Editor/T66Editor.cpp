// Copyright Tribulation 66. All Rights Reserved.

#include "T66Editor.h"
#include "T66ProceduralLandscapeEditorTool.h"
#include "T66UISetupSubsystem.h"
#include "Gameplay/T66ProceduralLandscapeParams.h"
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
	FT66ProceduralLandscapeParams Params;
	UGameInstance* GI = LoadedWorld->GetGameInstance();
	UT66GameInstance* T66GI = GI ? Cast<UT66GameInstance>(GI) : nullptr;
	// Use seed set by "Enter the Tribulation" when present; otherwise random (e.g. PIE from GameplayLevel)
	Params.Seed = (T66GI && T66GI->ProceduralTerrainSeed != 0) ? T66GI->ProceduralTerrainSeed : FMath::Rand();
	const bool bOk = T66ProceduralLandscapeEditor::GenerateProceduralHillsLandscape(LoadedWorld, Params);
	UE_LOG(LogT66Editor, Log, TEXT("[MAP] Auto-regenerated procedural hills for GameplayLevel (PIE), seed=%d, ok=%d"), Params.Seed, bOk);
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
					Params.bFlatTerrain = false;  // Menu action explicitly generates hills
					Params.Seed = FMath::Rand();
					const bool bOk = T66ProceduralLandscapeEditor::GenerateProceduralHillsLandscape(World, Params);
					if (!bOk)
					{
						UE_LOG(LogT66Editor, Error, TEXT("[MAP] Generate Procedural Hills: GenerateProceduralHillsLandscape returned false"));
					}
				}))
			);
			SubSection.AddMenuEntry(
				TEXT("SetupT66MapAssets"),
				NSLOCTEXT("T66Editor", "SetupT66MapAssets", "Setup T66 Map Assets"),
				NSLOCTEXT("T66Editor", "SetupT66MapAssetsTooltip", "Copy grass (Polytope), landscape/trees/rocks (Cozy Nature) into Content/T66MapAssets. Run once to consolidate."),
				FSlateIcon(),
				FUIAction(FExecuteAction::CreateLambda([]()
				{
					T66ProceduralLandscapeEditor::SetupT66MapAssets();
				}))
			);
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
