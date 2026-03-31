// Copyright Zibra AI Inc 2025-2026. All Rights Reserved.

#pragma once

#include "Framework/Commands/Commands.h"

class FZibraVDBAssetEditorCommands final : public TCommands<FZibraVDBAssetEditorCommands>
{
public:
	FZibraVDBAssetEditorCommands();

	void RegisterCommands() final;

	TSharedPtr<FUICommandInfo> ReimportCommand;
	TSharedPtr<FUICommandInfo> ExportCommand;
};
