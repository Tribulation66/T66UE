// Copyright Tribulation 66. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "T66MiasmaManager.generated.h"

class AT66MiasmaTile;
class AT66LavaPatch;
class UInstancedStaticMeshComponent;
class UMaterialInstanceDynamic;
class UTexture2D;

/**
 * Spawns thin miasma tiles across the active stage layout and grows coverage from an activation source.
 * Can follow the legacy stage timer or an explicit flood timer supplied by gameplay flow code.
 */
UCLASS(Blueprintable)
class T66_API AT66MiasmaManager : public AActor
{
	GENERATED_BODY()

public:
	AT66MiasmaManager();

	/** Half-extent of square area to cover (centered at world origin). (4x map = 2x linear) */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Miasma")
	float CoverageHalfExtent = 50000.f;

	/** Tile size (square). */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Miasma")
	float TileSize = 750.f;

	/** Miasma spawns slightly above ground. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Miasma")
	float TileZ = 5.f;

	/** Damage interval while the hero is standing on active lava/miasma tiles. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Miasma")
	float DamageIntervalSeconds = 1.0f;

	/** Spawn order random seed. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Miasma")
	int32 Seed = 1337;

	/** Total active expansion time needed to cover the full available miasma grid once the flood is active. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Miasma", meta = (ClampMin = "1.0", ClampMax = "600.0"))
	float FullCoverageSeconds = 300.f;

	/** Deprecated tuning knob retained for asset compatibility. Expansion now begins immediately when activated. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Miasma", meta = (ClampMin = "0.0", ClampMax = "60.0"))
	float ExpansionIntervalSeconds = 0.1f;

	/** Deprecated tuning knob retained for asset compatibility. Expansion now begins immediately when activated. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Miasma", meta = (ClampMin = "0.0", ClampMax = "420.0"))
	float ExpansionStartDelaySeconds = 0.f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Lava|Animation", meta = (ClampMin = "16", ClampMax = "256"))
	int32 TextureResolution = 64;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Lava|Animation", meta = (ClampMin = "4", ClampMax = "64"))
	int32 AnimationFrames = 18;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Lava|Animation", meta = (ClampMin = "1.0", ClampMax = "60.0"))
	float AnimationFPS = 12.f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Lava|Animation", meta = (ClampMin = "0.0", ClampMax = "5.0"))
	float WarpSpeed = 1.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Lava|Animation", meta = (ClampMin = "0.0", ClampMax = "1.5"))
	float WarpIntensity = 0.12f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Lava|Animation", meta = (ClampMin = "0.01", ClampMax = "10.0"))
	float WarpCloseness = 2.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Lava|Animation")
	FVector2D FlowDir = FVector2D(1.0f, 0.0f);

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Lava|Animation", meta = (ClampMin = "0.0", ClampMax = "5.0"))
	float FlowSpeed = 0.18f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Lava|Animation", meta = (ClampMin = "0.1", ClampMax = "30.0"))
	float UVScale = 4.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Lava|Look", meta = (ClampMin = "1.0", ClampMax = "24.0"))
	float CellDensity = 6.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Lava|Look", meta = (ClampMin = "1.0", ClampMax = "18.0"))
	float EdgeContrast = 7.5f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Lava|Look", meta = (ClampMin = "0.1", ClampMax = "6.0"))
	float Brightness = 2.2f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Lava|Look")
	FVector2D PatternOffset = FVector2D::ZeroVector;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Lava|Look")
	FLinearColor CoreColor = FLinearColor(0.09f, 0.01f, 0.00f, 1.0f);

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Lava|Look")
	FLinearColor MidColor = FLinearColor(0.78f, 0.11f, 0.02f, 1.0f);

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Lava|Look")
	FLinearColor GlowColor = FLinearColor(1.00f, 0.47f, 0.08f, 1.0f);

	/** When true, no ground miasma tiles are spawned or updated. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Miasma")
	bool bSpawningPaused = false;

	/** Clears all spawned miasma tiles. */
	void ClearAllMiasma();
	void RebuildForCurrentStage();
	int32 SpawnLegacyStageLavaPatchesForCurrentStage();

	/** Allows game flow code to drive the flood timer separately from the stage timer. */
	void SetExpansionActive(bool bActive, float ElapsedSeconds = 0.f);

	/** Overrides the source anchor used to order tower-floor coverage outward from a per-floor point. */
	void SetTowerSourceAnchor(int32 FloorNumber, const FVector& WorldAnchor);

	/** Clears all explicit tower source anchors so layout defaults are used again. */
	void ClearTowerSourceAnchors();

	/** Refreshes active coverage based on the legacy stage timer or an explicitly-driven flood timer. */
	void UpdateFromRunState();

protected:
	virtual void BeginPlay() override;
	virtual void Tick(float DeltaTime) override;

private:
	UPROPERTY()
	TObjectPtr<UInstancedStaticMeshComponent> TileInstances;

	UPROPERTY(Transient)
	TObjectPtr<UMaterialInstanceDynamic> LavaMID = nullptr;

	UPROPERTY(Transient)
	TArray<TObjectPtr<UTexture2D>> GeneratedFrames;

	UPROPERTY()
	TArray<FVector> TileCenters;

	TArray<int32> TileFloorNumbers;

	UPROPERTY()
	TArray<TWeakObjectPtr<AT66LavaPatch>> LegacyLavaPatches;

	TMap<int32, FVector> TowerDefaultSourceAnchors;
	TMap<int32, FVector> TowerSourceAnchorOverrides;

	int32 SpawnedTileCount = 0;
	float DamageTickAccumulator = 0.f;
	float AnimationStartTimeSeconds = 0.f;
	int32 CurrentFrameIndex = INDEX_NONE;
	bool bMaterialLookApplied = false;
	FLinearColor LastAppliedTint = FLinearColor::Transparent;
	float LastAppliedBrightness = -1.0f;
	bool bExplicitExpansionActive = false;
	float ExplicitExpansionStartTimeSeconds = 0.f;

	void BuildGrid();
	void EnsureSpawnedCount(int32 DesiredCount);
	void BuildMainMapCellGrid();
	void BuildTowerFloorGrid();
	void ApplyTowerCoverageOrdering();
	void RebuildSpawnedInstances();
	void TickDamageOverActiveTiles(float DeltaTime);
	void EnsureVisualMaterial();
	bool ShouldUseTowerBloodLook() const;
	bool TryGetExpansionElapsedSeconds(float& OutElapsedSeconds) const;
	FVector ResolveTowerSourceAnchor(int32 FloorNumber, const FVector& FallbackAnchor) const;
	void ClearLegacyLavaPatches();
	void GenerateAnimationFrames();
	UTexture2D* BuildFrameTexture(int32 FrameIndex, int32 ClampedFrames, int32 Resolution) const;
	FLinearColor SampleLavaColor(const FVector2D& BaseUV, float Phase) const;
	void ApplyAnimationFrame(int32 FrameIndex);
	void ApplyMaterialLookIfNeeded(const FLinearColor& Tint, float InBrightness);
};
