// Copyright Zibra AI Inc 2025-2026. All Rights Reserved.

#pragma once

#include "Components/PrimitiveComponent.h"

#include "ZibraVDBAssetComponent.generated.h"

class UZibraVDBVolume4;
class FZibraVDBNativeResource;
class UZibraVDBMaterialComponent;
class UZibraVDBPlaybackComponent;

UCLASS(Blueprintable, ClassGroup = Rendering, hideCategories = (Activation, Collision, Cooking, Tags, AssetUserData))
class ZIBRAVDBRUNTIME_API UZibraVDBAssetComponent final : public UActorComponent
{
	GENERATED_UCLASS_BODY()

public:
	UFUNCTION(BlueprintCallable, meta = (DisplayName = "Set ZibraVDBAssetComponent"), Category = Volume)
	void SetZibraVDBMaterialComponent(UZibraVDBMaterialComponent* InZibraVDBMaterialComponent);

	UFUNCTION(BlueprintCallable, meta = (DisplayName = "Set ZibraVDBPlaybackComponent"), Category = Volume)
	void SetZibraVDBPlaybackComponent(UZibraVDBPlaybackComponent* InZibraVDBPlaybackComponent);

	UFUNCTION(BlueprintCallable, Category = Volume)
	bool HasZibraVDBVolume() const noexcept;

	UFUNCTION(BlueprintGetter, Category = Volume)
	const UZibraVDBVolume4* GetZibraVDBVolume() const noexcept;

	UFUNCTION(BlueprintSetter, Category = Volume)
	void SetZibraVDBVolume(UZibraVDBVolume4* InZibraVDBVolume) noexcept;

	FString GetAssetName() const noexcept;
	void BroadcastFrameChanged(uint32 Frame) noexcept;
	uint32 GetCurrentFrame() const noexcept;
	void GetReferencedContentObjects(TArray<UObject*>& Objects) const noexcept;

#if WITH_EDITOR
	void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) final;
#endif

	DECLARE_MULTICAST_DELEGATE_OneParam(FOnFrameChanged, uint32);
	FOnFrameChanged OnFrameChanged;

	void OnZibraVDBVolumeReimport(TObjectPtr<UZibraVDBVolume4> InZibraVDBVolume);
	void OnVolumeChanged() noexcept;

	bool IsAssetValid() const noexcept;

private:
	UPROPERTY(EditAnywhere, Category = Volume, Interp, meta = (AllowPrivateAccess = "true", DisplayName = "ZibraVDB Volume"),
		BlueprintGetter = GetZibraVDBVolume, BlueprintSetter = SetZibraVDBVolume)
	TObjectPtr<UZibraVDBVolume4> ZibraVDBVolume;

	uint32 CurrentFrame = 0;

	TObjectPtr<UZibraVDBMaterialComponent> ZibraVDBMaterialComponent;
	TObjectPtr<UZibraVDBPlaybackComponent> ZibraVDBPlaybackComponent;

	bool bIsAssetValid = true;
};
