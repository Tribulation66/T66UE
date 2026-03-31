// Copyright Zibra AI Inc 2025-2026. All Rights Reserved.

#include "ZibraVDBShaders.h"

#include "Interfaces/IPluginManager.h"

void FZibraVDBShadersModule::StartupModule()
{
	TSharedPtr<IPlugin> Plugin = IPluginManager::Get().FindPlugin(TEXT("ZibraVDB"));

	FString PluginShaderDir = FPaths::Combine(Plugin->GetBaseDir(), TEXT("Shaders"));
	FString PluginPrivateShaderDir = FPaths::Combine(PluginShaderDir, TEXT("Private"));
	AddShaderSourceDirectoryMapping("/Plugin/ZibraVDB", PluginShaderDir);
	AddShaderSourceDirectoryMapping(TEXT(ZIBRA_VIRTUAL_SHADER_PATH), PluginPrivateShaderDir);
}

void FZibraVDBShadersModule::ShutdownModule()
{
}

IMPLEMENT_MODULE(FZibraVDBShadersModule, ZibraVDBShaders);
