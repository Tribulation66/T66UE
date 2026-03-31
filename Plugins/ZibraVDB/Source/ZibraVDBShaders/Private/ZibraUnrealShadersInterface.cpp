// Copyright Zibra AI Inc 2025-2026. All Rights Reserved.

#include "ZibraUnrealShadersInterface.h"

TShaderRef<FShader> ZIBRA_SHADERS_MODULE_INTERFACE::GetShaderByName(const FString& ShaderTypeName)
{
	FGlobalShaderMap* ShaderMap = GetGlobalShaderMap(GMaxRHIFeatureLevel);
	FShaderType* ShaderType = FShaderType::GetShaderTypeByName(*ShaderTypeName);

	return ShaderMap->GetShader(ShaderType, 0);
}
