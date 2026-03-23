// Copyright Tribulation 66. All Rights Reserved.

#include "Gameplay/T66MainMapTerrain.h"

#include "Core/T66GameInstance.h"
#include "Data/T66DataTypes.h"
#include "Gameplay/T66ProceduralLandscapeParams.h"
#include "Components/BoxComponent.h"
#include "Components/PrimitiveComponent.h"
#include "Components/SceneComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/StaticMesh.h"
#include "Engine/Texture.h"
#include "Engine/Texture2D.h"
#include "Engine/World.h"
#include "GameFramework/Actor.h"
#include "Materials/MaterialInterface.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "PhysicsEngine/BodySetup.h"
#include "TimerManager.h"
#include "UObject/UObjectGlobals.h"

namespace T66MainMapTerrain
{
	namespace
	{
		static constexpr float SourceCellSizeUU = 200.0f;
		static constexpr float SourceStepHeightUU = 120.0f;
		static constexpr float TargetMainMapBoardScale = 10.0f;
		static constexpr int32 WallHeightLevels = 50;
		static constexpr bool bRenderFarmGrass = false;
		static constexpr bool bRenderFarmDecor = true;
		static constexpr bool bRenderFarmSupports = true;
		static constexpr bool bRenderFarmBoundaryWalls = false;
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
			UMaterialInterface* WallMaterial = nullptr;
			UTexture* BlockTexture = nullptr;
			UTexture* SlopeTexture = nullptr;
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
			case ET66Difficulty::Medium: return { TEXT("MediumOcean"), TEXT("MediumOcean") };
			case ET66Difficulty::Hard: return { TEXT("HardMountain"), TEXT("HardMountain") };
			case ET66Difficulty::VeryHard: return { TEXT("VeryHardGraveyard"), TEXT("VeryHardGraveyard") };
			case ET66Difficulty::Impossible: return { TEXT("ImpossibleNorthPole"), TEXT("ImpossibleNorthPole") };
			case ET66Difficulty::Perdition: return { TEXT("PerditionMars"), TEXT("PerditionMars") };
			case ET66Difficulty::Final: return { TEXT("FinalHell"), TEXT("FinalHell") };
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
				UE_LOG(LogTemp, Warning, TEXT("[MAP] Missing difficulty terrain texture: %s"), *ThemeTexturePath);
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

			Component->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
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

			Component->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
			Component->SetCollisionObjectType(ECC_WorldStatic);
			Component->SetCollisionResponseToAllChannels(ECR_Block);
			Component->SetGenerateOverlapEvents(false);
			Component->SetCanEverAffectNavigation(false);
		}

		static void ConfigureTerrainSafetyCatchComponent(UPrimitiveComponent* Component)
		{
			if (!Component) return;

			Component->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
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
			Cell.Region = Region;
			Cell.DecorationLocalOffset = FVector::ZeroVector;
			Cell.DecorationLocalRotation = FRotator::ZeroRotator;
			Cell.DecorationLocalScale = FVector(1.0f, 1.0f, 1.0f);
		}

		static FIntPoint BuildExtensionArea(
			FBoard& Board,
			const FIntPoint& Anchor,
			const FIntPoint& OutwardDirection,
			ECellRegion PathRegion,
			ECellRegion AreaRegion,
			FIntPoint* OutPathCoordinate = nullptr)
		{
			const FCell* AnchorCell = Board.GetCell(Anchor.X, Anchor.Y);
			if (!AnchorCell)
			{
				return FIntPoint(INDEX_NONE, INDEX_NONE);
			}

			(void)PathRegion;
			const FIntPoint PathCoordinate = Anchor + OutwardDirection;

			if (OutPathCoordinate)
			{
				*OutPathCoordinate = PathCoordinate;
			}

			AddExtensionCell(Board, PathCoordinate, AnchorCell->Level, AreaRegion);
			return PathCoordinate;
		}

		static const FCell* FindPreferredSpawnCell(const FBoard& Board)
		{
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

			if (ObjectToSpawn > 0.82f)
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

			if (ObjectToSpawn > 0.60f)
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
			OutAssets.BlockMesh = LoadObject<UStaticMesh>(nullptr, TEXT("/Game/World/Terrain/Megabonk/SM_MegabonkBlock.SM_MegabonkBlock"));
			OutAssets.SlopeMesh = LoadObject<UStaticMesh>(nullptr, TEXT("/Game/World/Terrain/Megabonk/SM_MegabonkSlope.SM_MegabonkSlope"));
			if (!OutAssets.BlockMesh || !OutAssets.SlopeMesh)
			{
				return false;
			}

			OutAssets.EnvironmentUnlitMaterial = LoadObject<UMaterialInterface>(nullptr, TEXT("/Game/Materials/M_Environment_Unlit.M_Environment_Unlit"));
			OutAssets.BlockMaterial = LoadObject<UMaterialInterface>(nullptr, TEXT("/Game/World/Terrain/Megabonk/MI_MegabonkBlock.MI_MegabonkBlock"));
			OutAssets.SlopeMaterial = LoadObject<UMaterialInterface>(nullptr, TEXT("/Game/World/Terrain/Megabonk/MI_MegabonkSlope.MI_MegabonkSlope"));
			OutAssets.WallMaterial = LoadObject<UMaterialInterface>(nullptr, TEXT("/Game/World/Terrain/Megabonk/MI_MegabonkWall.MI_MegabonkWall"));
			OutAssets.BlockTexture = LoadObject<UTexture>(nullptr, TEXT("/Game/World/Terrain/Megabonk/T_MegabonkBlock.T_MegabonkBlock"));
			OutAssets.SlopeTexture = LoadObject<UTexture>(nullptr, TEXT("/Game/World/Terrain/Megabonk/T_MegabonkSlope.T_MegabonkSlope"));
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
			if (!OutAssets.GrassMesh)
			{
				OutAssets.GrassMesh = LoadObject<UStaticMesh>(nullptr, TEXT("/Game/T66MapAssets/Grass/SM_Grass_02.SM_Grass_02"));
			}
			if (UMaterialInterface* GrassMaterial1 = LoadObject<UMaterialInterface>(nullptr, TEXT("/Game/World/Ground/MI_Grass1.MI_Grass1")))
			{
				OutAssets.GrassMaterials.Add(GrassMaterial1);
			}
			if (UMaterialInterface* GrassMaterial2 = LoadObject<UMaterialInterface>(nullptr, TEXT("/Game/World/Ground/MI_Grass2.MI_Grass2")))
			{
				OutAssets.GrassMaterials.Add(GrassMaterial2);
			}
			if (UMaterialInterface* GrassMaterial3 = LoadObject<UMaterialInterface>(nullptr, TEXT("/Game/World/Ground/MI_Grass3.MI_Grass3")))
			{
				OutAssets.GrassMaterials.Add(GrassMaterial3);
			}
			if (UMaterialInterface* GrassMaterial4 = LoadObject<UMaterialInterface>(nullptr, TEXT("/Game/World/Ground/MI_Grass4.MI_Grass4")))
			{
				OutAssets.GrassMaterials.Add(GrassMaterial4);
			}
			if (OutAssets.GrassMaterials.Num() == 0)
			{
				if (UMaterialInterface* FallbackGrassMaterial = LoadObject<UMaterialInterface>(nullptr, TEXT("/Game/World/Props/Grass/Materials/Material_0_014.Material_0_014")))
				{
					OutAssets.GrassMaterials.Add(FallbackGrassMaterial);
				}
			}

			OutAssets.TreeMesh1 = LoadObject<UStaticMesh>(nullptr, TEXT("/Game/World/Props/Tree.Tree"));
			OutAssets.TreeMesh2 = LoadObject<UStaticMesh>(nullptr, TEXT("/Game/World/Props/Tree2.Tree2"));
			OutAssets.TreeMesh3 = LoadObject<UStaticMesh>(nullptr, TEXT("/Game/World/Props/Tree3.Tree3"));
			OutAssets.RockMesh1 = LoadObject<UStaticMesh>(nullptr, TEXT("/Game/World/Props/Rocks.Rocks"));
			OutAssets.RockMesh2 = OutAssets.RockMesh1;
			OutAssets.RockMesh3 = OutAssets.RockMesh1;
			OutAssets.LogMesh = LoadObject<UStaticMesh>(nullptr, TEXT("/Game/World/Props/Log.Log"));
			UE_LOG(LogTemp, Log, TEXT("[MAP] Main map terrain assets: WallMaterial=%s WallTexture=%s Tree1=%s Tree2=%s Tree3=%s Rock=%s Rocks=%s"),
				OutAssets.WallMaterial ? TEXT("yes") : TEXT("no"),
				OutAssets.WallTexture ? TEXT("yes") : TEXT("no"),
				OutAssets.TreeMesh1 ? TEXT("yes") : TEXT("no"),
				OutAssets.TreeMesh2 ? TEXT("yes") : TEXT("no"),
				OutAssets.TreeMesh3 ? TEXT("yes") : TEXT("no"),
				OutAssets.RockMesh1 ? TEXT("yes") : TEXT("no"),
				OutAssets.RockMesh2 ? TEXT("yes") : TEXT("no"));
			return true;
		}
	}

	FT66MapPreset BuildPresetForDifficulty(ET66Difficulty Difficulty, int32 Seed)
	{
		FT66MapPreset Preset = FT66MapPreset::GetDefaultForTheme(ET66MapTheme::Farm);
		Preset.Seed = Seed;

		switch (Difficulty)
		{
		case ET66Difficulty::Medium:
			Preset.ElevationMin = -10000.0f;
			Preset.ElevationMax = 0.0f;
			break;

		case ET66Difficulty::Hard:
			Preset.ElevationMin = 0.0f;
			Preset.ElevationMax = 10000.0f;
			break;

		case ET66Difficulty::Easy:
		case ET66Difficulty::VeryHard:
		case ET66Difficulty::Impossible:
		case ET66Difficulty::Perdition:
		case ET66Difficulty::Final:
		default:
			Preset.ElevationMin = -5000.0f;
			Preset.ElevationMax = 5000.0f;
			break;
		}

		return Preset;
	}

	FSettings MakeSettings(const FT66MapPreset& Preset)
	{
		FSettings Settings;
		Settings.BoardSize = 40;
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

	FVector GetSpawnLocation(const FT66MapPreset& Preset, float Z)
	{
		FBoard Board;
		if (Generate(Preset, Board))
		{
			if (const FCell* SpawnCell = FindPreferredSpawnCell(Board))
			{
				return GetCellCenter(Preset, SpawnCell->Z, SpawnCell->X, Z);
			}
		}

		const FSettings Settings = MakeSettings(Preset);
		return GetCellCenter(Preset, Settings.BoardSize / 2, Settings.BoardSize / 2, Z);
	}

	FVector GetPreferredSpawnLocation(const FT66MapPreset& Preset, float HeightOffset)
	{
		FBoard Board;
		if (!Generate(Preset, Board))
		{
			const FSettings Settings = MakeSettings(Preset);
			return GetCellCenter(Preset, Settings.BoardSize / 2, Settings.BoardSize / 2, Preset.BaselineZ + Settings.StepHeight + HeightOffset);
		}

		const FCell* SpawnCell = FindPreferredSpawnCell(Board);
		if (!SpawnCell)
		{
			const FSettings Settings = MakeSettings(Preset);
			return GetCellCenter(Preset, Settings.BoardSize / 2, Settings.BoardSize / 2, Preset.BaselineZ + Settings.StepHeight + HeightOffset);
		}

		return GetCellCenter(
			Preset,
			SpawnCell->Z,
			SpawnCell->X,
			GetCellTopSurfaceZ(Board.Settings, *SpawnCell) + HeightOffset);
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
			Cell->Region = ECellRegion::MainBoard;
			Cell->DecorationLocalOffset = FVector::ZeroVector;
			Cell->DecorationLocalRotation = FRotator::ZeroRotator;
			Cell->DecorationLocalScale = FVector(1.0f, 1.0f, 1.0f);
			++OutBoard.OccupiedCount;
			return Cell;
		};

		FCell* CurrentCell = CreateElement(
			Rng.RandRange(0, OutBoard.Settings.BoardSize - 1),
			0,
			Rng.RandRange(0, OutBoard.Settings.BoardSize - 1));
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

			if (bDescendingMode
				&& CurrentCell->Level > MinElevationLevel
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
			UE_LOG(LogTemp, Error, TEXT("[MAP] Main map terrain board incomplete: %d / %d cells occupied"), Board.OccupiedCount, Board.Cells.Num());
			return false;
		}

		FLoadedAssets Assets;
		if (!LoadAssets(Assets))
		{
			UE_LOG(LogTemp, Error, TEXT("[MAP] Main map terrain tile meshes missing; aborting terrain spawn."));
			return false;
		}

		const UT66GameInstance* T66GI = World ? Cast<UT66GameInstance>(World->GetGameInstance()) : nullptr;
		const ET66Difficulty Difficulty = T66GI ? T66GI->SelectedDifficulty : ET66Difficulty::Easy;
		ApplyDifficultyTextureOverrides(Assets, Difficulty);

		EnableComplexCollisionForMesh(Assets.BlockMesh);
		EnableComplexCollisionForMesh(Assets.SlopeMesh);

		const FVector BoardOrigin = GetBoardOrigin(Preset);
		const float CellSize = Board.Settings.CellSize;
		const float StepHeight = Board.Settings.StepHeight;
		const float MapSize = static_cast<float>(Board.Settings.BoardSize) * CellSize;

		AActor* VisualActor = World->SpawnActor<AActor>(AActor::StaticClass(), BoardOrigin, FRotator::ZeroRotator, SpawnParams);
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
		VisualRoot->RegisterComponent();

		auto ResolveForcedTextureMaterial = [VisualActor, Assets](
			UTexture* Texture,
			const TCHAR* TexturePath,
			const TCHAR* FallbackMaterialPath,
			const TCHAR* DebugName) -> FResolvedFarmMaterial
		{
			UMaterialInterface* EnvironmentUnlitMaterial = Assets.EnvironmentUnlitMaterial;
			if (!EnvironmentUnlitMaterial)
			{
				EnvironmentUnlitMaterial = LoadObject<UMaterialInterface>(nullptr, TEXT("/Game/Materials/M_Environment_Unlit.M_Environment_Unlit"));
			}
			if (!Texture && TexturePath)
			{
				Texture = LoadObject<UTexture>(nullptr, TexturePath);
			}

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

			if (FallbackMaterialPath)
			{
				if (UMaterialInterface* FallbackMaterial = LoadObject<UMaterialInterface>(nullptr, FallbackMaterialPath))
				{
					UE_LOG(LogTemp, Warning, TEXT("[MAP] Main map terrain material %s fell back to %s"), DebugName, FallbackMaterialPath);
					return { FallbackMaterial, false };
				}
			}

			UE_LOG(LogTemp, Warning, TEXT("[MAP] Main map terrain material %s is still missing its real texture-backed material"), DebugName);
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
			TEXT("/Game/World/Terrain/Megabonk/T_MegabonkBlock.T_MegabonkBlock"),
			TEXT("/Game/World/Terrain/Megabonk/MI_MegabonkBlock.MI_MegabonkBlock"),
			TEXT("FarmBlockMID"));
		const FResolvedFarmMaterial InitialSlopeMaterial = ResolveForcedTextureMaterial(
			Assets.SlopeTexture,
			TEXT("/Game/World/Terrain/Megabonk/T_MegabonkSlope.T_MegabonkSlope"),
			TEXT("/Game/World/Terrain/Megabonk/MI_MegabonkSlope.MI_MegabonkSlope"),
			TEXT("FarmSlopeMID"));
		const FResolvedFarmMaterial InitialWallMaterial = ResolveForcedTextureMaterial(
			Assets.WallTexture,
			TEXT("/Game/World/Terrain/Megabonk/T_MegabonkWall.T_MegabonkWall"),
			TEXT("/Game/World/Terrain/Megabonk/MI_MegabonkWall.MI_MegabonkWall"),
			TEXT("FarmWallMID"));
		UMaterialInterface* EffectiveBlockMaterial = InitialBlockMaterial.Material;
		UMaterialInterface* EffectiveSlopeMaterial = InitialSlopeMaterial.Material;
		UMaterialInterface* EffectiveWallMaterial = InitialWallMaterial.Material;
		UpdateFarmMaterialReadyTag(
			InitialBlockMaterial.bUsingRealTexture &&
			InitialSlopeMaterial.bUsingRealTexture &&
			(!bRenderFarmBoundaryWalls || InitialWallMaterial.bUsingRealTexture));

		auto RefreshFarmVisualMaterials = [VisualActor, Assets, ResolveForcedTextureMaterial, UpdateFarmMaterialReadyTag]() -> bool
		{
			if (!VisualActor || !IsValid(VisualActor))
			{
				return false;
			}

			const FResolvedFarmMaterial BlockMaterial = ResolveForcedTextureMaterial(
				Assets.BlockTexture,
				TEXT("/Game/World/Terrain/Megabonk/T_MegabonkBlock.T_MegabonkBlock"),
				TEXT("/Game/World/Terrain/Megabonk/MI_MegabonkBlock.MI_MegabonkBlock"),
				TEXT("FarmBlockMID"));
			const FResolvedFarmMaterial SlopeMaterial = ResolveForcedTextureMaterial(
				Assets.SlopeTexture,
				TEXT("/Game/World/Terrain/Megabonk/T_MegabonkSlope.T_MegabonkSlope"),
				TEXT("/Game/World/Terrain/Megabonk/MI_MegabonkSlope.MI_MegabonkSlope"),
				TEXT("FarmSlopeMID"));
			const FResolvedFarmMaterial WallMaterial = ResolveForcedTextureMaterial(
				Assets.WallTexture,
				TEXT("/Game/World/Terrain/Megabonk/T_MegabonkWall.T_MegabonkWall"),
				TEXT("/Game/World/Terrain/Megabonk/MI_MegabonkWall.MI_MegabonkWall"),
				TEXT("FarmWallMID"));
			const bool bMaterialsReady =
				BlockMaterial.bUsingRealTexture &&
				SlopeMaterial.bUsingRealTexture &&
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
					MeshComponent->SetMaterial(0, BlockMaterial.Material);
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

		auto SetTileActiveState = [&](UStaticMeshComponent* Component, bool bActive)
		{
			if (!Component)
			{
				return;
			}

			Component->SetVisibility(bActive, true);
			Component->SetHiddenInGame(!bActive, true);
			Component->SetCollisionEnabled(bActive ? ECollisionEnabled::QueryAndPhysics : ECollisionEnabled::NoCollision);
			if (bActive)
			{
				ConfigureTerrainVisualCollisionComponent(Component);
				Component->CanCharacterStepUpOn = ECanBeCharacterBase::ECB_Yes;
			}
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
		const FVector TerrainMeshScale = GetTerrainMeshScale(Board.Settings);
		const int32 LowestSupportLevel = GetMinBoardElevationLevel(Preset);

		auto SpawnFarmCell = [&](const FCell& Cell, int32 CellIndex)
		{
			if (!Cell.bOccupied)
			{
				return;
			}

			USceneComponent* TileRoot = CreateSceneChild(
				VisualRoot,
				FString::Printf(TEXT("FarmTile_%d"), CellIndex),
				FVector(
					static_cast<float>(Cell.X) * CellSize,
					static_cast<float>(Cell.Z) * CellSize,
					static_cast<float>(Cell.Level) * StepHeight));

			UStaticMeshComponent* CubeComponent = CreateMeshChild(
				TileRoot,
				FString::Printf(TEXT("Cube_%d"), CellIndex),
				Assets.BlockMesh,
				EffectiveBlockMaterial,
				FTransform(FRotator::ZeroRotator, FVector::ZeroVector, TerrainMeshScale),
				true);
			UStaticMeshComponent* SlopeComponent = CreateMeshChild(
				TileRoot,
				FString::Printf(TEXT("Slope_%d"), CellIndex),
				Assets.SlopeMesh,
				EffectiveSlopeMaterial,
				FTransform(FRotator(0.0f, GetSlopeYaw(Cell.Shape), 0.0f), FVector::ZeroVector, TerrainMeshScale),
				true);
			SetTileActiveState(CubeComponent, !Cell.bSlope);
			SetTileActiveState(SlopeComponent, Cell.bSlope);

			if (!Cell.bSlope)
			{
				UBoxComponent* BlockCollision = NewObject<UBoxComponent>(VisualActor, FName(*FString::Printf(TEXT("BlockCollision_%d"), CellIndex)));
				VisualActor->AddInstanceComponent(BlockCollision);
				BlockCollision->SetupAttachment(TileRoot);
				BlockCollision->SetMobility(EComponentMobility::Static);
				BlockCollision->SetRelativeLocation(FVector::ZeroVector);
				BlockCollision->SetBoxExtent(FVector(CellSize * 0.5f, CellSize * 0.5f, StepHeight * 0.5f));
				ConfigureTerrainCollisionComponent(BlockCollision);
				BlockCollision->CanCharacterStepUpOn = ECanBeCharacterBase::ECB_Yes;
				BlockCollision->RegisterComponent();
			}

			if (bRenderFarmSupports)
			{
				USceneComponent* BottomPartRoot = CreateSceneChild(
					TileRoot,
					FString::Printf(TEXT("BottomPart_%d"), CellIndex),
					FVector::ZeroVector);
				for (int32 SupportLevel = Cell.Level - 1; SupportLevel >= LowestSupportLevel; --SupportLevel)
				{
					const FVector SupportOffset(0.0f, 0.0f, static_cast<float>(SupportLevel - Cell.Level) * StepHeight);
					CreateMeshChild(
						BottomPartRoot,
						FString::Printf(TEXT("Support_%d_L%d"), CellIndex, SupportLevel),
						Assets.BlockMesh,
						EffectiveBlockMaterial,
						FTransform(
							FRotator::ZeroRotator,
							SupportOffset,
							TerrainMeshScale),
						true);
					UBoxComponent* SupportCollision = NewObject<UBoxComponent>(VisualActor, FName(*FString::Printf(TEXT("SupportCollision_%d_L%d"), CellIndex, SupportLevel)));
					VisualActor->AddInstanceComponent(SupportCollision);
					SupportCollision->SetupAttachment(BottomPartRoot);
					SupportCollision->SetMobility(EComponentMobility::Static);
					SupportCollision->SetRelativeLocation(SupportOffset);
					SupportCollision->SetBoxExtent(FVector(CellSize * 0.5f, CellSize * 0.5f, StepHeight * 0.5f));
					ConfigureTerrainCollisionComponent(SupportCollision);
					SupportCollision->CanCharacterStepUpOn = ECanBeCharacterBase::ECB_Yes;
					SupportCollision->RegisterComponent();
					++SupportCount;
				}
			}

			if (bRenderFarmGrass)
			{
				USceneComponent* GrassFlatRoot = CreateSceneChild(
					TileRoot,
					FString::Printf(TEXT("GrassFlat_%d"), CellIndex),
					FVector::ZeroVector);
				USceneComponent* GrassSlopeRoot = CreateSceneChild(
					TileRoot,
					FString::Printf(TEXT("GrassSlope_%d"), CellIndex),
					FVector::ZeroVector,
					FRotator(-30.59f, GetSlopeYaw(Cell.Shape), 0.0f));

				const bool bShowFlatGrass = !Cell.bSlope;
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

			if (bRenderFarmDecor && !Cell.bSlope && Cell.Decoration != ET66MapCellDecoration::None)
			{
				if (UStaticMesh* DecorMesh = PickDecorationMesh(Assets, Cell.Decoration))
				{
					if (CreateMeshChild(
						TileRoot,
						FString::Printf(TEXT("Decor_%d"), CellIndex),
						DecorMesh,
						nullptr,
						FTransform(Cell.DecorationLocalRotation, Cell.DecorationLocalOffset, Cell.DecorationLocalScale),
						false))
					{
						++DecorCount;
					}
				}
			}
		};

		for (int32 CellIndex = 0; CellIndex < Board.Cells.Num(); ++CellIndex)
		{
			SpawnFarmCell(Board.Cells[CellIndex], CellIndex);
		}

		for (int32 ExtraIndex = 0; ExtraIndex < Board.ExtraCells.Num(); ++ExtraIndex)
		{
			SpawnFarmCell(Board.ExtraCells[ExtraIndex], Board.Cells.Num() + ExtraIndex);
		}

		const float WallThickness = CellSize;
		const float WallHeight = StepHeight * static_cast<float>(WallHeightLevels);
		const float WallCenterZ = 0.0f;
		const float WallOffset = static_cast<float>(Board.Settings.BoardSize) * CellSize * 0.5f - CellSize * 0.5f;

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

		const float SafetyCatchThickness = FMath::Max(1800.0f, StepHeight * 8.0f);
		const float SafetyCatchTopZ = GetLowestCollisionBottomZ(Preset) - 200.0f;
		UBoxComponent* SafetyCatch = NewObject<UBoxComponent>(VisualActor, TEXT("TerrainSafetyCatch"));
		VisualActor->AddInstanceComponent(SafetyCatch);
		SafetyCatch->SetupAttachment(VisualRoot);
		SafetyCatch->SetMobility(EComponentMobility::Static);
		SafetyCatch->SetRelativeLocation(FVector(WallOffset, WallOffset, SafetyCatchTopZ - SafetyCatchThickness * 0.5f));
		SafetyCatch->SetBoxExtent(FVector(MapSize * 0.5f + CellSize, MapSize * 0.5f + CellSize, SafetyCatchThickness * 0.5f));
		ConfigureTerrainSafetyCatchComponent(SafetyCatch);
		SafetyCatch->RegisterComponent();

		auto QueueMaterialRefresh = [World, RefreshFarmVisualMaterials](float DelaySeconds)
		{
			FTimerHandle RefreshHandle;
			World->GetTimerManager().SetTimer(
				RefreshHandle,
				FTimerDelegate::CreateLambda([RefreshFarmVisualMaterials]()
				{
					RefreshFarmVisualMaterials();
				}),
				DelaySeconds,
				false);
		};

		RefreshFarmVisualMaterials();
		World->GetTimerManager().SetTimerForNextTick(FTimerDelegate::CreateLambda([RefreshFarmVisualMaterials]()
		{
			RefreshFarmVisualMaterials();
		}));
		QueueMaterialRefresh(0.25f);
		QueueMaterialRefresh(0.60f);
		QueueMaterialRefresh(1.00f);
		QueueMaterialRefresh(1.50f);
		QueueMaterialRefresh(2.00f);
		QueueMaterialRefresh(3.00f);

		bOutCollisionReady = true;
		UE_LOG(LogTemp, Log, TEXT("[MAP] Main map terrain spawned: %d occupied tiles, %d flat tiles, %d slope tiles, %d support tiles, %d decor"),
			Board.OccupiedCount,
			FlatCount,
			SlopeCount,
			SupportCount,
			DecorCount);
		return true;
	}
}

