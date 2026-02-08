// Copyright Tribulation 66. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Data/T66DataTypes.h"
#include "T66HeroPreviewStage.generated.h"

class USceneCaptureComponent2D;
class UTextureRenderTarget2D;
class AT66HeroBase;
class AT66CompanionBase;
class USceneComponent;
class UStaticMeshComponent;

/**
 * Preview stage for the Hero Selection screen.
 * Renders the selected hero (cylinder/cube placeholder or future FBX) to a texture
 * via SceneCapture2D. Update only on hero/body-type change (no per-frame logic).
 */
UCLASS(Blueprintable)
class T66_API AT66HeroPreviewStage : public AActor
{
	GENERATED_BODY()

public:
	AT66HeroPreviewStage();

	/** Render target that the UI displays (SceneCapture writes to this) */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Preview")
	UTextureRenderTarget2D* GetRenderTarget() const { return RenderTarget; }

	/**
	 * Set the preview hero. Spawns or updates the hero pawn and captures to the render target.
	 * Call when hero focus, body type, or skin changes.
	 * @param SkinID Skin to show (e.g. Default, Beachgoer); used for preview-only display.
	 * @param CompanionID If set, show this companion behind the hero (like in-game follow). NAME_None hides companion.
	 */
	UFUNCTION(BlueprintCallable, Category = "Preview")
	void SetPreviewHero(FName HeroID, ET66BodyType BodyType, FName SkinID = NAME_None, FName CompanionID = NAME_None);

	/** Rotate preview hero by yaw delta (degrees). Intended for UI drag-rotate. */
	UFUNCTION(BlueprintCallable, Category = "Preview")
	void AddPreviewYaw(float DeltaYawDegrees);

	/** Zoom preview in/out (wheel delta). Max zoom-out is the default framing. */
	UFUNCTION(BlueprintCallable, Category = "Preview")
	void AddPreviewZoom(float WheelDelta);

	/** Orbit the preview camera (yaw rotates hero, pitch moves camera up/down). */
	UFUNCTION(BlueprintCallable, Category = "Preview")
	void AddPreviewOrbit(float DeltaYawDegrees, float DeltaPitchDegrees);

	/** Capture the current scene to the render target (called after SetPreviewHero) */
	UFUNCTION(BlueprintCallable, Category = "Preview")
	void CapturePreview();

	/** Re-copy world post process to the scene capture and apply fixed exposure (call when theme/lighting changes). */
	void RefreshCapturePostProcess();

protected:
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
	virtual void Tick(float DeltaSeconds) override;

	/** Create the render target and scene capture at runtime if not set in editor */
	void EnsureCaptureSetup();

	/** Spawn or update the preview hero pawn */
	void UpdatePreviewPawn(FName HeroID, ET66BodyType BodyType, FName SkinID);

	/** Spawn/update/hide companion behind the hero. Position uses CompanionFollowOffset (same as in-game). */
	void UpdatePreviewCompanion(FName CompanionID);

	/** Update companion position/rotation to stay behind hero (call after ApplyPreviewRotation). */
	void UpdateCompanionPlacement();

	void ApplyPreviewRotation();
	void FrameCameraToPreview();
	class UPrimitiveComponent* GetPreviewTargetComponent() const;
	void ApplyShadowSettings();

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Preview")
	TObjectPtr<USceneCaptureComponent2D> SceneCapture;

	/** Simple platform so the hero looks grounded (same level lighting as gameplay). */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Preview")
	TObjectPtr<UStaticMeshComponent> PreviewPlatform;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Preview")
	TObjectPtr<UTextureRenderTarget2D> RenderTarget;

	/** Size of the render target (matches typical preview area) */
	UPROPERTY(EditDefaultsOnly, Category = "Preview")
	FIntPoint RenderTargetSize = FIntPoint(1024, 1440);

	/** Hero pawn class (same as gameplay - unified hero) */
	UPROPERTY(EditDefaultsOnly, Category = "Preview")
	TSubclassOf<AT66HeroBase> HeroPawnClass;

	/** Currently displayed preview pawn */
	UPROPERTY()
	TObjectPtr<AT66HeroBase> PreviewPawn;

	/** Optional companion shown behind the hero when a companion is selected (e.g. GI->SelectedCompanionID). */
	UPROPERTY()
	TObjectPtr<AT66CompanionBase> PreviewCompanionPawn;

	/** Offset for companion behind hero (match AT66CompanionBase::FollowOffset: -120 back, 80 to side). */
	UPROPERTY(EditDefaultsOnly, Category = "Preview")
	FVector CompanionFollowOffset = FVector(-120.f, 80.f, 0.f);

	/** Spawn location for the preview pawn (relative to this actor) */
	UPROPERTY(EditDefaultsOnly, Category = "Preview")
	FVector PreviewPawnOffset = FVector::ZeroVector;

	UPROPERTY(Transient)
	float PreviewYawDegrees = 0.f;

	/** Orbit camera pitch (degrees). Negative looks slightly down. */
	UPROPERTY(Transient)
	float OrbitPitchDegrees = 15.f;

	/** Cached orbit framing so the platform doesn't "swim" as you rotate. */
	UPROPERTY(Transient)
	bool bHasOrbitFrame = false;

	UPROPERTY(Transient)
	FVector OrbitCenter = FVector::ZeroVector;

	UPROPERTY(Transient)
	float OrbitRadius = 200.f;

	UPROPERTY(Transient)
	float OrbitBottomZ = 0.f;

	/** Push the platform slightly back so the hero reads "forward" on it (toward camera). */
	UPROPERTY(EditDefaultsOnly, Category = "Preview|Tuning")
	float PlatformForwardOffset = 35.f;

	/** Multiplier on the auto-framed camera distance (smaller = character appears bigger). */
	UPROPERTY(EditDefaultsOnly, Category = "Preview|Tuning")
	float CameraDistanceMultiplier = 4.5f;

	/** User zoom factor applied on top of CameraDistanceMultiplier (<= 1.0 means zoom-in). */
	UPROPERTY(Transient)
	float PreviewZoomMultiplier = 1.0f;

	/** Minimum zoom-in multiplier (smaller = closer zoom). */
	UPROPERTY(EditDefaultsOnly, Category = "Preview|Tuning")
	float MinPreviewZoomMultiplier = 0.65f;

	/** If true, disable shadow casting for preview meshes/platform. */
	UPROPERTY(EditDefaultsOnly, Category = "Preview|Tuning")
	bool bDisablePreviewShadows = true;
};
