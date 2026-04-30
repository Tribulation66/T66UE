// Copyright Tribulation 66. All Rights Reserved.

#include "Gameplay/GameMode/T66GameModePrivate.h"

using namespace T66GameModePrivate;

namespace
{
	static const FName T66TowerMapTerrainVisualTag(TEXT("T66_MainMapTerrain_Visual"));
	static const TCHAR* T66TowerTerrainFloorTagPrefix = TEXT("T66_Floor_Tower_");

	static int32 T66ReadTerrainFloorTag(const AActor* Actor)
	{
		if (!Actor)
		{
			return INDEX_NONE;
		}

		for (const FName& Tag : Actor->Tags)
		{
			const FString TagString = Tag.ToString();
			if (!TagString.StartsWith(T66TowerTerrainFloorTagPrefix))
			{
				continue;
			}

			const FString NumberString = TagString.RightChop(FCString::Strlen(T66TowerTerrainFloorTagPrefix));
			if (NumberString.Len() != 2 || !NumberString.IsNumeric())
			{
				continue;
			}

			return FCString::Atoi(*NumberString);
		}

		return INDEX_NONE;
	}

	static void T66SetTowerTerrainVisualFloor(UWorld* World, const int32 VisibleFloorNumber)
	{
		if (!World || VisibleFloorNumber == INDEX_NONE)
		{
			return;
		}

		for (TActorIterator<AActor> It(World); It; ++It)
		{
			AActor* Actor = *It;
			if (!Actor || !Actor->ActorHasTag(T66TowerMapTerrainVisualTag))
			{
				continue;
			}

			const int32 ActorFloorNumber = T66ReadTerrainFloorTag(Actor);
			if (ActorFloorNumber == INDEX_NONE)
			{
				continue;
			}

			Actor->SetActorHiddenInGame(ActorFloorNumber != VisibleFloorNumber);
		}
	}
}

void AT66GameMode::ResetTowerMiasmaState()
{
	bTowerMiasmaActive = false;
	TowerMiasmaStartWorldSeconds = 0.f;
	TowerMiasmaUpdateAccumulator = 0.f;
	TowerIdolSelectionsAtStageStart = 0;

	if (MiasmaManager)
	{
		MiasmaManager->SetExpansionActive(false);
		MiasmaManager->ClearTowerSourceAnchors();
		T66PauseTowerMiasma(MiasmaManager);
	}
}

void AT66GameMode::UpdateTowerMiasma(float DeltaTime)
{
	if (!IsUsingTowerMainMapLayout() || !bTowerMiasmaActive || !MiasmaManager)
	{
		TowerMiasmaUpdateAccumulator = 0.f;
		return;
	}

	TowerMiasmaUpdateAccumulator += DeltaTime;
	if (TowerMiasmaUpdateAccumulator < 0.20f)
	{
		return;
	}

	TowerMiasmaUpdateAccumulator = 0.f;
	MiasmaManager->UpdateFromRunState();
}

void AT66GameMode::TryStartTowerMiasma(const FVector* SourceAnchor, const int32 SourceFloorNumber)
{
	if (!IsUsingTowerMainMapLayout() || bTowerMiasmaActive)
	{
		return;
	}

	if (UWorld* World = GetWorld())
	{
		TowerMiasmaStartWorldSeconds = World->GetTimeSeconds();
	}
	else
	{
		TowerMiasmaStartWorldSeconds = 0.f;
	}

	bTowerMiasmaActive = true;
	TowerMiasmaUpdateAccumulator = 0.20f;

	if (SourceAnchor && SourceFloorNumber != INDEX_NONE)
	{
		SyncTowerMiasmaSourceAnchor(SourceFloorNumber, *SourceAnchor);
	}

	if (MiasmaManager)
	{
		T66ActivateStageMiasma(MiasmaManager);
		MiasmaManager->SetExpansionActive(true, 0.f);
		MiasmaManager->UpdateFromRunState();
	}
}

void AT66GameMode::SyncTowerMiasmaSourceAnchor(const int32 FloorNumber, const FVector& WorldAnchor) const
{
	if (!IsUsingTowerMainMapLayout() || FloorNumber == INDEX_NONE || !MiasmaManager)
	{
		return;
	}

	MiasmaManager->SetTowerSourceAnchor(FloorNumber, WorldAnchor);
}

float AT66GameMode::GetTowerMiasmaElapsedSeconds() const
{
	if (!bTowerMiasmaActive)
	{
		return 0.f;
	}

	const UWorld* World = GetWorld();
	return World ? FMath::Max(World->GetTimeSeconds() - TowerMiasmaStartWorldSeconds, 0.f) : 0.f;
}

void AT66GameMode::SpawnTowerDescentHolesIfNeeded()
{
	if (!IsUsingTowerMainMapLayout())
	{
		return;
	}

	UWorld* World = GetWorld();
	if (!World || TowerDescentHoles.Num() > 0)
	{
		return;
	}

	FActorSpawnParameters SpawnParams;
	SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

	if (IsBossRushFinaleStage())
	{
		const T66TowerMapTerrain::FFloor* StartFloor = T66FindTowerFloorByNumber(CachedTowerMainMapLayout, CachedTowerMainMapLayout.StartFloorNumber);
		const T66TowerMapTerrain::FFloor* BossFloor = T66FindTowerFloorByNumber(CachedTowerMainMapLayout, CachedTowerMainMapLayout.BossFloorNumber);
		if (!StartFloor || !BossFloor)
		{
			return;
		}

		const FVector HoleCenter = StartFloor->bHasDropHole
			? StartFloor->HoleCenter
			: FVector(StartFloor->Center.X, StartFloor->Center.Y, StartFloor->SurfaceZ);
		const FVector2D HoleHalfExtent = StartFloor->bHasDropHole
			? StartFloor->HoleHalfExtent
			: FVector2D(320.0f, 320.0f);
		const float DropHeight = FMath::Max(StartFloor->SurfaceZ - BossFloor->SurfaceZ, 1200.0f);
		const float VerticalExtent = FMath::Clamp((DropHeight * 0.5f) - 550.0f, 800.0f, 1400.0f);
		const FVector BoxExtent(
			FMath::Max(250.0f, HoleHalfExtent.X * 0.88f),
			FMath::Max(250.0f, HoleHalfExtent.Y * 0.88f),
			VerticalExtent);
		const FVector HoleLocation = HoleCenter + FVector(0.0f, 0.0f, -VerticalExtent + 120.0f);

		if (AT66TowerDescentHole* HoleActor = World->SpawnActor<AT66TowerDescentHole>(
			AT66TowerDescentHole::StaticClass(),
			HoleLocation,
			FRotator::ZeroRotator,
			SpawnParams))
		{
			HoleActor->InitializeHole(StartFloor->FloorNumber, BossFloor->FloorNumber, BoxExtent);
			HoleActor->Tags.AddUnique(FName(TEXT("T66_Tower_DescentHole")));
			TowerDescentHoles.Add(HoleActor);
		}
		return;
	}

	for (const T66TowerMapTerrain::FFloor& Floor : CachedTowerMainMapLayout.Floors)
	{
		if (!Floor.bHasDropHole)
		{
			continue;
		}

		const T66TowerMapTerrain::FFloor* DestinationFloor = nullptr;
		for (const T66TowerMapTerrain::FFloor& CandidateFloor : CachedTowerMainMapLayout.Floors)
		{
			if (CandidateFloor.FloorNumber == (Floor.FloorNumber + 1))
			{
				DestinationFloor = &CandidateFloor;
				break;
			}
		}
		if (!DestinationFloor)
		{
			continue;
		}

		const float DropHeight = FMath::Max(Floor.SurfaceZ - DestinationFloor->SurfaceZ, 1200.0f);
		const float VerticalExtent = FMath::Clamp((DropHeight * 0.5f) - 550.0f, 800.0f, 1400.0f);
		const FVector BoxExtent(
			FMath::Max(250.0f, Floor.HoleHalfExtent.X * 0.88f),
			FMath::Max(250.0f, Floor.HoleHalfExtent.Y * 0.88f),
			VerticalExtent);
		const FVector HoleLocation = Floor.HoleCenter + FVector(0.0f, 0.0f, -VerticalExtent + 120.0f);

		AT66TowerDescentHole* HoleActor = World->SpawnActor<AT66TowerDescentHole>(
			AT66TowerDescentHole::StaticClass(),
			HoleLocation,
			FRotator::ZeroRotator,
			SpawnParams);
		if (!HoleActor)
		{
			continue;
		}

		HoleActor->InitializeHole(Floor.FloorNumber, DestinationFloor->FloorNumber, BoxExtent);
		HoleActor->Tags.AddUnique(FName(TEXT("T66_Tower_DescentHole")));
		TowerDescentHoles.Add(HoleActor);
	}
}

bool AT66GameMode::IsUsingTowerMainMapLayout() const
{
	return bUsingTowerMainMapLayout && CachedTowerMainMapLayout.Floors.Num() > 0;
}

bool AT66GameMode::GetTowerMainMapLayout(T66TowerMapTerrain::FLayout& OutLayout) const
{
	if (!IsUsingTowerMainMapLayout())
	{
		return false;
	}

	OutLayout = CachedTowerMainMapLayout;
	return true;
}

int32 AT66GameMode::GetTowerFloorIndexForLocation(const FVector& Location) const
{
	if (!IsUsingTowerMainMapLayout())
	{
		return INDEX_NONE;
	}

	return T66TowerMapTerrain::FindFloorIndexForLocation(CachedTowerMainMapLayout, Location);
}

int32 AT66GameMode::GetCurrentTowerFloorIndex() const
{
	if (!IsUsingTowerMainMapLayout())
	{
		return INDEX_NONE;
	}

	if (const UWorld* World = GetWorld())
	{
		for (FConstPlayerControllerIterator It = World->GetPlayerControllerIterator(); It; ++It)
		{
			const APawn* Pawn = It->Get() ? It->Get()->GetPawn() : nullptr;
			if (!Pawn)
			{
				continue;
			}

			const int32 FloorNumber = GetTowerFloorIndexForLocation(Pawn->GetActorLocation());
			if (FloorNumber != INDEX_NONE)
			{
				return FloorNumber;
			}
		}
	}

	return CachedTowerMainMapLayout.StartFloorNumber;
}

bool AT66GameMode::TryGetTowerEnemySpawnLocation(const FVector& PlayerLocation, float MinDistance, float MaxDistance, FRandomStream& Rng, FVector& OutLocation) const
{
	FVector WallNormal = FVector::ZeroVector;
	return TryGetTowerEnemySpawnLocation(PlayerLocation, MinDistance, MaxDistance, Rng, OutLocation, WallNormal);
}

bool AT66GameMode::TryGetTowerEnemySpawnLocation(
	const FVector& PlayerLocation,
	float MinDistance,
	float MaxDistance,
	FRandomStream& Rng,
	FVector& OutLocation,
	FVector& OutWallNormal) const
{
	if (!IsUsingTowerMainMapLayout())
	{
		return false;
	}

	return T66TowerMapTerrain::TryGetWallSpawnLocation(
		GetWorld(),
		CachedTowerMainMapLayout,
		PlayerLocation,
		MinDistance,
		MaxDistance,
		Rng,
		OutLocation,
		OutWallNormal);
}

void AT66GameMode::HandleTowerDescentHoleTriggered(APawn* Pawn, const int32 FromFloorNumber, const int32 ToFloorNumber)
{
	if (!IsUsingTowerMainMapLayout() || !Pawn)
	{
		return;
	}

	UE_LOG(
		LogT66GameMode,
		Log,
		TEXT("[MAP] Tower descent entered by %s (%d -> %d)"),
		*Pawn->GetName(),
		FromFloorNumber,
		ToFloorNumber);

	if (ToFloorNumber != INDEX_NONE && ToFloorNumber != CachedTowerMainMapLayout.BossFloorNumber)
	{
		SyncTowerMiasmaSourceAnchor(ToFloorNumber, Pawn->GetActorLocation());
	}

	T66SetTowerTerrainVisualFloor(GetWorld(), ToFloorNumber);

	if (!bTowerMiasmaActive && ToFloorNumber >= CachedTowerMainMapLayout.FirstGameplayFloorNumber)
	{
		const FVector FloorAnchor = Pawn->GetActorLocation();
		TryStartTowerMiasma(&FloorAnchor, ToFloorNumber);
	}

	if (ToFloorNumber == CachedTowerMainMapLayout.BossFloorNumber)
	{
		bTowerBossEntryTriggered = true;
		bTowerBossEntryApplied = false;
		SyncTowerBossEntryState();
	}
}

void AT66GameMode::SyncTowerBossEntryState()
{
	if (!IsUsingTowerMainMapLayout() || !bTowerBossEntryTriggered || bTowerBossEntryApplied)
	{
		return;
	}

	UWorld* World = GetWorld();
	if (!World)
	{
		return;
	}

	bool bHasEnemyDirector = false;
	if (AT66EnemyDirector* ExistingEnemyDirector = FindOrCacheEnemyDirector(World))
	{
		ExistingEnemyDirector->SetSpawningPaused(true);
		bHasEnemyDirector = true;
	}

	bool bHasBoss = false;
	if (UT66ActorRegistrySubsystem* Registry = World->GetSubsystem<UT66ActorRegistrySubsystem>())
	{
		for (const TWeakObjectPtr<AT66BossBase>& WeakBoss : Registry->GetBosses())
		{
			if (AT66BossBase* Boss = WeakBoss.Get())
			{
				bHasBoss = true;
				if (Boss->IsAlive() && !Boss->IsAwakened())
				{
					Boss->ForceAwaken();
				}
			}
		}
	}

	if ((bHasEnemyDirector || IsBossRushFinaleStage()) && bHasBoss)
	{
		bTowerBossEntryApplied = true;
		UE_LOG(LogT66GameMode, Log, TEXT("[MAP] Tower boss-floor entry activated via descent hole."));
	}
}

void AT66GameMode::SyncTowerTrapActivation(const bool bForce)
{
	UWorld* World = GetWorld();
	if (!World)
	{
		return;
	}

	UT66TrapSubsystem* TrapSubsystem = World->GetSubsystem<UT66TrapSubsystem>();
	if (!TrapSubsystem)
	{
		return;
	}

	if (!IsUsingTowerMainMapLayout())
	{
		TowerTrapActivationAccumulator = 0.f;
		if (bForce || ActiveTowerTrapFloorNumber != INDEX_NONE)
		{
			ActiveTowerTrapFloorNumber = INDEX_NONE;
			TrapSubsystem->SetActiveTowerFloor(INDEX_NONE);
		}
		return;
	}

	const int32 CurrentFloorNumber = GetCurrentTowerFloorIndex();
	const bool bFloorChanged = CurrentFloorNumber != ActiveTowerTrapFloorNumber;
	if (!bForce && !bFloorChanged)
	{
		TowerTrapActivationAccumulator += World->GetDeltaSeconds();
		if (TowerTrapActivationAccumulator < 0.10f)
		{
			return;
		}
	}

	TowerTrapActivationAccumulator = 0.f;
	ActiveTowerTrapFloorNumber = CurrentFloorNumber;
	TrapSubsystem->SetActiveTowerFloor(CurrentFloorNumber);
	T66SetTowerTerrainVisualFloor(World, CurrentFloorNumber);
}
