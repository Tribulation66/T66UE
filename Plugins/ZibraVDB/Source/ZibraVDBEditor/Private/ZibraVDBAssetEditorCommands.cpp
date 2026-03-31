// Copyright Zibra AI Inc 2025-2026. All Rights Reserved.

#include "ZibraVDBAssetEditorCommands.h"

#include "Editor/EditorStyle/Public/EditorStyleSet.h"

#define LOCTEXT_NAMESPACE "ZibraVDBForUnreal"

FZibraVDBAssetEditorCommands::FZibraVDBAssetEditorCommands()
	: TCommands<FZibraVDBAssetEditorCommands>(TEXT("ZibraVDBAssetEditor"),
		  LOCTEXT("ZibraVDBAssetEditorName", "ZibraVDB Asset Editor"), NAME_None, FAppStyle::GetAppStyleSetName())
{
}

void FZibraVDBAssetEditorCommands::RegisterCommands()
{
	UI_COMMAND(ReimportCommand, "Reimport", "Reimport the asset from the original source file.", EUserInterfaceActionType::Button,
		FInputChord());
	UI_COMMAND(
		ExportCommand, "Export .zibravdb", "Export .zibravdb volume to file.", EUserInterfaceActionType::Button, FInputChord());
}

#undef LOCTEXT_NAMESPACE
