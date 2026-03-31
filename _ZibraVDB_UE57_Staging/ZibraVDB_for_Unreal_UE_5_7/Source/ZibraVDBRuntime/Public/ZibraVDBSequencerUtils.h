// Copyright Zibra AI Inc 2025-2026. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"

class AZibraVDBActor;
class UZibraVDBPlaybackComponent;
class UZibraVDBVolume4;

namespace ZibraVDBSequencerUtils
{
	constexpr float DefaultFramerate = 30.0f;

	ZIBRAVDBRUNTIME_API UZibraVDBPlaybackComponent* GetPlaybackComponent(UObject* BoundObject);
	ZIBRAVDBRUNTIME_API const UZibraVDBVolume4* GetVDBAsset(UObject* BoundObject);

}
