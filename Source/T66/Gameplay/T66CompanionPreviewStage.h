// Copyright Tribulation 66. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Data/T66DataTypes.h"
#include "T66CompanionPreviewStage.generated.h"

class AT66CompanionBase;
class USceneComponent;
class UStaticMeshComponent;

/**
 * Preview stage for Companion Selection screen.
 * Manages the preview companion pawn and floor. The main viewport camera
 * renders the character directly (full Lumen GI), no SceneCapture needed.
 */
UCLASS(Blueprintable)
class T66_API AT66CompanionPreviewStage : public AActor
{
	GENERATED_BODY()

public:
	AT66CompanionPreviewStage();

	/** Set companion to display; SkinID = Default or Beachgoer for the 3D preview. */
	UFUNCTION(BlueprintCallable, Category = "Preview")
	void SetPreviewCompanion(FName CompanionID, FName SkinID = NAME_None);

	/** Rotate preview companion by yaw delta (degrees). Intended for UI drag-rotate. */
	UFUNCTION(BlueprintCallable, Category = "Preview")
	void AddPreviewYaw(float DeltaYawDegrees);

	/** Zoom preview in/out (wheel delta). Max zoom-out is the default framing. */
	UFUNCTION(BlueprintCallable, Category = "Preview")
	void AddPreviewZoom(float WheelDelta);

	/** Orbit the preview camera (yaw rotates companion, pitch moves camera up/down). */
	UFUNCTION(BlueprintCallable, Category = "Preview")
	void AddPreviewOrbit(float DeltaYawDegrees, float DeltaPitchDegrees);

	/** Get the ideal camera location for viewing this preview (computed by FrameCameraToPreview). */
	FVector GetIdealCameraLocation() const { return IdealCameraLocation; }

	/** Get the ideal camera rotation for viewing this preview (computed by FrameCameraToPreview). */
	FRotator GetIdealCameraRotation() const { return IdealCameraRotation; }

	/** Show or hide the entire stage (floor + companion pawn). */
	void SetStageVisible(bool bVisible);

protected:
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

	void UpdatePreviewPawn(FName CompanionID, FName SkinID);
	void ApplyPreviewRotation();
	void FrameCameraToPreview();
	class UPrimitiveComponent* GetPreviewTargetComponent() const;

	/** Simple floor so "ground level" is visible in preview (same level lighting as gameplay). */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Preview")
	TObjectPtr<UStaticMeshComponent> PreviewFloor;

	UPROPERTY(EditDefaultsOnly, Category = "Preview")
	TSubclassOf<AT66CompanionBase> CompanionPawnClass;

	UPROPERTY()
	TObjectPtr<AT66CompanionBase> PreviewPawn;

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

	/** Push the floor slightly back so the companion reads "forward" on it (toward camera). */
	UPROPERTY(EditDefaultsOnly, Category = "Preview|Tuning")
	float FloorForwardOffset = 35.f;

	/** Multiplier on the auto-framed camera distance (smaller = companion appears bigger). */
	UPROPERTY(EditDefaultsOnly, Category = "Preview|Tuning")
	float CameraDistanceMultiplier = 4.5f;

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
