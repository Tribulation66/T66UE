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

	UFUNCTION(BlueprintCallable, Category = "Preview")
	void CapturePreview();

protected:
	virtual void BeginPlay() override;
	void EnsureCaptureSetup();
	void UpdatePreviewPawn(FName CompanionID);

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Preview")
	TObjectPtr<USceneCaptureComponent2D> SceneCapture;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Preview")
	TObjectPtr<UPointLightComponent> PreviewLight;

	/** Simple floor so "ground level" is visible in preview. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Preview")
	TObjectPtr<UStaticMeshComponent> PreviewFloor;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Preview")
	TObjectPtr<UTextureRenderTarget2D> RenderTarget;

	UPROPERTY(EditDefaultsOnly, Category = "Preview")
	FIntPoint RenderTargetSize = FIntPoint(512, 512);

	UPROPERTY(EditDefaultsOnly, Category = "Preview")
	TSubclassOf<AT66CompanionBase> CompanionPawnClass;

	UPROPERTY()
	TObjectPtr<AT66CompanionBase> PreviewPawn;

	UPROPERTY(EditDefaultsOnly, Category = "Preview")
	FVector PreviewPawnOffset = FVector(150.f, 0.f, 0.f);
};
