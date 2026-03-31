// Copyright Zibra AI Inc 2025-2026. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "RHI.h"
#include "RHIDefinitions.h"
#include "RHIResources.h"
#include "Runtime/Launch/Resources/Version.h"

#if (100 * ENGINE_MAJOR_VERSION + ENGINE_MINOR_VERSION) >= 502
#include "DataDrivenShaderPlatformInfo.h"
#endif

#include "GlobalShader.h"
#include "Misc/Paths.h"
#include "RenderGraphUtils.h"
#include "Shader.h"
#include "ShaderCore.h"
#include "ShaderParameterStruct.h"
#include "ShaderParameterUtils.h"

#include "ZibraVDBShadersInclude.h"
