// Copyright Tribulation 66. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Data/T66DataTypes.h"
#include "T66HeroPreviewStage.generated.h"

class USceneCaptureComponent2D;
class UTextureRenderTarget2D;
class AT66HeroBase;
class USceneComponent;
class UPointLightComponent;

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
	 * Call when hero focus or body type changes.
	 */
	UFUNCTION(BlueprintCallable, Category = "Preview")
	void SetPreviewHero(FName HeroID, ET66BodyType BodyType);

	/** Capture the current scene to the render target (called after SetPreviewHero) */
	UFUNCTION(BlueprintCallable, Category = "Preview")
	void CapturePreview();

protected:
	virtual void BeginPlay() override;

	/** Create the render target and scene capture at runtime if not set in editor */
	void EnsureCaptureSetup();

	/** Spawn or update the preview hero pawn */
	void UpdatePreviewPawn(FName HeroID, ET66BodyType BodyType);

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Preview")
	TObjectPtr<USceneCaptureComponent2D> SceneCapture;

	/** Optional light so the preview hero is visible */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Preview")
	TObjectPtr<UPointLightComponent> PreviewLight;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Preview")
	TObjectPtr<UTextureRenderTarget2D> RenderTarget;

	/** Size of the render target (matches typical preview area) */
	UPROPERTY(EditDefaultsOnly, Category = "Preview")
	FIntPoint RenderTargetSize = FIntPoint(512, 512);

	/** Hero pawn class (same as gameplay - unified hero) */
	UPROPERTY(EditDefaultsOnly, Category = "Preview")
	TSubclassOf<AT66HeroBase> HeroPawnClass;

	/** Currently displayed preview pawn */
	UPROPERTY()
	TObjectPtr<AT66HeroBase> PreviewPawn;

	/** Spawn location for the preview pawn (relative to this actor) */
	UPROPERTY(EditDefaultsOnly, Category = "Preview")
	FVector PreviewPawnOffset = FVector(150.f, 0.f, 0.f);
};
