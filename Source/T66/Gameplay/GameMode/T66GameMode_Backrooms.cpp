// Copyright Tribulation 66. All Rights Reserved.

#include "Gameplay/GameMode/T66GameModePrivate.h"

#include "Components/ActorComponent.h"
#include "Core/T66WeaponManagerSubsystem.h"
#include "Gameplay/Enemies/Projectiles/T66EnemyProjectileBase.h"
#include "Gameplay/T66BackroomsChaser.h"
#include "Gameplay/T66BackroomsDoorInteractable.h"
#include "Gameplay/T66HeroProjectile.h"
#include "Gameplay/T66LavaPatch.h"
#include "Gameplay/T66MiasmaTile.h"
#include "Gameplay/T66MobBase.h"
#include "Gameplay/T66MobManagerSubsystem.h"
#include "Gameplay/Traps/T66TrapArrowProjectile.h"
#include "GameFramework/SpringArmComponent.h"
#include "HAL/FileManager.h"
#include "HAL/PlatformMisc.h"
#include "Misc/CommandLine.h"
#include "Misc/Parse.h"
#include "Misc/Paths.h"
#include "UnrealClient.h"

using namespace T66GameModePrivate;

namespace
{
	static TAutoConsoleVariable<float> CVarT66BackroomsSpawnChance(
		TEXT("T66.Backrooms.SpawnChance"),
		0.08f,
		TEXT("Chance that a Backrooms door is spawned into an eligible tower stage. Ignored when T66.Backrooms.ForceSpawn is 1."),
#if UE_BUILD_SHIPPING
		ECVF_ReadOnly
#else
		ECVF_Default
#endif
	);

	static TAutoConsoleVariable<int32> CVarT66BackroomsForceSpawn(
		TEXT("T66.Backrooms.ForceSpawn"),
		0,
		TEXT("Non-shipping diagnostic: force a Backrooms door to spawn on eligible tower stages."),
#if UE_BUILD_SHIPPING
		ECVF_ReadOnly
#else
		ECVF_Default
#endif
	);

	static const FName T66BackroomsActorTag(TEXT("T66_Backrooms"));
	static const TCHAR* T66BackroomsWallTexturePath = TEXT("/Game/World/Backrooms/Textures/T_Backrooms_Wall.T_Backrooms_Wall");
	static const TCHAR* T66BackroomsFloorTexturePath = TEXT("/Game/World/Backrooms/Textures/T_Backrooms_Floor.T_Backrooms_Floor");
	static constexpr int32 T66BackroomsDefaultMazeWidth = 15;
	static constexpr int32 T66BackroomsDefaultMazeHeight = 15;
	static constexpr float T66BackroomsWallHeight = 760.f;
	static constexpr float T66BackroomsFloorThickness = 90.f;

	static UMaterialInstanceDynamic* T66CreateBackroomsTextureMaterial(UObject* Outer, const TCHAR* TexturePath, const FLinearColor& FallbackColor)
	{
		UMaterialInterface* BaseMaterial = FT66VisualUtil::GetFlatColorMaterial();
		if (!BaseMaterial)
		{
			return nullptr;
		}

		UMaterialInstanceDynamic* Material = UMaterialInstanceDynamic::Create(BaseMaterial, Outer);
		if (!Material)
		{
			return nullptr;
		}

		if (UTexture* Texture = LoadObject<UTexture>(nullptr, TexturePath))
		{
			Material->SetTextureParameterValue(TEXT("DiffuseColorMap"), Texture);
			Material->SetTextureParameterValue(TEXT("BaseColorTexture"), Texture);
			Material->SetVectorParameterValue(TEXT("Color"), FLinearColor::White);
			Material->SetVectorParameterValue(TEXT("BaseColor"), FLinearColor::White);
			Material->SetVectorParameterValue(TEXT("Tint"), FLinearColor::White);
		}
		else
		{
			FT66VisualUtil::ConfigureFlatColorMaterial(Material, FallbackColor);
		}

		return Material;
	}

	static AStaticMeshActor* T66SpawnBackroomsBox(
		UWorld* World,
		UStaticMesh* CubeMesh,
		UMaterialInterface* Material,
		const FVector& Center,
		const FVector& FullSize,
		const TCHAR* ActorName,
		const FActorSpawnParameters& SpawnParams)
	{
		if (!World || !CubeMesh)
		{
			return nullptr;
		}

		AStaticMeshActor* Box = World->SpawnActor<AStaticMeshActor>(
			AStaticMeshActor::StaticClass(),
			Center,
			FRotator::ZeroRotator,
			SpawnParams);
		if (!Box || !Box->GetStaticMeshComponent())
		{
			return nullptr;
		}

		const FName UniqueActorName = MakeUniqueObjectName(Box->GetOuter(), Box->GetClass(), FName(ActorName));
		Box->Rename(*UniqueActorName.ToString());
		Box->Tags.AddUnique(T66BackroomsActorTag);
		T66_SetStaticMeshActorMobility(Box, EComponentMobility::Movable);
		Box->GetStaticMeshComponent()->SetStaticMesh(CubeMesh);
		Box->SetActorScale3D(FVector(
			FMath::Max(1.f, FullSize.X) / 100.f,
			FMath::Max(1.f, FullSize.Y) / 100.f,
			FMath::Max(1.f, FullSize.Z) / 100.f));
		if (Material)
		{
			Box->GetStaticMeshComponent()->SetMaterial(0, Material);
		}
		Box->GetStaticMeshComponent()->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
		Box->GetStaticMeshComponent()->SetCollisionObjectType(ECC_WorldStatic);
		Box->GetStaticMeshComponent()->SetCollisionResponseToAllChannels(ECR_Block);
		T66_SetStaticMeshActorMobility(Box, EComponentMobility::Static);
		return Box;
	}

	static bool T66ShouldPauseForBackrooms(AActor* Actor)
	{
		if (!Actor)
		{
			return false;
		}

		return Actor->IsA(AT66EnemyDirector::StaticClass())
			|| Actor->IsA(AT66EnemyBase::StaticClass())
			|| Actor->IsA(AT66MobBase::StaticClass())
			|| Actor->IsA(AT66BossBase::StaticClass())
			|| Actor->IsA(AT66LoanShark::StaticClass())
			|| Actor->IsA(AT66CompanionBase::StaticClass())
			|| Actor->IsA(AT66MiasmaManager::StaticClass())
			|| Actor->IsA(AT66MiasmaBoundary::StaticClass())
			|| Actor->IsA(AT66MiasmaTile::StaticClass())
			|| Actor->IsA(AT66LavaPatch::StaticClass())
			|| Actor->IsA(AT66TrapBase::StaticClass())
			|| Actor->IsA(AT66EnemyProjectileBase::StaticClass())
			|| Actor->IsA(AT66HeroProjectile::StaticClass())
			|| Actor->IsA(AT66TrapArrowProjectile::StaticClass());
	}

	static bool T66TryBuildBackroomsDoorLocationFromWallBox(
		const T66TowerMapTerrain::FFloor& Floor,
		const FBox2D& WallBox,
		FRandomStream& Rng,
		FVector& OutLocation,
		FVector& OutWallNormal)
	{
		const FVector2D WallSize = WallBox.Max - WallBox.Min;
		if (WallSize.X <= 20.f || WallSize.Y <= 20.f)
		{
			return false;
		}

		const FVector2D WallCenter = (WallBox.Min + WallBox.Max) * 0.5f;
		const bool bWallRunsAlongX = WallSize.X >= WallSize.Y;
		const float DoorInset = 170.f;
		const float DoorZ = Floor.SurfaceZ + 8.f;

		if (bWallRunsAlongX)
		{
			const float SpanPadding = FMath::Min(720.f, FMath::Max(0.f, WallSize.X * 0.30f));
			const float CandidateX = (WallBox.Max.X - WallBox.Min.X) > (SpanPadding * 2.f)
				? Rng.FRandRange(WallBox.Min.X + SpanPadding, WallBox.Max.X - SpanPadding)
				: WallCenter.X;
			const float InwardY = (WallCenter.Y >= Floor.Center.Y) ? -1.f : 1.f;
			const float InnerFaceY = WallCenter.Y + (InwardY * ((WallSize.Y * 0.5f) + DoorInset));
			OutLocation = FVector(CandidateX, InnerFaceY, DoorZ);
			OutWallNormal = FVector(0.f, InwardY, 0.f);
			return true;
		}

		const float SpanPadding = FMath::Min(720.f, FMath::Max(0.f, WallSize.Y * 0.30f));
		const float CandidateY = (WallBox.Max.Y - WallBox.Min.Y) > (SpanPadding * 2.f)
			? Rng.FRandRange(WallBox.Min.Y + SpanPadding, WallBox.Max.Y - SpanPadding)
			: WallCenter.Y;
		const float InwardX = (WallCenter.X >= Floor.Center.X) ? -1.f : 1.f;
		const float InnerFaceX = WallCenter.X + (InwardX * ((WallSize.X * 0.5f) + DoorInset));
		OutLocation = FVector(InnerFaceX, CandidateY, DoorZ);
		OutWallNormal = FVector(InwardX, 0.f, 0.f);
		return true;
	}

	static bool T66TryGetBackroomsDoorWallLocation(
		const T66TowerMapTerrain::FLayout& Layout,
		const T66TowerMapTerrain::FFloor& Floor,
		FRandomStream& Rng,
		FVector& OutLocation,
		FVector& OutWallNormal)
	{
		if (!Floor.OuterShellWallBoxes.IsEmpty())
		{
			for (int32 Attempt = 0; Attempt < FMath::Max(8, Floor.OuterShellWallBoxes.Num() * 2); ++Attempt)
			{
				const FBox2D& WallBox = Floor.OuterShellWallBoxes[Rng.RandRange(0, Floor.OuterShellWallBoxes.Num() - 1)];
				if (T66TryBuildBackroomsDoorLocationFromWallBox(Floor, WallBox, Rng, OutLocation, OutWallNormal))
				{
					return true;
				}
			}
		}

		const float ShellInset = FMath::Max(240.f, Layout.WallThickness + 120.f);
		const float WallSpawnHalfExtent = FMath::Max(Floor.BoundsHalfExtent - ShellInset, 0.f);
		const float SideSweep = FMath::Max(700.f, WallSpawnHalfExtent - 1200.f);
		if (WallSpawnHalfExtent <= 1.f)
		{
			return false;
		}

		switch (Rng.RandRange(0, 3))
		{
		case 0:
			OutWallNormal = FVector(-1.f, 0.f, 0.f);
			OutLocation = Floor.Center + FVector(WallSpawnHalfExtent - 170.f, Rng.FRandRange(-SideSweep, SideSweep), 8.f);
			break;
		case 1:
			OutWallNormal = FVector(1.f, 0.f, 0.f);
			OutLocation = Floor.Center + FVector(-WallSpawnHalfExtent + 170.f, Rng.FRandRange(-SideSweep, SideSweep), 8.f);
			break;
		case 2:
			OutWallNormal = FVector(0.f, -1.f, 0.f);
			OutLocation = Floor.Center + FVector(Rng.FRandRange(-SideSweep, SideSweep), WallSpawnHalfExtent - 170.f, 8.f);
			break;
		default:
			OutWallNormal = FVector(0.f, 1.f, 0.f);
			OutLocation = Floor.Center + FVector(Rng.FRandRange(-SideSweep, SideSweep), -WallSpawnHalfExtent + 170.f, 8.f);
			break;
		}

		OutLocation.Z = Floor.SurfaceZ + 8.f;
		return true;
	}

#if !UE_BUILD_SHIPPING
#define T66_LOG_BACKROOMS_QA_RESULT(Condition, Format, ...) \
	do \
	{ \
		if (Condition) \
		{ \
			UE_LOG(LogT66GameMode, Log, Format, __VA_ARGS__); \
		} \
		else \
		{ \
			UE_LOG(LogT66GameMode, Error, Format, __VA_ARGS__); \
		} \
	} while (false)

	static const FName T66BackroomsQASeedItemID(TEXT("Item_AoeDamage"));

	static FString T66NormalizeBackroomsQAMode(const FString& RawMode)
	{
		FString Mode = RawMode.TrimStartAndEnd().ToLower();
		if (Mode == TEXT("success"))
		{
			Mode = TEXT("exit");
		}
		else if (Mode == TEXT("revive") || Mode == TEXT("quickrevive"))
		{
			Mode = TEXT("consume");
		}
		return Mode;
	}

	static bool T66IsValidBackroomsQAMode(const FString& Mode)
	{
		return Mode == TEXT("exit") || Mode == TEXT("death") || Mode == TEXT("consume");
	}

	static bool T66BackroomsInventoryHasItem(const UT66RunStateSubsystem* RunState, const FName ItemID)
	{
		if (!RunState || ItemID.IsNone())
		{
			return false;
		}

		for (const FT66InventorySlot& Slot : RunState->GetInventorySlots())
		{
			if (Slot.IsValid() && Slot.ItemTemplateID == ItemID)
			{
				return true;
			}
		}

		return false;
	}

	static void T66RequestBackroomsQAExitIfNeeded(const bool bSuccess, const TCHAR* Reason)
	{
		if (FParse::Param(FCommandLine::Get(), TEXT("T66BackroomsAutoQAKeepAlive")))
		{
			return;
		}

		FPlatformMisc::RequestExitWithStatus(false, bSuccess ? 0 : 70, Reason ? Reason : TEXT("T66BackroomsQAComplete"));
	}

	static void T66ConfigureBackroomsQACamera(AT66HeroBase* Hero, APlayerController* PC)
	{
		if (!Hero)
		{
			return;
		}

		Hero->SetActorRotation(FRotator(0.f, 35.f, 0.f));
		if (PC)
		{
			PC->SetControlRotation(FRotator(-38.f, 35.f, 0.f));
		}
		if (Hero->CameraBoom)
		{
			Hero->CameraBoom->TargetArmLength = 860.f;
			Hero->CameraBoom->SetRelativeLocation(FVector(0.f, 0.f, 185.f));
			Hero->CameraBoom->bDoCollisionTest = false;
		}
	}
#endif
}

bool AT66GameMode::TryBuildBackroomsMaze()
{
	BackroomsMazeWidth = T66BackroomsDefaultMazeWidth;
	BackroomsMazeHeight = T66BackroomsDefaultMazeHeight;
	BackroomsCellSize = 620.f;
	BackroomsOrigin = FVector(300000.f, 300000.f, 14000.f);
	BackroomsMazeOpenCells.Init(0, BackroomsMazeWidth * BackroomsMazeHeight);

	auto SetOpen = [this](const FIntPoint& Cell)
	{
		if (Cell.X >= 0 && Cell.X < BackroomsMazeWidth && Cell.Y >= 0 && Cell.Y < BackroomsMazeHeight)
		{
			BackroomsMazeOpenCells[(Cell.Y * BackroomsMazeWidth) + Cell.X] = 1;
		}
	};

	UT66GameInstance* T66GI = GetT66GameInstance();
	UGameInstance* GI = GetGameInstance();
	UT66RunStateSubsystem* RunState = GI ? GI->GetSubsystem<UT66RunStateSubsystem>() : nullptr;
	const int32 RunSeed = T66EnsureRunSeed(T66GI);
	const int32 StageNum = RunState ? RunState->GetCurrentStage() : 1;
	FRandomStream Rng(RunSeed ^ (StageNum * 100003) ^ 0xBACC00);

	const FIntPoint StartCell(1, 1);
	TArray<FIntPoint> Stack;
	Stack.Add(StartCell);
	SetOpen(StartCell);

	while (Stack.Num() > 0)
	{
		const FIntPoint Current = Stack.Last();
		TArray<FIntPoint> Directions =
		{
			FIntPoint(2, 0),
			FIntPoint(-2, 0),
			FIntPoint(0, 2),
			FIntPoint(0, -2)
		};
		for (int32 Index = Directions.Num() - 1; Index > 0; --Index)
		{
			Directions.Swap(Index, Rng.RandRange(0, Index));
		}

		bool bAdvanced = false;
		for (const FIntPoint& Direction : Directions)
		{
			const FIntPoint Next(Current.X + Direction.X, Current.Y + Direction.Y);
			if (Next.X <= 0 || Next.X >= BackroomsMazeWidth - 1 || Next.Y <= 0 || Next.Y >= BackroomsMazeHeight - 1)
			{
				continue;
			}
			if (IsBackroomsMazeOpen(Next.X, Next.Y))
			{
				continue;
			}

			SetOpen(FIntPoint(Current.X + Direction.X / 2, Current.Y + Direction.Y / 2));
			SetOpen(Next);
			Stack.Add(Next);
			bAdvanced = true;
			break;
		}

		if (!bAdvanced)
		{
			Stack.Pop();
		}
	}

	return IsBackroomsMazeOpen(1, 1) && IsBackroomsMazeOpen(BackroomsMazeWidth - 2, BackroomsMazeHeight - 2);
}

bool AT66GameMode::IsBackroomsMazeOpen(const int32 X, const int32 Y) const
{
	return X >= 0
		&& X < BackroomsMazeWidth
		&& Y >= 0
		&& Y < BackroomsMazeHeight
		&& BackroomsMazeOpenCells.IsValidIndex((Y * BackroomsMazeWidth) + X)
		&& BackroomsMazeOpenCells[(Y * BackroomsMazeWidth) + X] != 0;
}

FVector AT66GameMode::GetBackroomsCellCenter(const int32 X, const int32 Y, const float ZOffset) const
{
	const float HalfX = static_cast<float>(BackroomsMazeWidth - 1) * BackroomsCellSize * 0.5f;
	const float HalfY = static_cast<float>(BackroomsMazeHeight - 1) * BackroomsCellSize * 0.5f;
	return FVector(
		BackroomsOrigin.X + (static_cast<float>(X) * BackroomsCellSize) - HalfX,
		BackroomsOrigin.Y + (static_cast<float>(Y) * BackroomsCellSize) - HalfY,
		BackroomsOrigin.Z + ZOffset);
}

FIntPoint AT66GameMode::WorldToBackroomsCell(const FVector& Location) const
{
	const float HalfX = static_cast<float>(BackroomsMazeWidth - 1) * BackroomsCellSize * 0.5f;
	const float HalfY = static_cast<float>(BackroomsMazeHeight - 1) * BackroomsCellSize * 0.5f;
	const float MinCenterX = BackroomsOrigin.X - HalfX;
	const float MinCenterY = BackroomsOrigin.Y - HalfY;
	return FIntPoint(
		FMath::RoundToInt((Location.X - MinCenterX) / FMath::Max(1.f, BackroomsCellSize)),
		FMath::RoundToInt((Location.Y - MinCenterY) / FMath::Max(1.f, BackroomsCellSize)));
}

bool AT66GameMode::HasBackroomsQuickReviveReward() const
{
	const UGameInstance* GI = GetGameInstance();
	const UT66RunStateSubsystem* RunState = GI ? GI->GetSubsystem<UT66RunStateSubsystem>() : nullptr;
	return RunState && RunState->HasBackroomsQuickReviveItem();
}

void AT66GameMode::SpawnBackroomsPocketIfNeeded()
{
	UWorld* World = GetWorld();
	const bool bForceSpawn = CVarT66BackroomsForceSpawn.GetValueOnGameThread() != 0;
	auto LogForcedSkip = [bForceSpawn](const TCHAR* Reason)
	{
		if (bForceSpawn)
		{
			UE_LOG(LogT66GameMode, Warning, TEXT("[Backrooms] Force spawn skipped: %s."), Reason);
		}
	};

	if (!World)
	{
		LogForcedSkip(TEXT("missing world"));
		return;
	}
	if (bBackroomsPocketSpawned)
	{
		LogForcedSkip(TEXT("pocket already spawned"));
		return;
	}
	if (bBackroomsChallengeActive)
	{
		LogForcedSkip(TEXT("challenge already active"));
		return;
	}
	if (IsLabRun())
	{
		LogForcedSkip(TEXT("lab run"));
		return;
	}
	if (!IsUsingTowerMainMapLayout())
	{
		LogForcedSkip(TEXT("not tower layout"));
		return;
	}
	if (HasBackroomsQuickReviveReward())
	{
		LogForcedSkip(TEXT("Quick Revive item already owned"));
		return;
	}

	UT66GameInstance* T66GI = GetT66GameInstance();
	UGameInstance* GI = GetGameInstance();
	UT66RunStateSubsystem* RunState = GI ? GI->GetSubsystem<UT66RunStateSubsystem>() : nullptr;
	const int32 RunSeed = T66EnsureRunSeed(T66GI);
	const int32 StageNum = RunState ? RunState->GetCurrentStage() : 1;
	FRandomStream Rng(RunSeed ^ (StageNum * 31177) ^ 0xB40666);

	const float SpawnChance = FMath::Clamp(CVarT66BackroomsSpawnChance.GetValueOnGameThread(), 0.f, 1.f);
	if (!bForceSpawn && Rng.GetFraction() > SpawnChance)
	{
		return;
	}

	if (!TryBuildBackroomsMaze())
	{
		return;
	}

	TArray<const T66TowerMapTerrain::FFloor*> MobFloors;
	for (const T66TowerMapTerrain::FFloor& Floor : CachedTowerMainMapLayout.Floors)
	{
		if (Floor.bMobFloor || Floor.FloorRole == T66TowerMapTerrain::ET66TowerFloorRole::Mob)
		{
			MobFloors.Add(&Floor);
		}
	}
	if (MobFloors.IsEmpty())
	{
		LogForcedSkip(TEXT("no mob floors available"));
		return;
	}

	const T66TowerMapTerrain::FFloor* DoorFloor = MobFloors[Rng.RandRange(0, MobFloors.Num() - 1)];
	if (!DoorFloor)
	{
		LogForcedSkip(TEXT("selected mob floor is invalid"));
		return;
	}

	FVector EntryLocation = FVector::ZeroVector;
	FVector WallNormal = FVector::ForwardVector;
	if (!T66TryGetBackroomsDoorWallLocation(
		CachedTowerMainMapLayout,
		*DoorFloor,
		Rng,
		EntryLocation,
		WallNormal))
	{
		LogForcedSkip(TEXT("tower wall placement failed"));
		return;
	}

	FActorSpawnParameters SpawnParams;
	SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
	UStaticMesh* CubeMesh = GetCubeMesh();
	UMaterialInstanceDynamic* FloorMaterial = T66CreateBackroomsTextureMaterial(this, T66BackroomsFloorTexturePath, FLinearColor(0.88f, 0.72f, 0.24f, 1.f));
	UMaterialInstanceDynamic* WallMaterial = T66CreateBackroomsTextureMaterial(this, T66BackroomsWallTexturePath, FLinearColor(0.82f, 0.68f, 0.20f, 1.f));

	const FVector FloorSize(
		static_cast<float>(BackroomsMazeWidth) * BackroomsCellSize,
		static_cast<float>(BackroomsMazeHeight) * BackroomsCellSize,
		T66BackroomsFloorThickness);
	if (AStaticMeshActor* Floor = T66SpawnBackroomsBox(
		World,
		CubeMesh,
		FloorMaterial,
		BackroomsOrigin - FVector(0.f, 0.f, T66BackroomsFloorThickness * 0.5f),
		FloorSize,
		TEXT("T66_Backrooms_Floor"),
		SpawnParams))
	{
		BackroomsActors.Add(Floor);
	}

	for (int32 Y = 0; Y < BackroomsMazeHeight; ++Y)
	{
		for (int32 X = 0; X < BackroomsMazeWidth; ++X)
		{
			if (IsBackroomsMazeOpen(X, Y))
			{
				continue;
			}

			if (AStaticMeshActor* Wall = T66SpawnBackroomsBox(
				World,
				CubeMesh,
				WallMaterial,
				GetBackroomsCellCenter(X, Y, T66BackroomsWallHeight * 0.5f),
				FVector(BackroomsCellSize, BackroomsCellSize, T66BackroomsWallHeight),
				TEXT("T66_Backrooms_Wall"),
				SpawnParams))
			{
				BackroomsActors.Add(Wall);
			}
		}
	}

	const FRotator EntryRotation(0.f, WallNormal.Rotation().Yaw + 90.f, 0.f);
	BackroomsEntryDoor = World->SpawnActor<AT66BackroomsDoorInteractable>(
		AT66BackroomsDoorInteractable::StaticClass(),
		EntryLocation,
		EntryRotation,
		SpawnParams);
	if (!BackroomsEntryDoor)
	{
		DestroyBackroomsPocket();
		return;
	}
	BackroomsEntryDoor->InitializeBackroomsDoor(false);
	BackroomsEntryDoor->Tags.AddUnique(T66BackroomsActorTag);
	BackroomsActors.Add(BackroomsEntryDoor);

	BackroomsClosedEntranceDoor = World->SpawnActor<AT66BackroomsDoorInteractable>(
		AT66BackroomsDoorInteractable::StaticClass(),
		GetBackroomsCellCenter(1, 1, 5.f),
		FRotator(0.f, 180.f, 0.f),
		SpawnParams);
	if (BackroomsClosedEntranceDoor)
	{
		BackroomsClosedEntranceDoor->InitializeBackroomsDoor(false, true);
		BackroomsClosedEntranceDoor->Tags.AddUnique(T66BackroomsActorTag);
		BackroomsActors.Add(BackroomsClosedEntranceDoor);
	}

	BackroomsExitDoor = World->SpawnActor<AT66BackroomsDoorInteractable>(
		AT66BackroomsDoorInteractable::StaticClass(),
		GetBackroomsCellCenter(BackroomsMazeWidth - 2, BackroomsMazeHeight - 2, 5.f),
		FRotator::ZeroRotator,
		SpawnParams);
	if (BackroomsExitDoor)
	{
		BackroomsExitDoor->InitializeBackroomsDoor(true);
		BackroomsExitDoor->Tags.AddUnique(T66BackroomsActorTag);
		BackroomsActors.Add(BackroomsExitDoor);
	}

	bBackroomsPocketSpawned = true;
	UE_LOG(LogT66GameMode, Log, TEXT("[Backrooms] Spawned pocket maze and entry door at %s."), *EntryLocation.ToCompactString());
#if !UE_BUILD_SHIPPING
	ScheduleBackroomsAutomationIfRequested();
#endif
}

#if !UE_BUILD_SHIPPING
void AT66GameMode::ScheduleBackroomsAutomationIfRequested()
{
	UWorld* World = GetWorld();
	if (!World)
	{
		return;
	}

	FString RawMode;
	if (!FParse::Value(FCommandLine::Get(), TEXT("T66BackroomsAutoQA="), RawMode))
	{
		return;
	}

	const FString Mode = T66NormalizeBackroomsQAMode(RawMode);
	if (!T66IsValidBackroomsQAMode(Mode))
	{
		UE_LOG(LogT66GameMode, Error, TEXT("[BackroomsQA] Phase=InvalidMode RawMode=%s"), *RawMode);
		T66RequestBackroomsQAExitIfNeeded(false, TEXT("T66BackroomsQAInvalidMode"));
		return;
	}

	float StartDelaySeconds = 1.0f;
	FParse::Value(FCommandLine::Get(), TEXT("T66BackroomsAutoQAStartDelay="), StartDelaySeconds);
	StartDelaySeconds = FMath::Clamp(StartDelaySeconds, 0.1f, 20.0f);

	FTimerHandle TimerHandle;
	World->GetTimerManager().SetTimer(
		TimerHandle,
		FTimerDelegate::CreateWeakLambda(this, [this, Mode]()
		{
			RunBackroomsAutomationStart(Mode);
		}),
		StartDelaySeconds,
		false);

	UE_LOG(LogT66GameMode, Log, TEXT("[BackroomsQA] Phase=Scheduled Mode=%s StartDelay=%.2f"), *Mode, StartDelaySeconds);
}

void AT66GameMode::RunBackroomsAutomationStart(const FString Mode)
{
	UWorld* World = GetWorld();
	APlayerController* PC = UGameplayStatics::GetPlayerController(this, 0);
	AT66HeroBase* Hero = PC ? Cast<AT66HeroBase>(PC->GetPawn()) : Cast<AT66HeroBase>(UGameplayStatics::GetPlayerPawn(this, 0));
	UGameInstance* GI = GetGameInstance();
	UT66RunStateSubsystem* RunState = GI ? GI->GetSubsystem<UT66RunStateSubsystem>() : nullptr;
	UT66WeaponManagerSubsystem* WeaponManager = GI ? GI->GetSubsystem<UT66WeaponManagerSubsystem>() : nullptr;
	UT66GameInstance* T66GI = Cast<UT66GameInstance>(GI);

	if (!World || !Hero || !RunState || !BackroomsEntryDoor)
	{
		UE_LOG(
			LogT66GameMode,
			Error,
			TEXT("[BackroomsQA] Phase=StartFailed Mode=%s World=%d Hero=%d RunState=%d EntryDoor=%d"),
			*Mode,
			World ? 1 : 0,
			Hero ? 1 : 0,
			RunState ? 1 : 0,
			BackroomsEntryDoor ? 1 : 0);
		T66RequestBackroomsQAExitIfNeeded(false, TEXT("T66BackroomsQAStartFailed"));
		return;
	}

	if (!T66BackroomsInventoryHasItem(RunState, T66BackroomsQASeedItemID))
	{
		RunState->AddItemWithRarity(T66BackroomsQASeedItemID, ET66ItemRarity::Yellow);
	}

	FName WeaponIDBeforeEntry = WeaponManager ? WeaponManager->GetEquippedWeaponID() : NAME_None;
	if (WeaponManager && WeaponIDBeforeEntry.IsNone())
	{
		const FName HeroID = (T66GI && !T66GI->SelectedHeroID.IsNone()) ? T66GI->SelectedHeroID : FName(TEXT("Hero_1"));
		const FName SeedWeaponID = UT66WeaponManagerSubsystem::MakeWeaponID(HeroID, ET66WeaponRarity::Black, ET66AttackCategory::AOE);
		FWeaponData SeedWeaponData;
		if (T66GI && T66GI->GetWeaponData(SeedWeaponID, SeedWeaponData))
		{
			WeaponManager->RestoreState(SeedWeaponID);
			WeaponIDBeforeEntry = SeedWeaponID;
		}
	}

	const int32 InventoryCountBeforeEntry = RunState->GetInventorySlots().Num();
	RunState->SetStageTimerActive(true);
	RunState->StartSpeedRunTimer(true);

	const bool bEntryInteractReturned = BackroomsEntryDoor->Interact(PC);
	const bool bEntered = bBackroomsChallengeActive;
	const int32 InventoryCountDuringBackrooms = RunState->GetInventorySlots().Num();
	const FName WeaponIDDuringBackrooms = WeaponManager ? WeaponManager->GetEquippedWeaponID() : NAME_None;
	float ChaserDistanceAtEntry = -1.0f;
	bool bMoveTargetResolved = false;
	FVector MoveTarget = FVector::ZeroVector;
	if (BackroomsChaser && Hero)
	{
		ChaserDistanceAtEntry = FVector::Dist2D(BackroomsChaser->GetActorLocation(), Hero->GetActorLocation());
		bMoveTargetResolved = GetBackroomsChaserMoveTarget(BackroomsChaser->GetActorLocation(), Hero->GetActorLocation(), MoveTarget);
	}

	const bool bStageTimerPaused = !RunState->GetStageTimerActive();
	const bool bSpeedRunPaused = !RunState->IsSpeedRunTimerActive();
	const bool bPauseFlag = RunState->IsBackroomsGameplayPaused();
	const bool bInventoryHidden = InventoryCountDuringBackrooms == 0;
	const bool bWeaponHidden = WeaponIDDuringBackrooms.IsNone();

	UE_LOG(
		LogT66GameMode,
		Log,
		TEXT("[BackroomsQA] Phase=Entered Mode=%s InteractReturned=%d Entered=%d InventoryBefore=%d InventoryDuring=%d InventoryHidden=%d WeaponBefore=%s WeaponDuring=%s WeaponHidden=%d StageTimerPaused=%d SpeedRunPaused=%d PauseFlag=%d ChaserSpawned=%d ChaserDistance=%.1f MoveTargetResolved=%d MoveTarget=%s"),
		*Mode,
		bEntryInteractReturned ? 1 : 0,
		bEntered ? 1 : 0,
		InventoryCountBeforeEntry,
		InventoryCountDuringBackrooms,
		bInventoryHidden ? 1 : 0,
		*WeaponIDBeforeEntry.ToString(),
		*WeaponIDDuringBackrooms.ToString(),
		bWeaponHidden ? 1 : 0,
		bStageTimerPaused ? 1 : 0,
		bSpeedRunPaused ? 1 : 0,
		bPauseFlag ? 1 : 0,
		BackroomsChaser ? 1 : 0,
		ChaserDistanceAtEntry,
		bMoveTargetResolved ? 1 : 0,
		*MoveTarget.ToCompactString());

	if (!bEntered || !bInventoryHidden || !bWeaponHidden || !bStageTimerPaused || !bSpeedRunPaused || !bPauseFlag || !BackroomsChaser)
	{
		T66RequestBackroomsQAExitIfNeeded(false, TEXT("T66BackroomsQAEntryFailed"));
		return;
	}

	FString ScreenshotPath;
	if (FParse::Value(FCommandLine::Get(), TEXT("T66BackroomsAutoQAScreenshot="), ScreenshotPath))
	{
		const FString FullScreenshotPath = FPaths::ConvertRelativePathToFull(ScreenshotPath);
		IFileManager::Get().MakeDirectory(*FPaths::GetPath(FullScreenshotPath), true);
		T66ConfigureBackroomsQACamera(Hero, PC);
		FScreenshotRequest::RequestScreenshot(FullScreenshotPath, true, false, false);
		UE_LOG(
			LogT66GameMode,
			Log,
			TEXT("[BackroomsQA] Phase=Screenshot Mode=%s Active=%d Path=%s"),
			*Mode,
			bBackroomsChallengeActive ? 1 : 0,
			*FullScreenshotPath);
	}

	float FinishDelaySeconds = 5.0f;
	FParse::Value(FCommandLine::Get(), TEXT("T66BackroomsAutoQAFinishDelay="), FinishDelaySeconds);
	FinishDelaySeconds = FMath::Clamp(FinishDelaySeconds, 0.2f, 30.0f);

	FTimerHandle TimerHandle;
	World->GetTimerManager().SetTimer(
		TimerHandle,
		FTimerDelegate::CreateWeakLambda(this, [this, Mode, InventoryCountBeforeEntry, WeaponIDBeforeEntry, ChaserDistanceAtEntry]()
		{
			RunBackroomsAutomationFinish(Mode, InventoryCountBeforeEntry, T66BackroomsQASeedItemID, WeaponIDBeforeEntry, ChaserDistanceAtEntry);
		}),
		FinishDelaySeconds,
		false);
}

void AT66GameMode::RunBackroomsAutomationFinish(
	const FString Mode,
	const int32 InventoryCountBeforeEntry,
	const FName SeedItemID,
	const FName WeaponIDBeforeEntry,
	const float ChaserDistanceAtEntry)
{
	APlayerController* PC = UGameplayStatics::GetPlayerController(this, 0);
	AT66HeroBase* Hero = PC ? Cast<AT66HeroBase>(PC->GetPawn()) : Cast<AT66HeroBase>(UGameplayStatics::GetPlayerPawn(this, 0));
	UGameInstance* GI = GetGameInstance();
	UT66RunStateSubsystem* RunState = GI ? GI->GetSubsystem<UT66RunStateSubsystem>() : nullptr;
	UT66WeaponManagerSubsystem* WeaponManager = GI ? GI->GetSubsystem<UT66WeaponManagerSubsystem>() : nullptr;

	if (!Hero || !RunState)
	{
		UE_LOG(LogT66GameMode, Error, TEXT("[BackroomsQA] Phase=FinishFailed Mode=%s Hero=%d RunState=%d"), *Mode, Hero ? 1 : 0, RunState ? 1 : 0);
		T66RequestBackroomsQAExitIfNeeded(false, TEXT("T66BackroomsQAFinishFailed"));
		return;
	}

	if (Mode == TEXT("death"))
	{
		const float ChaserDistanceBeforeTouch = BackroomsChaser ? FVector::Dist2D(BackroomsChaser->GetActorLocation(), Hero->GetActorLocation()) : -1.0f;
		const bool bChaserMovedCloser = ChaserDistanceAtEntry >= 0.f && ChaserDistanceBeforeTouch >= 0.f && ChaserDistanceBeforeTouch < (ChaserDistanceAtEntry - 10.0f);
		AT66BackroomsChaser* Chaser = BackroomsChaser.Get();
		HandleBackroomsChaserTouchedHero(Chaser, Hero);

		const bool bDead = RunState->GetCurrentHP() <= 0.f;
		const bool bChallengeEnded = !bBackroomsChallengeActive;
		const bool bInventoryRestored = T66BackroomsInventoryHasItem(RunState, SeedItemID) && RunState->GetInventorySlots().Num() >= InventoryCountBeforeEntry;
		const bool bWeaponRestored = !WeaponManager || WeaponManager->GetEquippedWeaponID() == WeaponIDBeforeEntry;
		const bool bPass = bDead && bChallengeEnded && bInventoryRestored && bWeaponRestored && bChaserMovedCloser;

		T66_LOG_BACKROOMS_QA_RESULT(
			bPass,
			TEXT("[BackroomsQA] Phase=Death Mode=%s Dead=%d ChallengeEnded=%d InventoryRestored=%d WeaponRestored=%d ChaserDistanceAtEntry=%.1f ChaserDistanceBeforeTouch=%.1f ChaserMovedCloser=%d HeroHP=%.1f"),
			*Mode,
			bDead ? 1 : 0,
			bChallengeEnded ? 1 : 0,
			bInventoryRestored ? 1 : 0,
			bWeaponRestored ? 1 : 0,
			ChaserDistanceAtEntry,
			ChaserDistanceBeforeTouch,
			bChaserMovedCloser ? 1 : 0,
			RunState->GetCurrentHP());
		T66RequestBackroomsQAExitIfNeeded(bPass, bPass ? TEXT("T66BackroomsQADeathPass") : TEXT("T66BackroomsQADeathFail"));
		return;
	}

	if (!BackroomsExitDoor)
	{
		UE_LOG(LogT66GameMode, Error, TEXT("[BackroomsQA] Phase=ExitFailed Mode=%s MissingExitDoor=1"), *Mode);
		T66RequestBackroomsQAExitIfNeeded(false, TEXT("T66BackroomsQAExitMissingDoor"));
		return;
	}

	Hero->SetActorLocation(BackroomsExitDoor->GetActorLocation() + FVector(0.f, 0.f, 96.f), false, nullptr, ETeleportType::TeleportPhysics);
	const bool bExitInteractReturned = BackroomsExitDoor->Interact(PC);
	const bool bChallengeEnded = !bBackroomsChallengeActive;
	const bool bRewardGranted = RunState->HasBackroomsQuickReviveItem();
	const bool bInventoryRestored = T66BackroomsInventoryHasItem(RunState, SeedItemID) && RunState->GetInventorySlots().Num() >= InventoryCountBeforeEntry;
	const bool bWeaponRestored = !WeaponManager || WeaponManager->GetEquippedWeaponID() == WeaponIDBeforeEntry;
	const bool bStageTimerRestored = RunState->GetStageTimerActive();
	const bool bSpeedRunRestored = RunState->IsSpeedRunTimerActive();
	const bool bExitPass = bExitInteractReturned && bChallengeEnded && bRewardGranted && bInventoryRestored && bWeaponRestored && bStageTimerRestored && bSpeedRunRestored;

	T66_LOG_BACKROOMS_QA_RESULT(
		bExitPass,
		TEXT("[BackroomsQA] Phase=Exit Mode=%s InteractReturned=%d ChallengeEnded=%d RewardGranted=%d InventoryRestored=%d WeaponRestored=%d StageTimerRestored=%d SpeedRunRestored=%d InventoryCount=%d"),
		*Mode,
		bExitInteractReturned ? 1 : 0,
		bChallengeEnded ? 1 : 0,
		bRewardGranted ? 1 : 0,
		bInventoryRestored ? 1 : 0,
		bWeaponRestored ? 1 : 0,
		bStageTimerRestored ? 1 : 0,
		bSpeedRunRestored ? 1 : 0,
		RunState->GetInventorySlots().Num());

	if (Mode != TEXT("consume"))
	{
		T66RequestBackroomsQAExitIfNeeded(bExitPass, bExitPass ? TEXT("T66BackroomsQAExitPass") : TEXT("T66BackroomsQAExitFail"));
		return;
	}

	const float HPBeforeConsume = RunState->GetCurrentHP();
	const bool bDamageApplied = RunState->ApplyDamage(999999, this, FName(TEXT("BackroomsQAConsume")), this);
	const bool bConsumed = !RunState->HasBackroomsQuickReviveItem();
	const bool bAlive = RunState->GetCurrentHP() > 0.f;
	const bool bConsumePass = bExitPass && bDamageApplied && bConsumed && bAlive;

	T66_LOG_BACKROOMS_QA_RESULT(
		bConsumePass,
		TEXT("[BackroomsQA] Phase=Consume Mode=%s DamageApplied=%d Consumed=%d Alive=%d HPBefore=%.1f HPAfter=%.1f"),
		*Mode,
		bDamageApplied ? 1 : 0,
		bConsumed ? 1 : 0,
		bAlive ? 1 : 0,
		HPBeforeConsume,
		RunState->GetCurrentHP());
	T66RequestBackroomsQAExitIfNeeded(bConsumePass, bConsumePass ? TEXT("T66BackroomsQAConsumePass") : TEXT("T66BackroomsQAConsumeFail"));
}
#undef T66_LOG_BACKROOMS_QA_RESULT
#endif

void AT66GameMode::DestroyBackroomsPocket()
{
	if (bBackroomsChallengeActive)
	{
		ApplyBackroomsPauseState(false);
		RestoreBackroomsInventoryAndWeapon();
	}

	for (AActor* Actor : BackroomsActors)
	{
		if (Actor)
		{
			Actor->Destroy();
		}
	}
	BackroomsActors.Reset();
	BackroomsEntryDoor = nullptr;
	BackroomsExitDoor = nullptr;
	BackroomsClosedEntranceDoor = nullptr;
	BackroomsChaser = nullptr;
	bBackroomsPocketSpawned = false;
	bBackroomsChallengeActive = false;
	BackroomsMazeOpenCells.Reset();
	BackroomsTickSnapshot.Reset();
	BackroomsComponentTickSnapshot.Reset();
}

void AT66GameMode::HandleBackroomsDoorInteracted(AT66BackroomsDoorInteractable* Door, AT66HeroBase* Hero)
{
	if (!Door || !Hero)
	{
		return;
	}

	if (Door->IsExitDoor())
	{
		if (bBackroomsChallengeActive)
		{
			CompleteBackrooms(true, Hero, BackroomsChaser.Get());
		}
		return;
	}

	EnterBackrooms(Hero, Door);
}

void AT66GameMode::EnterBackrooms(AT66HeroBase* Hero, AT66BackroomsDoorInteractable* EntryDoor)
{
	if (!Hero || bBackroomsChallengeActive || !bBackroomsPocketSpawned || HasBackroomsQuickReviveReward())
	{
		return;
	}

	UWorld* World = GetWorld();
	UGameInstance* GI = GetGameInstance();
	UT66RunStateSubsystem* RunState = GI ? GI->GetSubsystem<UT66RunStateSubsystem>() : nullptr;
	if (!World || !RunState)
	{
		return;
	}

	BackroomsReturnTransform = Hero->GetActorTransform();
	bBackroomsChallengeActive = true;
	bBackroomsStageTimerWasActive = RunState->GetStageTimerActive();
	bBackroomsSpeedRunWasActive = RunState->IsSpeedRunTimerActive();

	ApplyBackroomsPauseState(true);
	if (bBackroomsStageTimerWasActive)
	{
		RunState->SetStageTimerActive(false);
	}
	if (bBackroomsSpeedRunWasActive)
	{
		RunState->StopSpeedRunTimer(true);
	}

	if (UT66WeaponManagerSubsystem* WeaponManager = GI->GetSubsystem<UT66WeaponManagerSubsystem>())
	{
		BackroomsSavedWeaponID = WeaponManager->GetEquippedWeaponID();
		WeaponManager->RestoreState(NAME_None);
	}
	RunState->SnapshotAndClearInventoryForBackrooms(BackroomsSavedInventory);
	bBackroomsInventorySuppressed = true;

	if (EntryDoor)
	{
		EntryDoor->SetDoorConsumed();
	}

	const float HeroHalfHeight = Hero->GetCapsuleComponent() ? Hero->GetCapsuleComponent()->GetScaledCapsuleHalfHeight() : 96.f;
	const FVector EntryCellLocation = GetBackroomsCellCenter(1, 1, HeroHalfHeight + 12.f);
	Hero->SetActorLocation(EntryCellLocation, false, nullptr, ETeleportType::TeleportPhysics);
	T66SyncPawnAndControllerRotation(Hero, Hero->GetController(), FRotator::ZeroRotator);

	FVector ChaserSpawnLocation = GetBackroomsCellCenter(BackroomsMazeWidth - 2, 1, HeroHalfHeight + 12.f);
	if (!IsBackroomsMazeOpen(BackroomsMazeWidth - 2, 1))
	{
		ChaserSpawnLocation = GetBackroomsCellCenter(BackroomsMazeWidth - 2, BackroomsMazeHeight - 2, HeroHalfHeight + 12.f);
	}

	FActorSpawnParameters SpawnParams;
	SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
	BackroomsChaser = World->SpawnActor<AT66BackroomsChaser>(
		AT66BackroomsChaser::StaticClass(),
		ChaserSpawnLocation,
		FRotator::ZeroRotator,
		SpawnParams);
	if (BackroomsChaser)
	{
		BackroomsChaser->Tags.AddUnique(T66BackroomsActorTag);
		BackroomsActors.Add(BackroomsChaser);
	}

	UE_LOG(LogT66GameMode, Log, TEXT("[Backrooms] Entered challenge. Inventory and weapon suppressed; timers paused."));
}

void AT66GameMode::HandleBackroomsChaserTouchedHero(AT66BackroomsChaser* Chaser, AT66HeroBase* Hero)
{
	if (!Chaser || !Hero || !bBackroomsChallengeActive)
	{
		return;
	}

	const int32 DamageHP = Chaser->GetTouchDamageHP();
	CompleteBackrooms(false, Hero, Chaser);
	if (UGameInstance* GI = GetGameInstance())
	{
		if (UT66RunStateSubsystem* RunState = GI->GetSubsystem<UT66RunStateSubsystem>())
		{
			RunState->ApplyDamage(DamageHP, Chaser, FName(TEXT("BackroomsChaserTouch")), Chaser);
		}
	}
}

void AT66GameMode::CompleteBackrooms(const bool bSucceeded, AT66HeroBase* Hero, AT66BackroomsChaser* Chaser)
{
	if (!bBackroomsChallengeActive)
	{
		return;
	}

	UGameInstance* GI = GetGameInstance();
	UT66RunStateSubsystem* RunState = GI ? GI->GetSubsystem<UT66RunStateSubsystem>() : nullptr;

	RestoreBackroomsInventoryAndWeapon();
	if (Hero)
	{
		Hero->SetActorTransform(BackroomsReturnTransform, false, nullptr, ETeleportType::TeleportPhysics);
		T66SyncPawnAndControllerRotation(Hero, Hero->GetController(), BackroomsReturnTransform.GetRotation().Rotator());
	}

	ApplyBackroomsPauseState(false);
	if (RunState)
	{
		if (bBackroomsStageTimerWasActive)
		{
			RunState->SetStageTimerActive(true);
		}
		if (bBackroomsSpeedRunWasActive)
		{
			RunState->StartSpeedRunTimer(false);
		}

		if (bSucceeded && !RunState->HasBackroomsQuickReviveItem())
		{
			RunState->AddItemSlot(FT66InventorySlot(UT66RunStateSubsystem::BackroomsQuickReviveItemID, ET66ItemRarity::Black, 1));
		}
	}

	bBackroomsChallengeActive = false;
	bBackroomsStageTimerWasActive = false;
	bBackroomsSpeedRunWasActive = false;

	if (Chaser)
	{
		Chaser->Destroy();
	}
	BackroomsChaser = nullptr;
	if (BackroomsEntryDoor)
	{
		BackroomsEntryDoor->SetDoorConsumed();
	}

	if (bSucceeded)
	{
		DestroyBackroomsPocket();
	}
}

void AT66GameMode::RestoreBackroomsInventoryAndWeapon()
{
	if (!bBackroomsInventorySuppressed)
	{
		return;
	}

	if (UGameInstance* GI = GetGameInstance())
	{
		if (UT66RunStateSubsystem* RunState = GI->GetSubsystem<UT66RunStateSubsystem>())
		{
			RunState->RestoreInventoryFromBackroomsSnapshot(BackroomsSavedInventory);
		}
		if (UT66WeaponManagerSubsystem* WeaponManager = GI->GetSubsystem<UT66WeaponManagerSubsystem>())
		{
			WeaponManager->RestoreState(BackroomsSavedWeaponID);
		}
	}

	BackroomsSavedInventory.Reset();
	BackroomsSavedWeaponID = NAME_None;
	bBackroomsInventorySuppressed = false;
}

void AT66GameMode::ApplyBackroomsPauseState(const bool bPaused)
{
	UWorld* World = GetWorld();
	UGameInstance* GI = GetGameInstance();

	if (UT66RunStateSubsystem* RunState = GI ? GI->GetSubsystem<UT66RunStateSubsystem>() : nullptr)
	{
		RunState->SetBackroomsGameplayPaused(bPaused);
	}
	if (World)
	{
		if (UT66MobManagerSubsystem* MobManager = World->GetSubsystem<UT66MobManagerSubsystem>())
		{
			MobManager->SetBackroomsGameplayPaused(bPaused);
		}
	}

	SetEnemyDirectorSpawningPaused(bPaused);

	if (!World)
	{
		return;
	}

	if (bPaused)
	{
		BackroomsTickSnapshot.Reset();
		BackroomsComponentTickSnapshot.Reset();
		for (TActorIterator<AActor> It(World); It; ++It)
		{
			AActor* Actor = *It;
			if (!Actor || Actor->IsA(AT66HeroBase::StaticClass()) || BackroomsActors.Contains(Actor) || !T66ShouldPauseForBackrooms(Actor))
			{
				continue;
			}

			BackroomsTickSnapshot.Add(Actor, Actor->IsActorTickEnabled());
			Actor->SetActorTickEnabled(false);

			TArray<UActorComponent*> Components;
			Actor->GetComponents(Components);
			for (UActorComponent* Component : Components)
			{
				if (!Component || !Component->PrimaryComponentTick.bCanEverTick)
				{
					continue;
				}
				BackroomsComponentTickSnapshot.Add(Component, Component->IsComponentTickEnabled());
				Component->SetComponentTickEnabled(false);
			}
		}
		return;
	}

	for (const TPair<TWeakObjectPtr<AActor>, bool>& Pair : BackroomsTickSnapshot)
	{
		if (AActor* Actor = Pair.Key.Get())
		{
			Actor->SetActorTickEnabled(Pair.Value);
		}
	}
	for (const TPair<TWeakObjectPtr<UActorComponent>, bool>& Pair : BackroomsComponentTickSnapshot)
	{
		if (UActorComponent* Component = Pair.Key.Get())
		{
			Component->SetComponentTickEnabled(Pair.Value);
		}
	}
	BackroomsTickSnapshot.Reset();
	BackroomsComponentTickSnapshot.Reset();
}

bool AT66GameMode::GetBackroomsChaserMoveTarget(const FVector& ChaserLocation, const FVector& HeroLocation, FVector& OutTarget) const
{
	if (BackroomsMazeWidth <= 0 || BackroomsMazeHeight <= 0 || BackroomsMazeOpenCells.Num() != BackroomsMazeWidth * BackroomsMazeHeight)
	{
		return false;
	}

	auto FindNearestOpenCell = [this](const FIntPoint& RequestedCell) -> FIntPoint
	{
		if (IsBackroomsMazeOpen(RequestedCell.X, RequestedCell.Y))
		{
			return RequestedCell;
		}

		FIntPoint BestCell(INDEX_NONE, INDEX_NONE);
		int32 BestDistSq = MAX_int32;
		for (int32 Y = 0; Y < BackroomsMazeHeight; ++Y)
		{
			for (int32 X = 0; X < BackroomsMazeWidth; ++X)
			{
				if (!IsBackroomsMazeOpen(X, Y))
				{
					continue;
				}

				const int32 DistSq = FMath::Square(X - RequestedCell.X) + FMath::Square(Y - RequestedCell.Y);
				if (DistSq < BestDistSq)
				{
					BestDistSq = DistSq;
					BestCell = FIntPoint(X, Y);
				}
			}
		}
		return BestCell;
	};

	const FIntPoint Start = FindNearestOpenCell(WorldToBackroomsCell(ChaserLocation));
	const FIntPoint Target = FindNearestOpenCell(WorldToBackroomsCell(HeroLocation));
	if (Start.X == INDEX_NONE || Target.X == INDEX_NONE)
	{
		return false;
	}
	if (Start == Target)
	{
		OutTarget = HeroLocation;
		return true;
	}

	const int32 CellCount = BackroomsMazeWidth * BackroomsMazeHeight;
	TArray<int32> Previous;
	Previous.Init(INDEX_NONE, CellCount);
	TArray<int32> Queue;
	Queue.Reserve(CellCount);
	const int32 StartIndex = (Start.Y * BackroomsMazeWidth) + Start.X;
	const int32 TargetIndex = (Target.Y * BackroomsMazeWidth) + Target.X;
	Queue.Add(StartIndex);
	Previous[StartIndex] = StartIndex;

	for (int32 ReadIndex = 0; ReadIndex < Queue.Num(); ++ReadIndex)
	{
		const int32 CurrentIndex = Queue[ReadIndex];
		if (CurrentIndex == TargetIndex)
		{
			break;
		}

		const int32 X = CurrentIndex % BackroomsMazeWidth;
		const int32 Y = CurrentIndex / BackroomsMazeWidth;
		const FIntPoint Neighbors[] =
		{
			FIntPoint(X + 1, Y),
			FIntPoint(X - 1, Y),
			FIntPoint(X, Y + 1),
			FIntPoint(X, Y - 1)
		};
		for (const FIntPoint& Neighbor : Neighbors)
		{
			if (!IsBackroomsMazeOpen(Neighbor.X, Neighbor.Y))
			{
				continue;
			}
			const int32 NeighborIndex = (Neighbor.Y * BackroomsMazeWidth) + Neighbor.X;
			if (Previous[NeighborIndex] != INDEX_NONE)
			{
				continue;
			}
			Previous[NeighborIndex] = CurrentIndex;
			Queue.Add(NeighborIndex);
		}
	}

	if (Previous[TargetIndex] == INDEX_NONE)
	{
		OutTarget = HeroLocation;
		return true;
	}

	int32 StepIndex = TargetIndex;
	while (Previous[StepIndex] != StartIndex && Previous[StepIndex] != StepIndex && Previous[StepIndex] != INDEX_NONE)
	{
		StepIndex = Previous[StepIndex];
	}

	const int32 StepX = StepIndex % BackroomsMazeWidth;
	const int32 StepY = StepIndex / BackroomsMazeWidth;
	OutTarget = GetBackroomsCellCenter(StepX, StepY, 96.f);
	return true;
}
