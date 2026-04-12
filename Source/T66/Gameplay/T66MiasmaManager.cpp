// Copyright Tribulation 66. All Rights Reserved.

#include "Gameplay/T66MiasmaManager.h"
#include "Gameplay/T66LavaShared.h"

#include "Core/T66GameInstance.h"
#include "Core/T66LagTrackerSubsystem.h"
#include "Core/T66RunStateSubsystem.h"
#include "Engine/StaticMesh.h"
#include "Engine/Texture.h"
#include "Engine/Texture2D.h"
#include "Engine/World.h"
#include "Gameplay/T66HeroBase.h"
#include "Gameplay/T66GameMode.h"
#include "Gameplay/T66MainMapTerrain.h"
#include "Gameplay/T66TowerMapTerrain.h"
#include "Components/InstancedStaticMeshComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "Materials/MaterialInterface.h"

DEFINE_LOG_CATEGORY_STATIC(LogT66MiasmaManager, Log, All);

namespace
{
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

	static UTexture* T66GetFallbackTexture()
	{
		static TObjectPtr<UTexture> Cached = nullptr;
		if (!Cached)
		{
			Cached = LoadObject<UTexture>(nullptr, T66LavaShared::DefaultTexturePath);
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

void AT66MiasmaManager::BuildMainMapCellGrid()
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

	TileSize = Board.Settings.CellSize;

	auto AddCellCenter = [&](const T66MainMapTerrain::FCell& Cell)
	{
		if (!Cell.bOccupied)
		{
			return;
		}

		FVector CellCenter = FVector::ZeroVector;
		const float CoverHeightOffset = T66MainMapTerrain::GetSurfaceFeatureLavaCoverHeight(Cell);
		if (!T66MainMapTerrain::TryGetCellLocation(Board, FIntPoint(Cell.X, Cell.Z), TileZ + CoverHeightOffset, CellCenter))
		{
			return;
		}

		TileCenters.Add(CellCenter);
	};

	for (const T66MainMapTerrain::FCell& Cell : Board.Cells)
	{
		AddCellCenter(Cell);
	}
	for (const T66MainMapTerrain::FCell& Cell : Board.ExtraCells)
	{
		AddCellCenter(Cell);
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

	UE_LOG(
		LogT66MiasmaManager,
		Log,
		TEXT("[LAVA] Built main-map lava grid with %d full-cell patches (cellSize=%.0f, stage=%d)."),
		TileCenters.Num(),
		TileSize,
		StageNum);
}

void AT66MiasmaManager::BuildTowerFloorGrid()
{
	TileCenters.Reset();

	UWorld* World = GetWorld();
	AT66GameMode* GameMode = World ? Cast<AT66GameMode>(World->GetAuthGameMode()) : nullptr;
	T66TowerMapTerrain::FLayout Layout;
	if (!World || !GameMode || !GameMode->GetTowerMainMapLayout(Layout))
	{
		return;
	}

	TileSize = 1200.0f;
	for (const T66TowerMapTerrain::FFloor& Floor : Layout.Floors)
	{
		if (!Floor.bGameplayFloor)
		{
			continue;
		}

		for (float X = -Floor.WalkableHalfExtent + (TileSize * 0.5f); X < Floor.WalkableHalfExtent; X += TileSize)
		{
			for (float Y = -Floor.WalkableHalfExtent + (TileSize * 0.5f); Y < Floor.WalkableHalfExtent; Y += TileSize)
			{
				const FVector Candidate = Floor.Center + FVector(X, Y, TileZ);
				if (Floor.bHasDropHole
					&& FMath::Abs(Candidate.X - Floor.HoleCenter.X) <= (Floor.HoleHalfExtent.X + TileSize * 0.45f)
					&& FMath::Abs(Candidate.Y - Floor.HoleCenter.Y) <= (Floor.HoleHalfExtent.Y + TileSize * 0.45f))
				{
					continue;
				}

				TileCenters.Add(FVector(Candidate.X, Candidate.Y, Floor.SurfaceZ + TileZ));
			}
		}
	}

	FRandomStream Stream(Layout.Preset.Seed + 1703);
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
	if (ShouldUseTowerBloodLook())
	{
		BuildTowerFloorGrid();
		if (TileCenters.Num() > 0)
		{
			return;
		}
	}

	if (T66ShouldUseMainBoardCoverage(GetWorld()))
	{
		BuildMainMapCellGrid();
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
	const int32 Total = TileCenters.Num();
	int32 Desired = 0;
	if (Duration <= 0.f)
	{
		Desired = Total;
	}
	else
	{
		const float ExpansionStartDelay = FMath::Clamp(ExpansionStartDelaySeconds, 0.f, Duration);
		const float ActiveDuration = FMath::Max(Duration - ExpansionStartDelay, 0.f);
		const float ExpansionInterval = FMath::Max(ExpansionIntervalSeconds, 1.0f);
		if (Elapsed >= ExpansionStartDelay && ActiveDuration > KINDA_SMALL_NUMBER)
		{
			const int32 ExpansionSteps = FMath::Max(1, FMath::CeilToInt(ActiveDuration / ExpansionInterval));
			const float ActiveElapsed = FMath::Clamp(Elapsed - ExpansionStartDelay, 0.f, ActiveDuration);
			const int32 CompletedSteps = FMath::Clamp(
				FMath::FloorToInt(ActiveElapsed / ExpansionInterval) + 1,
				0,
				ExpansionSteps);
			const float StepAlpha = static_cast<float>(CompletedSteps) / static_cast<float>(ExpansionSteps);
			Desired = FMath::Clamp(FMath::CeilToInt(StepAlpha * static_cast<float>(Total)), 0, Total);
		}
	}
	if (Remaining <= 0.05f)
	{
		Desired = Total;
	}

	EnsureSpawnedCount(Desired);
}

void AT66MiasmaManager::RebuildForCurrentStage()
{
	if (TileInstances)
	{
		TileInstances->ClearInstances();
	}

	TileCenters.Reset();
	SpawnedTileCount = 0;
	DamageTickAccumulator = 0.f;

	GenerateAnimationFrames();
	EnsureVisualMaterial();
	BuildGrid();
	UpdateFromRunState();
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
	const bool bTowerBlood = ShouldUseTowerBloodLook();
	for (int32 Index = 0; Index < SpawnedTileCount; ++Index)
	{
		const FVector& TileCenter = TileCenters[Index];
		if ((!bTowerBlood || FMath::Abs(HeroLocation.Z - TileCenter.Z) <= 1600.0f)
			&& FMath::Abs(HeroLocation.X - TileCenter.X) <= HalfExtent
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
		if (UMaterialInterface* BaseMaterial = LoadObject<UMaterialInterface>(nullptr, T66LavaShared::BaseMaterialPath))
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

	if (ShouldUseTowerBloodLook())
	{
		CoreColor = FLinearColor(0.06f, 0.00f, 0.00f, 1.0f);
		MidColor = FLinearColor(0.48f, 0.00f, 0.02f, 1.0f);
		GlowColor = FLinearColor(0.86f, 0.05f, 0.08f, 1.0f);
		Brightness = 1.45f;
		LavaMID->SetVectorParameterValue(TEXT("Tint"), FLinearColor(0.95f, 0.22f, 0.24f, 1.0f));
		LavaMID->SetVectorParameterValue(TEXT("BaseColor"), FLinearColor(0.95f, 0.22f, 0.24f, 1.0f));
		LavaMID->SetScalarParameterValue(TEXT("Brightness"), Brightness);
	}
}

bool AT66MiasmaManager::ShouldUseTowerBloodLook() const
{
	const UWorld* World = GetWorld();
	const AT66GameMode* GameMode = World ? Cast<AT66GameMode>(World->GetAuthGameMode()) : nullptr;
	return GameMode && GameMode->IsUsingTowerMainMapLayout();
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

	const float Phase = (static_cast<float>(FrameIndex) / static_cast<float>(ClampedFrames)) * T66LavaShared::TwoPi;

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

	const T66LavaShared::FCellSample Cells = T66LavaShared::SampleCells(FinalUV, FMath::Max(CellDensity, 1.0f), Phase);
	const float Border = FMath::Max(Cells.SecondClosest - Cells.Closest, 0.0f);
	const float Crack = FMath::Pow(T66LavaShared::Saturate(1.0f - Border * FMath::Max(EdgeContrast, 0.1f)), 2.3f);
	const float Pulse = 0.5f + 0.5f * FMath::Sin((FinalUV.X * 1.8f + FinalUV.Y * 1.25f) + Phase * 2.2f);
	const float Ember = T66LavaShared::Saturate(1.0f - T66LavaShared::SmoothStep(0.18f, 0.52f, Cells.Closest + Pulse * 0.06f));
	const float Heat = T66LavaShared::Saturate(Crack * 1.18f + Ember * 0.22f);

	FLinearColor Color = FMath::Lerp(CoreColor, MidColor, T66LavaShared::Saturate(Heat * 0.72f + Pulse * 0.10f));
	Color = FMath::Lerp(Color, GlowColor, T66LavaShared::Saturate(FMath::Pow(Crack, 0.72f)));

	const float PoolMask = T66LavaShared::SmoothStep(0.20f, 0.48f, Cells.Closest);
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
