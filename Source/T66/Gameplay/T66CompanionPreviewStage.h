// Copyright Tribulation 66. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Data/T66DataTypes.h"
#include "T66CompanionPreviewStage.generated.h"

class USceneCaptureComponent2D;
class UTextureRenderTarget2D;
class AT66CompanionBase;
class USceneComponent;
class UPointLightComponent;
class UStaticMeshComponent;

/**
 * Preview stage for Companion Selection screen.
 * Renders the selected companion (sphere placeholder) to a texture.
 */
UCLASS(Blueprintable)
class T66_API AT66CompanionPreviewStage : public AActor
{
	GENERATED_BODY()

public:
	AT66CompanionPreviewStage();

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Preview")
	UTextureRenderTarget2D* GetRenderTarget() const { return RenderTarget; }

	UFUNCTION(BlueprintCallable, Category = "Preview")
	void SetPreviewCompanion(FName CompanionID);

	/** Rotate preview companion by yaw delta (degrees). Intended for UI drag-rotate. */
	UFUNCTION(BlueprintCallable, Category = "Preview")
	void AddPreviewYaw(float DeltaYawDegrees);

	/** Zoom preview in/out (wheel delta). Max zoom-out is the default framing. */
	UFUNCTION(BlueprintCallable, Category = "Preview")
	void AddPreviewZoom(float WheelDelta);

	/** Orbit the preview camera (yaw rotates companion, pitch moves camera up/down). */
	UFUNCTION(BlueprintCallable, Category = "Preview")
	void AddPreviewOrbit(float DeltaYawDegrees, float DeltaPitchDegrees);

	UFUNCTION(BlueprintCallable, Category = "Preview")
	void CapturePreview();

protected:
	virtual void BeginPlay() override;
	void EnsureCaptureSetup();
	void UpdatePreviewPawn(FName CompanionID);
	void ApplyPreviewRotation();
	void FrameCameraToPreview();
	class UPrimitiveComponent* GetPreviewTargetComponent() const;
	void ApplyShadowSettings();

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Preview")
	TObjectPtr<USceneCaptureComponent2D> SceneCapture;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Preview")
	TObjectPtr<UPointLightComponent> PreviewLight;

	/** Fill light to keep shadows readable. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Preview")
	TObjectPtr<UPointLightComponent> FillLight;

	/** Rim light for separation from background. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Preview")
	TObjectPtr<UPointLightComponent> RimLight;

	/** Simple floor so "ground level" is visible in preview. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Preview")
	TObjectPtr<UStaticMeshComponent> PreviewFloor;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Preview")
	TObjectPtr<UTextureRenderTarget2D> RenderTarget;

	UPROPERTY(EditDefaultsOnly, Category = "Preview")
	FIntPoint RenderTargetSize = FIntPoint(1024, 1440);

	UPROPERTY(EditDefaultsOnly, Category = "Preview")
	TSubclassOf<AT66CompanionBase> CompanionPawnClass;

	UPROPERTY()
	TObjectPtr<AT66CompanionBase> PreviewPawn;

	UPROPERTY(EditDefaultsOnly, Category = "Preview")
	FVector PreviewPawnOffset = FVector::ZeroVector;

	UPROPERTY(Transient)
	float PreviewYawDegrees = 0.f;

	/** Orbit camera pitch (degrees). Negative looks slightly down. */
	UPROPERTY(Transient)
	float OrbitPitchDegrees = 8.f;

	/** Cached orbit framing so the platform doesn't "swim" as you rotate. */
	UPROPERTY(Transient)
	bool bHasOrbitFrame = false;

	UPROPERTY(Transient)
	FVector OrbitCenter = FVector::ZeroVector;

	UPROPERTY(Transient)
	float OrbitRadius = 200.f;

	UPROPERTY(Transient)
	float OrbitBottomZ = 0.f;

	/** Push the floor slightly back so the companion reads "forward" on it (toward camera). */
	UPROPERTY(EditDefaultsOnly, Category = "Preview|Tuning")
	float FloorForwardOffset = 35.f;

	/** Multiplier on the auto-framed camera distance (smaller = companion appears bigger). */
	UPROPERTY(EditDefaultsOnly, Category = "Preview|Tuning")
	float CameraDistanceMultiplier = 0.92f;

	/** User zoom factor applied on top of CameraDistanceMultiplier (<= 1.0 means zoom-in). */
	UPROPERTY(Transient)
	float PreviewZoomMultiplier = 1.0f;

	/** Minimum zoom-in multiplier (smaller = closer zoom). */
	UPROPERTY(EditDefaultsOnly, Category = "Preview|Tuning")
	float MinPreviewZoomMultiplier = 0.65f;

	/** If true, disable shadow casting for preview meshes/floor. */
	UPROPERTY(EditDefaultsOnly, Category = "Preview|Tuning")
	bool bDisablePreviewShadows = true;
};
