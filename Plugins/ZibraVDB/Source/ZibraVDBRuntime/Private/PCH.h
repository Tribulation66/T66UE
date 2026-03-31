// Copyright Zibra AI Inc 2025-2026. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GlobalShader.h"
#include "Misc/Paths.h"
#include "RHI.h"
#include "RHIResources.h"
#include "RenderGraphResources.h"
#include "RenderGraphUtils.h"
#include "RenderResource.h"
#include "RendererInterface.h"
#include "Runtime/Launch/Resources/Version.h"
#include "Shader.h"
#include "ShaderCore.h"
#include "ShaderParameterStruct.h"
#include "ShaderParameterUtils.h"
#include "Templates/UniquePtr.h"

#if (100 * ENGINE_MAJOR_VERSION + ENGINE_MINOR_VERSION) >= 502
#include "DataDrivenShaderPlatformInfo.h"
#endif

#if WITH_EDITOR
#include "Editor.h"
#include "Editor/EditorEngine.h"
#endif

#include "MeshMaterialShader.h"

#include "ZibraVDBShaders/Public/ZibraVDBShadersInclude.h"
