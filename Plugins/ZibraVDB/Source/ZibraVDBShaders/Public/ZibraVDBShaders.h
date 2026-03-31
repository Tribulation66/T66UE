// Copyright Zibra AI Inc 2025-2026. All Rights Reserved.

#pragma once

#include "Modules/ModuleManager.h"

class FZibraVDBShadersModule final : public IModuleInterface
{
public:
	/** IModuleInterface implementation */
	void StartupModule() final;
	void ShutdownModule() final;
};
