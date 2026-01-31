// Copyright Tribulation 66. All Rights Reserved.

#include "T66.h"
#include "Modules/ModuleManager.h"
#include "UI/Style/T66Style.h"

DEFINE_LOG_CATEGORY(LogT66);

class FT66GameModule : public FDefaultGameModuleImpl
{
public:
	virtual void StartupModule() override
	{
		FDefaultGameModuleImpl::StartupModule();
		FT66Style::Initialize();
	}

	virtual void ShutdownModule() override
	{
		FT66Style::Shutdown();
		FDefaultGameModuleImpl::ShutdownModule();
	}
};

IMPLEMENT_PRIMARY_GAME_MODULE(FT66GameModule, T66, "T66");
