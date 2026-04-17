// Copyright Tribulation 66. All Rights Reserved.

#include "Gameplay/T66MainMapTerrain.h"

#include "Core/T66GameInstance.h"
#include "Data/T66DataTypes.h"
#include "Gameplay/T66LavaPatch.h"
#include "Gameplay/T66ProceduralLandscapeParams.h"
#include "Components/BoxComponent.h"
#include "Components/InstancedStaticMeshComponent.h"
#include "Components/PrimitiveComponent.h"
#include "Components/SceneComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/StaticMesh.h"
#include "Engine/Texture.h"
#include "Engine/Texture2D.h"
#include "Engine/World.h"
#include "GameFramework/Actor.h"
#include "KismetProceduralMeshLibrary.h"
#include "Materials/MaterialInterface.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "PhysicsEngine/BodySetup.h"
#include "ProceduralMeshComponent.h"
#include "TimerManager.h"
#include "UObject/UObjectGlobals.h"

DEFINE_LOG_CATEGORY_STATIC(LogT66MainMapTerrain, Log, All);

namespace T66MainMapTerrain
{
	namespace
	{
		static constexpr float SourceCellSizeUU = 200.0f;
		static constexpr float SourceStepHeightUU = 120.0f;
		static constexpr float TargetMainMapBoardScale = 10.0f;
		static constexpr int32 MainMapBoardSizeCells = 81;
		static constexpr int32 MainMapExtensionRoomRadiusCells = 2;
		static constexpr float TreeDecorationSpawnThreshold = 0.82f;
		static constexpr float RockDecorationSpawnThreshold = 0.69f;
		static constexpr int32 WallHeightLevels = 50;
		static constexpr bool bRenderFarmGrass = false;
		// Terrain entry was stalling because the board spawned thousands of optional
		// under-board support and per-cell decor components before gameplay became interactive.
		// Keep the walkable board and major props, and let the separate prop pass carry the clutter.
		static constexpr bool bRenderFarmDecor = false;
		static constexpr bool bRenderFarmSupports = false;
		static constexpr bool bRenderFarmBoundaryWalls = false;
		static constexpr bool bSpawnFarmPerimeterBoundary = false;
		static constexpr int32 GrassObjectCount = 30;
		static constexpr float GrassMinScale = 0.25f;
		static constexpr float GrassMaxScale = 1.0f;
		static constexpr float FlatGrassVariationX = 0.45f;
		static constexpr float FlatGrassVariationY = 0.45f;
		static constexpr float SlopeGrassVariationX = 0.45f;
		static constexpr float SlopeGrassVariationY = 0.50f;
		static constexpr float GrassDesiredWidthUU = 32.0f;
		static constexpr float GrassDesiredHeightUU = 32.0f;
		static constexpr float GrassAlternateLiftFraction = 0.05f;
		static constexpr float SurfaceFeatureAmplitudeUU = 1800.0f;
		static constexpr float SurfaceFeatureMeshBiasUU = 1.0f;
		static constexpr int32 SurfaceFeatureMinimumFootprintCells = 4;
		static constexpr int32 SurfaceFeatureGridResolutionPerCell = 3;
		static constexpr int32 SurfaceFeatureCollisionGridResolutionPerCell = 2;
		static constexpr int32 SurfaceFeatureFootprintPaddingCells = 1;
		static constexpr int32 SurfaceFeatureSafeRadiusCells = 3;
		static constexpr float SurfaceFeatureCountFactor = 0.18f;
		static constexpr int32 SurfaceFeatureMinCount = 8;
		static constexpr int32 SurfaceFeatureMaxCount = 16;
		static constexpr float SurfaceFeatureTargetWalkableAngleDegrees = 44.765083f;
		static constexpr float SurfaceFeatureWalkableSlopeSafetyMultiplier = 1.15f;
		static constexpr float SurfaceFeatureNoiseLowFrequency = 1.25f;
		static constexpr float SurfaceFeatureNoiseHighFrequency = 3.10f;
		static constexpr float SurfaceFeatureNoiseStrength = 0.30f;
		static constexpr float SurfaceFeatureCraterRimHeight = 0.28f;
		static constexpr float LavaRiverCountFactor = 0.08f;
		static constexpr int32 LavaRiverMinCount = 4;
		static constexpr int32 LavaRiverMaxCount = 8;
		static constexpr int32 LavaRiverMinLengthCells = 2;
		static constexpr int32 LavaRiverMaxLengthCells = 4;
		static constexpr float LavaRiverPuddleCellFraction = 0.25f;
		static constexpr float LavaRiverWidthCellFraction = 1.0f / 6.0f;
		static constexpr float LavaRiverHoverHeightUU = 1.0f;
		static const FName MapPlatformTag(TEXT("T66_Map_Platform"));
		static const FName FloorMainTag(TEXT("T66_Floor_Main"));
		static const FName FarmVisualTag(TEXT("T66_MainMapTerrain_Visual"));
		static const FName FarmMaterialsReadyTag(TEXT("T66_MainMapTerrain_MaterialsReady"));

		struct FLoadedAssets
		{
			UStaticMesh* BlockMesh = nullptr;
			UStaticMesh* SlopeMesh = nullptr;
			UStaticMesh* PlaneMesh = nullptr;
			UStaticMesh* GrassMesh = nullptr;
			UMaterialInterface* EnvironmentUnlitMaterial = nullptr;
			UMaterialInterface* BlockMaterial = nullptr;
			UMaterialInterface* SlopeMaterial = nullptr;
			UMaterialInterface* DirtMaterial = nullptr;
			UMaterialInterface* WallMaterial = nullptr;
			UTexture* BlockTexture = nullptr;
			UTexture* SlopeTexture = nullptr;
			UTexture* DirtTexture = nullptr;
			UTexture* WallTexture = nullptr;
			TArray<UMaterialInterface*> GrassMaterials;
			UStaticMesh* TreeMesh1 = nullptr;
			UStaticMesh* TreeMesh2 = nullptr;
			UStaticMesh* TreeMesh3 = nullptr;
			UStaticMesh* RockMesh1 = nullptr;
			UStaticMesh* RockMesh2 = nullptr;
			UStaticMesh* RockMesh3 = nullptr;
			UStaticMesh* LogMesh = nullptr;
		};

		struct FResolvedFarmMaterial
		{
			UMaterialInterface* Material = nullptr;
			bool bUsingRealTexture = false;
		};

		struct FDifficultyThemeAssetInfo
		{
			const TCHAR* FolderName = nullptr;
			const TCHAR* AssetSuffix = nullptr;
		};

		static FDifficultyThemeAssetInfo GetDifficultyThemeAssetInfo(ET66Difficulty Difficulty)
		{
			switch (Difficulty)
			{
			case ET66Difficulty::Medium: return { TEXT("VeryHardGraveyard"), TEXT("VeryHardGraveyard") };
			case ET66Difficulty::Hard: return { TEXT("ImpossibleNorthPole"), TEXT("ImpossibleNorthPole") };
			case ET66Difficulty::VeryHard: return { TEXT("PerditionMars"), TEXT("PerditionMars") };
			case ET66Difficulty::Impossible: return { TEXT("FinalHell"), TEXT("FinalHell") };
			case ET66Difficulty::Easy:
			default:
				return {};
			}
		}

		static UTexture2D* LoadDifficultyThemeTexture(ET66Difficulty Difficulty, const TCHAR* BaseAssetName)
		{
			const FDifficultyThemeAssetInfo ThemeInfo = GetDifficultyThemeAssetInfo(Difficulty);
			if (!ThemeInfo.FolderName || !ThemeInfo.AssetSuffix || !BaseAssetName)
			{
				return nullptr;
			}

			const FString AssetName = FString::Printf(TEXT("%s_%s"), BaseAssetName, ThemeInfo.AssetSuffix);
			const FString ThemeTexturePath = FString::Printf(
				TEXT("/Game/World/Terrain/MegabonkThemes/%s/%s.%s"),
				ThemeInfo.FolderName,
				*AssetName,
				*AssetName);

			static TMap<FString, UTexture2D*> CachedThemeTextures;
			if (UTexture2D** Existing = CachedThemeTextures.Find(ThemeTexturePath))
			{
				return *Existing;
			}

			UTexture2D* ThemeTexture = LoadObject<UTexture2D>(nullptr, *ThemeTexturePath);
			if (!ThemeTexture)
			{
				UE_LOG(LogT66MainMapTerrain, Warning, TEXT("[MAP] Missing difficulty terrain texture: %s"), *ThemeTexturePath);
				return nullptr;
			}

			CachedThemeTextures.Add(ThemeTexturePath, ThemeTexture);
			return ThemeTexture;
		}

		static void ApplyDifficultyTextureOverrides(FLoadedAssets& Assets, ET66Difficulty Difficulty)
		{
			if (UTexture2D* BlockOverride = LoadDifficultyThemeTexture(Difficulty, TEXT("T_MegabonkBlock")))
			{
				Assets.BlockTexture = BlockOverride;
			}

			if (UTexture2D* SlopeOverride = LoadDifficultyThemeTexture(Difficulty, TEXT("T_MegabonkSlope")))
			{
				Assets.SlopeTexture = SlopeOverride;
			}
		}

		static void ConfigureTerrainCollisionComponent(UPrimitiveComponent* Component)
		{
			if (!Component) return;

			Component->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
			Component->SetCollisionObjectType(ECC_WorldStatic);
			Component->SetCollisionResponseToAllChannels(ECR_Block);
			Component->SetGenerateOverlapEvents(false);
			Component->SetCanEverAffectNavigation(false);
			Component->SetVisibility(false, true);
			Component->SetHiddenInGame(true, true);
		}

		static void ConfigureTerrainVisualCollisionComponent(UPrimitiveComponent* Component)
		{
			if (!Component) return;

			Component->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
			Component->SetCollisionObjectType(ECC_WorldStatic);
			Component->SetCollisionResponseToAllChannels(ECR_Block);
			Component->SetGenerateOverlapEvents(false);
			Component->SetCanEverAffectNavigation(false);
		}

		static void ConfigureTerrainSafetyCatchComponent(UPrimitiveComponent* Component)
		{
			if (!Component) return;

			Component->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
			Component->SetCollisionObjectType(ECC_WorldDynamic);
			Component->SetCollisionResponseToAllChannels(ECR_Ignore);
			Component->SetCollisionResponseToChannel(ECC_Pawn, ECR_Block);
			Component->SetCollisionResponseToChannel(ECC_Vehicle, ECR_Block);
			Component->SetGenerateOverlapEvents(false);
			Component->SetCanEverAffectNavigation(false);
			Component->SetVisibility(false, true);
			Component->SetHiddenInGame(true, true);
		}

		static void EnableComplexCollisionForMesh(UStaticMesh* Mesh)
		{
			if (!Mesh)
			{
				return;
			}

			if (UBodySetup* BodySetup = Mesh->GetBodySetup())
			{
				if (BodySetup->CollisionTraceFlag != CTF_UseComplexAsSimple)
				{
					BodySetup->CollisionTraceFlag = CTF_UseComplexAsSimple;
					BodySetup->CreatePhysicsMeshes();
				}
			}
		}

		static int32 GetElevationLevelForTopSurfaceZ(const FSettings& Settings, float TopSurfaceZ)
		{
			return FMath::RoundToInt(
				(TopSurfaceZ - Settings.BaselineZ - Settings.StepHeight * 0.5f)
				/ FMath::Max(Settings.StepHeight, 1.0f));
		}

		static float GetTopSurfaceZForLevel(const FSettings& Settings, int32 Level)
		{
			return Settings.BaselineZ + static_cast<float>(Level) * Settings.StepHeight + Settings.StepHeight * 0.5f;
		}

		static FVector GetTerrainMeshScale(const FSettings& Settings)
		{
			return FVector(
				Settings.BoardScale,
				Settings.BoardScale,
				Settings.StepHeight / FMath::Max(SourceStepHeightUU, 1.0f));
		}

		static ET66MapCellShape GetSlopeShapeFromDirection(const FIntPoint& Direction)
		{
			if (Direction.X > 0) return ET66MapCellShape::SlopePosX;
			if (Direction.X < 0) return ET66MapCellShape::SlopeNegX;
			if (Direction.Y > 0) return ET66MapCellShape::SlopePosY;
			return ET66MapCellShape::SlopeNegY;
		}

		static FIntPoint GetOppositeDirection(const FIntPoint& Direction)
		{
			return FIntPoint(-Direction.X, -Direction.Y);
		}

		static int32 GetMinBoardElevationLevel(const FT66MapPreset& Preset)
		{
			const FSettings Settings = MakeSettings(Preset);
			return GetElevationLevelForTopSurfaceZ(Settings, FMath::Min(Preset.ElevationMin, Preset.ElevationMax));
		}

		static int32 GetMaxBoardElevationLevel(const FT66MapPreset& Preset)
		{
			const FSettings Settings = MakeSettings(Preset);
			return GetElevationLevelForTopSurfaceZ(Settings, FMath::Max(Preset.ElevationMin, Preset.ElevationMax));
		}

		static float GetCellCenterZ(const FSettings& Settings, const FCell& Cell)
		{
			return Settings.BaselineZ + static_cast<float>(Cell.Level) * Settings.StepHeight;
		}

		static float GetCellTopSurfaceZ(const FSettings& Settings, const FCell& Cell)
		{
			return GetTopSurfaceZForLevel(Settings, Cell.Level);
		}

		static FVector GetGridCellCenter(const FSettings& Settings, int32 GridX, int32 GridZ, float Z)
		{
			return FVector(
				-Settings.HalfExtent + (static_cast<float>(GridX) + 0.5f) * Settings.CellSize,
				-Settings.HalfExtent + (static_cast<float>(GridZ) + 0.5f) * Settings.CellSize,
				Z);
		}

		static bool IsPerimeterCoordinate(int32 X, int32 Z, int32 BoardSize)
		{
			return X == 0 || Z == 0 || X == BoardSize - 1 || Z == BoardSize - 1;
		}

		static bool IsCornerCoordinate(int32 X, int32 Z, int32 BoardSize)
		{
			const bool bMinX = X == 0;
			const bool bMaxX = X == BoardSize - 1;
			const bool bMinZ = Z == 0;
			const bool bMaxZ = Z == BoardSize - 1;
			return (bMinX || bMaxX) && (bMinZ || bMaxZ);
		}

		static FIntPoint GetPerimeterOutwardDirection(const FIntPoint& Coordinate, int32 BoardSize)
		{
			if (Coordinate.X <= 0) return FIntPoint(-1, 0);
			if (Coordinate.X >= BoardSize - 1) return FIntPoint(1, 0);
			if (Coordinate.Y <= 0) return FIntPoint(0, -1);
			return FIntPoint(0, 1);
		}

		static FIntPoint GetPreferredPerpendicularDirection(const FIntPoint& OutwardDirection, const FIntPoint& Anchor, int32 BoardSize)
		{
			if (OutwardDirection.X != 0)
			{
				return FIntPoint(0, Anchor.Y < BoardSize / 2 ? 1 : -1);
			}

			return FIntPoint(Anchor.X < BoardSize / 2 ? 1 : -1, 0);
		}

		static TArray<FIntPoint> BuildPerimeterCandidates(int32 BoardSize, bool bIncludeCorners)
		{
			TArray<FIntPoint> Candidates;
			for (int32 Z = 0; Z < BoardSize; ++Z)
			{
				for (int32 X = 0; X < BoardSize; ++X)
				{
					if (!IsPerimeterCoordinate(X, Z, BoardSize))
					{
						continue;
					}

					if (!bIncludeCorners && IsCornerCoordinate(X, Z, BoardSize))
					{
						continue;
					}

					Candidates.Add(FIntPoint(X, Z));
				}
			}
			return Candidates;
		}

		static FIntPoint ChooseStartAnchor(int32 BoardSize, FRandomStream& Rng)
		{
			TArray<FIntPoint> Candidates = BuildPerimeterCandidates(BoardSize, false);
			if (Candidates.Num() == 0)
			{
				Candidates = BuildPerimeterCandidates(BoardSize, true);
			}

			return Candidates.Num() > 0
				? Candidates[Rng.RandRange(0, Candidates.Num() - 1)]
				: FIntPoint::ZeroValue;
		}

		static FIntPoint ChooseBossAnchor(int32 BoardSize, const FIntPoint& StartAnchor, FRandomStream& Rng)
		{
			(void)Rng;
			if (StartAnchor.X <= 0)
			{
				return FIntPoint(BoardSize - 1, StartAnchor.Y);
			}
			if (StartAnchor.X >= BoardSize - 1)
			{
				return FIntPoint(0, StartAnchor.Y);
			}
			if (StartAnchor.Y <= 0)
			{
				return FIntPoint(StartAnchor.X, BoardSize - 1);
			}
			return FIntPoint(StartAnchor.X, 0);
		}

		static bool GetRegionCenterLocation(const FBoard& Board, ECellRegion Region, float HeightOffset, FVector& OutLocation)
		{
			FVector LocationSum = FVector::ZeroVector;
			int32 Count = 0;

			for (const FCell& Cell : Board.ExtraCells)
			{
				if (!Cell.bOccupied || Cell.Region != Region)
				{
					continue;
				}

				LocationSum += GetGridCellCenter(
					Board.Settings,
					Cell.X,
					Cell.Z,
					GetCellTopSurfaceZ(Board.Settings, Cell) + HeightOffset);
				++Count;
			}

			if (Count <= 0)
			{
				return false;
			}

			OutLocation = LocationSum / static_cast<float>(Count);
			return true;
		}

		static const FCell* FindAnyCellAtCoordinate(const FBoard& Board, const FIntPoint& Coordinate)
		{
			if (const FCell* MainCell = Board.GetCell(Coordinate.X, Coordinate.Y))
			{
				if (MainCell->bOccupied)
				{
					return MainCell;
				}
			}

			if (const FCell* ExtraCell = Board.FindExtraCell(Coordinate.X, Coordinate.Y))
			{
				if (ExtraCell->bOccupied)
				{
					return ExtraCell;
				}
			}

			return nullptr;
		}

		static bool GetCellLocation(const FBoard& Board, const FIntPoint& Coordinate, float HeightOffset, FVector& OutLocation)
		{
			const FCell* Cell = FindAnyCellAtCoordinate(Board, Coordinate);
			if (!Cell)
			{
				return false;
			}

			OutLocation = GetGridCellCenter(
				Board.Settings,
				Cell->X,
				Cell->Z,
				GetCellTopSurfaceZ(Board.Settings, *Cell) + HeightOffset);
			return true;
		}

		static bool GetCellLocationXY(const FBoard& Board, const FIntPoint& Coordinate, FVector2D& OutXY)
		{
			const FCell* Cell = FindAnyCellAtCoordinate(Board, Coordinate);
			if (!Cell)
			{
				return false;
			}

			const FVector Center = GetGridCellCenter(Board.Settings, Cell->X, Cell->Z, 0.0f);
			OutXY = FVector2D(Center.X, Center.Y);
			return true;
		}

		static bool GetRegionCenterXY(const FBoard& Board, ECellRegion Region, FVector2D& OutXY)
		{
			FVector2D PositionSum = FVector2D::ZeroVector;
			int32 Count = 0;

			for (const FCell& Cell : Board.ExtraCells)
			{
				if (!Cell.bOccupied || Cell.Region != Region)
				{
					continue;
				}

				const FVector Center = GetGridCellCenter(Board.Settings, Cell.X, Cell.Z, 0.0f);
				PositionSum += FVector2D(Center.X, Center.Y);
				++Count;
			}

			if (Count <= 0)
			{
				return false;
			}

			OutXY = PositionSum / static_cast<float>(Count);
			return true;
		}

		static void AddExtensionCell(FBoard& Board, const FIntPoint& Coordinate, int32 Level, ECellRegion Region)
		{
			if (Board.FindExtraCell(Coordinate.X, Coordinate.Y))
			{
				return;
			}

			FCell& Cell = Board.ExtraCells.AddDefaulted_GetRef();
			Cell.bOccupied = true;
			Cell.bSlope = false;
			Cell.X = Coordinate.X;
			Cell.Z = Coordinate.Y;
			Cell.Level = Level;
			Cell.Shape = ET66MapCellShape::Flat;
			Cell.Decoration = ET66MapCellDecoration::None;
			Cell.SurfaceFeature = ET66MapCellSurfaceFeature::None;
			Cell.SurfaceFeatureOrigin = FIntPoint(INDEX_NONE, INDEX_NONE);
			Cell.Region = Region;
			Cell.DecorationLocalOffset = FVector::ZeroVector;
			Cell.DecorationLocalRotation = FRotator::ZeroRotator;
			Cell.DecorationLocalScale = FVector(1.0f, 1.0f, 1.0f);
		}

		static void ClearDecoration(FCell& Cell)
		{
			Cell.Decoration = ET66MapCellDecoration::None;
			Cell.DecorationLocalOffset = FVector::ZeroVector;
			Cell.DecorationLocalRotation = FRotator::ZeroRotator;
			Cell.DecorationLocalScale = FVector(1.0f, 1.0f, 1.0f);
		}

		static void ClearSurfaceFeature(FCell& Cell)
		{
			Cell.SurfaceFeature = ET66MapCellSurfaceFeature::None;
			Cell.SurfaceFeatureOrigin = FIntPoint(INDEX_NONE, INDEX_NONE);
		}

		static bool HasSurfaceFeature(const FCell& Cell)
		{
			return Cell.SurfaceFeature != ET66MapCellSurfaceFeature::None;
		}

		static bool IsSurfaceFeatureOriginCell(const FCell& Cell)
		{
			return HasSurfaceFeature(Cell)
				&& Cell.SurfaceFeatureOrigin == FIntPoint(Cell.X, Cell.Z);
		}

		static float GetSurfaceFeatureProfile(float LocalX, float LocalY, float HalfExtentUU)
		{
			if (HalfExtentUU <= KINDA_SMALL_NUMBER)
			{
				return 0.0f;
			}

			const float Radial = FMath::Clamp(
				FMath::Sqrt(FMath::Square(LocalX / HalfExtentUU) + FMath::Square(LocalY / HalfExtentUU)),
				0.0f,
				1.0f);
			float Profile = 1.0f - Radial;
			Profile = FMath::Clamp(Profile, 0.0f, 1.0f);
			return Profile * Profile * (3.0f - 2.0f * Profile);
		}

		static float GetSurfaceFeatureBaseHeightFromRadial(ET66MapCellSurfaceFeature SurfaceFeature, float Radial)
		{
			const float BaseProfile = GetSurfaceFeatureProfile(FMath::Clamp(Radial, 0.0f, 1.0f), 0.0f, 1.0f);
			if (BaseProfile <= KINDA_SMALL_NUMBER)
			{
				return 0.0f;
			}

			if (SurfaceFeature == ET66MapCellSurfaceFeature::Hill)
			{
				const float HillProfile = FMath::Pow(BaseProfile, 0.62f);
				return SurfaceFeatureAmplitudeUU * HillProfile;
			}

			const float ClampedRadial = FMath::Clamp(Radial, 0.0f, 1.0f);
			const float CraterBowl = FMath::Pow(BaseProfile, 0.92f);
			const float RimOuter = FMath::SmoothStep(0.34f, 0.70f, ClampedRadial);
			const float RimInner = FMath::SmoothStep(0.70f, 0.98f, ClampedRadial);
			const float RimBand = FMath::Clamp(RimOuter - RimInner, 0.0f, 1.0f);
			return (-SurfaceFeatureAmplitudeUU * CraterBowl)
				+ (SurfaceFeatureAmplitudeUU * SurfaceFeatureCraterRimHeight * RimBand);
		}

		static float GetSurfaceFeatureMaxRisePerNormalizedRadius(ET66MapCellSurfaceFeature SurfaceFeature)
		{
			static constexpr int32 SampleCount = 4096;
			float MaxSlope = 0.0f;
			float PreviousHeight = GetSurfaceFeatureBaseHeightFromRadial(SurfaceFeature, 0.0f);

			for (int32 SampleIndex = 1; SampleIndex <= SampleCount; ++SampleIndex)
			{
				const float Radial = static_cast<float>(SampleIndex) / static_cast<float>(SampleCount);
				const float Height = GetSurfaceFeatureBaseHeightFromRadial(SurfaceFeature, Radial);
				const float Slope = FMath::Abs(Height - PreviousHeight) * static_cast<float>(SampleCount);
				MaxSlope = FMath::Max(MaxSlope, Slope);
				PreviousHeight = Height;
			}

			return MaxSlope;
		}

		static int32 GetSurfaceFeatureFootprintCellCount()
		{
			static const int32 CachedFootprintCellCount = []() -> int32
			{
				const float CellSizeUU = SourceCellSizeUU * TargetMainMapBoardScale;
				const float WalkableSlope = FMath::Tan(FMath::DegreesToRadians(SurfaceFeatureTargetWalkableAngleDegrees));
				if (CellSizeUU <= KINDA_SMALL_NUMBER || WalkableSlope <= KINDA_SMALL_NUMBER)
				{
					return SurfaceFeatureMinimumFootprintCells;
				}

				const float MaxRisePerNormalizedRadius = FMath::Max(
					GetSurfaceFeatureMaxRisePerNormalizedRadius(ET66MapCellSurfaceFeature::Hill),
					GetSurfaceFeatureMaxRisePerNormalizedRadius(ET66MapCellSurfaceFeature::Crater));
				const float RequiredHalfExtentUU =
					(MaxRisePerNormalizedRadius * SurfaceFeatureWalkableSlopeSafetyMultiplier) / WalkableSlope;
				const float RequiredFeatureSizeUU = RequiredHalfExtentUU * 2.0f;
				const float RequiredFootprintCells = RequiredFeatureSizeUU / CellSizeUU;
				return FMath::Max(
					SurfaceFeatureMinimumFootprintCells,
					FMath::CeilToInt(RequiredFootprintCells));
			}();

			return CachedFootprintCellCount;
		}

		static float SampleSurfaceFeatureNoise(int32 Seed, const FIntPoint& Origin, float LocalX, float LocalY, float FeatureSizeUU)
		{
			if (FeatureSizeUU <= KINDA_SMALL_NUMBER)
			{
				return 0.0f;
			}

			const FVector2D NormalizedPosition(
				LocalX / FeatureSizeUU + static_cast<float>(Origin.X) * 0.173f + static_cast<float>(Seed & 1023) * 0.0071f,
				LocalY / FeatureSizeUU + static_cast<float>(Origin.Y) * 0.167f + static_cast<float>((Seed >> 10) & 1023) * 0.0067f);
			const float Low = FMath::PerlinNoise2D(NormalizedPosition * SurfaceFeatureNoiseLowFrequency);
			const float High = FMath::PerlinNoise2D(NormalizedPosition * SurfaceFeatureNoiseHighFrequency);
			return Low * 0.7f + High * 0.3f;
		}

		static float GetSurfaceFeatureHeight(
			ET66MapCellSurfaceFeature SurfaceFeature,
			int32 Seed,
			const FIntPoint& Origin,
			float LocalX,
			float LocalY,
			float HalfExtentUU)
		{
			if (SurfaceFeature == ET66MapCellSurfaceFeature::None || HalfExtentUU <= KINDA_SMALL_NUMBER)
			{
				return 0.0f;
			}

			const float FeatureSizeUU = HalfExtentUU * 2.0f;
			const float BaseProfile = GetSurfaceFeatureProfile(LocalX, LocalY, HalfExtentUU);
			if (BaseProfile <= KINDA_SMALL_NUMBER)
			{
				return 0.0f;
			}

			const float Noise = SampleSurfaceFeatureNoise(Seed, Origin, LocalX, LocalY, FeatureSizeUU);
			const float NoiseMultiplier = FMath::Clamp(1.0f + Noise * SurfaceFeatureNoiseStrength, 0.72f, 1.28f);

			if (SurfaceFeature == ET66MapCellSurfaceFeature::Hill)
			{
				const float HillProfile = FMath::Pow(BaseProfile, 0.62f);
				return SurfaceFeatureAmplitudeUU * HillProfile * NoiseMultiplier;
			}

			const float Radial = FMath::Clamp(
				FMath::Sqrt(FMath::Square(LocalX / HalfExtentUU) + FMath::Square(LocalY / HalfExtentUU)),
				0.0f,
				1.0f);
			const float CraterBowl = FMath::Pow(BaseProfile, 0.92f);
			const float RimOuter = FMath::SmoothStep(0.34f, 0.70f, Radial);
			const float RimInner = FMath::SmoothStep(0.70f, 0.98f, Radial);
			const float RimBand = FMath::Clamp(RimOuter - RimInner, 0.0f, 1.0f);
			return (-SurfaceFeatureAmplitudeUU * CraterBowl * NoiseMultiplier)
				+ (SurfaceFeatureAmplitudeUU * SurfaceFeatureCraterRimHeight * RimBand);
		}

		static FIntPoint BuildExtensionArea(
			FBoard& Board,
			const FIntPoint& Anchor,
			const FIntPoint& OutwardDirection,
			int32 RoomRadius,
			ECellRegion PathRegion,
			ECellRegion AreaRegion,
			FIntPoint* OutPathCoordinate = nullptr,
			FIntPoint* OutSpawnCoordinate = nullptr)
		{
			const FCell* AnchorCell = Board.GetCell(Anchor.X, Anchor.Y);
			if (!AnchorCell)
			{
				return FIntPoint(INDEX_NONE, INDEX_NONE);
			}

			const FIntPoint WidthDirection = GetPreferredPerpendicularDirection(
				OutwardDirection,
				Anchor,
				Board.Settings.BoardSize);
			const FIntPoint PathCoordinate = Anchor + OutwardDirection;
			const FIntPoint SpawnCoordinate = Anchor + OutwardDirection * (RoomRadius + 2);

			if (OutPathCoordinate)
			{
				*OutPathCoordinate = PathCoordinate;
			}
			if (OutSpawnCoordinate)
			{
				*OutSpawnCoordinate = SpawnCoordinate;
			}

			AddExtensionCell(Board, PathCoordinate, AnchorCell->Level, PathRegion);
			for (int32 DepthOffset = -RoomRadius; DepthOffset <= RoomRadius; ++DepthOffset)
			{
				for (int32 WidthOffset = -RoomRadius; WidthOffset <= RoomRadius; ++WidthOffset)
				{
					AddExtensionCell(
						Board,
						SpawnCoordinate + OutwardDirection * DepthOffset + WidthDirection * WidthOffset,
						AnchorCell->Level,
						AreaRegion);
				}
			}

			return SpawnCoordinate;
		}

		static const FCell* FindPreferredSpawnCell(const FBoard& Board)
		{
			if (const FCell* StartSpawnCell = FindAnyCellAtCoordinate(Board, Board.StartSpawnCell))
			{
				if (!StartSpawnCell->bSlope)
				{
					return StartSpawnCell;
				}
			}

			const int32 Center = Board.Settings.BoardSize / 2;
			const FCell* BestCell = nullptr;
			int32 BestDistanceSq = TNumericLimits<int32>::Max();

			auto ConsiderCell = [&](const FCell* Candidate)
			{
				if (!Candidate || !Candidate->bOccupied || Candidate->bSlope)
				{
					return;
				}

				const int32 DX = Candidate->X - Center;
				const int32 DZ = Candidate->Z - Center;
				const int32 DistanceSq = DX * DX + DZ * DZ;
				if (!BestCell || DistanceSq < BestDistanceSq)
				{
					BestCell = Candidate;
					BestDistanceSq = DistanceSq;
				}
			};

			for (const FCell& Cell : Board.Cells)
			{
				ConsiderCell(&Cell);
			}

			if (BestCell)
			{
				return BestCell;
			}

			for (const FCell& Cell : Board.Cells)
			{
				if (Cell.bOccupied)
				{
					return &Cell;
				}
			}

			return nullptr;
		}

		static void GetOccupiedGridBounds(
			const FBoard& Board,
			int32& OutMinX,
			int32& OutMaxX,
			int32& OutMinZ,
			int32& OutMaxZ)
		{
			OutMinX = TNumericLimits<int32>::Max();
			OutMaxX = TNumericLimits<int32>::Lowest();
			OutMinZ = TNumericLimits<int32>::Max();
			OutMaxZ = TNumericLimits<int32>::Lowest();

			auto ConsiderCell = [&](const FCell& Cell)
			{
				if (!Cell.bOccupied)
				{
					return;
				}

				OutMinX = FMath::Min(OutMinX, Cell.X);
				OutMaxX = FMath::Max(OutMaxX, Cell.X);
				OutMinZ = FMath::Min(OutMinZ, Cell.Z);
				OutMaxZ = FMath::Max(OutMaxZ, Cell.Z);
			};

			for (const FCell& Cell : Board.Cells)
			{
				ConsiderCell(Cell);
			}
			for (const FCell& Cell : Board.ExtraCells)
			{
				ConsiderCell(Cell);
			}

			if (OutMinX == TNumericLimits<int32>::Max())
			{
				OutMinX = 0;
				OutMaxX = Board.Settings.BoardSize - 1;
				OutMinZ = 0;
				OutMaxZ = Board.Settings.BoardSize - 1;
			}
		}

		static float GetSlopeYaw(ET66MapCellShape Shape)
		{
			// Unity's direction-to-yaw table assumes Unity's axis conventions.
			// In Unreal, this imported slope rises toward +Y at yaw 0, but the X-axis
			// directions need to be mirrored to match the Unity-facing result.
			switch (Shape)
			{
			case ET66MapCellShape::SlopePosX:
				return 270.0f;
			case ET66MapCellShape::SlopeNegX:
				return 90.0f;
			case ET66MapCellShape::SlopePosY:
				return 0.0f;
			case ET66MapCellShape::SlopeNegY:
				return 180.0f;
			default:
				return 0.0f;
			}
		}

		static void AssignDecoration(FCell& Cell, FRandomStream& Rng, const FSettings& Settings)
		{
			const float ObjectToSpawn = Rng.FRand();
			const float VerticalScale = Settings.StepHeight / 12.0f;
			const float TopSurfaceOffset = Settings.StepHeight * 0.5f;
			const float ImportedScale = Settings.BoardScale;
			const float TreeJitter = Settings.CellSize * 0.34f;
			const float RockJitter = Settings.CellSize * 0.40f;

			if (ObjectToSpawn > TreeDecorationSpawnThreshold)
			{
				const float TreeScale = Rng.FRandRange(0.22f, 0.32f) * ImportedScale;
				switch (Rng.RandRange(0, 2))
				{
				case 0: Cell.Decoration = ET66MapCellDecoration::Tree1; break;
				case 1: Cell.Decoration = ET66MapCellDecoration::Tree2; break;
				default: Cell.Decoration = ET66MapCellDecoration::Tree3; break;
				}

				Cell.DecorationLocalOffset = FVector(
					Rng.FRandRange(-TreeJitter, TreeJitter),
					Rng.FRandRange(-TreeJitter, TreeJitter),
					TopSurfaceOffset + Rng.FRandRange(0.12f, 0.36f) * VerticalScale);
				Cell.DecorationLocalRotation = FRotator(
					Rng.FRandRange(-3.5f, 3.5f),
					Rng.FRandRange(0.0f, 360.0f),
					Rng.FRandRange(-3.5f, 3.5f));
				Cell.DecorationLocalScale = FVector(TreeScale);
				return;
			}

			if (ObjectToSpawn > RockDecorationSpawnThreshold)
			{
				const float RockScale = Rng.FRandRange(0.22f, 0.58f) * ImportedScale;
				Cell.Decoration = (Rng.FRand() < 0.5f) ? ET66MapCellDecoration::Rock : ET66MapCellDecoration::Rocks;
				Cell.DecorationLocalOffset = FVector(
					Rng.FRandRange(-RockJitter, RockJitter),
					Rng.FRandRange(-RockJitter, RockJitter),
					TopSurfaceOffset + Rng.FRandRange(0.08f, 0.28f) * VerticalScale);
				Cell.DecorationLocalRotation = FRotator(
					Rng.FRandRange(-10.0f, 10.0f),
					Rng.FRandRange(0.0f, 360.0f),
					Rng.FRandRange(-10.0f, 10.0f));
				Cell.DecorationLocalScale = FVector(
					RockScale * Rng.FRandRange(0.82f, 1.18f),
					RockScale * Rng.FRandRange(0.82f, 1.18f),
					RockScale * Rng.FRandRange(0.70f, 1.12f));
				return;
			}

			Cell.Decoration = ET66MapCellDecoration::None;
			Cell.DecorationLocalOffset = FVector::ZeroVector;
			Cell.DecorationLocalRotation = FRotator::ZeroRotator;
			Cell.DecorationLocalScale = FVector(1.0f, 1.0f, 1.0f);
		}

		static bool BuildScaledStaticMeshTransform(
			const UStaticMesh* Mesh,
			const FVector& DesiredCenter,
			const FVector& DesiredSize,
			const FRotator& Rotation,
			FTransform& OutTransform)
		{
			if (!Mesh)
			{
				return false;
			}

			const FBox MeshBounds = Mesh->GetBoundingBox();
			const FVector MeshSize = MeshBounds.GetSize();
			if (MeshSize.X <= KINDA_SMALL_NUMBER || MeshSize.Y <= KINDA_SMALL_NUMBER || MeshSize.Z <= KINDA_SMALL_NUMBER)
			{
				return false;
			}

			const FVector Scale(
				DesiredSize.X / MeshSize.X,
				DesiredSize.Y / MeshSize.Y,
				DesiredSize.Z / MeshSize.Z);
			const FVector ScaledLocalCenter = MeshBounds.GetCenter() * Scale;
			const FQuat RotationQuat = Rotation.Quaternion();
			OutTransform = FTransform(RotationQuat, DesiredCenter - RotationQuat.RotateVector(ScaledLocalCenter), Scale);
			return true;
		}

		static UStaticMesh* PickDecorationMesh(const FLoadedAssets& Assets, ET66MapCellDecoration Decoration)
		{
			switch (Decoration)
			{
			case ET66MapCellDecoration::Tree1: return Assets.TreeMesh1;
			case ET66MapCellDecoration::Tree2: return Assets.TreeMesh2;
			case ET66MapCellDecoration::Tree3: return Assets.TreeMesh3;
			case ET66MapCellDecoration::Rock: return Assets.RockMesh1 ? Assets.RockMesh1 : Assets.RockMesh2;
			case ET66MapCellDecoration::Rocks: return Assets.RockMesh3 ? Assets.RockMesh3 : Assets.RockMesh2;
			case ET66MapCellDecoration::Log: return Assets.LogMesh;
			default: return nullptr;
			}
		}

		static bool LoadAssets(FLoadedAssets& OutAssets)
		{
			static FLoadedAssets CachedLoadedAssets;
			if (IsValid(CachedLoadedAssets.BlockMesh) && IsValid(CachedLoadedAssets.SlopeMesh))
			{
				OutAssets = CachedLoadedAssets;
				return true;
			}

			OutAssets.BlockMesh = LoadObject<UStaticMesh>(nullptr, TEXT("/Game/World/Terrain/Megabonk/SM_MegabonkBlock.SM_MegabonkBlock"));
			OutAssets.SlopeMesh = LoadObject<UStaticMesh>(nullptr, TEXT("/Game/World/Terrain/Megabonk/SM_MegabonkSlope.SM_MegabonkSlope"));
			if (!OutAssets.BlockMesh || !OutAssets.SlopeMesh)
			{
				return false;
			}

			OutAssets.EnvironmentUnlitMaterial = LoadObject<UMaterialInterface>(nullptr, TEXT("/Game/Materials/M_Environment_Unlit.M_Environment_Unlit"));
			OutAssets.BlockMaterial = LoadObject<UMaterialInterface>(nullptr, TEXT("/Game/World/Terrain/Megabonk/MI_MegabonkBlock.MI_MegabonkBlock"));
			OutAssets.SlopeMaterial = LoadObject<UMaterialInterface>(nullptr, TEXT("/Game/World/Terrain/Megabonk/MI_MegabonkSlope.MI_MegabonkSlope"));
			OutAssets.DirtMaterial = LoadObject<UMaterialInterface>(nullptr, TEXT("/Game/World/Terrain/Megabonk/MI_MegabonkDirt.MI_MegabonkDirt"));
			OutAssets.WallMaterial = LoadObject<UMaterialInterface>(nullptr, TEXT("/Game/World/Terrain/Megabonk/MI_MegabonkWall.MI_MegabonkWall"));
			OutAssets.BlockTexture = LoadObject<UTexture>(nullptr, TEXT("/Game/World/Terrain/Megabonk/T_MegabonkBlock.T_MegabonkBlock"));
			OutAssets.SlopeTexture = LoadObject<UTexture>(nullptr, TEXT("/Game/World/Terrain/Megabonk/T_MegabonkSlope.T_MegabonkSlope"));
			OutAssets.DirtTexture = LoadObject<UTexture>(nullptr, TEXT("/Game/World/Terrain/Megabonk/T_MegabonkDirt.T_MegabonkDirt"));
			OutAssets.WallTexture = LoadObject<UTexture>(nullptr, TEXT("/Game/World/Terrain/Megabonk/T_MegabonkWall.T_MegabonkWall"));
			if (!OutAssets.BlockMaterial)
			{
				OutAssets.BlockMaterial = OutAssets.SlopeMaterial;
			}
			if (!OutAssets.SlopeMaterial)
			{
				OutAssets.SlopeMaterial = OutAssets.BlockMaterial;
			}
			if (!OutAssets.WallMaterial)
			{
				OutAssets.WallMaterial = OutAssets.BlockMaterial;
			}

			OutAssets.PlaneMesh = LoadObject<UStaticMesh>(nullptr, TEXT("/Engine/BasicShapes/Plane.Plane"));
			OutAssets.GrassMesh = LoadObject<UStaticMesh>(nullptr, TEXT("/Game/World/Props/Grass.Grass"));
			if (UMaterialInterface* GrassMaterial = LoadObject<UMaterialInterface>(nullptr, TEXT("/Game/World/Props/Grass/Materials/Material_0_014.Material_0_014")))
			{
				OutAssets.GrassMaterials.Add(GrassMaterial);
			}

			OutAssets.TreeMesh1 = LoadObject<UStaticMesh>(nullptr, TEXT("/Game/World/Props/Tree.Tree"));
			OutAssets.TreeMesh2 = LoadObject<UStaticMesh>(nullptr, TEXT("/Game/World/Props/Tree2.Tree2"));
			OutAssets.TreeMesh3 = LoadObject<UStaticMesh>(nullptr, TEXT("/Game/World/Props/Tree3.Tree3"));
			OutAssets.RockMesh1 = LoadObject<UStaticMesh>(nullptr, TEXT("/Game/World/Props/Rocks.Rocks"));
			OutAssets.RockMesh2 = OutAssets.RockMesh1;
			OutAssets.RockMesh3 = OutAssets.RockMesh1;
			OutAssets.LogMesh = LoadObject<UStaticMesh>(nullptr, TEXT("/Game/World/Props/Log.Log"));
			UE_LOG(LogT66MainMapTerrain, Log, TEXT("[MAP] Main map terrain assets: DirtMaterial=%s DirtTexture=%s WallMaterial=%s WallTexture=%s Tree1=%s Tree2=%s Tree3=%s Rock=%s Rocks=%s"),
				OutAssets.DirtMaterial ? TEXT("yes") : TEXT("no"),
				OutAssets.DirtTexture ? TEXT("yes") : TEXT("no"),
				OutAssets.WallMaterial ? TEXT("yes") : TEXT("no"),
				OutAssets.WallTexture ? TEXT("yes") : TEXT("no"),
				OutAssets.TreeMesh1 ? TEXT("yes") : TEXT("no"),
				OutAssets.TreeMesh2 ? TEXT("yes") : TEXT("no"),
				OutAssets.TreeMesh3 ? TEXT("yes") : TEXT("no"),
				OutAssets.RockMesh1 ? TEXT("yes") : TEXT("no"),
				OutAssets.RockMesh2 ? TEXT("yes") : TEXT("no"));
			CachedLoadedAssets = OutAssets;
			return true;
		}
	}

	FT66MapPreset BuildPresetForDifficulty(ET66Difficulty Difficulty, int32 Seed)
	{
		FT66MapPreset Preset = FT66MapPreset::GetDefaultForTheme(ET66MapTheme::Farm);
		Preset.Seed = Seed;
		Preset.LayoutVariant = ET66MainMapLayoutVariant::Tower;

		switch (Difficulty)
		{
		case ET66Difficulty::Easy:
		case ET66Difficulty::Medium:
		case ET66Difficulty::Hard:
		case ET66Difficulty::VeryHard:
		case ET66Difficulty::Impossible:
		default:
			Preset.ElevationMin = -5000.0f;
			Preset.ElevationMax = 5000.0f;
			break;
		}

		Preset.FarmHilliness = 0.0f;

		return Preset;
	}

	FSettings MakeSettings(const FT66MapPreset& Preset)
	{
		FSettings Settings;
		Settings.BoardSize = MainMapBoardSizeCells;
		Settings.BoardScale = TargetMainMapBoardScale;
		Settings.CellSize = SourceCellSizeUU * Settings.BoardScale;
		Settings.StepHeight = FMath::Max(Preset.ElevationStep, 1.0f);
		Settings.HalfExtent = Settings.CellSize * static_cast<float>(Settings.BoardSize) * 0.5f;
		Settings.BaselineZ = Preset.BaselineZ;
		Settings.Hilliness = FMath::Clamp(Preset.FarmHilliness, 0.0f, 1.0f);
		return Settings;
	}

	FVector GetBoardOrigin(const FT66MapPreset& Preset)
	{
		const FSettings Settings = MakeSettings(Preset);
		return FVector(
			-Settings.HalfExtent + Settings.CellSize * 0.5f,
			-Settings.HalfExtent + Settings.CellSize * 0.5f,
			Settings.BaselineZ);
	}

	FVector GetCellCenter(const FT66MapPreset& Preset, int32 Row, int32 Col, float Z)
	{
		const FSettings Settings = MakeSettings(Preset);
		const int32 SafeRow = FMath::Clamp(Row, 0, Settings.BoardSize - 1);
		const int32 SafeCol = FMath::Clamp(Col, 0, Settings.BoardSize - 1);
		return GetGridCellCenter(Settings, SafeCol, SafeRow, Z);
	}

	bool TryGetCellLocation(const FBoard& Board, const FIntPoint& Coordinate, float HeightOffset, FVector& OutLocation)
	{
		return GetCellLocation(Board, Coordinate, HeightOffset, OutLocation);
	}

	bool TryGetRegionCenterLocation(const FBoard& Board, ECellRegion Region, float HeightOffset, FVector& OutLocation)
	{
		return GetRegionCenterLocation(Board, Region, HeightOffset, OutLocation);
	}

	FVector GetSpawnLocation(const FBoard& Board, float Z)
	{
		if (const FCell* SpawnCell = FindPreferredSpawnCell(Board))
		{
			return GetGridCellCenter(Board.Settings, SpawnCell->X, SpawnCell->Z, Z);
		}

		return GetGridCellCenter(
			Board.Settings,
			Board.Settings.BoardSize / 2,
			Board.Settings.BoardSize / 2,
			Z);
	}

	FVector GetSpawnLocation(const FT66MapPreset& Preset, float Z)
	{
		FBoard Board;
		if (Generate(Preset, Board))
		{
			return GetSpawnLocation(Board, Z);
		}

		const FSettings Settings = MakeSettings(Preset);
		return GetCellCenter(Preset, Settings.BoardSize / 2, Settings.BoardSize / 2, Z);
	}

	FVector GetPreferredSpawnLocation(const FBoard& Board, float HeightOffset)
	{
		const FCell* SpawnCell = FindPreferredSpawnCell(Board);
		if (!SpawnCell)
		{
			return GetGridCellCenter(
				Board.Settings,
				Board.Settings.BoardSize / 2,
				Board.Settings.BoardSize / 2,
				Board.Settings.BaselineZ + Board.Settings.StepHeight + HeightOffset);
		}

		return GetGridCellCenter(
			Board.Settings,
			SpawnCell->X,
			SpawnCell->Z,
			GetCellTopSurfaceZ(Board.Settings, *SpawnCell) + HeightOffset);
	}

	FVector GetPreferredSpawnLocation(const FT66MapPreset& Preset, float HeightOffset)
	{
		FBoard Board;
		if (!Generate(Preset, Board))
		{
			const FSettings Settings = MakeSettings(Preset);
			return GetCellCenter(Preset, Settings.BoardSize / 2, Settings.BoardSize / 2, Preset.BaselineZ + Settings.StepHeight + HeightOffset);
		}

		return GetPreferredSpawnLocation(Board, HeightOffset);
	}

	float GetSurfaceFeatureLavaCoverHeight(const FCell& Cell)
	{
		const int32 SurfaceFeatureFootprintCells = GetSurfaceFeatureFootprintCellCount();
		if (Cell.SurfaceFeature != ET66MapCellSurfaceFeature::Hill
			|| Cell.SurfaceFeatureOrigin.X == INDEX_NONE
			|| Cell.SurfaceFeatureOrigin.Y == INDEX_NONE)
		{
			return 0.0f;
		}

		const float HalfExtentCells = static_cast<float>(SurfaceFeatureFootprintCells) * 0.5f;
		const float LocalX = static_cast<float>(Cell.X - Cell.SurfaceFeatureOrigin.X) + 0.5f - HalfExtentCells;
		const float LocalY = static_cast<float>(Cell.Z - Cell.SurfaceFeatureOrigin.Y) + 0.5f - HalfExtentCells;
		return GetSurfaceFeatureHeight(
			Cell.SurfaceFeature,
			0,
			Cell.SurfaceFeatureOrigin,
			LocalX,
			LocalY,
			HalfExtentCells);
	}

	float GetTraceZ(const FT66MapPreset& Preset)
	{
		const FSettings Settings = MakeSettings(Preset);
		return Settings.BaselineZ + Settings.StepHeight * static_cast<float>(FMath::Max(Settings.BoardSize * 2, 8));
	}

	float GetLowestCollisionBottomZ(const FT66MapPreset& Preset)
	{
		const FSettings Settings = MakeSettings(Preset);
		const int32 MinElevationLevel = GetMinBoardElevationLevel(Preset);
		return Settings.BaselineZ
			+ static_cast<float>(MinElevationLevel) * Settings.StepHeight
			- Settings.StepHeight * 0.5f;
	}

	bool Generate(const FT66MapPreset& Preset, FBoard& OutBoard)
	{
		OutBoard = FBoard();
		OutBoard.Settings = MakeSettings(Preset);
		OutBoard.Cells.SetNum(OutBoard.Settings.BoardSize * OutBoard.Settings.BoardSize);
		const int32 SurfaceFeatureFootprintCells = GetSurfaceFeatureFootprintCellCount();

		for (int32 Z = 0; Z < OutBoard.Settings.BoardSize; ++Z)
		{
			for (int32 X = 0; X < OutBoard.Settings.BoardSize; ++X)
			{
				FCell& Cell = OutBoard.Cells[OutBoard.Index(X, Z)];
				Cell.X = X;
				Cell.Z = Z;
			}
		}

		FRandomStream Rng(Preset.Seed);
		const TArray<FIntPoint> Directions = { FIntPoint(1, 0), FIntPoint(-1, 0), FIntPoint(0, 1), FIntPoint(0, -1) };

		const float RaiseChance = OutBoard.Settings.Hilliness / 5.0f;
		const int32 MaxElevationLevel = GetMaxBoardElevationLevel(Preset);
		const int32 MinElevationLevel = GetMinBoardElevationLevel(Preset);
		const int32 DescentModeStartLevel = MaxElevationLevel;
		const int32 AscentModeStartLevel = MinElevationLevel;
		bool bDescendingMode = false;

		const FIntPoint ReservedBossCell = FIntPoint(INDEX_NONE, INDEX_NONE);

		auto CanOccupyCoordinate = [&](int32 X, int32 Z, bool bAllowReservedBossCell = false) -> bool
		{
			FCell* Cell = OutBoard.GetCell(X, Z);
			if (!Cell || Cell->bOccupied)
			{
				return false;
			}

			return bAllowReservedBossCell || X != ReservedBossCell.X || Z != ReservedBossCell.Y;
		};

		auto HasUngeneratedNeighbor = [&](const FCell& Cell, const FIntPoint& Direction) -> bool
		{
			return CanOccupyCoordinate(Cell.X + Direction.X, Cell.Z + Direction.Y);
		};

		auto GetRandomDirectionFromElement = [&](const FCell& Cell) -> FIntPoint
		{
			TArray<FIntPoint> DirectionPool;
			for (const FIntPoint& Direction : Directions)
			{
				if (HasUngeneratedNeighbor(Cell, Direction))
				{
					DirectionPool.Add(Direction);
				}
			}

			return DirectionPool.Num() > 0
				? DirectionPool[Rng.RandRange(0, DirectionPool.Num() - 1)]
				: FIntPoint::ZeroValue;
		};

		auto PickExpandableCell = [&](const FCell* ExcludedCell = nullptr) -> FCell*
		{
			TArray<FCell*> PreferredCandidates;
			TArray<FCell*> FallbackCandidates;
			bool bExcludedCellIsExpandable = false;

			for (int32 X = 0; X < OutBoard.Settings.BoardSize; ++X)
			{
				for (int32 Z = 0; Z < OutBoard.Settings.BoardSize; ++Z)
				{
					FCell* Cell = OutBoard.GetCell(X, Z);
					if (!Cell || !Cell->bOccupied || Cell->bSlope)
					{
						continue;
					}

					for (const FIntPoint& Direction : Directions)
					{
						if (CanOccupyCoordinate(Cell->X + Direction.X, Cell->Z + Direction.Y))
						{
							if (Cell == ExcludedCell)
							{
								bExcludedCellIsExpandable = true;
							}
							else
							{
								FallbackCandidates.Add(Cell);
								const bool bPreferredCandidate = bDescendingMode
									? Cell->Level > MinElevationLevel
									: Cell->Level < MaxElevationLevel;
								if (bPreferredCandidate)
								{
									PreferredCandidates.Add(Cell);
								}
							}
							break;
						}
					}
				}
			}

			if (PreferredCandidates.Num() > 0)
			{
				return PreferredCandidates[Rng.RandRange(0, PreferredCandidates.Num() - 1)];
			}
			if (FallbackCandidates.Num() > 0)
			{
				return FallbackCandidates[Rng.RandRange(0, FallbackCandidates.Num() - 1)];
			}
			return bExcludedCellIsExpandable ? const_cast<FCell*>(ExcludedCell) : nullptr;
		};

		auto CanRaiseElevationInDirection = [&](const FCell& Cell, const FIntPoint& Direction) -> bool
		{
			return CanOccupyCoordinate(Cell.X + Direction.X, Cell.Z + Direction.Y);
		};

		auto CanBeginDescentInDirection = [&](const FCell& Cell, const FIntPoint& Direction) -> bool
		{
			if (Cell.Level <= MinElevationLevel)
			{
				return false;
			}

			return CanOccupyCoordinate(Cell.X + Direction.X, Cell.Z + Direction.Y)
				&& CanOccupyCoordinate(Cell.X + Direction.X * 2, Cell.Z + Direction.Y * 2);
		};

		auto CreateElement = [&](int32 X, int32 Level, int32 Z, bool bAllowReservedBossCell = false) -> FCell*
		{
			if (!CanOccupyCoordinate(X, Z, bAllowReservedBossCell))
			{
				return nullptr;
			}

			FCell* Cell = OutBoard.GetCell(X, Z);
			Cell->bOccupied = true;
			Cell->bSlope = false;
			Cell->Level = Level;
			Cell->Shape = ET66MapCellShape::Flat;
			Cell->Decoration = ET66MapCellDecoration::None;
			Cell->SurfaceFeature = ET66MapCellSurfaceFeature::None;
			Cell->SurfaceFeatureOrigin = FIntPoint(INDEX_NONE, INDEX_NONE);
			Cell->Region = ECellRegion::MainBoard;
			Cell->DecorationLocalOffset = FVector::ZeroVector;
			Cell->DecorationLocalRotation = FRotator::ZeroRotator;
			Cell->DecorationLocalScale = FVector(1.0f, 1.0f, 1.0f);
			++OutBoard.OccupiedCount;
			return Cell;
		};

		const int32 CenterCoordinate = OutBoard.Settings.BoardSize / 2;
		const FIntPoint InitialCoordinate(CenterCoordinate, 0);
		const FIntPoint OppositeBossCoordinate(CenterCoordinate, OutBoard.Settings.BoardSize - 1);
		const FIntPoint InitialOutwardDirection = GetPerimeterOutwardDirection(InitialCoordinate, OutBoard.Settings.BoardSize);
		FCell* CurrentCell = CreateElement(InitialCoordinate.X, 0, InitialCoordinate.Y);
		FIntPoint LockedDirection = FIntPoint::ZeroValue;
		const int32 MainBoardTargetCount = OutBoard.Cells.Num();

		while (CurrentCell != nullptr && OutBoard.OccupiedCount < MainBoardTargetCount)
		{
			if (bDescendingMode && CurrentCell->Level <= AscentModeStartLevel)
			{
				bDescendingMode = false;
				LockedDirection = FIntPoint::ZeroValue;
			}
			else if (!bDescendingMode && CurrentCell->Level >= DescentModeStartLevel)
			{
				bDescendingMode = true;
				LockedDirection = FIntPoint::ZeroValue;
			}

			FIntPoint Direction = LockedDirection != FIntPoint::ZeroValue
				? LockedDirection
				: GetRandomDirectionFromElement(*CurrentCell);

			if (Direction == FIntPoint::ZeroValue)
			{
				CurrentCell = PickExpandableCell();
				if (CurrentCell == nullptr)
				{
					break;
				}

				Direction = LockedDirection != FIntPoint::ZeroValue
					? LockedDirection
					: GetRandomDirectionFromElement(*CurrentCell);
			}

			const FIntPoint DescendSlopeCoordinate(CurrentCell->X + Direction.X, CurrentCell->Z + Direction.Y);
			if (bDescendingMode
				&& CurrentCell->Level > MinElevationLevel
				&& !(DescendSlopeCoordinate == OppositeBossCoordinate)
				&& CanBeginDescentInDirection(*CurrentCell, Direction)
				&& Rng.FRand() < RaiseChance)
			{
				FCell* DescendSlope = CreateElement(CurrentCell->X + Direction.X, CurrentCell->Level, CurrentCell->Z + Direction.Y);
				FCell* LandingCell = CreateElement(CurrentCell->X + Direction.X * 2, CurrentCell->Level - 1, CurrentCell->Z + Direction.Y * 2);
				if (!DescendSlope || !LandingCell)
				{
					LockedDirection = FIntPoint::ZeroValue;
					CurrentCell = PickExpandableCell(CurrentCell);
					continue;
				}

				DescendSlope->bSlope = true;
				DescendSlope->Shape = GetSlopeShapeFromDirection(GetOppositeDirection(Direction));
				CurrentCell = LandingCell;
				LockedDirection = Direction;
				continue;
			}

			FCell* NewCell = CreateElement(CurrentCell->X + Direction.X, CurrentCell->Level, CurrentCell->Z + Direction.Y);
			if (!NewCell)
			{
				LockedDirection = FIntPoint::ZeroValue;
				CurrentCell = PickExpandableCell(CurrentCell);
				continue;
			}

			CurrentCell = NewCell;

			if (!bDescendingMode
				&& !(CurrentCell->X == OppositeBossCoordinate.X && CurrentCell->Z == OppositeBossCoordinate.Y)
				&& CurrentCell->Level < MaxElevationLevel
				&& CanRaiseElevationInDirection(*CurrentCell, Direction)
				&& Rng.FRand() < RaiseChance)
			{
				CurrentCell->Level += 1;
				CurrentCell->bSlope = true;
				CurrentCell->Shape = GetSlopeShapeFromDirection(Direction);
				LockedDirection = Direction;
			}
			else
			{
				LockedDirection = FIntPoint::ZeroValue;
				AssignDecoration(*CurrentCell, Rng, OutBoard.Settings);
			}
		}

		if (OutBoard.OccupiedCount != OutBoard.Cells.Num())
		{
			return false;
		}

		OutBoard.StartAnchor = InitialCoordinate;
		OutBoard.StartOutwardDirection = InitialOutwardDirection;
		if (FCell* StartAnchorCell = OutBoard.GetCell(OutBoard.StartAnchor.X, OutBoard.StartAnchor.Y))
		{
			ClearDecoration(*StartAnchorCell);
		}
		BuildExtensionArea(
			OutBoard,
			InitialCoordinate,
			InitialOutwardDirection,
			MainMapExtensionRoomRadiusCells,
			ECellRegion::StartPath,
			ECellRegion::StartArea,
			&OutBoard.StartPathCell,
			&OutBoard.StartSpawnCell);

		OutBoard.BossAnchor = OppositeBossCoordinate;
		OutBoard.BossOutwardDirection = GetPerimeterOutwardDirection(OutBoard.BossAnchor, OutBoard.Settings.BoardSize);
		if (FCell* BossAnchorCell = OutBoard.GetCell(OutBoard.BossAnchor.X, OutBoard.BossAnchor.Y))
		{
			ClearDecoration(*BossAnchorCell);
		}
		BuildExtensionArea(
			OutBoard,
			OutBoard.BossAnchor,
			OutBoard.BossOutwardDirection,
			MainMapExtensionRoomRadiusCells,
			ECellRegion::BossPath,
			ECellRegion::BossArea,
			&OutBoard.BossPathCell,
			&OutBoard.BossSpawnCell);

		auto ShuffleCellPointers = [&](TArray<FCell*>& Cells)
		{
			for (int32 Index = Cells.Num() - 1; Index > 0; --Index)
			{
				const int32 SwapIndex = Rng.RandRange(0, Index);
				if (Index != SwapIndex)
				{
					Cells.Swap(Index, SwapIndex);
				}
			}
		};

		auto IsNearReservedCoordinate = [&](const FCell& Cell) -> bool
		{
			const TArray<FIntPoint> ReservedCoordinates =
			{
				OutBoard.StartAnchor,
				OutBoard.StartPathCell,
				OutBoard.BossAnchor,
				OutBoard.BossPathCell,
			};

			for (const FIntPoint& Coordinate : ReservedCoordinates)
			{
				if (Coordinate.X == INDEX_NONE || Coordinate.Y == INDEX_NONE)
				{
					continue;
				}

				if (FMath::Max(FMath::Abs(Cell.X - Coordinate.X), FMath::Abs(Cell.Z - Coordinate.Y)) <= SurfaceFeatureSafeRadiusCells)
				{
					return true;
				}
			}

			return false;
		};

		TSet<int32> ReservedFeatureCells;

		auto IsSurfaceFeatureCandidate = [&](const FCell& Cell) -> bool
		{
			const int32 MaxFeatureX = Cell.X + SurfaceFeatureFootprintCells - 1;
			const int32 MaxFeatureZ = Cell.Z + SurfaceFeatureFootprintCells - 1;
			if (!Cell.bOccupied
				|| Cell.bSlope
				|| Cell.Region != ECellRegion::MainBoard
				|| Cell.X <= 0
				|| Cell.Z <= 0
				|| MaxFeatureX >= OutBoard.Settings.BoardSize - 1
				|| MaxFeatureZ >= OutBoard.Settings.BoardSize - 1)
			{
				return false;
			}

			for (int32 OffsetZ = 0; OffsetZ < SurfaceFeatureFootprintCells; ++OffsetZ)
			{
				for (int32 OffsetX = 0; OffsetX < SurfaceFeatureFootprintCells; ++OffsetX)
				{
					const FCell* FootprintCell = OutBoard.GetCell(Cell.X + OffsetX, Cell.Z + OffsetZ);
					if (!FootprintCell
						|| !FootprintCell->bOccupied
						|| FootprintCell->bSlope
						|| FootprintCell->Region != ECellRegion::MainBoard
						|| IsNearReservedCoordinate(*FootprintCell))
					{
						return false;
					}
				}
			}

			return true;
		};

		auto CanPlaceSurfaceFeatureAt = [&](const FIntPoint& Origin) -> bool
		{
			const FCell* OriginCell = OutBoard.GetCell(Origin.X, Origin.Y);
			if (!OriginCell || !IsSurfaceFeatureCandidate(*OriginCell))
			{
				return false;
			}

			for (int32 OffsetZ = -SurfaceFeatureFootprintPaddingCells; OffsetZ < SurfaceFeatureFootprintCells + SurfaceFeatureFootprintPaddingCells; ++OffsetZ)
			{
				for (int32 OffsetX = -SurfaceFeatureFootprintPaddingCells; OffsetX < SurfaceFeatureFootprintCells + SurfaceFeatureFootprintPaddingCells; ++OffsetX)
				{
					const int32 FeatureX = Origin.X + OffsetX;
					const int32 FeatureZ = Origin.Y + OffsetZ;
					if (FeatureX < 0 || FeatureX >= OutBoard.Settings.BoardSize || FeatureZ < 0 || FeatureZ >= OutBoard.Settings.BoardSize)
					{
						continue;
					}

					if (ReservedFeatureCells.Contains(OutBoard.Index(FeatureX, FeatureZ)))
					{
						return false;
					}
				}
			}

			for (int32 OffsetZ = 0; OffsetZ < SurfaceFeatureFootprintCells; ++OffsetZ)
			{
				for (int32 OffsetX = 0; OffsetX < SurfaceFeatureFootprintCells; ++OffsetX)
				{
					const FCell* FootprintCell = OutBoard.GetCell(Origin.X + OffsetX, Origin.Y + OffsetZ);
					if (!FootprintCell || HasSurfaceFeature(*FootprintCell))
					{
						return false;
					}
				}
			}

			return true;
		};

		auto AssignSurfaceFeatureFootprint = [&](const FIntPoint& Origin, ET66MapCellSurfaceFeature SurfaceFeature)
		{
			for (int32 OffsetZ = 0; OffsetZ < SurfaceFeatureFootprintCells; ++OffsetZ)
			{
				for (int32 OffsetX = 0; OffsetX < SurfaceFeatureFootprintCells; ++OffsetX)
				{
					if (FCell* FeatureCell = OutBoard.GetCell(Origin.X + OffsetX, Origin.Y + OffsetZ))
					{
						FeatureCell->SurfaceFeature = SurfaceFeature;
						FeatureCell->SurfaceFeatureOrigin = Origin;
						ClearDecoration(*FeatureCell);
					}
				}
			}

			for (int32 OffsetZ = -SurfaceFeatureFootprintPaddingCells; OffsetZ < SurfaceFeatureFootprintCells + SurfaceFeatureFootprintPaddingCells; ++OffsetZ)
			{
				for (int32 OffsetX = -SurfaceFeatureFootprintPaddingCells; OffsetX < SurfaceFeatureFootprintCells + SurfaceFeatureFootprintPaddingCells; ++OffsetX)
				{
					const int32 FeatureX = Origin.X + OffsetX;
					const int32 FeatureZ = Origin.Y + OffsetZ;
					if (FeatureX < 0 || FeatureX >= OutBoard.Settings.BoardSize || FeatureZ < 0 || FeatureZ >= OutBoard.Settings.BoardSize)
					{
						continue;
					}

					ReservedFeatureCells.Add(OutBoard.Index(FeatureX, FeatureZ));
				}
			}
		};

		TArray<FCell*> SurfaceCandidates;
		SurfaceCandidates.Reserve(OutBoard.Cells.Num());
		for (FCell& Cell : OutBoard.Cells)
		{
			ClearSurfaceFeature(Cell);
			if (IsSurfaceFeatureCandidate(Cell))
			{
				SurfaceCandidates.Add(&Cell);
			}
		}

		ShuffleCellPointers(SurfaceCandidates);
		const int32 HillTargetCount = FMath::Clamp(
			FMath::RoundToInt(static_cast<float>(OutBoard.Settings.BoardSize) * SurfaceFeatureCountFactor),
			SurfaceFeatureMinCount,
			SurfaceFeatureMaxCount);
		const int32 CraterTargetCount = HillTargetCount;
		int32 AssignedHills = 0;
		int32 AssignedCraters = 0;

		for (FCell* Cell : SurfaceCandidates)
		{
			if (!Cell)
			{
				continue;
			}

			const FIntPoint FeatureOrigin(Cell->X, Cell->Z);
			if (!CanPlaceSurfaceFeatureAt(FeatureOrigin))
			{
				continue;
			}

			if (AssignedHills < HillTargetCount)
			{
				AssignSurfaceFeatureFootprint(FeatureOrigin, ET66MapCellSurfaceFeature::Hill);
				++AssignedHills;
				continue;
			}

			if (AssignedCraters < CraterTargetCount)
			{
				AssignSurfaceFeatureFootprint(FeatureOrigin, ET66MapCellSurfaceFeature::Crater);
				++AssignedCraters;
			}
		}

		OutBoard.LavaRivers.Reset();
		const int32 LavaRiverTargetCount = FMath::Clamp(
			FMath::RoundToInt(static_cast<float>(OutBoard.Settings.BoardSize) * LavaRiverCountFactor),
			LavaRiverMinCount,
			LavaRiverMaxCount);

		auto IsRiverCellUsable = [&](int32 X, int32 Z) -> bool
		{
			const FCell* Cell = OutBoard.GetCell(X, Z);
			if (!Cell || !Cell->bOccupied || Cell->bSlope || Cell->Region != ECellRegion::MainBoard || IsNearReservedCoordinate(*Cell))
			{
				return false;
			}

			return Cell->SurfaceFeature == ET66MapCellSurfaceFeature::None
				&& !ReservedFeatureCells.Contains(OutBoard.Index(X, Z));
		};

		TArray<FCell*> RiverCandidates;
		for (FCell& Cell : OutBoard.Cells)
		{
			if (IsRiverCellUsable(Cell.X, Cell.Z))
			{
				RiverCandidates.Add(&Cell);
			}
		}
		ShuffleCellPointers(RiverCandidates);

		for (FCell* OriginCell : RiverCandidates)
		{
			if (!OriginCell || OutBoard.LavaRivers.Num() >= LavaRiverTargetCount)
			{
				break;
			}

			const int32 OriginIndex = OutBoard.Index(OriginCell->X, OriginCell->Z);
			if (ReservedFeatureCells.Contains(OriginIndex))
			{
				continue;
			}

			TArray<FIntPoint> ShuffledDirections = Directions;
			for (int32 DirectionIndex = ShuffledDirections.Num() - 1; DirectionIndex > 0; --DirectionIndex)
			{
				const int32 SwapIndex = Rng.RandRange(0, DirectionIndex);
				if (DirectionIndex != SwapIndex)
				{
					ShuffledDirections.Swap(DirectionIndex, SwapIndex);
				}
			}

			for (const FIntPoint& Direction : ShuffledDirections)
			{
				int32 MaxReachableLength = 0;
				for (int32 Step = 1; Step <= LavaRiverMaxLengthCells; ++Step)
				{
					if (!IsRiverCellUsable(OriginCell->X + Direction.X * Step, OriginCell->Z + Direction.Y * Step))
					{
						break;
					}

					++MaxReachableLength;
				}

				if (MaxReachableLength < LavaRiverMinLengthCells)
				{
					continue;
				}

				const int32 RiverLength = Rng.RandRange(LavaRiverMinLengthCells, MaxReachableLength);
				T66MainMapTerrain::FLavaRiver& River = OutBoard.LavaRivers.AddDefaulted_GetRef();
				River.PuddleCell = FIntPoint(OriginCell->X, OriginCell->Z);
				River.Direction = Direction;
				River.LengthCells = RiverLength;

				ClearDecoration(*OriginCell);
				ReservedFeatureCells.Add(OriginIndex);

				for (int32 Step = 1; Step <= RiverLength; ++Step)
				{
					if (FCell* PathCell = OutBoard.GetCell(OriginCell->X + Direction.X * Step, OriginCell->Z + Direction.Y * Step))
					{
						ClearDecoration(*PathCell);
						ReservedFeatureCells.Add(OutBoard.Index(PathCell->X, PathCell->Z));
					}
				}
				break;
			}
		}

		return OutBoard.OccupiedCount == OutBoard.Cells.Num();
	}

	bool Spawn(UWorld* World, const FBoard& Board, const FT66MapPreset& Preset, const FActorSpawnParameters& SpawnParams, bool& bOutCollisionReady)
	{
		bOutCollisionReady = false;
		if (!World || Board.Settings.BoardSize <= 0 || Board.Cells.Num() == 0)
		{
			return false;
		}

		if (Board.OccupiedCount != Board.Cells.Num())
		{
			UE_LOG(LogT66MainMapTerrain, Error, TEXT("[MAP] Main map terrain board incomplete: %d / %d cells occupied"), Board.OccupiedCount, Board.Cells.Num());
			return false;
		}

		FLoadedAssets Assets;
		if (!LoadAssets(Assets))
		{
			UE_LOG(LogT66MainMapTerrain, Error, TEXT("[MAP] Main map terrain tile meshes missing; aborting terrain spawn."));
			return false;
		}

		const UT66GameInstance* T66GI = World ? Cast<UT66GameInstance>(World->GetGameInstance()) : nullptr;
		const ET66Difficulty Difficulty = T66GI ? T66GI->SelectedDifficulty : ET66Difficulty::Easy;
		ApplyDifficultyTextureOverrides(Assets, Difficulty);

		const FVector BoardOrigin = GetBoardOrigin(Preset);
		const float CellSize = Board.Settings.CellSize;
		const float StepHeight = Board.Settings.StepHeight;
		const int32 SurfaceFeatureFootprintCells = GetSurfaceFeatureFootprintCellCount();
		const double SpawnStartSeconds = FPlatformTime::Seconds();

		AActor* VisualActor = World->SpawnActor<AActor>(AActor::StaticClass(), FVector::ZeroVector, FRotator::ZeroRotator, SpawnParams);
		if (!VisualActor)
		{
			return false;
		}

		VisualActor->Tags.Add(MapPlatformTag);
		VisualActor->Tags.Add(FloorMainTag);
		VisualActor->Tags.Add(FarmVisualTag);

		USceneComponent* VisualRoot = NewObject<USceneComponent>(VisualActor, TEXT("MainMapTerrainRoot"));
		VisualActor->AddInstanceComponent(VisualRoot);
		VisualActor->SetRootComponent(VisualRoot);
		VisualRoot->SetMobility(EComponentMobility::Static);
		VisualRoot->SetRelativeLocation(BoardOrigin);
		VisualRoot->RegisterComponent();

		auto ResolveForcedTextureMaterial = [VisualActor](
			UTexture* Texture,
			UMaterialInterface* EnvironmentUnlitMaterial,
			UMaterialInterface* FallbackMaterial,
			const TCHAR* FallbackMaterialPath,
			const TCHAR* DebugName) -> FResolvedFarmMaterial
		{
			if (EnvironmentUnlitMaterial && Texture)
			{
				if (UMaterialInstanceDynamic* ForcedMaterial = UMaterialInstanceDynamic::Create(EnvironmentUnlitMaterial, VisualActor, FName(DebugName)))
				{
					ForcedMaterial->SetTextureParameterValue(TEXT("DiffuseColorMap"), Texture);
					ForcedMaterial->SetTextureParameterValue(TEXT("BaseColorTexture"), Texture);
					ForcedMaterial->SetScalarParameterValue(TEXT("Brightness"), 1.0f);
					ForcedMaterial->SetVectorParameterValue(TEXT("Tint"), FLinearColor::White);
					ForcedMaterial->SetVectorParameterValue(TEXT("BaseColor"), FLinearColor::White);
					return { ForcedMaterial, true };
				}
			}

			if (FallbackMaterial)
			{
				if (FallbackMaterialPath)
				{
					UE_LOG(LogT66MainMapTerrain, Warning, TEXT("[MAP] Main map terrain material %s fell back to %s"), DebugName, FallbackMaterialPath);
				}
				return { FallbackMaterial, false };
			}

			UE_LOG(LogT66MainMapTerrain, Warning, TEXT("[MAP] Main map terrain material %s is still missing its real texture-backed material"), DebugName);
			return {};
		};

		auto UpdateFarmMaterialReadyTag = [VisualActor](bool bMaterialsReady)
		{
			if (!VisualActor || !IsValid(VisualActor))
			{
				return;
			}

			if (bMaterialsReady)
			{
				VisualActor->Tags.AddUnique(FarmMaterialsReadyTag);
			}
			else
			{
				VisualActor->Tags.Remove(FarmMaterialsReadyTag);
			}
		};

		const FResolvedFarmMaterial InitialBlockMaterial = ResolveForcedTextureMaterial(
			Assets.BlockTexture,
			Assets.EnvironmentUnlitMaterial,
			Assets.BlockMaterial,
			TEXT("/Game/World/Terrain/Megabonk/MI_MegabonkBlock.MI_MegabonkBlock"),
			TEXT("FarmBlockMID"));
		const FResolvedFarmMaterial InitialSlopeMaterial = ResolveForcedTextureMaterial(
			Assets.SlopeTexture,
			Assets.EnvironmentUnlitMaterial,
			Assets.SlopeMaterial,
			TEXT("/Game/World/Terrain/Megabonk/MI_MegabonkSlope.MI_MegabonkSlope"),
			TEXT("FarmSlopeMID"));
		const FResolvedFarmMaterial InitialDirtMaterial = ResolveForcedTextureMaterial(
			Assets.DirtTexture,
			Assets.EnvironmentUnlitMaterial,
			Assets.DirtMaterial ? Assets.DirtMaterial : Assets.BlockMaterial,
			Assets.DirtMaterial
				? TEXT("/Game/World/Terrain/Megabonk/MI_MegabonkDirt.MI_MegabonkDirt")
				: TEXT("/Game/World/Terrain/Megabonk/MI_MegabonkBlock.MI_MegabonkBlock"),
			TEXT("FarmDirtMID"));
		const FResolvedFarmMaterial InitialWallMaterial = ResolveForcedTextureMaterial(
			Assets.WallTexture,
			Assets.EnvironmentUnlitMaterial,
			Assets.WallMaterial,
			TEXT("/Game/World/Terrain/Megabonk/MI_MegabonkWall.MI_MegabonkWall"),
			TEXT("FarmWallMID"));
		UMaterialInterface* EffectiveBlockMaterial = InitialBlockMaterial.Material;
		UMaterialInterface* EffectiveSlopeMaterial = InitialSlopeMaterial.Material;
		UMaterialInterface* EffectiveDirtMaterial = InitialDirtMaterial.Material;
		UMaterialInterface* EffectiveWallMaterial = InitialWallMaterial.Material;
		UMaterialInterface* EffectiveSurfaceFeatureMaterial =
			Assets.SlopeMaterial ? Assets.SlopeMaterial :
			(Assets.BlockMaterial ? Assets.BlockMaterial : EffectiveBlockMaterial);
		UpdateFarmMaterialReadyTag(
			InitialBlockMaterial.bUsingRealTexture &&
			InitialSlopeMaterial.bUsingRealTexture &&
			InitialDirtMaterial.bUsingRealTexture &&
			(!bRenderFarmBoundaryWalls || InitialWallMaterial.bUsingRealTexture));

		auto RefreshFarmVisualMaterials = [VisualActor, Assets, ResolveForcedTextureMaterial, UpdateFarmMaterialReadyTag]() -> bool
		{
			if (!VisualActor || !IsValid(VisualActor))
			{
				return false;
			}

			const FResolvedFarmMaterial BlockMaterial = ResolveForcedTextureMaterial(
				Assets.BlockTexture,
				Assets.EnvironmentUnlitMaterial,
				Assets.BlockMaterial,
				TEXT("/Game/World/Terrain/Megabonk/MI_MegabonkBlock.MI_MegabonkBlock"),
				TEXT("FarmBlockMID"));
			const FResolvedFarmMaterial SlopeMaterial = ResolveForcedTextureMaterial(
				Assets.SlopeTexture,
				Assets.EnvironmentUnlitMaterial,
				Assets.SlopeMaterial,
				TEXT("/Game/World/Terrain/Megabonk/MI_MegabonkSlope.MI_MegabonkSlope"),
				TEXT("FarmSlopeMID"));
			const FResolvedFarmMaterial DirtMaterial = ResolveForcedTextureMaterial(
				Assets.DirtTexture,
				Assets.EnvironmentUnlitMaterial,
				Assets.DirtMaterial ? Assets.DirtMaterial : Assets.BlockMaterial,
				Assets.DirtMaterial
					? TEXT("/Game/World/Terrain/Megabonk/MI_MegabonkDirt.MI_MegabonkDirt")
					: TEXT("/Game/World/Terrain/Megabonk/MI_MegabonkBlock.MI_MegabonkBlock"),
				TEXT("FarmDirtMID"));
			const FResolvedFarmMaterial WallMaterial = ResolveForcedTextureMaterial(
				Assets.WallTexture,
				Assets.EnvironmentUnlitMaterial,
				Assets.WallMaterial,
				TEXT("/Game/World/Terrain/Megabonk/MI_MegabonkWall.MI_MegabonkWall"),
				TEXT("FarmWallMID"));
			UMaterialInterface* SurfaceFeatureMaterial =
				Assets.SlopeMaterial ? Assets.SlopeMaterial :
				(Assets.BlockMaterial ? Assets.BlockMaterial : BlockMaterial.Material);
			const bool bMaterialsReady =
				BlockMaterial.bUsingRealTexture &&
				SlopeMaterial.bUsingRealTexture &&
				DirtMaterial.bUsingRealTexture &&
				(!bRenderFarmBoundaryWalls || WallMaterial.bUsingRealTexture);
			UpdateFarmMaterialReadyTag(bMaterialsReady);

			TInlineComponentArray<UStaticMeshComponent*> MeshComponents(VisualActor);
			for (UStaticMeshComponent* MeshComponent : MeshComponents)
			{
				if (!MeshComponent)
				{
					continue;
				}

				const FString ComponentName = MeshComponent->GetName();
				if (ComponentName.StartsWith(TEXT("Cube_")))
				{
					MeshComponent->SetMaterial(0, BlockMaterial.Material);
				}
				else if (ComponentName.StartsWith(TEXT("Slope_")))
				{
					MeshComponent->SetMaterial(0, SlopeMaterial.Material);
				}
				else if (ComponentName.StartsWith(TEXT("Support_")))
				{
					MeshComponent->SetMaterial(0, DirtMaterial.Material);
				}
				else if (ComponentName.StartsWith(TEXT("FarmWall")))
				{
					MeshComponent->SetMaterial(0, WallMaterial.Material);
				}
				else if (ComponentName.StartsWith(TEXT("Decor_")))
				{
					MeshComponent->EmptyOverrideMaterials();
				}
			}

			TInlineComponentArray<UProceduralMeshComponent*> ProceduralMeshComponents(VisualActor);
			for (UProceduralMeshComponent* ProceduralMeshComponent : ProceduralMeshComponents)
			{
				if (!ProceduralMeshComponent)
				{
					continue;
				}

				if (ProceduralMeshComponent->GetName().StartsWith(TEXT("SurfaceFeature_")))
				{
					ProceduralMeshComponent->SetMaterial(0, SurfaceFeatureMaterial);
				}
			}

			return bMaterialsReady;
		};

		auto CreateSceneChild = [&](USceneComponent* Parent, const FString& Name, const FVector& RelativeLocation, const FRotator& RelativeRotation = FRotator::ZeroRotator) -> USceneComponent*
		{
			USceneComponent* Component = NewObject<USceneComponent>(VisualActor, FName(*Name));
			VisualActor->AddInstanceComponent(Component);
			Component->SetupAttachment(Parent);
			Component->SetMobility(EComponentMobility::Static);
			Component->SetRelativeLocation(RelativeLocation);
			Component->SetRelativeRotation(RelativeRotation);
			Component->RegisterComponent();
			return Component;
		};

		auto CreateMeshChild = [&](USceneComponent* Parent, const FString& Name, UStaticMesh* Mesh, UMaterialInterface* Material, const FTransform& RelativeTransform, bool bEnableCollision) -> UStaticMeshComponent*
		{
			if (!Mesh)
			{
				return nullptr;
			}

			UStaticMeshComponent* Component = NewObject<UStaticMeshComponent>(VisualActor, FName(*Name));
			VisualActor->AddInstanceComponent(Component);
			Component->SetupAttachment(Parent);
			Component->SetMobility(EComponentMobility::Static);
			Component->SetStaticMesh(Mesh);
			if (Material)
			{
				Component->SetMaterial(0, Material);
			}
			Component->SetRelativeTransform(RelativeTransform);
			if (bEnableCollision)
			{
				ConfigureTerrainVisualCollisionComponent(Component);
				Component->CanCharacterStepUpOn = ECanBeCharacterBase::ECB_Yes;
			}
			else
			{
				Component->SetCollisionEnabled(ECollisionEnabled::NoCollision);
				Component->SetCanEverAffectNavigation(false);
			}
			Component->RegisterComponent();
			return Component;
		};

		auto CreateInstancedMeshBatch = [&](const FString& Name, UStaticMesh* Mesh, UMaterialInterface* Material, bool bEnableCollision) -> UInstancedStaticMeshComponent*
		{
			if (!Mesh)
			{
				return nullptr;
			}

			UInstancedStaticMeshComponent* Component = NewObject<UInstancedStaticMeshComponent>(VisualActor, FName(*Name));
			VisualActor->AddInstanceComponent(Component);
			Component->SetupAttachment(VisualRoot);
			Component->SetMobility(EComponentMobility::Static);
			Component->SetStaticMesh(Mesh);
			if (Material)
			{
				Component->SetMaterial(0, Material);
			}

			if (bEnableCollision)
			{
				ConfigureTerrainVisualCollisionComponent(Component);
				Component->CanCharacterStepUpOn = ECanBeCharacterBase::ECB_Yes;
			}
			else
			{
				Component->SetCollisionEnabled(ECollisionEnabled::NoCollision);
				Component->SetCanEverAffectNavigation(false);
			}
			return Component;
		};

		auto PickGrassMaterial = [&](FRandomStream& GrassRng) -> UMaterialInterface*
		{
			if (Assets.GrassMaterials.Num() == 0)
			{
				return nullptr;
			}

			return Assets.GrassMaterials[GrassRng.RandRange(0, Assets.GrassMaterials.Num() - 1)];
		};

		auto AddRandomizedGrass = [&](USceneComponent* Parent, const FString& BaseName, uint32 Seed, float VariationX, float VariationY)
		{
			if (!Assets.GrassMesh)
			{
				return;
			}

			const FBox GrassBounds = Assets.GrassMesh->GetBoundingBox();
			const FVector GrassSize = GrassBounds.GetSize();
			if (GrassSize.X <= KINDA_SMALL_NUMBER || GrassSize.Y <= KINDA_SMALL_NUMBER || GrassSize.Z <= KINDA_SMALL_NUMBER)
			{
				return;
			}

			const FVector BaseGrassScale(
				(GrassDesiredWidthUU * Board.Settings.BoardScale) / GrassSize.X,
				(GrassDesiredWidthUU * Board.Settings.BoardScale) / GrassSize.Y,
				(GrassDesiredHeightUU * Board.Settings.BoardScale) / GrassSize.Z);
			const float AlternateLift = StepHeight * GrassAlternateLiftFraction;

			FRandomStream GrassRng(Seed);
			for (int32 Index = 0; Index < GrassObjectCount; ++Index)
			{
				USceneComponent* GrassObjectRoot = CreateSceneChild(
					Parent,
					FString::Printf(TEXT("%s_Object_%d"), *BaseName, Index),
					FVector(
						GrassRng.FRandRange(-CellSize * VariationX, CellSize * VariationX),
						GrassRng.FRandRange(-CellSize * VariationY, CellSize * VariationY),
						(Index & 1) == 0 ? 0.0f : AlternateLift),
					FRotator(0.0f, GrassRng.FRandRange(0.0f, 360.0f), 0.0f));
				GrassObjectRoot->SetRelativeScale3D(FVector(1.0f, 1.0f, GrassRng.FRandRange(GrassMinScale, GrassMaxScale)));

				CreateMeshChild(
					GrassObjectRoot,
					FString::Printf(TEXT("%s_Mesh_%d"), *BaseName, Index),
					Assets.GrassMesh,
					PickGrassMaterial(GrassRng),
					FTransform(FRotator::ZeroRotator, FVector::ZeroVector, BaseGrassScale),
					false);
			}
		};

		double SurfaceFeatureSpawnSeconds = 0.0;
		double FlatCollisionSpawnSeconds = 0.0;

		auto CreateSurfaceFeatureChild = [&](USceneComponent* Parent, const FString& Name, const FCell& FeatureCell) -> UProceduralMeshComponent*
		{
			if (!IsSurfaceFeatureOriginCell(FeatureCell) || !EffectiveSurfaceFeatureMaterial)
			{
				return nullptr;
			}

			const double SurfaceFeatureStartSeconds = FPlatformTime::Seconds();
			const float FeatureSizeUU = CellSize * static_cast<float>(SurfaceFeatureFootprintCells);
			const float HalfFeatureSizeUU = FeatureSizeUU * 0.5f;
			const FVector FeatureRelativeLocation(
				static_cast<float>(SurfaceFeatureFootprintCells - 1) * CellSize * 0.5f,
				static_cast<float>(SurfaceFeatureFootprintCells - 1) * CellSize * 0.5f,
				StepHeight * 0.5f + SurfaceFeatureMeshBiasUU);

			auto BuildSurfaceFeatureMeshData = [&](int32 GridResolution, TArray<FVector>& OutVertices, TArray<int32>& OutTriangles, TArray<FVector2D>& OutUVs)
			{
				const int32 VertexStride = GridResolution + 1;
				OutVertices.Reset();
				OutTriangles.Reset();
				OutUVs.Reset();
				OutVertices.Reserve(VertexStride * VertexStride);
				OutUVs.Reserve(VertexStride * VertexStride);
				OutTriangles.Reserve(GridResolution * GridResolution * 6);

				for (int32 Y = 0; Y <= GridResolution; ++Y)
				{
					const float V = static_cast<float>(Y) / static_cast<float>(GridResolution);
					const float LocalY = (V - 0.5f) * FeatureSizeUU;

					for (int32 X = 0; X <= GridResolution; ++X)
					{
						const float U = static_cast<float>(X) / static_cast<float>(GridResolution);
						const float LocalX = (U - 0.5f) * FeatureSizeUU;
						const float Height = GetSurfaceFeatureHeight(
							FeatureCell.SurfaceFeature,
							Preset.Seed,
							FeatureCell.SurfaceFeatureOrigin,
							LocalX,
							LocalY,
							HalfFeatureSizeUU);

						OutVertices.Add(FVector(LocalX, LocalY, Height));
						OutUVs.Add(FVector2D(U, V));
					}
				}

				for (int32 Y = 0; Y < GridResolution; ++Y)
				{
					for (int32 X = 0; X < GridResolution; ++X)
					{
						const int32 A = Y * VertexStride + X;
						const int32 B = A + 1;
						const int32 C = A + VertexStride;
						const int32 D = C + 1;

						OutTriangles.Add(A);
						OutTriangles.Add(C);
						OutTriangles.Add(B);

						OutTriangles.Add(B);
						OutTriangles.Add(C);
						OutTriangles.Add(D);
					}
				}
			};

			TArray<FVector> VisualVertices;
			TArray<int32> VisualTriangles;
			TArray<FVector2D> VisualUVs;
			BuildSurfaceFeatureMeshData(
				SurfaceFeatureGridResolutionPerCell * SurfaceFeatureFootprintCells,
				VisualVertices,
				VisualTriangles,
				VisualUVs);

			TArray<FVector> VisualNormals;
			TArray<FProcMeshTangent> VisualTangents;
			TArray<FLinearColor> VertexColors;
			UKismetProceduralMeshLibrary::CalculateTangentsForMesh(
				VisualVertices,
				VisualTriangles,
				VisualUVs,
				VisualNormals,
				VisualTangents);

			UProceduralMeshComponent* VisualComponent = NewObject<UProceduralMeshComponent>(VisualActor, FName(*Name));
			VisualActor->AddInstanceComponent(VisualComponent);
			VisualComponent->SetupAttachment(Parent);
			VisualComponent->SetMobility(EComponentMobility::Static);
			VisualComponent->SetCollisionEnabled(ECollisionEnabled::NoCollision);
			VisualComponent->SetGenerateOverlapEvents(false);
			VisualComponent->SetCanEverAffectNavigation(false);
			VisualComponent->SetCastShadow(true);
			VisualComponent->SetRelativeLocation(FeatureRelativeLocation);
			VisualComponent->CreateMeshSection_LinearColor(0, VisualVertices, VisualTriangles, VisualNormals, VisualUVs, VertexColors, VisualTangents, false);
			VisualComponent->SetMaterial(0, EffectiveSurfaceFeatureMaterial);
			VisualComponent->RegisterComponent();

			TArray<FVector> CollisionVertices;
			TArray<int32> CollisionTriangles;
			TArray<FVector2D> CollisionUVs;
			BuildSurfaceFeatureMeshData(
				SurfaceFeatureCollisionGridResolutionPerCell * SurfaceFeatureFootprintCells,
				CollisionVertices,
				CollisionTriangles,
				CollisionUVs);

			TArray<FVector> CollisionNormals;
			TArray<FProcMeshTangent> CollisionTangents;
			UProceduralMeshComponent* CollisionComponent = NewObject<UProceduralMeshComponent>(VisualActor, FName(*(Name + TEXT("_Collision"))));
			VisualActor->AddInstanceComponent(CollisionComponent);
			CollisionComponent->SetupAttachment(Parent);
			CollisionComponent->SetMobility(EComponentMobility::Static);
			ConfigureTerrainCollisionComponent(CollisionComponent);
			CollisionComponent->SetGenerateOverlapEvents(false);
			CollisionComponent->SetCanEverAffectNavigation(false);
			CollisionComponent->SetVisibility(false, true);
			CollisionComponent->SetHiddenInGame(true, true);
			CollisionComponent->SetCastShadow(false);
			CollisionComponent->bUseAsyncCooking = true;
			CollisionComponent->bUseComplexAsSimpleCollision = true;
			CollisionComponent->CanCharacterStepUpOn = ECanBeCharacterBase::ECB_Yes;
			CollisionComponent->SetRelativeLocation(FeatureRelativeLocation);
			CollisionComponent->CreateMeshSection_LinearColor(0, CollisionVertices, CollisionTriangles, CollisionNormals, CollisionUVs, VertexColors, CollisionTangents, true);
			CollisionComponent->RegisterComponent();
			SurfaceFeatureSpawnSeconds += (FPlatformTime::Seconds() - SurfaceFeatureStartSeconds);
			return VisualComponent;
		};

		auto SpawnDecorativeLavaPatch = [&](const FString& Name, const FVector& WorldLocation, float SizeX, float SizeY, const FVector2D& FlowDirection) -> AT66LavaPatch*
		{
			FTransform SpawnTransform(FRotator::ZeroRotator, WorldLocation);
			AT66LavaPatch* LavaPatch = World->SpawnActorDeferred<AT66LavaPatch>(
				AT66LavaPatch::StaticClass(),
				SpawnTransform,
				VisualActor,
				nullptr,
				ESpawnActorCollisionHandlingMethod::AlwaysSpawn);
			if (!LavaPatch)
			{
				return nullptr;
			}

			LavaPatch->Tags.Add(MapPlatformTag);
			LavaPatch->Tags.Add(FarmVisualTag);
			LavaPatch->PatchSize = FMath::Max(SizeX, SizeY);
			LavaPatch->PatchSizeX = SizeX;
			LavaPatch->PatchSizeY = SizeY;
			LavaPatch->HoverHeight = LavaRiverHoverHeightUU;
			LavaPatch->CollisionHeight = 80.0f;
			LavaPatch->bSnapToGroundOnBeginPlay = false;
			LavaPatch->TextureResolution = 48;
			LavaPatch->AnimationFrames = 12;
			LavaPatch->AnimationFPS = 10.0f;
			LavaPatch->WarpSpeed = 1.0f;
			LavaPatch->WarpIntensity = 0.11f;
			LavaPatch->WarpCloseness = 2.1f;
			LavaPatch->FlowDir = FlowDirection.IsNearlyZero() ? FVector2D(1.0f, 0.0f) : FlowDirection.GetSafeNormal();
			LavaPatch->FlowSpeed = 0.16f;
			LavaPatch->UVScale = FMath::Clamp((FMath::Max(SizeX, SizeY) / FMath::Max(FMath::Min(SizeX, SizeY), 1.0f)) * 2.0f, 4.0f, 16.0f);
			LavaPatch->CellDensity = 6.2f;
			LavaPatch->EdgeContrast = 7.0f;
			LavaPatch->Brightness = 2.3f;
			LavaPatch->bDamageHero = false;
			LavaPatch->DamagePerTick = 0;
			LavaPatch->DamageIntervalSeconds = 1.0f;
			LavaPatch->FinishSpawning(SpawnTransform);
			return LavaPatch;
		};

		auto CreateWallPrefab = [&](const FString& BaseName, const FVector& LocalCenter, const FVector& Size)
		{
			if (!Assets.PlaneMesh)
			{
				return;
			}

			USceneComponent* WallRoot = CreateSceneChild(VisualRoot, BaseName + TEXT("_Root"), LocalCenter);
			WallRoot->SetRelativeScale3D(Size);

			auto AddWallQuad = [&](const FString& FaceName, const FVector& LocalPosition, const FRotator& LocalRotation)
			{
				FTransform FaceTransform(
					LocalRotation,
					LocalPosition,
					FVector(1.0f, 0.6f, 1.0f));
				CreateMeshChild(
					WallRoot,
					FaceName,
					Assets.PlaneMesh,
					EffectiveWallMaterial,
					FaceTransform,
					false);
			};

			// Unity Wall.prefab: root + four quads at +/-0.5 with Y scale 0.6.
			AddWallQuad(BaseName + TEXT("_West"),  FVector(-0.5f, 0.0f, 0.0f), FRotator(0.0f,  90.0f, 0.0f));
			AddWallQuad(BaseName + TEXT("_East"),  FVector( 0.5f, 0.0f, 0.0f), FRotator(0.0f, -90.0f, 0.0f));
			AddWallQuad(BaseName + TEXT("_North"), FVector( 0.0f, 0.0f, 0.5f), FRotator(0.0f, 180.0f, 0.0f));
			AddWallQuad(BaseName + TEXT("_South"), FVector( 0.0f, 0.0f,-0.5f), FRotator(0.0f,   0.0f, 0.0f));
		};

		int32 FlatCount = 0;
		int32 SlopeCount = 0;
		int32 SupportCount = 0;
		int32 DecorCount = 0;
		int32 SurfaceFeatureCount = 0;
		int32 LavaRiverVisualCount = 0;
		int32 FlatCollisionCount = 0;
		const FVector TerrainMeshScale = GetTerrainMeshScale(Board.Settings);
		const int32 LowestSupportLevel = GetMinBoardElevationLevel(Preset);
		TArray<FTransform> FlatTileTransforms;
		TArray<FTransform> SlopeTileTransforms;
		FlatTileTransforms.Reserve(Board.Cells.Num() + Board.ExtraCells.Num());
		SlopeTileTransforms.Reserve(FMath::Max(Board.Cells.Num() / 8, 64));
		UInstancedStaticMeshComponent* FlatTileInstances = CreateInstancedMeshBatch(
			TEXT("Cube_Instances"),
			Assets.BlockMesh,
			EffectiveBlockMaterial,
			false);
		UInstancedStaticMeshComponent* SlopeTileInstances = CreateInstancedMeshBatch(
			TEXT("Slope_Instances"),
			Assets.SlopeMesh,
			EffectiveSlopeMaterial,
			true);

		auto AddFlatCollisionBox = [&](const FString& Name, const FVector& LocalCenter, const FVector& Extent)
		{
			const double FlatCollisionStartSeconds = FPlatformTime::Seconds();
			UBoxComponent* Box = NewObject<UBoxComponent>(VisualActor, FName(*Name));
			VisualActor->AddInstanceComponent(Box);
			Box->SetupAttachment(VisualRoot);
			Box->SetMobility(EComponentMobility::Static);
			Box->SetRelativeLocation(LocalCenter);
			Box->SetBoxExtent(Extent);
			ConfigureTerrainCollisionComponent(Box);
			Box->CanCharacterStepUpOn = ECanBeCharacterBase::ECB_Yes;
			Box->RegisterComponent();
			FlatCollisionSpawnSeconds += (FPlatformTime::Seconds() - FlatCollisionStartSeconds);
			++FlatCollisionCount;
		};

		auto SpawnFarmCell = [&](const FCell& Cell, int32 CellIndex)
		{
			if (!Cell.bOccupied)
			{
				return;
			}

			const bool bHasSurfaceFeature = HasSurfaceFeature(Cell);
			const bool bIsSurfaceFeatureOrigin = IsSurfaceFeatureOriginCell(Cell);
			const FVector CellRelativeLocation(
				static_cast<float>(Cell.X) * CellSize,
				static_cast<float>(Cell.Z) * CellSize,
				static_cast<float>(Cell.Level) * StepHeight);

			USceneComponent* TileRoot = nullptr;
			auto GetOrCreateTileRoot = [&]() -> USceneComponent*
			{
				if (!TileRoot)
				{
					TileRoot = CreateSceneChild(
						VisualRoot,
						FString::Printf(TEXT("FarmTile_%d"), CellIndex),
						CellRelativeLocation);
				}

				return TileRoot;
			};

			if (Cell.bSlope)
			{
				SlopeTileTransforms.Add(FTransform(
					FRotator(0.0f, GetSlopeYaw(Cell.Shape), 0.0f),
					CellRelativeLocation,
					TerrainMeshScale));
			}
			else if (!bHasSurfaceFeature)
			{
				FlatTileTransforms.Add(FTransform(
					FRotator::ZeroRotator,
					CellRelativeLocation,
					TerrainMeshScale));

				if (Cell.Region != ECellRegion::MainBoard)
				{
					AddFlatCollisionBox(
						FString::Printf(TEXT("FlatCollision_%d"), CellIndex),
						CellRelativeLocation,
						FVector(CellSize * 0.5f, CellSize * 0.5f, StepHeight * 0.5f));
				}
			}

			if (bRenderFarmSupports)
			{
				USceneComponent* BottomPartRoot = CreateSceneChild(
					GetOrCreateTileRoot(),
					FString::Printf(TEXT("BottomPart_%d"), CellIndex),
					FVector::ZeroVector);
				for (int32 SupportLevel = Cell.Level - 1; SupportLevel >= LowestSupportLevel; --SupportLevel)
				{
					const FVector SupportOffset(0.0f, 0.0f, static_cast<float>(SupportLevel - Cell.Level) * StepHeight);
					CreateMeshChild(
						BottomPartRoot,
						FString::Printf(TEXT("Support_%d_L%d"), CellIndex, SupportLevel),
						Assets.BlockMesh,
						EffectiveDirtMaterial,
						FTransform(
							FRotator::ZeroRotator,
							SupportOffset,
							TerrainMeshScale),
						true);
					++SupportCount;
				}
			}

			if (bRenderFarmGrass)
			{
				USceneComponent* GrassFlatRoot = CreateSceneChild(
					GetOrCreateTileRoot(),
					FString::Printf(TEXT("GrassFlat_%d"), CellIndex),
					FVector::ZeroVector);
				USceneComponent* GrassSlopeRoot = CreateSceneChild(
					GetOrCreateTileRoot(),
					FString::Printf(TEXT("GrassSlope_%d"), CellIndex),
					FVector::ZeroVector,
					FRotator(-30.59f, GetSlopeYaw(Cell.Shape), 0.0f));

				const bool bShowFlatGrass = !Cell.bSlope && !bHasSurfaceFeature;
				const bool bShowSlopeGrass = Cell.bSlope;
				GrassFlatRoot->SetVisibility(bShowFlatGrass, true);
				GrassFlatRoot->SetHiddenInGame(!bShowFlatGrass, true);
				GrassSlopeRoot->SetVisibility(bShowSlopeGrass, true);
				GrassSlopeRoot->SetHiddenInGame(!bShowSlopeGrass, true);

				if (bShowFlatGrass)
				{
					AddRandomizedGrass(
						GrassFlatRoot,
						FString::Printf(TEXT("GrassFlat_%d"), CellIndex),
						HashCombineFast(static_cast<uint32>(Preset.Seed), static_cast<uint32>(CellIndex * 17 + 11)),
						FlatGrassVariationX,
						FlatGrassVariationY);
				}
				if (bShowSlopeGrass)
				{
					AddRandomizedGrass(
						GrassSlopeRoot,
						FString::Printf(TEXT("GrassSlope_%d"), CellIndex),
						HashCombineFast(static_cast<uint32>(Preset.Seed ^ 0x5A17), static_cast<uint32>(CellIndex * 31 + 7)),
						SlopeGrassVariationX,
						SlopeGrassVariationY);
				}
			}

			if (Cell.bSlope)
			{
				++SlopeCount;
			}
			else
			{
				++FlatCount;
			}

			if (!Cell.bSlope && bIsSurfaceFeatureOrigin)
			{
				if (CreateSurfaceFeatureChild(
					GetOrCreateTileRoot(),
					FString::Printf(TEXT("SurfaceFeature_%d"), CellIndex),
					Cell))
				{
					++SurfaceFeatureCount;
				}
			}

			if (bRenderFarmDecor && !Cell.bSlope && Cell.Decoration != ET66MapCellDecoration::None)
			{
				if (UStaticMesh* DecorMesh = PickDecorationMesh(Assets, Cell.Decoration))
				{
					if (UStaticMeshComponent* DecorComponent = CreateMeshChild(
						GetOrCreateTileRoot(),
						FString::Printf(TEXT("Decor_%d"), CellIndex),
						DecorMesh,
						nullptr,
						FTransform(Cell.DecorationLocalRotation, Cell.DecorationLocalOffset, Cell.DecorationLocalScale),
						true))
					{
						DecorComponent->CanCharacterStepUpOn = ECanBeCharacterBase::ECB_No;
						++DecorCount;
					}
				}
			}
		};

		const double BoardCellLoopStartSeconds = FPlatformTime::Seconds();
		for (int32 CellIndex = 0; CellIndex < Board.Cells.Num(); ++CellIndex)
		{
			SpawnFarmCell(Board.Cells[CellIndex], CellIndex);
		}
		const double BoardCellLoopEndSeconds = FPlatformTime::Seconds();

		const double CollisionStripLoopStartSeconds = FPlatformTime::Seconds();
		for (int32 Z = 0; Z < Board.Settings.BoardSize; ++Z)
		{
			int32 RunStartX = INDEX_NONE;
			int32 RunLevel = 0;
			auto FlushFlatRun = [&](int32 EndX)
			{
				if (RunStartX == INDEX_NONE)
				{
					return;
				}

				const int32 WidthCells = EndX - RunStartX + 1;
				const float CenterX = (static_cast<float>(RunStartX + EndX) * 0.5f) * CellSize;
				AddFlatCollisionBox(
					FString::Printf(TEXT("FlatCollisionStrip_%d_%d"), Z, RunStartX),
					FVector(CenterX, static_cast<float>(Z) * CellSize, static_cast<float>(RunLevel) * StepHeight),
					FVector(static_cast<float>(WidthCells) * CellSize * 0.5f, CellSize * 0.5f, StepHeight * 0.5f));
				RunStartX = INDEX_NONE;
			};

			for (int32 X = 0; X < Board.Settings.BoardSize; ++X)
			{
				const FCell& Cell = Board.Cells[Board.Index(X, Z)];
				const bool bFlatCollisionCell = Cell.bOccupied && !Cell.bSlope && !HasSurfaceFeature(Cell);
				if (!bFlatCollisionCell)
				{
					FlushFlatRun(X - 1);
					continue;
				}

				if (RunStartX == INDEX_NONE)
				{
					RunStartX = X;
					RunLevel = Cell.Level;
					continue;
				}

				if (Cell.Level != RunLevel)
				{
					FlushFlatRun(X - 1);
					RunStartX = X;
					RunLevel = Cell.Level;
				}
			}

			FlushFlatRun(Board.Settings.BoardSize - 1);
		}
		const double CollisionStripLoopEndSeconds = FPlatformTime::Seconds();

		const double ExtraCellLoopStartSeconds = FPlatformTime::Seconds();
		for (int32 ExtraIndex = 0; ExtraIndex < Board.ExtraCells.Num(); ++ExtraIndex)
		{
			SpawnFarmCell(Board.ExtraCells[ExtraIndex], Board.Cells.Num() + ExtraIndex);
		}
		const double ExtraCellLoopEndSeconds = FPlatformTime::Seconds();

		const double CollisionStartSeconds = FPlatformTime::Seconds();

		auto FinalizeInstancedMeshBatch = [](UInstancedStaticMeshComponent* Component, const TArray<FTransform>& InstanceTransforms)
		{
			if (!Component)
			{
				return;
			}

			if (InstanceTransforms.Num() > 0)
			{
				Component->PreAllocateInstancesMemory(InstanceTransforms.Num());
				Component->AddInstances(InstanceTransforms, false, false, false);
			}

			Component->RegisterComponent();
		};

		FinalizeInstancedMeshBatch(FlatTileInstances, FlatTileTransforms);
		FinalizeInstancedMeshBatch(SlopeTileInstances, SlopeTileTransforms);

		UE_LOG(LogT66MainMapTerrain, Log, TEXT("[MAP] Main map terrain base geometry spawned in %.1f ms (board loop %.1f ms, strip loop %.1f ms, extra loop %.1f ms, feature cook %.1f ms, flat collision register %.1f ms, instancing %.1f ms)."),
			(FPlatformTime::Seconds() - SpawnStartSeconds) * 1000.0,
			(BoardCellLoopEndSeconds - BoardCellLoopStartSeconds) * 1000.0,
			(CollisionStripLoopEndSeconds - CollisionStripLoopStartSeconds) * 1000.0,
			(ExtraCellLoopEndSeconds - ExtraCellLoopStartSeconds) * 1000.0,
			SurfaceFeatureSpawnSeconds * 1000.0,
			FlatCollisionSpawnSeconds * 1000.0,
			(FPlatformTime::Seconds() - CollisionStartSeconds) * 1000.0);

		for (int32 RiverIndex = 0; RiverIndex < Board.LavaRivers.Num(); ++RiverIndex)
		{
			const FLavaRiver& River = Board.LavaRivers[RiverIndex];
			if (River.PuddleCell.X == INDEX_NONE || River.PuddleCell.Y == INDEX_NONE || River.LengthCells <= 0)
			{
				continue;
			}

			FVector PuddleLocation = FVector::ZeroVector;
			if (!TryGetCellLocation(Board, River.PuddleCell, 0.0f, PuddleLocation))
			{
				continue;
			}

			const FVector2D FlowDirection(static_cast<float>(River.Direction.X), static_cast<float>(River.Direction.Y));
			const float PuddleSize = CellSize * LavaRiverPuddleCellFraction;
			if (SpawnDecorativeLavaPatch(
				FString::Printf(TEXT("LavaPuddle_%d"), RiverIndex),
				PuddleLocation,
				PuddleSize,
				PuddleSize,
				FlowDirection))
			{
				++LavaRiverVisualCount;
			}

			const FVector StripOffset(
				static_cast<float>(River.Direction.X) * CellSize * 0.5f * static_cast<float>(River.LengthCells),
				static_cast<float>(River.Direction.Y) * CellSize * 0.5f * static_cast<float>(River.LengthCells),
				0.0f);
			const FVector StripLocation = PuddleLocation + StripOffset;
			const float StripWidth = CellSize * LavaRiverWidthCellFraction;
			const float StripLength = CellSize * static_cast<float>(River.LengthCells);
			const float StripSizeX = River.Direction.X != 0 ? StripLength : StripWidth;
			const float StripSizeY = River.Direction.Y != 0 ? StripLength : StripWidth;
			if (SpawnDecorativeLavaPatch(
				FString::Printf(TEXT("LavaRiver_%d"), RiverIndex),
				StripLocation,
				StripSizeX,
				StripSizeY,
				FlowDirection))
			{
				++LavaRiverVisualCount;
			}
		}

		const float WallThickness = CellSize;
		const float WallHeight = StepHeight * static_cast<float>(WallHeightLevels);
		const float WallCenterZ = 0.0f;
		auto AddWallCollision = [&](const FString& Name, const FVector& LocalCenter, const FVector& Extent)
		{
			UBoxComponent* Box = NewObject<UBoxComponent>(VisualActor, FName(*Name));
			VisualActor->AddInstanceComponent(Box);
			Box->SetupAttachment(VisualRoot);
			Box->SetMobility(EComponentMobility::Static);
			Box->SetRelativeLocation(LocalCenter);
			Box->SetBoxExtent(Extent);
			ConfigureTerrainCollisionComponent(Box);
			Box->RegisterComponent();
		};

		if (bSpawnFarmPerimeterBoundary)
		{
			auto ShouldOpenNorth = [&](int32 X) -> bool
			{
				return (Board.StartOutwardDirection == FIntPoint(0, 1) && Board.StartAnchor.X == X)
					|| (Board.BossOutwardDirection == FIntPoint(0, 1) && Board.BossAnchor.X == X);
			};

			auto ShouldOpenSouth = [&](int32 X) -> bool
			{
				return (Board.StartOutwardDirection == FIntPoint(0, -1) && Board.StartAnchor.X == X)
					|| (Board.BossOutwardDirection == FIntPoint(0, -1) && Board.BossAnchor.X == X);
			};

			auto ShouldOpenEast = [&](int32 Z) -> bool
			{
				return (Board.StartOutwardDirection == FIntPoint(1, 0) && Board.StartAnchor.Y == Z)
					|| (Board.BossOutwardDirection == FIntPoint(1, 0) && Board.BossAnchor.Y == Z);
			};

			auto ShouldOpenWest = [&](int32 Z) -> bool
			{
				return (Board.StartOutwardDirection == FIntPoint(-1, 0) && Board.StartAnchor.Y == Z)
					|| (Board.BossOutwardDirection == FIntPoint(-1, 0) && Board.BossAnchor.Y == Z);
			};

			for (int32 X = 0; X < Board.Settings.BoardSize; ++X)
			{
				if (!ShouldOpenNorth(X))
				{
					if (bRenderFarmBoundaryWalls)
					{
						CreateWallPrefab(
							FString::Printf(TEXT("FarmWallNorth_%d"), X),
							FVector(static_cast<float>(X) * CellSize, static_cast<float>(Board.Settings.BoardSize) * CellSize, WallCenterZ),
							FVector(CellSize, WallThickness, WallHeight));
					}
					AddWallCollision(
						*FString::Printf(TEXT("FarmWallCollisionNorth_%d"), X),
						FVector(static_cast<float>(X) * CellSize, static_cast<float>(Board.Settings.BoardSize) * CellSize, WallCenterZ),
						FVector(CellSize * 0.5f, WallThickness * 0.5f, WallHeight * 0.5f));
				}

				if (!ShouldOpenSouth(X))
				{
					if (bRenderFarmBoundaryWalls)
					{
						CreateWallPrefab(
							FString::Printf(TEXT("FarmWallSouth_%d"), X),
							FVector(static_cast<float>(X) * CellSize, -CellSize, WallCenterZ),
							FVector(CellSize, WallThickness, WallHeight));
					}
					AddWallCollision(
						*FString::Printf(TEXT("FarmWallCollisionSouth_%d"), X),
						FVector(static_cast<float>(X) * CellSize, -CellSize, WallCenterZ),
						FVector(CellSize * 0.5f, WallThickness * 0.5f, WallHeight * 0.5f));
				}
			}

			for (int32 Z = 0; Z < Board.Settings.BoardSize; ++Z)
			{
				if (!ShouldOpenEast(Z))
				{
					if (bRenderFarmBoundaryWalls)
					{
						CreateWallPrefab(
							FString::Printf(TEXT("FarmWallEast_%d"), Z),
							FVector(static_cast<float>(Board.Settings.BoardSize) * CellSize, static_cast<float>(Z) * CellSize, WallCenterZ),
							FVector(WallThickness, CellSize, WallHeight));
					}
					AddWallCollision(
						*FString::Printf(TEXT("FarmWallCollisionEast_%d"), Z),
						FVector(static_cast<float>(Board.Settings.BoardSize) * CellSize, static_cast<float>(Z) * CellSize, WallCenterZ),
						FVector(WallThickness * 0.5f, CellSize * 0.5f, WallHeight * 0.5f));
				}

				if (!ShouldOpenWest(Z))
				{
					if (bRenderFarmBoundaryWalls)
					{
						CreateWallPrefab(
							FString::Printf(TEXT("FarmWallWest_%d"), Z),
							FVector(-CellSize, static_cast<float>(Z) * CellSize, WallCenterZ),
							FVector(WallThickness, CellSize, WallHeight));
					}
					AddWallCollision(
						*FString::Printf(TEXT("FarmWallCollisionWest_%d"), Z),
						FVector(-CellSize, static_cast<float>(Z) * CellSize, WallCenterZ),
						FVector(WallThickness * 0.5f, CellSize * 0.5f, WallHeight * 0.5f));
				}
			}
		}

		int32 MinOccupiedX = 0;
		int32 MaxOccupiedX = Board.Settings.BoardSize - 1;
		int32 MinOccupiedZ = 0;
		int32 MaxOccupiedZ = Board.Settings.BoardSize - 1;
		GetOccupiedGridBounds(Board, MinOccupiedX, MaxOccupiedX, MinOccupiedZ, MaxOccupiedZ);
		const float MinOccupiedLocalX = static_cast<float>(MinOccupiedX) * CellSize - CellSize * 0.5f;
		const float MaxOccupiedLocalX = static_cast<float>(MaxOccupiedX) * CellSize + CellSize * 0.5f;
		const float MinOccupiedLocalY = static_cast<float>(MinOccupiedZ) * CellSize - CellSize * 0.5f;
		const float MaxOccupiedLocalY = static_cast<float>(MaxOccupiedZ) * CellSize + CellSize * 0.5f;
		const float SafetyCatchThickness = FMath::Max(1800.0f, StepHeight * 8.0f);
		const float SafetyCatchTopZ = GetLowestCollisionBottomZ(Preset) - 200.0f;
		UBoxComponent* SafetyCatch = NewObject<UBoxComponent>(VisualActor, TEXT("TerrainSafetyCatch"));
		VisualActor->AddInstanceComponent(SafetyCatch);
		SafetyCatch->SetupAttachment(VisualRoot);
		SafetyCatch->SetMobility(EComponentMobility::Static);
		SafetyCatch->SetRelativeLocation(FVector(
			(MinOccupiedLocalX + MaxOccupiedLocalX) * 0.5f,
			(MinOccupiedLocalY + MaxOccupiedLocalY) * 0.5f,
			SafetyCatchTopZ - SafetyCatchThickness * 0.5f));
		SafetyCatch->SetBoxExtent(FVector(
			(MaxOccupiedLocalX - MinOccupiedLocalX) * 0.5f + CellSize,
			(MaxOccupiedLocalY - MinOccupiedLocalY) * 0.5f + CellSize,
			SafetyCatchThickness * 0.5f));
		ConfigureTerrainSafetyCatchComponent(SafetyCatch);
		SafetyCatch->RegisterComponent();

		RefreshFarmVisualMaterials();

		bOutCollisionReady = true;
		UE_LOG(LogT66MainMapTerrain, Log, TEXT("[MAP] Main map terrain spawned in %.1f ms: %d occupied tiles, %d flat tiles, %d slope tiles, %d flat collision strips, %d support tiles, %d decor, %d surface features, %d lava visuals"),
			(FPlatformTime::Seconds() - SpawnStartSeconds) * 1000.0,
			Board.OccupiedCount,
			FlatCount,
			SlopeCount,
			FlatCollisionCount,
			SupportCount,
			DecorCount,
			SurfaceFeatureCount,
			LavaRiverVisualCount);
		return true;
	}
}

