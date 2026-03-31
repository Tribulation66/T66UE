// Copyright Zibra AI Inc 2025-2026. All Rights Reserved.

#include "ZibraVDBSequencerUtils.h"
#include "ZibraVDBActor.h"
#include "ZibraVDBPlaybackComponent.h"
#include "ZibraVDBAssetComponent.h"
#include "ZibraVDBVolume4.h"

namespace ZibraVDBSequencerUtils
{
	UZibraVDBPlaybackComponent* GetPlaybackComponent(UObject* BoundObject)
	{
		AZibraVDBActor* Actor = Cast<AZibraVDBActor>(BoundObject);
		if (!Actor)
		{
			return nullptr;
		}
		return Actor->GetPlaybackComponent();
	}

	const UZibraVDBVolume4* GetVDBAsset(UObject* BoundObject)
	{
		AZibraVDBActor* Actor = Cast<AZibraVDBActor>(BoundObject);
		if (!Actor)
		{
			return nullptr;
		}
		UZibraVDBAssetComponent* AssetComp = Actor->GetAssetComponent();
		if (!AssetComp)
		{
			return nullptr;
		}
		return AssetComp->GetZibraVDBVolume();
	}
}
