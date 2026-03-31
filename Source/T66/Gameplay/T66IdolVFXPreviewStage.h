// Copyright Tribulation 66. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Data/T66DataTypes.h"
#include "Gameplay/T66FireIdolAttackVFX.h"
#include "GameFramework/Actor.h"
#include "T66IdolVFXPreviewStage.generated.h"

class UStaticMeshComponent;
class USceneComponent;
class UTextRenderComponent;

UCLASS(NotBlueprintable)
class T66_API AT66IdolVFXPreviewStage : public AActor
{
	GENERATED_BODY()

public:
	AT66IdolVFXPreviewStage();

	UFUNCTION(CallInEditor, BlueprintCallable, Category = "Preview")
	void BuildPreview();

	UFUNCTION(CallInEditor, BlueprintCallable, Category = "Preview")
	void ClearPreview();

protected:
	virtual void BeginPlay() override;

private:
	void SpawnPierceRow(const FVector& RowOrigin);
	void SpawnBounceRow(const FVector& RowOrigin);
	void SpawnAOERow(const FVector& RowOrigin);
	void SpawnDOTRow(const FVector& RowOrigin);
	void AddSlotLabel(const FString& LabelText, const FVector& RelativeLocation);

	UPROPERTY(VisibleAnywhere)
	TObjectPtr<USceneComponent> SceneRoot;

	UPROPERTY(VisibleAnywhere)
	TObjectPtr<UStaticMeshComponent> PreviewFloor;

	UPROPERTY(Transient)
	TArray<TObjectPtr<AActor>> SpawnedPreviewActors;

	UPROPERTY(Transient)
	TArray<TObjectPtr<UTextRenderComponent>> SpawnedLabels;

	UPROPERTY(EditAnywhere, Category = "Preview")
	ET66ItemRarity PreviewRarity = ET66ItemRarity::White;

	UPROPERTY(EditAnywhere, Category = "Preview")
	float ColumnSpacing = 420.f;

	UPROPERTY(EditAnywhere, Category = "Preview")
	float RowSpacing = 470.f;

	UPROPERTY(EditAnywhere, Category = "Preview")
	bool bAutoBuildOnBeginPlay = true;

	uint8 FirePreviewCandidateRaw = static_cast<uint8>(ET66FireIdolTestCandidate::ZibraShockwave);
};
