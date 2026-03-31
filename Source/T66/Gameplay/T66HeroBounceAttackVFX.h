// Copyright Tribulation 66. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "T66HeroBounceAttackVFX.generated.h"

class UMaterialInstanceDynamic;
class UMaterialInterface;
class USceneComponent;
class UStaticMesh;
class UStaticMeshComponent;

USTRUCT()
struct FT66HeroBounceVisualPiece
{
	GENERATED_BODY()

	UPROPERTY()
	TObjectPtr<UStaticMeshComponent> Mesh = nullptr;

	UPROPERTY(Transient)
	TObjectPtr<UMaterialInstanceDynamic> MID = nullptr;

	FVector BaseLocation = FVector::ZeroVector;
	FRotator BaseRotation = FRotator::ZeroRotator;
	FVector BaseScale = FVector::OneVector;
	float Phase = 0.f;
	bool bImpact = false;
};

UCLASS(NotBlueprintable)
class T66_API AT66HeroBounceAttackVFX : public AActor
{
	GENERATED_BODY()

public:
	AT66HeroBounceAttackVFX();

	void SetPaletteOverride(const FLinearColor& InTint, const FLinearColor& InPrimary, const FLinearColor& InSecondary, const FLinearColor& InOutline, float InGlowStrength, FName InPaletteMode = NAME_None);
	void InitEffect(const TArray<FVector>& InChainPositions, const FLinearColor& InTint, int32 InDebugRequestId = INDEX_NONE, FName InDebugHeroID = NAME_None);

protected:
	virtual void BeginPlay() override;
	virtual void Tick(float DeltaSeconds) override;

private:
	void ResolveAssets();
	void BuildEffect();
	void ResolveRuntimePalette(FLinearColor& OutTintColor, FLinearColor& OutPrimaryColor, FLinearColor& OutSecondaryColor, FLinearColor& OutOutlineColor, float& OutGlowStrength, FString& OutPaletteMode) const;
	void UpdatePieces(float Age01) const;
	void LogConfiguredBegin() const;

	UPROPERTY(VisibleAnywhere)
	TObjectPtr<USceneComponent> SceneRoot;

	UPROPERTY()
	TObjectPtr<UStaticMesh> PlaneMesh = nullptr;

	UPROPERTY()
	TObjectPtr<UMaterialInterface> SegmentBaseMaterial = nullptr;

	UPROPERTY()
	TObjectPtr<UMaterialInterface> ImpactBaseMaterial = nullptr;

	UPROPERTY()
	TArray<TObjectPtr<UStaticMeshComponent>> SpawnedMeshes;

	UPROPERTY(Transient)
	TArray<TObjectPtr<UMaterialInstanceDynamic>> SpawnedMIDs;

	TArray<FT66HeroBounceVisualPiece> VisualPieces;
	TArray<FVector> ChainPositions;
	FLinearColor TintColor = FLinearColor::White;
	float ElapsedSeconds = 0.f;
	float LifetimeSeconds = 0.24f;
	int32 DebugRequestId = INDEX_NONE;
	FName DebugHeroID = NAME_None;
	FLinearColor OverrideTintColor = FLinearColor::White;
	FLinearColor OverridePrimaryColor = FLinearColor::White;
	FLinearColor OverrideSecondaryColor = FLinearColor(0.35f, 0.35f, 0.35f, 1.f);
	FLinearColor OverrideOutlineColor = FLinearColor::Black;
	float OverrideGlowStrength = 0.f;
	FName OverridePaletteMode = NAME_None;
	bool bUsePaletteOverride = false;
	bool bInitialized = false;
	bool bBuilt = false;
};
