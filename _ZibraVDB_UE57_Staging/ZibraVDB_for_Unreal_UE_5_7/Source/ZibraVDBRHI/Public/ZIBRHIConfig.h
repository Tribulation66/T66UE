// Copyright Zibra AI Inc 2025-2026. All Rights Reserved.

#pragma once

#include "ZibraVDBShaders/Public/ZibraUnrealShadersInterface.h"
#define ZIBRA_RHI_MODULE_NAME ZibraVDBRHI

#define CONCATENATE_DETAIL(x, y) x##y
#define CONCATENATE(x, y) CONCATENATE_DETAIL(x, y)

#define CONCATENATE3_DETAIL(x, y, z) x##y##z
#define CONCATENATE3(x, y, z) CONCATENATE3_DETAIL(x, y, z)

// API related to the module
#define ZIBRA_RHI_MODULE_API ZIBRAVDBRHI_API

// Log prefix for the module
#define ZIBRA_RHI_MODULE_LOG CONCATENATE3(LogF, ZIBRA_RHI_MODULE_NAME, Module)

// Class name for the module
#define ZIBRA_RHI_MODULE_CLASSNAME CONCATENATE3(F, ZIBRA_RHI_MODULE_NAME, Module)
