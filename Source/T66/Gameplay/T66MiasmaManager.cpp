// Copyright Tribulation 66. All Rights Reserved.

#include "Gameplay/T66MiasmaManager.h"

#include "Core/T66GameInstance.h"
#include "Core/T66LagTrackerSubsystem.h"
#include "Core/T66RunStateSubsystem.h"
#include "Engine/StaticMesh.h"
#include "Engine/Texture.h"
#include "Engine/Texture2D.h"
#include "Engine/World.h"
#include "Gameplay/T66HeroBase.h"
#include "Gameplay/T66MainMapTerrain.h"
#include "Components/InstancedStaticMeshComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "Materials/MaterialInterface.h"

namespace
{
	constexpr float T66LavaTwoPi = 6.28318530718f;
	const TCHAR* T66LavaBaseMaterialPath = TEXT("/Game/Materials/M_Environment_Unlit.M_Environment_Unlit");
	const TCHAR* T66DefaultTexturePath = TEXT("/Engine/EngineResources/DefaultTexture.DefaultTexture");

	static bool T66ShouldUseMainBoardCoverage(const UWorld* World)
	{
		if (!World)
		{
			return false;
		}

		const FString MapName = UWorld::RemovePIEPrefix(World->GetMapName());
		return !MapName.Contains(TEXT("Coliseum"))
			&& !MapName.Contains(TEXT("Tutorial"))
			&& !MapName.Contains(TEXT("Lab"));
	}

	static float T66LavaFrac(float Value)
	{
		return Value - FMath::FloorToFloat(Value);
	}

	static float T66LavaSaturate(float Value)
	{
		return FMath::Clamp(Value, 0.f, 1.f);
	}

	static float T66LavaSmoothStep(float Edge0, float Edge1, float Value)
	{
		if (FMath::IsNearlyEqual(Edge0, Edge1))
		{
			return Value < Edge0 ? 0.f : 1.f;
		}

		const float T = T66LavaSaturate((Value - Edge0) / (Edge1 - Edge0));
		return T * T * (3.f - 2.f * T);
	}

	static FVector2D T66LavaHash2(const FVector2D& P)
	{
		return FVector2D(
			T66LavaFrac(FMath::Sin(FVector2D::DotProduct(P, FVector2D(127.1f, 311.7f))) * 43758.5453123f),
			T66LavaFrac(FMath::Sin(FVector2D::DotProduct(P, FVector2D(269.5f, 183.3f))) * 43758.5453123f));
	}

	struct FT66LavaCellSample
	{
		float Closest = BIG_NUMBER;
		float SecondClosest = BIG_NUMBER;
	};

	static FT66LavaCellSample T66SampleLavaCells(const FVector2D& UV, float Density, float Phase)
	{
		FT66LavaCellSample Result;

		const FVector2D P = UV * Density;
		const FVector2D Cell(FMath::FloorToFloat(P.X), FMath::FloorToFloat(P.Y));
		const FVector2D Local = P - Cell;

		for (int32 Y = -1; Y <= 1; ++Y)
		{
			for (int32 X = -1; X <= 1; ++X)
			{
				const FVector2D NeighborCell = Cell + FVector2D(static_cast<float>(X), static_cast<float>(Y));
				const FVector2D Jitter = T66LavaHash2(NeighborCell);
				const FVector2D Drift(
					FMath::Sin(Phase * (0.65f + Jitter.X * 0.25f) + NeighborCell.Y * 1.73f + Jitter.Y * 4.0f),
					FMath::Cos(Phase * (0.50f + Jitter.Y * 0.25f) + NeighborCell.X * 1.37f + Jitter.X * 4.0f));
				const FVector2D FeaturePoint =
					FVector2D(static_cast<float>(X), static_cast<float>(Y)) + Jitter + Drift * 0.16f;

				const float DistSq = (Local - FeaturePoint).SizeSquared();
				if (DistSq < Result.Closest)
				{
					Result.SecondClosest = Result.Closest;
					Result.Closest = DistSq;
				}
				else if (DistSq < Result.SecondClosest)
				{
					Result.SecondClosest = DistSq;
				}
			}
		}

		Result.Closest = FMath::Sqrt(Result.Closest);
		Result.SecondClosest = FMath::Sqrt(Result.SecondClosest);
		return Result;
	}

	static UTexture* T66GetFallbackTexture()
	{
		static TObjectPtr<UTexture> Cached = nullptr;
		if (!Cached)
		{
			Cached = LoadObject<UTexture>(nullptr, T66DefaultTexturePath);
		}
		return Cached.Get();
	}
}

AT66MiasmaManager::AT66MiasmaManager()
{
	PrimaryActorTick.bCanEverTick = true;

	TileInstances = CreateDefaultSubobject<UInstancedStaticMeshComponent>(TEXT("TileInstances"));
	TileInstances->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	TileInstances->SetGenerateOverlapEvents(false);
	TileInstances->SetCanEverAffectNavigation(false);
	RootComponent = TileInstances;

	if (UStaticMesh* Plane = LoadObject<UStaticMesh>(nullptr, TEXT("/Engine/BasicShapes/Plane.Plane")))
	{
		TileInstances->SetStaticMesh(Plane);
	}
}

void AT66MiasmaManager::BeginPlay()
{
	Super::BeginPlay();

	GenerateAnimationFrames();
	EnsureVisualMaterial();

	if (UWorld* World = GetWorld())
	{
		AnimationStartTimeSeconds = World->GetTimeSeconds();
	}

	if (!bSpawningPaused)
	{
		BuildGrid();
		UpdateFromRunState();
	}
}

void AT66MiasmaManager::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	UpdateFromRunState();
	TickDamageOverActiveTiles(DeltaTime);

	if (!LavaMID)
	{
		return;
	}

	LavaMID->SetScalarParameterValue(TEXT("Brightness"), Brightness);

	UWorld* World = GetWorld();
	if (!World || AnimationFPS <= KINDA_SMALL_NUMBER || GeneratedFrames.Num() <= 1)
	{
		return;
	}

	const float Elapsed = FMath::Max(World->GetTimeSeconds() - AnimationStartTimeSeconds, 0.f);
	const int32 FrameIndex = FMath::FloorToInt(Elapsed * AnimationFPS) % GeneratedFrames.Num();
	if (FrameIndex != CurrentFrameIndex)
	{
		ApplyAnimationFrame(FrameIndex);
	}
}

void AT66MiasmaManager::BuildMainMapSubcellGrid()
{
	TileCenters.Reset();

	UWorld* World = GetWorld();
	UGameInstance* GI = World ? World->GetGameInstance() : nullptr;
	UT66GameInstance* T66GI = GI ? Cast<UT66GameInstance>(GI) : nullptr;
	UT66RunStateSubsystem* RunState = GI ? GI->GetSubsystem<UT66RunStateSubsystem>() : nullptr;
	if (!World || !T66GI)
	{
		return;
	}

	const int32 StageNum = RunState ? RunState->GetCurrentStage() : 1;
	const FT66MapPreset Preset = T66MainMapTerrain::BuildPresetForDifficulty(T66GI->SelectedDifficulty, T66GI->RunSeed);
	T66MainMapTerrain::FBoard Board;
	if (!T66MainMapTerrain::Generate(Preset, Board))
	{
		return;
	}

	TileSize = Board.Settings.CellSize * 0.5f;
	const float QuarterCellOffset = TileSize * 0.5f;

	for (const T66MainMapTerrain::FCell& Cell : Board.Cells)
	{
		if (!Cell.bOccupied)
		{
			continue;
		}

		FVector CellCenter = FVector::ZeroVector;
		if (!T66MainMapTerrain::TryGetCellLocation(Board, FIntPoint(Cell.X, Cell.Z), 0.f, CellCenter))
		{
			continue;
		}

		for (const float OffsetX : { -QuarterCellOffset, QuarterCellOffset })
		{
			for (const float OffsetY : { -QuarterCellOffset, QuarterCellOffset })
			{
				FVector SampleLocation = CellCenter + FVector(OffsetX, OffsetY, 0.f);
				FHitResult Hit;
				const FVector TraceStart = SampleLocation + FVector(0.f, 0.f, 4000.f);
				const FVector TraceEnd = SampleLocation - FVector(0.f, 0.f, 9000.f);
				if (World->LineTraceSingleByChannel(Hit, TraceStart, TraceEnd, ECC_WorldStatic))
				{
					SampleLocation.Z = Hit.ImpactPoint.Z + TileZ;
				}
				else
				{
					SampleLocation.Z = CellCenter.Z + TileZ;
				}

				TileCenters.Add(SampleLocation);
			}
		}
	}

	FRandomStream Stream(T66GI->RunSeed + (StageNum * 97));
	for (int32 Index = TileCenters.Num() - 1; Index > 0; --Index)
	{
		const int32 SwapIndex = Stream.RandRange(0, Index);
		if (Index != SwapIndex)
		{
			TileCenters.Swap(Index, SwapIndex);
		}
	}
}

void AT66MiasmaManager::BuildGrid()
{
	if (T66ShouldUseMainBoardCoverage(GetWorld()))
	{
		BuildMainMapSubcellGrid();
		if (TileCenters.Num() > 0)
		{
			return;
		}
	}

	TileCenters.Reset();

	const int32 Num = FMath::Max(1, FMath::FloorToInt((CoverageHalfExtent * 2.f) / TileSize));
	const float Start = -CoverageHalfExtent + (TileSize * 0.5f);

	for (int32 ix = 0; ix < Num; ++ix)
	{
		for (int32 iy = 0; iy < Num; ++iy)
		{
			const float X = Start + ix * TileSize;
			const float Y = Start + iy * TileSize;
			TileCenters.Add(FVector(X, Y, TileZ));
		}
	}

	FRandomStream Stream(Seed);
	for (int32 i = TileCenters.Num() - 1; i > 0; --i)
	{
		const int32 j = Stream.RandRange(0, i);
		if (i != j)
		{
			TileCenters.Swap(i, j);
		}
	}
}

void AT66MiasmaManager::UpdateFromRunState()
{
	if (bSpawningPaused)
	{
		return;
	}

	UWorld* World = GetWorld();
	if (!World)
	{
		return;
	}

	if (TileCenters.Num() <= 0)
	{
		BuildGrid();
	}

	FLagScopedScope LagScope(World, TEXT("MiasmaManager::UpdateFromRunState (EnsureSpawnedCount)"));
	UGameInstance* GI = World->GetGameInstance();
	UT66RunStateSubsystem* RunState = GI ? GI->GetSubsystem<UT66RunStateSubsystem>() : nullptr;
	if (!RunState || TileCenters.Num() <= 0 || !RunState->GetStageTimerActive())
	{
		return;
	}

	const float Remaining = RunState->GetStageTimerSecondsRemaining();
	const float Duration = UT66RunStateSubsystem::StageTimerDurationSeconds;
	const float Elapsed = FMath::Clamp(Duration - Remaining, 0.f, Duration);
	const float Alpha = (Duration <= 0.f) ? 1.f : (Elapsed / Duration);

	const int32 Total = TileCenters.Num();
	int32 Desired = FMath::Clamp(FMath::FloorToInt(Alpha * static_cast<float>(Total)), 0, Total);
	if (Desired == 0)
	{
		Desired = 1;
	}
	if (Remaining <= 0.05f)
	{
		Desired = Total;
	}

	EnsureSpawnedCount(Desired);
}

void AT66MiasmaManager::EnsureSpawnedCount(int32 DesiredCount)
{
	if (!TileInstances)
	{
		return;
	}

	const FVector InstanceScale(FMath::Max(TileSize / 100.f, 0.1f), FMath::Max(TileSize / 100.f, 0.1f), 1.f);
	while (SpawnedTileCount < DesiredCount && SpawnedTileCount < TileCenters.Num())
	{
		const FVector& Location = TileCenters[SpawnedTileCount];
		TileInstances->AddInstance(FTransform(FRotator::ZeroRotator, Location, InstanceScale));
		++SpawnedTileCount;
	}
}

void AT66MiasmaManager::TickDamageOverActiveTiles(float DeltaTime)
{
	if (bSpawningPaused || SpawnedTileCount <= 0 || DamageIntervalSeconds <= 0.f)
	{
		return;
	}

	DamageTickAccumulator += DeltaTime;
	if (DamageTickAccumulator < DamageIntervalSeconds)
	{
		return;
	}
	DamageTickAccumulator = 0.f;

	UWorld* World = GetWorld();
	UGameInstance* GI = World ? World->GetGameInstance() : nullptr;
	UT66RunStateSubsystem* RunState = GI ? GI->GetSubsystem<UT66RunStateSubsystem>() : nullptr;
	AT66HeroBase* Hero = Cast<AT66HeroBase>(UGameplayStatics::GetPlayerPawn(this, 0));
	if (!RunState || !Hero)
	{
		return;
	}

	const FVector HeroLocation = Hero->GetActorLocation();
	const float HalfExtent = TileSize * 0.5f;
	for (int32 Index = 0; Index < SpawnedTileCount; ++Index)
	{
		const FVector& TileCenter = TileCenters[Index];
		if (FMath::Abs(HeroLocation.X - TileCenter.X) <= HalfExtent
			&& FMath::Abs(HeroLocation.Y - TileCenter.Y) <= HalfExtent)
		{
			RunState->ApplyDamage(20, this);
			return;
		}
	}
}

void AT66MiasmaManager::EnsureVisualMaterial()
{
	if (!TileInstances)
	{
		return;
	}

	if (!LavaMID)
	{
		if (UMaterialInterface* BaseMaterial = LoadObject<UMaterialInterface>(nullptr, T66LavaBaseMaterialPath))
		{
			LavaMID = UMaterialInstanceDynamic::Create(BaseMaterial, this);
			if (LavaMID)
			{
				TileInstances->SetMaterial(0, LavaMID);
			}
		}
	}

	if (!LavaMID)
	{
		return;
	}

	UTexture* TextureToUse = GeneratedFrames.IsValidIndex(CurrentFrameIndex)
		? GeneratedFrames[CurrentFrameIndex].Get()
		: T66GetFallbackTexture();
	if (TextureToUse)
	{
		LavaMID->SetTextureParameterValue(TEXT("DiffuseColorMap"), TextureToUse);
		LavaMID->SetTextureParameterValue(TEXT("BaseColorTexture"), TextureToUse);
	}

	LavaMID->SetVectorParameterValue(TEXT("Tint"), FLinearColor::White);
	LavaMID->SetVectorParameterValue(TEXT("BaseColor"), FLinearColor::White);
	LavaMID->SetScalarParameterValue(TEXT("Brightness"), Brightness);
}

void AT66MiasmaManager::GenerateAnimationFrames()
{
	GeneratedFrames.Reset();
	CurrentFrameIndex = INDEX_NONE;

	const int32 Resolution = FMath::Clamp(TextureResolution, 16, 256);
	const int32 ClampedFrames = FMath::Clamp(AnimationFrames, 4, 64);

	GeneratedFrames.Reserve(ClampedFrames);
	for (int32 FrameIndex = 0; FrameIndex < ClampedFrames; ++FrameIndex)
	{
		if (UTexture2D* Texture = BuildFrameTexture(FrameIndex, ClampedFrames, Resolution))
		{
			GeneratedFrames.Add(Texture);
		}
	}

	if (GeneratedFrames.Num() > 0)
	{
		CurrentFrameIndex = 0;
	}
}

UTexture2D* AT66MiasmaManager::BuildFrameTexture(const int32 FrameIndex, const int32 ClampedFrames, const int32 Resolution) const
{
	if (Resolution <= 0 || ClampedFrames <= 0)
	{
		return nullptr;
	}

	TArray<FColor> Pixels;
	Pixels.SetNumUninitialized(Resolution * Resolution);

	const float Phase = (static_cast<float>(FrameIndex) / static_cast<float>(ClampedFrames)) * T66LavaTwoPi;

	for (int32 Y = 0; Y < Resolution; ++Y)
	{
		for (int32 X = 0; X < Resolution; ++X)
		{
			const FVector2D UV(
				(static_cast<float>(X) + 0.5f) / static_cast<float>(Resolution),
				(static_cast<float>(Y) + 0.5f) / static_cast<float>(Resolution));

			Pixels[Y * Resolution + X] = SampleLavaColor(UV, Phase).ToFColorSRGB();
		}
	}

	UTexture2D* Texture = UTexture2D::CreateTransient(Resolution, Resolution, PF_B8G8R8A8);
	if (!Texture || !Texture->GetPlatformData() || Texture->GetPlatformData()->Mips.Num() == 0)
	{
		return nullptr;
	}

	Texture->SRGB = true;
	Texture->Filter = TF_Nearest;
	Texture->AddressX = TA_Wrap;
	Texture->AddressY = TA_Wrap;
	Texture->LODGroup = TEXTUREGROUP_Pixels2D;
	Texture->NeverStream = true;
#if WITH_EDITORONLY_DATA
	Texture->MipGenSettings = TMGS_NoMipmaps;
#endif

	FTexture2DMipMap& Mip = Texture->GetPlatformData()->Mips[0];
	void* MipData = Mip.BulkData.Lock(LOCK_READ_WRITE);
	FMemory::Memcpy(MipData, Pixels.GetData(), Pixels.Num() * sizeof(FColor));
	Mip.BulkData.Unlock();
	Texture->UpdateResource();

	return Texture;
}

FLinearColor AT66MiasmaManager::SampleLavaColor(const FVector2D& BaseUV, const float Phase) const
{
	FVector2D UV = BaseUV * FMath::Max(UVScale, 0.1f) + PatternOffset;

	const FVector2D FlowNormal = FlowDir.IsNearlyZero() ? FVector2D(1.f, 0.f) : FlowDir.GetSafeNormal();
	UV += FlowNormal * Phase * FlowSpeed * 0.20f;

	const float SafeCloseness = FMath::Max(WarpCloseness, 0.01f);
	const float Nx = UV.X / SafeCloseness;
	const float Ny = UV.Y / SafeCloseness;
	const float WarpT = Phase * WarpSpeed;
	const float WarpedX = Nx + WarpIntensity * FMath::Sin(WarpT + Ny * 2.0f);
	const float WarpedY = Ny + WarpIntensity * FMath::Sin(WarpT + Nx * 2.0f);
	const FVector2D FinalUV = FVector2D(WarpedX, WarpedY) * SafeCloseness;

	const FT66LavaCellSample Cells = T66SampleLavaCells(FinalUV, FMath::Max(CellDensity, 1.0f), Phase);
	const float Border = FMath::Max(Cells.SecondClosest - Cells.Closest, 0.0f);
	const float Crack = FMath::Pow(T66LavaSaturate(1.0f - Border * FMath::Max(EdgeContrast, 0.1f)), 2.3f);
	const float Pulse = 0.5f + 0.5f * FMath::Sin((FinalUV.X * 1.8f + FinalUV.Y * 1.25f) + Phase * 2.2f);
	const float Ember = T66LavaSaturate(1.0f - T66LavaSmoothStep(0.18f, 0.52f, Cells.Closest + Pulse * 0.06f));
	const float Heat = T66LavaSaturate(Crack * 1.18f + Ember * 0.22f);

	FLinearColor Color = FMath::Lerp(CoreColor, MidColor, T66LavaSaturate(Heat * 0.72f + Pulse * 0.10f));
	Color = FMath::Lerp(Color, GlowColor, T66LavaSaturate(FMath::Pow(Crack, 0.72f)));

	const float PoolMask = T66LavaSmoothStep(0.20f, 0.48f, Cells.Closest);
	Color = FMath::Lerp(Color, CoreColor * 0.55f, PoolMask * 0.35f);
	Color.A = 1.f;
	return Color.GetClamped(0.f, 1.f);
}

void AT66MiasmaManager::ApplyAnimationFrame(const int32 FrameIndex)
{
	if (!GeneratedFrames.IsValidIndex(FrameIndex))
	{
		return;
	}

	CurrentFrameIndex = FrameIndex;
	EnsureVisualMaterial();
}

void AT66MiasmaManager::ClearAllMiasma()
{
	if (TileInstances)
	{
		TileInstances->ClearInstances();
	}
	SpawnedTileCount = 0;
	DamageTickAccumulator = 0.f;
}
