// Copyright Tribulation 66. All Rights Reserved.

#include "T66.h"
#include "Misc/Paths.h"
#include "Modules/ModuleManager.h"
#include "ShaderCore.h"
#include "UI/Style/T66Style.h"

DEFINE_LOG_CATEGORY(LogT66);

class FT66GameModule : public FDefaultGameModuleImpl
{
public:
	virtual void StartupModule() override
	{
		FDefaultGameModuleImpl::StartupModule();

		const FString ToonStyleShaderDirectory = FPaths::ProjectDir() / TEXT("ToonStyle/Shaders/Public");
		AddShaderSourceDirectoryMapping(TEXT("/Project/ToonStyle"), ToonStyleShaderDirectory);

		FT66Style::Initialize();
	}

	virtual void ShutdownModule() override
	{
		FT66Style::Shutdown();
		FDefaultGameModuleImpl::ShutdownModule();
	}
};

IMPLEMENT_PRIMARY_GAME_MODULE(FT66GameModule, T66, "T66");
