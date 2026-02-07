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
class UMaterialInstanceDynamic;

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

	UFUNCTION(BlueprintCallable, Category = "Preview")
	void CapturePreview();

	/** Update preview lighting to match the current Dark/Light theme (day = sunlight, night = moonlight). */
	UFUNCTION(BlueprintCallable, Category = "Preview")
	void ApplyThemeLighting();

protected:
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
	virtual void Tick(float DeltaSeconds) override;
	void EnsureCaptureSetup();
	void UpdatePreviewPawn(FName CompanionID, FName SkinID);
	void ApplyPreviewRotation();
	void FrameCameraToPreview();
	class UPrimitiveComponent* GetPreviewTargetComponent() const;
	void ApplyShadowSettings();

	/** Called when player settings change (theme toggle). */
	UFUNCTION()
	void OnThemeChanged();

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

	/** Sky dome (inverted sphere) providing the background color. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Preview")
	TObjectPtr<UStaticMeshComponent> BackdropSphere;

	/** Ambient light inside the sky dome for uniform sky coloring. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Preview")
	TObjectPtr<UPointLightComponent> AmbientSkyLight;

	/** Dynamic material for floor coloring (grass tint). */
	UPROPERTY(Transient)
	TObjectPtr<UMaterialInstanceDynamic> FloorMID;

	/** Dynamic material for backdrop sphere coloring (sky tint). */
	UPROPERTY(Transient)
	TObjectPtr<UMaterialInstanceDynamic> BackdropMID;

	/** Star dot meshes (small spheres on upper hemisphere, visible in dark mode). */
	UPROPERTY(Transient)
	TArray<TObjectPtr<UStaticMeshComponent>> StarMeshes;

	/** Position offsets for each star relative to dome center. */
	TArray<FVector> StarOffsets;

	/** Shared dynamic material for all star dots. */
	UPROPERTY(Transient)
	TObjectPtr<UMaterialInstanceDynamic> StarMID;
};
