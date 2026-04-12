// Copyright Tribulation 66. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"

class UTexture2D;

DECLARE_LOG_CATEGORY_EXTERN(LogT66RuntimeAssets, Log, All);

namespace T66LegacyRuntimeTextureAccess
{
	UTexture2D* ImportFileTexture(
		const FString& FilePath,
		TextureFilter Filter,
		TextureGroup LODGroup,
		bool bClamp,
		const TCHAR* DebugLabel = nullptr);
}
