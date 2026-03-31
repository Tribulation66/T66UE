// Copyright Zibra AI Inc 2025-2026. All Rights Reserved.

#pragma once

#include "ZibraVDBRuntime/Private/Render/ZibraVDBMaterialRendering.h"
#include "Modules/ModuleManager.h"

class FZibraVDBRuntimeModule final : public IModuleInterface
{
public:
	using TZibraRenderExtensionPtr = TSharedPtr<FZibraVDBMaterialRendering, ESPMode::ThreadSafe>;

	/** IModuleInterface implementation */
	virtual void StartupModule() noexcept override final;
	virtual void ShutdownModule() noexcept override final;

	static TZibraRenderExtensionPtr GetZibraRenderExtension();
};
