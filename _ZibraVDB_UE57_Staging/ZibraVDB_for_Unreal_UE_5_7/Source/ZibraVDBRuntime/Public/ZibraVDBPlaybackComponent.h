// Copyright Zibra AI Inc 2025-2026. All Rights Reserved.

#pragma once

#include "Components/ActorComponent.h"

#include "ZibraVDBPlaybackComponent.generated.h"

class UZibraVDBVolume4;
class UZibraVDBAssetComponent;

UCLASS(Blueprintable, hideCategories = (Activation, Collision, Cooking, Tags, AssetUserData))
class ZIBRAVDBRUNTIME_API UZibraVDBPlaybackComponent final : public UActorComponent
{
	GENERATED_UCLASS_BODY()

public:
	//~ Begin UActorComponent Interface.
	void TickComponent(float DeltaTime, enum ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) final;
	void PostLoad() final;
	void OnRegister() final;
	//~ End UActorComponent Interface.

	void Init() noexcept;
	void OnVolumeChanged() noexcept;
	void SetZibraVDBAssetComponent(TObjectPtr<UZibraVDBAssetComponent> InZibraVdbAssetComponent) noexcept;

	/** Toggles sequence animation  */
	UPROPERTY(EditAnywhere, Category = Playback, Interp)
	bool Animate = true;

	/** Shows the current rendered frame. If Animating is off you can set your desired frame to render  */
	UPROPERTY(EditAnywhere, Category = Playback, Interp, meta = (UIMin = 0, ClampMin = 0))
	float CurrentFrame = 0;

	/** The frame playback will start from. Overrides the Current Frame on initialization */
	UPROPERTY(EditAnywhere, Category = Playback, Interp)
	float StartFrame = 0;

	/** Rate of playing animation  */
	UPROPERTY(EditAnywhere, Category = Playback, Interp, meta = (UIMin = 0, ClampMin = 0))
	float Framerate = 30;

	/** Toggles decompression and rendering of the effect */
	UPROPERTY(EditAnywhere, Category = Playback, Interp)
	bool Visible = true;

	/** Internal flag set by sequencer to control visibility when outside sections */
	bool SequencerShouldHideWhenOutside = false;

#if WITH_EDITOR
	void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) final;
#endif

private:
	const UZibraVDBVolume4* GetZibraVDBVolume() const noexcept;

	/** Total number of frames in the effect  */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Playback", meta = (AllowPrivateAccess = "true"))
	int FrameCount = 0;

	TObjectPtr<UZibraVDBAssetComponent> ZibraVDBAssetComponent;
};
