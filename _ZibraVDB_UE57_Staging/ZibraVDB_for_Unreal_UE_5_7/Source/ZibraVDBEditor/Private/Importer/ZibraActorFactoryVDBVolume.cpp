// Copyright Zibra AI Inc 2025-2026. All Rights Reserved.

#include "ZibraActorFactoryVDBVolume.h"

#include "AssetRegistry/AssetData.h"
#include "ZibraVDBRuntime/Public/ZibraVDBActor.h"
#include "ZibraVDBRuntime/Public/ZibraVDBAssetComponent.h"
#include "ZibraVDBRuntime/Public/ZibraVDBVolume4.h"

DEFINE_LOG_CATEGORY(LogZibraVDBRuntime);

#define LOCTEXT_NAMESPACE "ZibraVDBForUnreal"

UActorFactoryZibraVDBVolume::UActorFactoryZibraVDBVolume(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer)
{
	DisplayName = LOCTEXT("ZibraVDBActor", "ZibraVDB Actor");
	NewActorClass = AZibraVDBActor::StaticClass();
	bUseSurfaceOrientation = true;
	bShowInEditorQuickMenu = true;
}

bool UActorFactoryZibraVDBVolume::CanCreateActorFrom(const FAssetData& AssetData, FText& OutErrorMsg)
{
	if (!AssetData.IsValid())
	{
		return true;
	}

	if (!AssetData.GetClass()->IsChildOf(UZibraVDBVolume4::StaticClass()))
	{
		OutErrorMsg = LOCTEXT("UZibraVDBVolume4Required", "Valid UZibraVDBVolume4 must be specified.");
		return false;
	}

	return true;
}

void UActorFactoryZibraVDBVolume::PostSpawnActor(UObject* Asset, AActor* NewActor)
{
	Super::PostSpawnActor(Asset, NewActor);

	UZibraVDBVolume4* Volume = CastChecked<UZibraVDBVolume4>(Asset);
	AZibraVDBActor* Actor = CastChecked<AZibraVDBActor>(NewActor);

	UZibraVDBAssetComponent* AssetComponent = Actor->GetAssetComponent();
	AssetComponent->SetZibraVDBVolume(Volume);
}

#if (100 * ENGINE_MAJOR_VERSION + ENGINE_MINOR_VERSION) < 504
void UActorFactoryZibraVDBVolume::PostCreateBlueprint(UObject* Asset, AActor* CDO)
{
	if (Asset != nullptr && CDO != nullptr)
	{
		UZibraVDBVolume4* Volume = CastChecked<UZibraVDBVolume4>(Asset);
		AZibraVDBActor* Actor = CastChecked<AZibraVDBActor>(CDO);

		UZibraVDBAssetComponent* AssetComponent = Actor->GetAssetComponent();
		AssetComponent->SetZibraVDBVolume(Volume);
	}
}
#endif

#undef LOCTEXT_NAMESPACE
