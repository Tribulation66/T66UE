// Copyright Zibra AI Inc 2025-2026. All Rights Reserved.

#pragma once

#include "ActorFactories/ActorFactory.h"
#include "UObject/ObjectMacros.h"

#include "Runtime/Launch/Resources/Version.h"

#include "ZibraActorFactoryVDBVolume.generated.h"

class AActor;
struct FAssetData;

UCLASS(MinimalAPI, config = Editor)
class UActorFactoryZibraVDBVolume final : public UActorFactory
{
	GENERATED_UCLASS_BODY()

	// UActorFactory Interface
	bool CanCreateActorFrom(const FAssetData& AssetData, FText& OutErrorMsg) final;
	void PostSpawnActor(UObject* Asset, AActor* NewActor) final;
#if (100 * ENGINE_MAJOR_VERSION + ENGINE_MINOR_VERSION) < 504
	void PostCreateBlueprint(UObject* Asset, AActor* CDO) final;
#endif
};
