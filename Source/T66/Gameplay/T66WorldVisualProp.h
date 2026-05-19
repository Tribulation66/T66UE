// Copyright Tribulation 66. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Data/T66DataTypes.h"
#include "T66WorldVisualProp.generated.h"

class USceneComponent;
class UStaticMeshComponent;
class UDataTable;

/**
 * Data-authored, non-interactable world decoration.
 * Use this for visual-only dungeon props that should never present an interaction prompt.
 */
UCLASS(Blueprintable)
class T66_API AT66WorldVisualProp : public AActor
{
	GENERATED_BODY()

public:
	AT66WorldVisualProp();

	virtual void OnConstruction(const FTransform& Transform) override;
	virtual void BeginPlay() override;

	UFUNCTION(BlueprintCallable, Category = "Visual Prop")
	void SetPropRowID(FName InPropRowID);

	UFUNCTION(BlueprintCallable, Category = "Visual Prop")
	void RefreshVisuals();

	UFUNCTION(BlueprintPure, Category = "Visual Prop")
	const FT66WorldVisualPropData& GetResolvedPropData() const { return ResolvedPropData; }

protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<USceneComponent> SceneRoot;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<UStaticMeshComponent> VisualMesh;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Visual Prop")
	FName PropRowID = FName(TEXT("WallLamp"));

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Visual Prop")
	bool bUseDifficultySpecificRow = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Visual Prop")
	TSoftObjectPtr<UDataTable> PropDataTable;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Visual Prop")
	FT66WorldVisualPropData DefaultPropData;

	UPROPERTY(Transient, VisibleInstanceOnly, BlueprintReadOnly, Category = "Visual Prop")
	FT66WorldVisualPropData ResolvedPropData;

private:
	void RefreshResolvedPropData();
	void ApplyVisualData();
};
