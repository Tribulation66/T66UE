// Copyright Zibra AI Inc 2025-2026. All Rights Reserved.

#pragma once

#include "Modules/ModuleManager.h"
#include "ZIBRHIConfig.h"

#define EXPAND_DECLARE_LOG_CATEGORY_EXTERN(Name, DefaultVerbosity, CompileTimeVerbosity) \
	DECLARE_LOG_CATEGORY_EXTERN(Name, DefaultVerbosity, CompileTimeVerbosity)
EXPAND_DECLARE_LOG_CATEGORY_EXTERN(ZIBRA_RHI_MODULE_LOG, Log, All);

class ZIBRA_RHI_MODULE_CLASSNAME final : public IModuleInterface
{
};
