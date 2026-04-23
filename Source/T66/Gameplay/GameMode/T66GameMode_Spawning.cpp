// Copyright Tribulation 66. All Rights Reserved.

#include "Gameplay/GameMode/T66GameModePrivate.h"

using namespace T66GameModePrivate;

void AT66GameMode::RestartPlayer(AController* NewPlayer)
{
	Super::RestartPlayer(NewPlayer);
	SpawnCompanionForPlayer(NewPlayer);
	// Start gate is spawned in SpawnLevelContentAfterLandscapeReady (after floor exists) so its ground trace hits the floor.

	UT66GameInstance* GI = GetT66GameInstance();
	APawn* Pawn = NewPlayer ? NewPlayer->GetPawn() : nullptr;
	if (GI && Pawn && GI->bApplyLoadedTransform)
	{
		const FRotator LoadedRotation = GI->PendingLoadedTransform.Rotator();
		Pawn->SetActorTransform(GI->PendingLoadedTransform);
		T66SyncPawnAndControllerRotation(Pawn, NewPlayer, LoadedRotation);
		GI->bApplyLoadedTransform = false;
		GI->PendingLoadedTransform = FTransform();
	}

	if (GI && NewPlayer)
	{
		if (AT66PlayerController* T66PlayerController = Cast<AT66PlayerController>(NewPlayer))
		{
			T66PlayerController->ClientApplyGameplayRunSettings(
				T66EnsureRunSeed(GI),
				GI->SelectedDifficulty,
				GI->CurrentMainMapLayoutVariant);
		}

		if (UGameInstance* GameInstance = GetGameInstance())
		{
			if (UT66RunStateSubsystem* RunState = GameInstance->GetSubsystem<UT66RunStateSubsystem>())
			{
				if (RunState->HasPendingDifficultyClearSummary())
				{
					if (AT66PlayerController* PC = Cast<AT66PlayerController>(NewPlayer))
					{
						PC->ShowDifficultyClearSummary();
					}
				}
			}
		}
	}

	MaintainPlayerTerrainSafety();

}

bool AT66GameMode::SwapCompanionForPlayer(AController* Player, FName NewCompanionID)
{
	if (!Player) return false;
	if (NewCompanionID.IsNone()) return false;

	UT66GameInstance* GI = GetT66GameInstance();
	if (!GI) return false;

	// If this companion is already selected, treat as handled (no swap).
	if (GI->SelectedCompanionID == NewCompanionID)
	{
		return true;
	}

	// Validate new companion exists in DT_Companions (prevents setting an invalid ID).
	{
		FCompanionData NewData;
		if (!GI->GetCompanionData(NewCompanionID, NewData))
		{
			return false;
		}
	}

	UWorld* World = GetWorld();
	if (!World) return false;

	// If a companion is currently following, spawn a recruitable version where it currently is.
	FName OldCompanionID = NAME_None;
	FVector OldCompanionLoc = FVector::ZeroVector;
	if (TWeakObjectPtr<AT66CompanionBase>* Existing = PlayerCompanions.Find(Player))
	{
		if (AT66CompanionBase* ExistingComp = Existing->Get())
		{
			OldCompanionID = ExistingComp->CompanionID;
			OldCompanionLoc = ExistingComp->GetActorLocation();
			ExistingComp->Destroy();
		}
		PlayerCompanions.Remove(Player);
	}

	if (!OldCompanionID.IsNone())
	{
		FCompanionData OldData;
		if (GI->GetCompanionData(OldCompanionID, OldData))
		{
			FActorSpawnParameters SpawnParams;
			SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
			if (AT66RecruitableCompanion* OldRecruit = World->SpawnActor<AT66RecruitableCompanion>(AT66RecruitableCompanion::StaticClass(), OldCompanionLoc, FRotator::ZeroRotator, SpawnParams))
			{
				OldRecruit->InitializeRecruit(OldData);
			}
		}
	}

	// Persist for the rest of the run (stage transitions read SelectedCompanionID).
	GI->SelectedCompanionID = NewCompanionID;

	SpawnCompanionForPlayer(Player);
	return true;
}

void AT66GameMode::SpawnCompanionForPlayer(AController* Player)
{
	UT66GameInstance* GI = GetT66GameInstance();
	if (!GI) return;
	if (T66IsStandaloneTutorialMap(GetWorld()))
	{
		return;
	}

	const FName SelectedCompanionID = T66GetSelectedCompanionID(GI, Player);
	if (SelectedCompanionID.IsNone()) return;

	FCompanionData CompanionData;
	if (!GI->GetCompanionData(SelectedCompanionID, CompanionData)) return;

	UWorld* World = GetWorld();
	if (!World) return;

	APawn* HeroPawn = Player ? Player->GetPawn() : nullptr;
	if (!HeroPawn) return;

	// Prevent duplicate companions on respawn.
	if (Player)
	{
		if (TWeakObjectPtr<AT66CompanionBase>* Existing = PlayerCompanions.Find(Player))
		{
			if (AT66CompanionBase* ExistingComp = Existing->Get())
			{
				ExistingComp->Destroy();
			}
			PlayerCompanions.Remove(Player);
		}
	}

	UClass* CompanionClass = AT66CompanionBase::StaticClass();
	const bool bWantsSpecificClass = !CompanionData.CompanionClass.IsNull();
	const bool bHasLoadedClass = bWantsSpecificClass && (CompanionData.CompanionClass.Get() != nullptr);
	if (bHasLoadedClass)
	{
		if (UClass* Loaded = CompanionData.CompanionClass.Get())
		{
			if (Loaded->IsChildOf(AT66CompanionBase::StaticClass()))
			{
				CompanionClass = Loaded;
			}
		}
	}

	FVector SpawnLoc = HeroPawn->GetActorLocation() + FVector(-150.f, 100.f, 0.f);
	FActorSpawnParameters SpawnParams;
	SpawnParams.Owner = HeroPawn;
	SpawnParams.Instigator = HeroPawn;
	SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

	FName CompanionSkinID = FName(TEXT("Default"));
	if (UGameInstance* GII = GetGameInstance())
	{
		if (UT66AchievementsSubsystem* Ach = GII->GetSubsystem<UT66AchievementsSubsystem>())
		{
			CompanionSkinID = Ach->GetEquippedCompanionSkinID(SelectedCompanionID);
		}
	}

	AT66CompanionBase* Companion = World->SpawnActor<AT66CompanionBase>(CompanionClass, SpawnLoc, FRotator::ZeroRotator, SpawnParams);
	if (Companion)
	{
		Companion->InitializeCompanion(CompanionData, CompanionSkinID);
		Companion->SetPreviewMode(false); // gameplay: follow hero
		if (Player)
		{
			PlayerCompanions.Add(Player, Companion);
		}
		// Snap companion to ground so it doesn't float.
		FHitResult Hit;
		const FVector Start = Companion->GetActorLocation() + FVector(0.f, 0.f, 2000.f);
		const FVector End = Companion->GetActorLocation() - FVector(0.f, 0.f, 9000.f);
		if (World->LineTraceSingleByChannel(Hit, Start, End, ECC_WorldStatic))
		{
			Companion->SetActorLocation(Hit.ImpactPoint, false, nullptr, ETeleportType::TeleportPhysics);
		}
		UE_LOG(LogT66GameMode, Log, TEXT("Spawned companion: %s"), *CompanionData.DisplayName.ToString());
	}

	// If the companion class is a soft reference and isn't loaded yet, load asynchronously and replace.
	if (bWantsSpecificClass && !bHasLoadedClass)
	{
		const FSoftObjectPath ClassPath = CompanionData.CompanionClass.ToSoftObjectPath();
		const TWeakObjectPtr<AController> WeakPlayer(Player);
		const TWeakObjectPtr<AT66CompanionBase> WeakExisting(Companion);
		const FCompanionData CompanionDataCopy = CompanionData;

		TSharedPtr<FStreamableHandle> Handle = UAssetManager::GetStreamableManager().RequestAsyncLoad(
			ClassPath,
			FStreamableDelegate::CreateWeakLambda(this, [this, WeakPlayer, WeakExisting, CompanionDataCopy]()
			{
				AController* PlayerCtrl = WeakPlayer.Get();
				if (!PlayerCtrl) return;

				UWorld* World2 = GetWorld();
				if (!World2) return;

				UClass* Loaded = CompanionDataCopy.CompanionClass.Get();
				if (!Loaded || !Loaded->IsChildOf(AT66CompanionBase::StaticClass()))
				{
					return;
				}

				AT66CompanionBase* ExistingComp = WeakExisting.Get();
				// If the existing companion is already the correct class (or was destroyed), do nothing.
				if (ExistingComp && ExistingComp->GetClass() == Loaded)
				{
					return;
				}

				// Remove the old companion if it's still around.
				if (TWeakObjectPtr<AT66CompanionBase>* Current = PlayerCompanions.Find(PlayerCtrl))
				{
					if (AT66CompanionBase* C = Current->Get())
					{
						C->Destroy();
					}
					PlayerCompanions.Remove(PlayerCtrl);
				}

				APawn* HeroPawn2 = PlayerCtrl->GetPawn();
				if (!HeroPawn2) return;

				FVector SpawnLoc2 = HeroPawn2->GetActorLocation() + FVector(-150.f, 100.f, 0.f);
				FActorSpawnParameters SpawnParams2;
				SpawnParams2.Owner = HeroPawn2;
				SpawnParams2.Instigator = HeroPawn2;
				SpawnParams2.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

				FName AsyncSkinID = FName(TEXT("Default"));
				if (UGameInstance* GII = GetGameInstance())
				{
					if (UT66AchievementsSubsystem* Ach = GII->GetSubsystem<UT66AchievementsSubsystem>())
					{
						AsyncSkinID = Ach->GetEquippedCompanionSkinID(CompanionDataCopy.CompanionID);
					}
				}
				AT66CompanionBase* NewComp = World2->SpawnActor<AT66CompanionBase>(Loaded, SpawnLoc2, FRotator::ZeroRotator, SpawnParams2);
				if (NewComp)
				{
					NewComp->InitializeCompanion(CompanionDataCopy, AsyncSkinID);
					NewComp->SetPreviewMode(false);
					PlayerCompanions.Add(PlayerCtrl, NewComp);
				}
			}));
		if (Handle.IsValid())
		{
			ActiveAsyncLoadHandles.Add(Handle);
		}
	}
}

void AT66GameMode::SpawnVendorForPlayer(AController* Player)
{
	APawn* HeroPawn = Player ? Player->GetPawn() : nullptr;
	if (!HeroPawn) return;

	UWorld* World = GetWorld();
	if (!World) return;

	FVector SpawnLoc = HeroPawn->GetActorLocation() + FVector(300.f, 0.f, 0.f);
	FActorSpawnParameters SpawnParams;
	SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

	AT66VendorNPC* Vendor = World->SpawnActor<AT66VendorNPC>(AT66VendorNPC::StaticClass(), SpawnLoc, FRotator::ZeroRotator, SpawnParams);
	if (Vendor)
	{
		UE_LOG(LogT66GameMode, Log, TEXT("Spawned vendor NPC near hero"));
	}
}

void AT66GameMode::SpawnStartGateForPlayer(AController* Player)
{
	(void)Player;
	UWorld* World = GetWorld();
	if (!World) return;
	if (T66UsesMainMapTerrainStage(World)) return;

	// Spawn once per level (gate is a world landmark).
	if (StartGate) return;

	// Gate at the start-corridor exit so combat starts only when the player enters the main arena.
	static constexpr float GateZOffset = 5.f;
	FVector GateLoc = T66GameplayLayout::GetStartGateLocation();
	float GateGroundZ = 200.f;
	FHitResult Hit;
	if (World->LineTraceSingleByChannel(Hit, GateLoc + FVector(0.f, 0.f, 3000.f), GateLoc - FVector(0.f, 0.f, 9000.f), ECC_WorldStatic))
	{
		GateGroundZ = Hit.ImpactPoint.Z;
		GateLoc.Z = GateGroundZ + GateZOffset;
	}
	else
	{
		GateLoc.Z = GateGroundZ;
	}

	FActorSpawnParameters SpawnParams;
	SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

	StartGate = World->SpawnActor<AT66StartGate>(AT66StartGate::StaticClass(), GateLoc, FRotator::ZeroRotator, SpawnParams);
	if (StartGate)
	{
		StartGate->TriggerDistance2D = 220.f;
		if (StartGate->TriggerBox)
		{
			StartGate->TriggerBox->SetBoxExtent(FVector(120.f, T66GameplayLayout::CorridorHalfHeightY * 0.92f, 220.f));
		}
		if (StartGate->PoleLeft)
		{
			StartGate->PoleLeft->SetVisibility(false, true);
			StartGate->PoleLeft->SetHiddenInGame(true, true);
		}
		if (StartGate->PoleRight)
		{
			StartGate->PoleRight->SetVisibility(false, true);
			StartGate->PoleRight->SetHiddenInGame(true, true);
		}
		UE_LOG(LogT66GameMode, Log, TEXT("Spawned Start Gate at main-area entrance (%.0f, %.0f, %.0f)"), GateLoc.X, GateLoc.Y, GateLoc.Z);
	}

}

void AT66GameMode::SpawnPlayerStartIfNeeded()
{
	UWorld* World = GetWorld();
	if (!World) return;

	const bool bUsingMainMapTerrain = T66UsesMainMapTerrainStage(World);

	if (bUsingMainMapTerrain)
	{
		T66DestroyCachedPlayerStarts(World);
		return;
	}

	// Check for existing player start
	if (!T66HasAnyCachedPlayerStarts(World))
	{
		UE_LOG(LogT66GameMode, Log, TEXT("No PlayerStart found - spawning development PlayerStart"));

		FActorSpawnParameters SpawnParams;
		SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
		UT66GameInstance* GI = GetT66GameInstance();

		// Default spawn: normal stage start area so the timer begins after passing the start pillars.
		FVector SpawnLoc;
		if (bUsingMainMapTerrain)
		{
			const FT66MapPreset Preset = T66BuildMainMapPreset(GI);
			SpawnLoc = T66MainMapTerrain::GetPreferredSpawnLocation(Preset, DefaultSpawnHeight);
		}
		else
		{
			SpawnLoc = T66GameplayLayout::GetStartAreaCenter(DefaultSpawnHeight);
		}

		APlayerStart* Start = World->SpawnActor<APlayerStart>(
			APlayerStart::StaticClass(),
			SpawnLoc,
			FRotator::ZeroRotator,
			SpawnParams
		);

		if (Start)
		{
			#if WITH_EDITOR
			Start->SetActorLabel(TEXT("DEV_PlayerStart"));
			#endif
			SpawnedSetupActors.Add(Start);
			T66InvalidatePlayerStartCache(World);
			UE_LOG(LogT66GameMode, Log, TEXT("Spawned development PlayerStart at %s"), *SpawnLoc.ToString());
		}
	}
}

void AT66GameMode::RestartPlayersMissingPawns()
{
	UWorld* World = GetWorld();
	if (!World)
	{
		return;
	}

	for (FConstPlayerControllerIterator It = World->GetPlayerControllerIterator(); It; ++It)
	{
		APlayerController* PC = It->Get();
		if (!PC || PC->GetPawn())
		{
			continue;
		}

		RestartPlayer(PC);
	}
}

UClass* AT66GameMode::GetDefaultPawnClassForController_Implementation(AController* InController)
{
	// Check if we have a specific hero class from the DataTable
	if (UT66GameInstance* GI = GetT66GameInstance())
	{
		const FName EffectiveHeroID = T66GetSelectedHeroID(GI, InController);
		FHeroData HeroData;
		if (!EffectiveHeroID.IsNone() && GI->GetHeroData(EffectiveHeroID, HeroData))
		{
			// If the hero has a specific class defined, use that
			if (!HeroData.HeroClass.IsNull())
			{
				// Avoid sync loads during spawn; prefer already-loaded class, otherwise fall back safely.
				if (UClass* HeroClass = HeroData.HeroClass.Get())
				{
					return HeroClass;
				}

				// Kick off an async load so subsequent spawns can use the intended class without hitching.
				TSharedPtr<FStreamableHandle> Handle = UAssetManager::GetStreamableManager().RequestAsyncLoad(
					HeroData.HeroClass.ToSoftObjectPath(),
					FStreamableDelegate());
				if (Handle.IsValid())
				{
					ActiveAsyncLoadHandles.Add(Handle);
				}
			}
		}
	}

	// Fall back to default hero class
	return DefaultHeroClass ? DefaultHeroClass.Get() : AT66HeroBase::StaticClass();
}

APawn* AT66GameMode::SpawnDefaultPawnFor_Implementation(AController* NewPlayer, AActor* StartSpot)
{
	// Get the pawn class
	UClass* PawnClass = GetDefaultPawnClassForController(NewPlayer);
	if (!PawnClass)
	{
		UE_LOG(LogT66GameMode, Error, TEXT("No pawn class for spawning!"));
		return nullptr;
	}

	// Get spawn transform - use a safe default height if no PlayerStart exists
	FVector SpawnLocation;
	FRotator SpawnRotation = FRotator::ZeroRotator;
	const bool bUsingMainMapTerrain = T66UsesMainMapTerrainStage(GetWorld());
	UT66GameInstance* CurrentGI = GetT66GameInstance();
	UT66RunStateSubsystem* CurrentRunState = GetGameInstance() ? GetGameInstance()->GetSubsystem<UT66RunStateSubsystem>() : nullptr;
	FVector MainMapSafeSpawnLocation = FVector::ZeroVector;
	const bool bHasMainMapSafeSpawnLocation = bUsingMainMapTerrain
		&& TryGetMainMapStartPlacementLocation(0.f, -1.10f, MainMapSafeSpawnLocation);
	const bool bUseTowerStageDropIntro = bUsingMainMapTerrain
		&& IsUsingTowerMainMapLayout()
		&& CurrentGI
		&& CurrentGI->bPendingTowerStageDropIntro
		&& CurrentRunState
		&& CurrentRunState->GetCurrentStage() > 1;

	if (bUsingMainMapTerrain && !bTerrainCollisionReady)
	{
		EnsureGameplayStartupInitialized(TEXT("SpawnDefaultPawnFor"));
		if (!bGameplayLevelSpawnCompleted)
		{
			UE_LOG(LogT66GameMode, Warning, TEXT("Recovering gameplay level spawn from SpawnDefaultPawnFor because terrain/content bootstrap was not ready."));
			SpawnLevelContentAfterLandscapeReady();
			if (NewPlayer && NewPlayer->GetPawn())
			{
				UE_LOG(LogT66GameMode, Warning, TEXT("Spawn recovery already created a pawn for this controller; reusing it to avoid duplicate hero spawns."));
				return NewPlayer->GetPawn();
			}
		}

		if (!bTerrainCollisionReady)
		{
			UE_LOG(LogT66GameMode, Warning, TEXT("Main-map terrain is still not ready during pawn spawn; spawning hero in frozen safe mode."));
		}
	}

	if (bUsingMainMapTerrain && bHasMainMapSpawnSurfaceLocation)
	{
		SpawnLocation = (bHasMainMapSafeSpawnLocation ? MainMapSafeSpawnLocation : MainMapSpawnSurfaceLocation) + FVector(0.f, 0.f, DefaultSpawnHeight);
		FVector Center = FVector::ZeroVector;
		FVector InwardDirection = FVector::ForwardVector;
		FVector SideDirection = FVector::RightVector;
		float CellSize = 0.f;
		if (TryGetMainMapStartAxes(Center, InwardDirection, SideDirection, CellSize) && !InwardDirection.IsNearlyZero())
		{
			SpawnRotation = InwardDirection.Rotation();
		}
		else
		{
			SpawnRotation = FRotator::ZeroRotator;
		}
	}
	else if (StartSpot)
	{
		SpawnLocation = StartSpot->GetActorLocation();
		SpawnRotation = StartSpot->GetActorRotation();
		UE_LOG(LogT66GameMode, Log, TEXT("Spawning at PlayerStart: (%.1f, %.1f, %.1f)"),
			SpawnLocation.X, SpawnLocation.Y, SpawnLocation.Z);
	}
	else
	{
		// No PlayerStart found - spawn at a safe default location.
		SpawnLocation = FVector(-35455.f, 0.f, 200.f);
		UE_LOG(LogT66GameMode, Warning, TEXT("No PlayerStart found! Spawning at default location (%.0f, %.0f, %.0f)."),
			SpawnLocation.X, SpawnLocation.Y, SpawnLocation.Z);
	}

	// Robust: always ensure Gameplay spawns in the Start Area (scaled for 100k map).
	if (IsLabLevel())
	{
		SpawnLocation = FVector(0.f, 0.f, 120.f);
		SpawnRotation = FRotator::ZeroRotator;
	}
	else if (bUsingMainMapTerrain)
	{
		if (bHasMainMapSafeSpawnLocation)
		{
			SpawnLocation = MainMapSafeSpawnLocation + FVector(0.f, 0.f, 200.f);
		}
		else if (bHasMainMapSpawnSurfaceLocation)
		{
			SpawnLocation = MainMapSpawnSurfaceLocation + FVector(0.f, 0.f, 200.f);
		}
		else
		{
			if (IsUsingTowerMainMapLayout())
			{
				SpawnLocation = T66TowerMapTerrain::GetPreferredSpawnLocation(CachedTowerMainMapLayout, 200.f);
			}
			else
			{
				const FT66MapPreset Preset = T66BuildMainMapPreset(GetT66GameInstance());
				SpawnLocation = T66MainMapTerrain::GetPreferredSpawnLocation(Preset, 200.f);
			}
		}
		FVector Center = FVector::ZeroVector;
		FVector InwardDirection = FVector::ForwardVector;
		FVector SideDirection = FVector::RightVector;
		float CellSize = 0.f;
		if (TryGetMainMapStartAxes(Center, InwardDirection, SideDirection, CellSize) && !InwardDirection.IsNearlyZero())
		{
			SpawnRotation = InwardDirection.Rotation();
		}
		else
		{
			SpawnRotation = FRotator::ZeroRotator;
		}

		if (bUseTowerStageDropIntro && bHasMainMapSpawnSurfaceLocation)
		{
			SpawnLocation = MainMapSpawnSurfaceLocation + FVector(0.f, 0.f, T66TowerStageTransitionDropHeight);
		}
	}
	else
	{
		SpawnLocation = FVector(-35455.f, 0.f, 200.f);
		SpawnRotation = FRotator::ZeroRotator;

		if (UT66GameInstance* T66GI = GetT66GameInstance())
		{
			if (T66IsStandaloneTutorialMap(GetWorld()) || bForceSpawnInTutorialArea)
			{
				FVector TutorialSpawnLocation = FVector(-3636.f, 56818.f, 200.f);
				FRotator TutorialSpawnRotation = FRotator::ZeroRotator;
				if (T66TryGetTaggedActorTransform(GetWorld(), FName(TEXT("T66_Tutorial_PlayerSpawn")), TutorialSpawnLocation, TutorialSpawnRotation))
				{
					SpawnLocation = TutorialSpawnLocation;
					SpawnRotation = TutorialSpawnRotation;
				}
				else
				{
					SpawnLocation = FVector(-3636.f, 56818.f, 200.f);
					SpawnRotation = FRotator::ZeroRotator;
				}
			}
		}
	}

	const UWorld* SpawnWorld = GetWorld();
	const int32 ConnectedPlayerCount = T66GetConnectedPlayerCount(SpawnWorld);
	const int32 PlayerSlotIndex = T66GetPlayerSlotIndex(SpawnWorld, NewPlayer);
	const float CenteredSlotOffset = static_cast<float>(PlayerSlotIndex) - (static_cast<float>(ConnectedPlayerCount - 1) * 0.5f);
	FVector MultiplayerSpawnOffset = FVector::ZeroVector;
	if (ConnectedPlayerCount > 1 && !FMath::IsNearlyZero(CenteredSlotOffset))
	{
		if (bUsingMainMapTerrain)
		{
			FVector Center = FVector::ZeroVector;
			FVector InwardDirection = FVector::ForwardVector;
			FVector SideDirection = FVector::RightVector;
			float CellSize = 0.f;
			if (TryGetMainMapStartAxes(Center, InwardDirection, SideDirection, CellSize) && !SideDirection.IsNearlyZero())
			{
				MultiplayerSpawnOffset = SideDirection * (CenteredSlotOffset * FMath::Max(CellSize * 0.55f, 180.f));
			}
		}
		else
		{
			MultiplayerSpawnOffset = FVector(0.f, CenteredSlotOffset * 220.f, 0.f);
		}

		SpawnLocation += MultiplayerSpawnOffset;
	}

	if (bUsingMainMapTerrain)
	{
		const FVector FacingTarget = !MainMapStartAreaCenterSurfaceLocation.IsNearlyZero()
			? MainMapStartAreaCenterSurfaceLocation
			: (!MainMapStartAnchorSurfaceLocation.IsNearlyZero()
				? MainMapStartAnchorSurfaceLocation
				: MainMapSpawnSurfaceLocation);
		FRotator FacingRotation = SpawnRotation;
		if (T66TryBuildFacingRotation2D(SpawnLocation, FacingTarget, FacingRotation))
		{
			SpawnRotation = FacingRotation;
		}
	}

	// Sky-drop: spawn the hero at a comfortable altitude; gravity + Landed() handle the rest.
	// This avoids trace-before-terrain-exists timing issues and gives a cinematic entrance.

	// Spawn parameters
	FActorSpawnParameters SpawnParams;
	SpawnParams.Owner = NewPlayer;
	SpawnParams.SpawnCollisionHandlingOverride = bUsingMainMapTerrain
		? ESpawnActorCollisionHandlingMethod::AlwaysSpawn
		: ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;

	// Spawn the pawn
	APawn* SpawnedPawn = GetWorld()->SpawnActor<APawn>(PawnClass, SpawnLocation, SpawnRotation, SpawnParams);

	if (SpawnedPawn)
	{
		T66SyncPawnAndControllerRotation(SpawnedPawn, NewPlayer, SpawnRotation);

		const bool bNeedsProceduralTerrain = !IsLabLevel();
		if (bUsingMainMapTerrain && bTerrainCollisionReady)
		{
			float HalfHeight = 0.f;
			if (const UCapsuleComponent* Capsule = SpawnedPawn->FindComponentByClass<UCapsuleComponent>())
			{
				HalfHeight = Capsule->GetScaledCapsuleHalfHeight();
			}

			FVector ExactSpawnLocation = SpawnLocation;
			if (bHasMainMapSafeSpawnLocation)
			{
				ExactSpawnLocation = MainMapSafeSpawnLocation + MultiplayerSpawnOffset + FVector(0.f, 0.f, HalfHeight + 5.f);
			}
			else if (bHasMainMapSpawnSurfaceLocation)
			{
				ExactSpawnLocation = MainMapSpawnSurfaceLocation + MultiplayerSpawnOffset + FVector(0.f, 0.f, HalfHeight + 5.f);
			}
			else
			{
				if (IsUsingTowerMainMapLayout())
				{
					ExactSpawnLocation = T66TowerMapTerrain::GetPreferredSpawnLocation(CachedTowerMainMapLayout, HalfHeight + 5.f) + MultiplayerSpawnOffset;
				}
				else
				{
					const FT66MapPreset Preset = T66BuildMainMapPreset(GetT66GameInstance());
					ExactSpawnLocation = T66MainMapTerrain::GetPreferredSpawnLocation(Preset, HalfHeight + 5.f) + MultiplayerSpawnOffset;
				}
			}
			if (bUseTowerStageDropIntro)
			{
				const FVector DropStartLocation = (bHasMainMapSpawnSurfaceLocation
					? MainMapSpawnSurfaceLocation
					: ExactSpawnLocation)
					+ MultiplayerSpawnOffset
					+ FVector(0.f, 0.f, T66TowerStageTransitionDropHeight);
				SpawnedPawn->SetActorLocation(DropStartLocation, false, nullptr, ETeleportType::TeleportPhysics);
				T66SyncPawnAndControllerRotation(SpawnedPawn, NewPlayer, SpawnRotation);
				if (ACharacter* Character = Cast<ACharacter>(SpawnedPawn))
				{
					if (UCharacterMovementComponent* Movement = Character->GetCharacterMovement())
					{
						Movement->StopMovementImmediately();
						Movement->SetMovementMode(MOVE_Falling);
					}
				}
				if (CurrentGI)
				{
					CurrentGI->bPendingTowerStageDropIntro = false;
				}
			}
			else
			{
				SpawnedPawn->SetActorLocation(ExactSpawnLocation, false, nullptr, ETeleportType::TeleportPhysics);
				T66SyncPawnAndControllerRotation(SpawnedPawn, NewPlayer, SpawnRotation);
				if (ACharacter* Character = Cast<ACharacter>(SpawnedPawn))
				{
					if (UCharacterMovementComponent* Movement = Character->GetCharacterMovement())
					{
						Movement->StopMovementImmediately();
						Movement->SetMovementMode(MOVE_Walking);
					}
				}
			}
		}
		else if (bTerrainCollisionReady)
		{
			TrySnapActorToTerrain(SpawnedPawn);
		}
		else if (bNeedsProceduralTerrain)
		{
			if (ACharacter* Character = Cast<ACharacter>(SpawnedPawn))
			{
				if (UCharacterMovementComponent* Movement = Character->GetCharacterMovement())
				{
					Movement->StopMovementImmediately();
					Movement->SetMovementMode(MOVE_None);
				}
			}
		}
	}

	if (SpawnedPawn && IsValid(IdolAltar))
	{
		T66FaceActorTowardLocation2D(IdolAltar, SpawnedPawn->GetActorLocation());
	}

	// If it's our hero class, initialize it with hero data and body type
	if (AT66HeroBase* Hero = Cast<AT66HeroBase>(SpawnedPawn))
	{

		if (UT66GameInstance* GI = GetT66GameInstance())
		{
			FHeroData HeroData;
			const FName EffectiveHeroID = T66GetSelectedHeroID(GI, NewPlayer);

			if (!EffectiveHeroID.IsNone() && GI->GetHeroData(EffectiveHeroID, HeroData))
			{
				if (GI->SelectedHeroID.IsNone() && NewPlayer && NewPlayer->IsLocalController())
				{
					GI->SelectedHeroID = EffectiveHeroID;
				}

				ET66BodyType SelectedBodyType = T66GetSelectedHeroBodyType(GI, NewPlayer);
				FName SelectedSkinID = T66GetSelectedHeroSkinID(GI, NewPlayer);
				Hero->InitializeHero(HeroData, SelectedBodyType, SelectedSkinID, false);

				UE_LOG(LogT66GameMode, Log, TEXT("Spawned hero: %s (%s), BodyType: %s, Skin: %s, Color: (%.2f, %.2f, %.2f)"),
					*HeroData.DisplayName.ToString(),
					*EffectiveHeroID.ToString(),
					SelectedBodyType == ET66BodyType::TypeA ? TEXT("TypeA") : TEXT("TypeB"),
					*SelectedSkinID.ToString(),
					HeroData.PlaceholderColor.R,
					HeroData.PlaceholderColor.G,
					HeroData.PlaceholderColor.B);
			}
			else
			{
				UE_LOG(LogT66GameMode, Warning, TEXT("No hero selected in Game Instance - spawning with defaults"));
			}
		}
	}

	if (SpawnedPawn && !PendingRunStartItemId.IsNone())
	{
		if (UT66RunStateSubsystem* RunState = GetGameInstance() ? GetGameInstance()->GetSubsystem<UT66RunStateSubsystem>() : nullptr)
		{
			UE_LOG(LogT66GameMode, Log, TEXT("[Community] Granting deferred run-start item after hero spawn: %s"), *PendingRunStartItemId.ToString());
			RunState->AddItem(PendingRunStartItemId);
		}
		PendingRunStartItemId = NAME_None;
	}

	return SpawnedPawn;
}

AT66HeroBase* AT66GameMode::SpawnSelectedHero(AController* Controller)
{
	// Find a player start
	AActor* StartSpot = FindPlayerStart(Controller);

	// Use our spawn logic
	APawn* SpawnedPawn = SpawnDefaultPawnFor(Controller, StartSpot);

	// Possess the pawn
	if (SpawnedPawn && Controller)
	{
		Controller->Possess(SpawnedPawn);
	}

	return Cast<AT66HeroBase>(SpawnedPawn);
}
