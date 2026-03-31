// Copyright Zibra AI Inc 2025-2026. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Runtime/Launch/Resources/Version.h"

#if defined(UE_BUILD_DEBUG) || defined(UE_BUILD_DEVELOPMENT)
#define ZIB_RHI_CHEAP_GPU_DEBUG
#endif

#ifdef ZIB_RHI_CHEAP_GPU_DEBUG
#define ZIB_CHEAP_GPU_DEBUG_ONLY(expression) expression
#else
#define ZIB_CHEAP_GPU_DEBUG_ONLY(expression)
#endif	  // ZIB_RHI_CHEAP_GPU_DEBUG
