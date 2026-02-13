// Copyright Tribulation 66. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Data/T66DataTypes.h"
#include "T66HeroPreviewStage.generated.h"

class AT66HeroBase;
class AT66CompanionBase;
class USceneComponent;
class UStaticMeshComponent;

/**
 * Preview stage for the Hero Selection screen.
 * Manages the preview hero pawn and platform. The main viewport camera
 * renders the character directly (full Lumen GI), no SceneCapture needed.
 */
UCLASS(Blueprintable)
class T66_API AT66HeroPreviewStage : public AActor
{
	GENERATED_BODY()

public:
	AT66HeroPreviewStage();

	/**
	 * Set the preview hero. Spawns or updates the hero pawn.
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

	/** Get the ideal camera location for viewing this preview (computed by FrameCameraToPreview). */
	FVector GetIdealCameraLocation() const { return IdealCameraLocation; }

	/** Get the ideal camera rotation for viewing this preview (computed by FrameCameraToPreview). */
	FRotator GetIdealCameraRotation() const { return IdealCameraRotation; }

	/** Show or hide the entire stage (platform + hero + companion pawns). */
	void SetStageVisible(bool bVisible);

protected:
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

	/** Spawn or update the preview hero pawn */
	void UpdatePreviewPawn(FName HeroID, ET66BodyType BodyType, FName SkinID);

	/** Spawn/update/hide companion behind the hero. Position uses CompanionFollowOffset (same as in-game). */
	void UpdatePreviewCompanion(FName CompanionID);

	/** Update companion position/rotation to stay behind hero (call after ApplyPreviewRotation). */
	void UpdateCompanionPlacement();

	void ApplyPreviewRotation();

	/** Compute the ideal camera location/rotation and update platform placement. */
	void FrameCameraToPreview();

	class UPrimitiveComponent* GetPreviewTargetComponent() const;

	/** Simple platform so the hero looks grounded (same level lighting as gameplay). */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Preview")
	TObjectPtr<UStaticMeshComponent> PreviewPlatform;

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

	/** Orbit camera pitch (degrees). Positive looks slightly down. */
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

	/** Computed ideal camera transform (read by FrontendGameMode to position the world camera). */
	UPROPERTY(Transient)
	FVector IdealCameraLocation = FVector::ZeroVector;

	UPROPERTY(Transient)
	FRotator IdealCameraRotation = FRotator::ZeroRotator;

	/** Push the platform slightly back so the hero reads "forward" on it (toward camera). */
	UPROPERTY(EditDefaultsOnly, Category = "Preview|Tuning")
	float PlatformForwardOffset = 35.f;

	/** Multiplier on the auto-framed camera distance (smaller = character appears bigger, more zoomed in). */
	UPROPERTY(EditDefaultsOnly, Category = "Preview|Tuning")
	float CameraDistanceMultiplier = 3.0f;

	/** User zoom factor applied on top of CameraDistanceMultiplier (<= 1.0 means zoom-in). */
	UPROPERTY(Transient)
	float PreviewZoomMultiplier = 1.0f;

	/** Minimum zoom-in multiplier (smaller = closer zoom). */
	UPROPERTY(EditDefaultsOnly, Category = "Preview|Tuning")
	float MinPreviewZoomMultiplier = 0.65f;

	/** FOV used for camera framing calculations (should match gameplay camera). */
	UPROPERTY(EditDefaultsOnly, Category = "Preview|Tuning")
	float CameraFOV = 90.f;
};
