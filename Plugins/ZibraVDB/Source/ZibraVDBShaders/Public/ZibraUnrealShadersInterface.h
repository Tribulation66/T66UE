// Copyright Zibra AI Inc 2025-2026. All Rights Reserved.

#pragma once

#include "Templates/SharedPointer.h"
#include "Shader.h"

#define ZIBRA_SHADERS_MODULE_INTERFACE FZibraVDBUnrealShadersInterface

class ZIBRAVDBSHADERS_API ZIBRA_SHADERS_MODULE_INTERFACE
{
public:
	static TShaderRef<FShader> GetShaderByName(const FString& ShaderTypeName);
};
