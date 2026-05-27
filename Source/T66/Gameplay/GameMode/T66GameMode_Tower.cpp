// Copyright Tribulation 66. All Rights Reserved.

#include "Gameplay/GameMode/T66GameModePrivate.h"

using namespace T66GameModePrivate;

namespace
{
	static const FName T66TowerMapTerrainVisualTag(TEXT("T66_MainMapTerrain_Visual"));
	static const FName T66TowerMapTerrainCollisionProxyTag(TEXT("T66_MainMapTerrain_CollisionProxy"));
	static const FName T66TowerDescentGuardianTag(TEXT("T66_Tower_DescentGuardian"));
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

		int32 ChangedVisualActors = 0;
		int32 ChangedCollisionProxyActors = 0;
		// Floor-transition visibility/collision pass only; not part of per-frame gameplay.
		for (TActorIterator<AActor> It(World); It; ++It)
		{
			AActor* Actor = *It;
			if (!Actor)
			{
				continue;
			}

			const bool bIsVisualActor = Actor->ActorHasTag(T66TowerMapTerrainVisualTag);
			const bool bIsCollisionProxyActor = Actor->ActorHasTag(T66TowerMapTerrainCollisionProxyTag);
			if (!bIsVisualActor && !bIsCollisionProxyActor)
			{
				continue;
			}

			const int32 ActorFloorNumber = T66ReadTerrainFloorTag(Actor);
			if (ActorFloorNumber == INDEX_NONE)
			{
				continue;
			}

			const bool bActiveFloor = ActorFloorNumber == VisibleFloorNumber;
			if (bIsVisualActor)
			{
				const bool bShouldBeHidden = !bActiveFloor;
				if (Actor->IsHidden() != bShouldBeHidden)
				{
					Actor->SetActorHiddenInGame(bShouldBeHidden);
					++ChangedVisualActors;
				}
			}

			if (bIsCollisionProxyActor)
			{
				if (Actor->GetActorEnableCollision() != bActiveFloor)
				{
					Actor->SetActorEnableCollision(bActiveFloor);
					++ChangedCollisionProxyActors;
				}
			}
		}

		if (ChangedVisualActors > 0 || ChangedCollisionProxyActors > 0)
		{
			UE_LOG(
				LogT66GameMode,
				Log,
				TEXT("[MAP] Tower terrain active floor %d (visual actors changed=%d, collision proxies changed=%d)."),
				VisibleFloorNumber,
				ChangedVisualActors,
				ChangedCollisionProxyActors);
		}
	}

	static FName T66PickTowerGateGuardianMob(UT66GameInstance* T66GI, const int32 StageNum, const int32 FromFloorNumber)
	{
		if (!T66GI)
		{
			return FName(TEXT("RegularEnemy"));
		}

		FStageData StageData;
		if (!T66GI->GetStageData(StageNum, StageData))
		{
			return FName(TEXT("RegularEnemy"));
		}

		const FName MobIDs[] =
		{
			StageData.EnemyA,
			StageData.EnemyB,
			StageData.EnemyC,
			StageData.EnemyD,
			StageData.EnemyE,
			StageData.EnemyF,
			StageData.EnemyG,
			StageData.EnemyH,
			StageData.EnemyI,
			StageData.EnemyJ,
		};

		int32 ValidCount = 0;
		for (const FName MobID : MobIDs)
		{
			if (!MobID.IsNone())
			{
				++ValidCount;
			}
		}
		if (ValidCount <= 0)
		{
			return FName(TEXT("RegularEnemy"));
		}

		const int32 DesiredValidIndex = FMath::Max(0, FromFloorNumber - 2) % ValidCount;
		int32 CurrentValidIndex = 0;
		for (const FName MobID : MobIDs)
		{
			if (MobID.IsNone())
			{
				continue;
			}
			if (CurrentValidIndex == DesiredValidIndex)
			{
				return MobID;
			}
			++CurrentValidIndex;
		}

		return FName(TEXT("RegularEnemy"));
	}

	static bool T66ValidateTowerGuardianCandidate(
		const T66TowerMapTerrain::FLayout& Layout,
		const T66TowerMapTerrain::FFloor& Floor,
		const FVector& Candidate,
		FVector& OutLocation)
	{
		FVector AdjustedCandidate(Candidate.X, Candidate.Y, Floor.SurfaceZ + 120.0f);
		if (T66TowerMapTerrain::FindFloorIndexForLocation(Layout, AdjustedCandidate) != Floor.FloorNumber)
		{
			return false;
		}

		OutLocation = AdjustedCandidate;
		return true;
	}

	static FVector T66ResolveTowerGateGuardianLocation(
		const T66TowerMapTerrain::FLayout& Layout,
		const T66TowerMapTerrain::FFloor& Floor)
	{
		FVector SpawnLocation = FVector::ZeroVector;
		if (Floor.bHasDropHole && T66ValidateTowerGuardianCandidate(Layout, Floor, Floor.HoleCenter, SpawnLocation))
		{
			return SpawnLocation;
		}

		if (!Floor.ExitPoint.IsNearlyZero() && T66ValidateTowerGuardianCandidate(Layout, Floor, Floor.ExitPoint, SpawnLocation))
		{
			return SpawnLocation;
		}

		return FVector(Floor.Center.X, Floor.Center.Y, Floor.SurfaceZ + 120.0f);
	}

	static AT66EnemyBase* T66SpawnTowerGateGuardian(
		UWorld* World,
		const T66TowerMapTerrain::FLayout& Layout,
		const T66TowerMapTerrain::FFloor& Floor)
	{
		if (!World)
		{
			return nullptr;
		}

		UGameInstance* GI = World->GetGameInstance();
		UT66GameInstance* T66GI = Cast<UT66GameInstance>(GI);
		UT66RunStateSubsystem* RunState = GI ? GI->GetSubsystem<UT66RunStateSubsystem>() : nullptr;
		const int32 StageNum = RunState ? RunState->GetCurrentStage() : 1;
		const FName MobID = T66PickTowerGateGuardianMob(T66GI, StageNum, Floor.FloorNumber);
		const TSubclassOf<AT66EnemyBase> GuardianClass = FT66EnemyFamilyResolver::ResolveEnemyClass(MobID, AT66EnemyBase::StaticClass());
		const FVector SpawnLocation = T66ResolveTowerGateGuardianLocation(Layout, Floor);

		FActorSpawnParameters SpawnParams;
		SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
		AT66EnemyBase* Guardian = World->SpawnActor<AT66EnemyBase>(GuardianClass, SpawnLocation, FRotator::ZeroRotator, SpawnParams);
		if (!Guardian)
		{
			UE_LOG(LogT66GameMode, Warning, TEXT("[MAP] Tower gate guardian spawn failed on floor %d (mob=%s)."), Floor.FloorNumber, *MobID.ToString());
			return nullptr;
		}

		Guardian->Tags.AddUnique(T66TowerDescentGuardianTag);
		Guardian->bDropsLoot = false;
		if (!MobID.IsNone())
		{
			Guardian->ConfigureAsMob(MobID);
		}
		if (RunState)
		{
			Guardian->ApplyStageScaling(StageNum);
			Guardian->ApplyDifficultyScalar(RunState->GetDifficultyScalar());
		}
		Guardian->ApplyMiniBossMultipliers(2.75f, 1.65f, 1.9f);

		T66TrySnapActorToTowerFloor(World, Guardian, Layout, Floor.FloorNumber, Guardian->GetActorLocation());
		if (AT66GameMode* GameMode = World->GetAuthGameMode<AT66GameMode>())
		{
			const int32 ResolvedFloor = GameMode->GetTowerFloorIndexForLocation(Guardian->GetActorLocation());
			if (ResolvedFloor != Floor.FloorNumber)
			{
				UE_LOG(
					LogT66GameMode,
					Warning,
					TEXT("[MAP] Tower gate guardian rejected after snap: requested floor=%d resolved floor=%d mob=%s loc=%s."),
					Floor.FloorNumber,
					ResolvedFloor,
					*MobID.ToString(),
					*Guardian->GetActorLocation().ToCompactString());
				Guardian->Destroy();
				return nullptr;
			}
		}

		T66AssignTowerFloorTag(Guardian, Floor.FloorNumber);
		UE_LOG(
			LogT66GameMode,
			Log,
			TEXT("[MAP] Tower gate guardian spawned floor=%d mob=%s hp=%d scale=%.2f loc=%s."),
			Floor.FloorNumber,
			*MobID.ToString(),
			Guardian->MaxHP,
			Guardian->GetActorScale3D().X,
			*Guardian->GetActorLocation().ToCompactString());
		return Guardian;
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
			HoleActor->InitializeHole(
				StartFloor->FloorNumber,
				BossFloor->FloorNumber,
				BoxExtent,
				/*bInRequiresWeaponSelection*/ true,
				/*bInRequiresGuardianDefeated*/ false);
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

		const bool bRequiresWeaponSelection = Floor.FloorNumber == CachedTowerMainMapLayout.StartFloorNumber;
		const bool bRequiresGuardianDefeated = Floor.FloorNumber != CachedTowerMainMapLayout.StartFloorNumber;
		HoleActor->InitializeHole(
			Floor.FloorNumber,
			DestinationFloor->FloorNumber,
			BoxExtent,
			bRequiresWeaponSelection,
			bRequiresGuardianDefeated);
		if (bRequiresGuardianDefeated)
		{
			AT66EnemyBase* Guardian = T66SpawnTowerGateGuardian(World, CachedTowerMainMapLayout, Floor);
			HoleActor->SetGuardianEnemy(Guardian);
			if (!Guardian)
			{
				UE_LOG(LogT66GameMode, Warning, TEXT("[MAP] Tower descent hole floor %d has no guardian after spawn attempt."), Floor.FloorNumber);
			}
		}
		HoleActor->Tags.AddUnique(FName(TEXT("T66_Tower_DescentHole")));
		TowerDescentHoles.Add(HoleActor);
	}
}

void AT66GameMode::HandleTowerGateGuardianDefeated(AT66EnemyBase* Guardian)
{
	if (!IsUsingTowerMainMapLayout() || !Guardian || !Guardian->ActorHasTag(T66TowerDescentGuardianTag))
	{
		return;
	}

	UWorld* World = GetWorld();
	if (!World)
	{
		return;
	}

	int32 GuardianFloorNumber = T66ReadTowerFloorTag(Guardian);
	if (GuardianFloorNumber == INDEX_NONE)
	{
		GuardianFloorNumber = GetTowerFloorIndexForLocation(Guardian->GetActorLocation());
	}

	const FVector DropLocation = Guardian->GetActorLocation();
	Guardian->Tags.Remove(T66TowerDescentGuardianTag);

	AT66IdolAltar* SpawnedAltar = SpawnIdolAltarAtLocation(DropLocation, /*bAllowMultiple*/ true);
	if (SpawnedAltar && GuardianFloorNumber != INDEX_NONE)
	{
		T66TrySnapActorToTowerFloor(World, SpawnedAltar, CachedTowerMainMapLayout, GuardianFloorNumber, SpawnedAltar->GetActorLocation());
		T66AssignTowerFloorTag(SpawnedAltar, GuardianFloorNumber);
		SyncTowerMiasmaSourceAnchor(GuardianFloorNumber, SpawnedAltar->GetActorLocation());
	}

	UE_LOG(
		LogT66GameMode,
		Log,
		TEXT("[MAP] Tower gate guardian defeated floor=%d; idol altar %s at %s."),
		GuardianFloorNumber,
		SpawnedAltar ? TEXT("spawned") : TEXT("failed"),
		SpawnedAltar ? *SpawnedAltar->GetActorLocation().ToCompactString() : TEXT("None"));
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

bool AT66GameMode::GetTowerFloorLayout(const int32 FloorNumber, T66TowerMapTerrain::FFloor& OutFloor) const
{
	if (!IsUsingTowerMainMapLayout())
	{
		return false;
	}

	for (const T66TowerMapTerrain::FFloor& Floor : CachedTowerMainMapLayout.Floors)
	{
		if (Floor.FloorNumber == FloorNumber)
		{
			OutFloor = Floor;
			return true;
		}
	}

	return false;
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

	if (ToFloorNumber != INDEX_NONE && ToFloorNumber != ActiveTowerTerrainVisualFloorNumber)
	{
		T66SetTowerTerrainVisualFloor(GetWorld(), ToFloorNumber);
		ActiveTowerTerrainVisualFloorNumber = ToFloorNumber;
	}

	if (!bTowerMiasmaActive && ToFloorNumber >= CachedTowerMainMapLayout.FirstGameplayFloorNumber)
	{
		const FVector FloorAnchor = Pawn->GetActorLocation();
		TryStartTowerMiasma(&FloorAnchor, ToFloorNumber);
	}

	if (ToFloorNumber >= CachedTowerMainMapLayout.FirstGameplayFloorNumber
		&& ToFloorNumber <= CachedTowerMainMapLayout.LastGameplayFloorNumber)
	{
		if (AT66EnemyDirector* ExistingEnemyDirector = FindOrCacheEnemyDirector(GetWorld()))
		{
			ExistingEnemyDirector->SpawnInitialPopulationForTowerFloor(ToFloorNumber);
		}
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
		ActiveTowerTerrainVisualFloorNumber = INDEX_NONE;
		return;
	}

	const int32 CurrentFloorNumber = GetCurrentTowerFloorIndex();
	const bool bFloorChanged = CurrentFloorNumber != ActiveTowerTrapFloorNumber;
	if (!bForce && !bFloorChanged)
	{
		return;
	}

	TowerTrapActivationAccumulator = 0.f;
	ActiveTowerTrapFloorNumber = CurrentFloorNumber;
	TrapSubsystem->SetActiveTowerFloor(CurrentFloorNumber);
	if (bForce || CurrentFloorNumber != ActiveTowerTerrainVisualFloorNumber)
	{
		T66SetTowerTerrainVisualFloor(World, CurrentFloorNumber);
		ActiveTowerTerrainVisualFloorNumber = CurrentFloorNumber;
	}
}
