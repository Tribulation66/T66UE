// Copyright Tribulation 66. All Rights Reserved.

#include "Gameplay/GameMode/T66GameModePrivate.h"

using namespace T66GameModePrivate;

void AT66GameMode::SpawnStageEffectsForStage()
{
	if (IsUsingTowerMainMapLayout()) return;

	UWorld* World = GetWorld();
	if (!World) return;

	UGameInstance* GI = GetGameInstance();
	UT66RunStateSubsystem* RunState = GI ? GI->GetSubsystem<UT66RunStateSubsystem>() : nullptr;
	UT66GameInstance* T66GI = GetT66GameInstance();
	if (!RunState || !T66GI) return;
	if (T66GI->bStageCatchUpPending) return;

	const ET66Difficulty Difficulty = T66GI->SelectedDifficulty;
	const int32 StageNum = RunState->GetCurrentStage();

	const int32 RunSeed = (T66GI && T66GI->RunSeed != 0) ? T66GI->RunSeed : FMath::Rand();
	FRandomStream Rng(RunSeed + StageNum * 971 + 17);

	static constexpr float MainHalfExtent = 50000.f;
	static constexpr float SpawnZ = 40.f;
	static constexpr float SafeBubbleMargin = 350.f;

	const int32 ShroomCount = 0;

	struct FUsedStageEffectLoc
	{
		FVector Loc = FVector::ZeroVector;
		float ExclusionRadius = 0.f;
	};
	TArray<FUsedStageEffectLoc> UsedLocs;

	auto IsInsideNoSpawnZone = [&](const FVector& L) -> bool
	{
		if (T66GameplayLayout::IsInsideReservedTraversalZone2D(L, 455.f))
		{
			return true;
		}

		static constexpr float ArenaHalf = 9091.f;
		static constexpr float ArenaMargin = 682.f;
		static constexpr float TutorialArenaHalf = 9091.f;
		struct FArena2D { float X; float Y; float Half; };
		static constexpr FArena2D Arenas[] = {
			{ -22727.f, 34091.f, ArenaHalf }, // Catch Up
			{      0.f, 61364.f, TutorialArenaHalf }, // Tutorial
		};
		for (const FArena2D& A : Arenas)
		{
			if (FMath::Abs(L.X - A.X) <= (A.Half + ArenaMargin) && FMath::Abs(L.Y - A.Y) <= (A.Half + ArenaMargin))
			{
				return true;
			}
		}

		return false;
	};

	auto IsGoodLoc = [&](const FVector& L, float CandidateRadius) -> bool
	{
		if (IsInsideNoSpawnZone(L)) return false;
		for (const FUsedStageEffectLoc& Used : UsedLocs)
		{
			const float RequiredRadius = FMath::Max(CandidateRadius, Used.ExclusionRadius);
			if (FVector::DistSquared2D(L, Used.Loc) < (RequiredRadius * RequiredRadius)) return false;
		}
		UT66ActorRegistrySubsystem* Registry = World ? World->GetSubsystem<UT66ActorRegistrySubsystem>() : nullptr;
		const TArray<TWeakObjectPtr<AT66HouseNPCBase>>& NPCs = Registry ? Registry->GetNPCs() : TArray<TWeakObjectPtr<AT66HouseNPCBase>>();
		for (const TWeakObjectPtr<AT66HouseNPCBase>& WeakNPC : NPCs)
		{
			const AT66HouseNPCBase* NPC = WeakNPC.Get();
			if (!NPC) continue;
			const float R = NPC->GetSafeZoneRadius() + SafeBubbleMargin + CandidateRadius * 0.35f;
			if (FVector::DistSquared2D(L, NPC->GetActorLocation()) < (R * R)) return false;
		}
		const TArray<TWeakObjectPtr<AT66CasinoInteractable>>& Casinos = Registry ? Registry->GetCasinos() : TArray<TWeakObjectPtr<AT66CasinoInteractable>>();
		for (const TWeakObjectPtr<AT66CasinoInteractable>& WeakCasino : Casinos)
		{
			const AT66CasinoInteractable* Casino = WeakCasino.Get();
			if (!Casino) continue;
			const float R = Casino->GetSafeZoneRadius() + SafeBubbleMargin + CandidateRadius * 0.35f;
			if (FVector::DistSquared2D(L, Casino->GetActorLocation()) < (R * R)) return false;
		}
		return true;
	};

	struct FStageEffectSpawnHit
	{
		FVector Loc = FVector::ZeroVector;
		FVector Normal = FVector::UpVector;
		bool bFound = false;
	};

	auto FindSpawnLoc = [&](float CandidateRadius, int32 MaxTries) -> FStageEffectSpawnHit
	{
		const FVector Up = FVector::UpVector;
		for (int32 Try = 0; Try < MaxTries; ++Try)
		{
			const float X = Rng.FRandRange(-MainHalfExtent, MainHalfExtent);
			const float Y = Rng.FRandRange(-MainHalfExtent, MainHalfExtent);
			FVector Loc(X, Y, SpawnZ);
			FVector Normal = Up;

			FHitResult Hit;
			const FVector Start = Loc + FVector(0.f, 0.f, 2000.f);
			const FVector End = Loc - FVector(0.f, 0.f, 6000.f);
			if (!World->LineTraceSingleByChannel(Hit, Start, End, ECC_WorldStatic))
			{
				continue;
			}
			if (!T66GameplayLayout::IsValidGameplayGroundNormal(Hit.ImpactNormal))
			{
				continue;
			}

			Loc = Hit.ImpactPoint;
			Normal = Hit.ImpactNormal.GetSafeNormal(1e-4f, Up);

			if (IsGoodLoc(Loc, CandidateRadius))
			{
				return { Loc, Normal, true };
			}
		}
		return {};
	};

	auto NoteUsedLoc = [&](const FVector& Loc, float Radius)
	{
		UsedLocs.Add({ Loc, Radius });
	};

	FActorSpawnParameters P;
	P.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

	if (!IsValid(MiasmaManager))
	{
		MiasmaManager = World->SpawnActor<AT66MiasmaManager>(AT66MiasmaManager::StaticClass(), FVector::ZeroVector, FRotator::ZeroRotator, P);
	}

	const int32 SpawnedLavaCount = MiasmaManager ? MiasmaManager->SpawnLegacyStageLavaPatchesForCurrentStage() : 0;
	int32 SpawnedShroomCount = 0;

	static constexpr float ShroomExclusionRadius = 420.f;
	for (int32 i = 0; i < ShroomCount; ++i)
	{
		const FStageEffectSpawnHit Th = FindSpawnLoc(ShroomExclusionRadius, 80);
		if (!Th.bFound)
		{
			continue;
		}

		AT66Shroom* Shroom = World->SpawnActor<AT66Shroom>(AT66Shroom::StaticClass(), Th.Loc, FRotator::ZeroRotator, P);
		if (Shroom)
		{
			NoteUsedLoc(Th.Loc, ShroomExclusionRadius);
			++SpawnedShroomCount;
		}
	}

	UE_LOG(LogT66GameMode, Log,
		TEXT("[StageEffects] Spawned %d lava patches and %d shrooms for stage %d (diff=%d)."),
		SpawnedLavaCount,
		SpawnedShroomCount,
		StageNum,
		static_cast<int32>(Difficulty));
}

void AT66GameMode::SpawnTricksterAndCowardiceGate()
{
	UGameInstance* GI = GetGameInstance();
	UT66RunStateSubsystem* RunState = GI ? GI->GetSubsystem<UT66RunStateSubsystem>() : nullptr;
	if (!RunState) return;

	const int32 StageNum = RunState->GetCurrentStage();
	// Only on non-boss stages. Stage 23 is also a difficulty boss.
	if (T66_IsDifficultyBossStage(StageNum)) return;

	UWorld* World = GetWorld();
	UT66GameInstance* T66GI = GetT66GameInstance();
	if (!World) return;

	FActorSpawnParameters SpawnParams;
	SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

	FVector GateLoc = T66GameplayLayout::GetCowardiceGateLocation();
	FVector TricksterLoc = T66GameplayLayout::GetTricksterLocation();
	FRotator SpawnRotation = FRotator::ZeroRotator;

	if (CowardiceGate)
	{
		CowardiceGate->Destroy();
		CowardiceGate = nullptr;
	}
	if (TricksterNPC)
	{
		TricksterNPC->Destroy();
		TricksterNPC = nullptr;
	}

	if (T66UsesMainMapTerrainStage(World) && !MainMapBossAnchorSurfaceLocation.IsNearlyZero())
	{
		const FVector SideOffset = IsUsingTowerMainMapLayout()
			? FVector(260.f, 0.f, 0.f)
			: FVector(T66MainMapTerrain::MakeSettings(T66BuildMainMapPreset(T66GI)).CellSize * 0.28f, 0.f, 0.f);
		GateLoc = MainMapBossAnchorSurfaceLocation;
		TricksterLoc = MainMapBossAnchorSurfaceLocation + SideOffset;
		SpawnRotation = FRotator(0.f, 180.f, 0.f);
	}
	else
	{
		FHitResult Hit;
		if (World->LineTraceSingleByChannel(Hit, GateLoc + FVector(0.f, 0.f, 3000.f), GateLoc - FVector(0.f, 0.f, 9000.f), ECC_WorldStatic))
		{
			GateLoc.Z = Hit.ImpactPoint.Z;
		}
		if (World->LineTraceSingleByChannel(Hit, TricksterLoc + FVector(0.f, 0.f, 3000.f), TricksterLoc - FVector(0.f, 0.f, 9000.f), ECC_WorldStatic))
		{
			TricksterLoc.Z = Hit.ImpactPoint.Z;
		}
	}

	CowardiceGate = World->SpawnActor<AT66CowardiceGate>(AT66CowardiceGate::StaticClass(), GateLoc, SpawnRotation, SpawnParams);
	if (CowardiceGate)
	{
		if (IsUsingTowerMainMapLayout())
		{
			T66TrySnapActorToTowerFloor(World, CowardiceGate, CachedTowerMainMapLayout, CachedTowerMainMapLayout.LastGameplayFloorNumber, GateLoc);
			T66AssignTowerFloorTag(CowardiceGate, CachedTowerMainMapLayout.LastGameplayFloorNumber);
		}
		else
		{
			TrySnapActorToTerrain(CowardiceGate);
		}
	}
	if (!IsUsingTowerMainMapLayout())
	{
		TricksterNPC = World->SpawnActor<AT66TricksterNPC>(AT66TricksterNPC::StaticClass(), TricksterLoc, SpawnRotation, SpawnParams);
		if (TricksterNPC)
		{
			TricksterNPC->ApplyVisuals();
			TrySnapActorToTerrain(TricksterNPC);
		}
	}
}

void AT66GameMode::SpawnCasinoInteractableIfNeeded()
{
	if (IsLabLevel())
	{
		return;
	}

	if (IsUsingTowerMainMapLayout())
	{
		return;
	}

	UWorld* World = GetWorld();
	if (!World)
	{
		return;
	}

	if (T66HasRegisteredCasino(World))
	{
		return;
	}

	const bool bUsingMainMapTerrain = T66UsesMainMapTerrainStage(World);
	float TraceStartZ = 2000.f;
	float TraceEndZ = -4000.f;
	float RefX = 0.f;
	float RefY = 0.f;
	if (bUsingMainMapTerrain)
	{
		FVector CasinoLoc = FVector::ZeroVector;
		if (!TryFindRandomMainMapSurfaceLocation(3201, CasinoLoc, 450.f))
		{
			return;
		}

		FActorSpawnParameters SpawnParams;
		SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
		World->SpawnActor<AT66CasinoInteractable>(AT66CasinoInteractable::StaticClass(), CasinoLoc, FRotator::ZeroRotator, SpawnParams);
		return;
	}

	auto FindClosestFlatSurface = [World, TraceStartZ, TraceEndZ](float InRefX, float InRefY) -> FVector
	{
		static constexpr float MinNormalZ = 0.92f;
		static constexpr float SearchRadiusMax = 2200.f;
		static constexpr float RadiusStep = 140.f;
		static constexpr int32 NumAngles = 20;

		FVector Fallback(InRefX, InRefY, 60.f);
		FHitResult Hit;
		if (World->LineTraceSingleByChannel(Hit, FVector(InRefX, InRefY, TraceStartZ), FVector(InRefX, InRefY, TraceEndZ), ECC_WorldStatic))
		{
			Fallback = Hit.ImpactPoint;
		}

		for (float R = 0.f; R <= SearchRadiusMax; R += RadiusStep)
		{
			const int32 AngleSteps = (R <= 0.f) ? 1 : NumAngles;
			for (int32 AngleIndex = 0; AngleIndex < AngleSteps; ++AngleIndex)
			{
				const float Angle = (AngleSteps == 1) ? 0.f : (2.f * PI * static_cast<float>(AngleIndex) / static_cast<float>(AngleSteps));
				const float X = InRefX + R * FMath::Cos(Angle);
				const float Y = InRefY + R * FMath::Sin(Angle);
				if (World->LineTraceSingleByChannel(Hit, FVector(X, Y, TraceStartZ), FVector(X, Y, TraceEndZ), ECC_WorldStatic) && Hit.ImpactNormal.Z >= MinNormalZ)
				{
					return Hit.ImpactPoint;
				}
			}
		}

		return Fallback;
	};

	const FVector FlatLoc = FindClosestFlatSurface(RefX, RefY);
	FActorSpawnParameters SpawnParams;
	SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
	World->SpawnActor<AT66CasinoInteractable>(AT66CasinoInteractable::StaticClass(), FlatLoc, FRotator::ZeroRotator, SpawnParams);
}

void AT66GameMode::SpawnSupportVendorAtStartIfNeeded()
{
	if (IsLabLevel())
	{
		return;
	}

	if (IsUsingTowerMainMapLayout())
	{
		return;
	}

	UWorld* World = GetWorld();
	if (!World)
	{
		return;
	}

	UT66GameInstance* T66GI = GetT66GameInstance();
	if (!T66GI)
	{
		return;
	}

	UT66PlayerExperienceSubSystem* PlayerExperience = T66GI->GetSubsystem<UT66PlayerExperienceSubSystem>();
	if (!PlayerExperience || !PlayerExperience->ShouldSpawnSupportVendorAtRunStart(T66GI->SelectedDifficulty))
	{
		return;
	}

	static const FName SupportVendorTag(TEXT("T66_StartSupportVendor"));
	if (T66FindTaggedActor(World, SupportVendorTag) != nullptr)
	{
		return;
	}

	const bool bUsingMainMapTerrain = T66UsesMainMapTerrainStage(World);
	float TraceStartZ = 2000.f;
	float TraceEndZ = -4000.f;
	FVector ReferenceLoc = FVector(-35455.f, 0.f, 60.f);
	float SideOffset = 700.f;
	if (bUsingMainMapTerrain)
	{
		FVector SpawnLoc = FVector::ZeroVector;
		if (!TryGetMainMapStartPlacementLocation(1.0f, 0.0f, SpawnLoc))
		{
			return;
		}

		FActorSpawnParameters SpawnParams;
		SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
		if (AT66VendorNPC* SupportVendor = World->SpawnActor<AT66VendorNPC>(AT66VendorNPC::StaticClass(), SpawnLoc, FRotator::ZeroRotator, SpawnParams))
		{
			SupportVendor->ConfigureAsSupportVendor(PlayerExperience->ShouldSupportVendorAllowSteal(T66GI->SelectedDifficulty));
			SupportVendor->Tags.AddUnique(SupportVendorTag);
			T66RememberTaggedActor(SupportVendor, SupportVendorTag);
			SupportVendor->ApplyVisuals();
		}
		return;
	}
	else if (AController* PC = World->GetFirstPlayerController())
	{
		if (APawn* HeroPawn = PC->GetPawn())
		{
			ReferenceLoc = HeroPawn->GetActorLocation();
		}
	}

	auto FindClosestFlatSurface = [World, TraceStartZ, TraceEndZ](const FVector& DesiredLoc) -> FVector
	{
		static constexpr float MinNormalZ = 0.92f;
		static constexpr float SearchRadiusMax = 900.f;
		static constexpr float RadiusStep = 100.f;
		static constexpr int32 NumAngles = 16;

		FVector Fallback = DesiredLoc;
		FHitResult Hit;
		if (World->LineTraceSingleByChannel(
			Hit,
			FVector(DesiredLoc.X, DesiredLoc.Y, TraceStartZ),
			FVector(DesiredLoc.X, DesiredLoc.Y, TraceEndZ),
			ECC_WorldStatic))
		{
			Fallback = Hit.ImpactPoint;
		}

		for (float Radius = 0.f; Radius <= SearchRadiusMax; Radius += RadiusStep)
		{
			const int32 AngleSteps = (Radius <= 0.f) ? 1 : NumAngles;
			for (int32 AngleIndex = 0; AngleIndex < AngleSteps; ++AngleIndex)
			{
				const float Angle = (AngleSteps == 1) ? 0.f : (2.f * PI * static_cast<float>(AngleIndex) / static_cast<float>(AngleSteps));
				const float X = DesiredLoc.X + Radius * FMath::Cos(Angle);
				const float Y = DesiredLoc.Y + Radius * FMath::Sin(Angle);
				if (World->LineTraceSingleByChannel(Hit, FVector(X, Y, TraceStartZ), FVector(X, Y, TraceEndZ), ECC_WorldStatic)
					&& Hit.ImpactNormal.Z >= MinNormalZ)
				{
					return Hit.ImpactPoint;
				}
			}
		}

		return Fallback;
	};

	const FVector DesiredLoc = ReferenceLoc + FVector(SideOffset, 0.f, 0.f);
	const FVector SpawnLoc = FindClosestFlatSurface(DesiredLoc);
	FActorSpawnParameters SpawnParams;
	SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
	if (AT66VendorNPC* SupportVendor = World->SpawnActor<AT66VendorNPC>(AT66VendorNPC::StaticClass(), SpawnLoc, FRotator::ZeroRotator, SpawnParams))
	{
		SupportVendor->ConfigureAsSupportVendor(PlayerExperience->ShouldSupportVendorAllowSteal(T66GI->SelectedDifficulty));
		SupportVendor->Tags.AddUnique(SupportVendorTag);
		T66RememberTaggedActor(SupportVendor, SupportVendorTag);
		SupportVendor->ApplyVisuals();
	}
}

void AT66GameMode::SpawnPlateauAtLocation(UWorld* World, const FVector& TopCenterLoc)
{
	if (!World) return;
	static constexpr float PlateauHalfHeight = 10.f; // top of disc at TopCenterLoc.Z
	const FVector PlateauLoc(TopCenterLoc.X, TopCenterLoc.Y, TopCenterLoc.Z - PlateauHalfHeight);
	FActorSpawnParameters P;
	P.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
	World->SpawnActor<AT66SpawnPlateau>(AT66SpawnPlateau::StaticClass(), PlateauLoc, FRotator::ZeroRotator, P);
}

void AT66GameMode::SpawnGuaranteedStartAreaInteractables()
{
	if (IsLabLevel() || IsUsingTowerMainMapLayout())
	{
		return;
	}

	UWorld* World = GetWorld();
	if (!World)
	{
		return;
	}

	UT66GameInstance* T66GI = GetT66GameInstance();
	if (!T66GI)
	{
		return;
	}

	UT66RunStateSubsystem* RunState = T66GI->GetSubsystem<UT66RunStateSubsystem>();
	UT66PlayerExperienceSubSystem* PlayerExperience = T66GI->GetSubsystem<UT66PlayerExperienceSubSystem>();
	UT66RngSubsystem* RngSub = T66GI->GetSubsystem<UT66RngSubsystem>();
	if (!RunState)
	{
		return;
	}

	if (RngSub)
	{
		RngSub->UpdateLuckStat(RunState->GetEffectiveLuckBiasStat());
	}

	const ET66Difficulty Difficulty = T66GI->SelectedDifficulty;
	const bool bUsingMainMapTerrain = T66UsesMainMapTerrainStage(World);
	const int32 RunSeed = T66EnsureRunSeed(T66GI);
	const int32 StageNum = RunState->GetCurrentStage();
	FRandomStream Rng(RunSeed + StageNum * 1337 + 807);

	FVector StartCenter = FVector::ZeroVector;
	if (bUsingMainMapTerrain)
	{
		StartCenter = MainMapStartAreaCenterSurfaceLocation.IsNearlyZero()
			? MainMapSpawnSurfaceLocation
			: MainMapStartAreaCenterSurfaceLocation;
	}
	else
	{
		StartCenter = T66GameplayLayout::GetStartAreaCenter();

		FHitResult Hit;
		const FVector TraceStart = StartCenter + FVector(0.f, 0.f, 3000.f);
		const FVector TraceEnd = StartCenter - FVector(0.f, 0.f, 9000.f);
		if (World->LineTraceSingleByChannel(Hit, TraceStart, TraceEnd, ECC_WorldStatic)
			|| World->LineTraceSingleByChannel(Hit, TraceStart, TraceEnd, ECC_Visibility))
		{
			StartCenter = Hit.ImpactPoint;
		}
	}

	auto FindExistingTaggedActor = [World](const FName Tag) -> AActor*
	{
		return T66FindTaggedActor(World, Tag);
	};

	auto FindClassicStartAreaSurface = [World](const FVector& DesiredLoc) -> FVector
	{
		static constexpr float TraceStartZ = 8000.f;
		static constexpr float TraceEndZ = -16000.f;
		static constexpr float MinNormalZ = 0.92f;
		static constexpr float SearchRadiusMax = 650.f;
		static constexpr float RadiusStep = 100.f;
		static constexpr int32 NumAngles = 12;

		FVector Fallback = DesiredLoc;
		FHitResult Hit;
		if (World->LineTraceSingleByChannel(
			Hit,
			FVector(DesiredLoc.X, DesiredLoc.Y, TraceStartZ),
			FVector(DesiredLoc.X, DesiredLoc.Y, TraceEndZ),
			ECC_WorldStatic))
		{
			Fallback = Hit.ImpactPoint;
		}

		for (float Radius = 0.f; Radius <= SearchRadiusMax; Radius += RadiusStep)
		{
			const int32 AngleSteps = (Radius <= 0.f) ? 1 : NumAngles;
			for (int32 AngleIndex = 0; AngleIndex < AngleSteps; ++AngleIndex)
			{
				const float Angle = (AngleSteps == 1) ? 0.f : (2.f * PI * static_cast<float>(AngleIndex) / static_cast<float>(AngleSteps));
				const float X = DesiredLoc.X + Radius * FMath::Cos(Angle);
				const float Y = DesiredLoc.Y + Radius * FMath::Sin(Angle);
				if (World->LineTraceSingleByChannel(Hit, FVector(X, Y, TraceStartZ), FVector(X, Y, TraceEndZ), ECC_WorldStatic)
					&& Hit.ImpactNormal.Z >= MinNormalZ)
				{
					return Hit.ImpactPoint;
				}
			}
		}

		return Fallback;
	};

	auto ResolveStartSpawnLocation = [&](const FVector& ClassicOffset, float SideCells, float InwardCells, FVector& OutLocation) -> bool
	{
		if (bUsingMainMapTerrain)
		{
			return TryGetMainMapStartPlacementLocation(SideCells, InwardCells, OutLocation);
		}

		OutLocation = FindClassicStartAreaSurface(StartCenter + ClassicOffset);
		return true;
	};

	auto BuildFacingRotation = [&](const FVector& SpawnLocation) -> FRotator
	{
		FRotator FacingRotation = FRotator::ZeroRotator;
		if (!T66TryBuildFacingRotation2D(SpawnLocation, StartCenter, FacingRotation))
		{
			FacingRotation = FRotator::ZeroRotator;
		}
		return FacingRotation;
	};

	auto RollWeightedRarity = [&](const FT66RarityWeights& Weights) -> ET66Rarity
	{
		return (RngSub && PlayerExperience)
			? RngSub->RollRarityWeighted(Weights, Rng)
			: FT66RarityUtil::RollDefaultRarity(Rng);
	};

	enum class EGuaranteedStartInteractable : uint8
	{
		QuickRevive,
		Fountain,
		Chest,
		Wheel,
		LootBag,
		Crate,
		ArcadeTruck,
		ArcadeMachine,
	};

	struct FGuaranteedStartSpec
	{
		EGuaranteedStartInteractable Kind;
		const TCHAR* TagName = nullptr;
		FVector ClassicOffset = FVector::ZeroVector;
		float SideCells = 0.f;
		float InwardCells = 0.f;
	};

	const FGuaranteedStartSpec SpawnSpecs[] = {
		{ EGuaranteedStartInteractable::QuickRevive, TEXT("T66_StartGuaranteed_QuickRevive"), FVector(-900.f, -1400.f, 0.f), -1.10f, -0.35f },
		{ EGuaranteedStartInteractable::Fountain,    TEXT("T66_StartGuaranteed_Fountain"),    FVector(-900.f,  1400.f, 0.f), -1.10f,  0.75f },
		{ EGuaranteedStartInteractable::Chest,       TEXT("T66_StartGuaranteed_Chest"),       FVector(   0.f,  1800.f, 0.f), -0.20f,  1.30f },
		{ EGuaranteedStartInteractable::LootBag,     TEXT("T66_StartGuaranteed_LootBag"),     FVector( 900.f, -1400.f, 0.f),  0.35f,  0.45f },
		{ EGuaranteedStartInteractable::Crate,       TEXT("T66_StartGuaranteed_Crate"),       FVector(   0.f, -1800.f, 0.f),  0.00f,  0.55f },
		{ EGuaranteedStartInteractable::ArcadeTruck,   TEXT("T66_StartGuaranteed_ArcadeTruck"),   FVector(1500.f,   900.f, 0.f),  0.95f, -0.05f },
		{ EGuaranteedStartInteractable::ArcadeMachine, TEXT("T66_StartGuaranteed_ArcadeMachine"), FVector(1500.f,  -900.f, 0.f),  1.05f,  0.70f },
	};

	FActorSpawnParameters SpawnParams;
	SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

	for (const FGuaranteedStartSpec& Spec : SpawnSpecs)
	{
		if (Spec.TagName == nullptr)
		{
			continue;
		}

		const FName Tag(Spec.TagName);
		if (FindExistingTaggedActor(Tag))
		{
			continue;
		}

		FVector SpawnLoc = FVector::ZeroVector;
		if (!ResolveStartSpawnLocation(Spec.ClassicOffset, Spec.SideCells, Spec.InwardCells, SpawnLoc))
		{
			continue;
		}

		const FRotator SpawnRotation = BuildFacingRotation(SpawnLoc);
		AActor* SpawnedActor = nullptr;

		switch (Spec.Kind)
		{
		case EGuaranteedStartInteractable::QuickRevive:
			SpawnedActor = World->SpawnActor<AT66QuickReviveVendingMachine>(
				AT66QuickReviveVendingMachine::StaticClass(), SpawnLoc, SpawnRotation, SpawnParams);
			break;

		case EGuaranteedStartInteractable::Fountain:
			SpawnedActor = World->SpawnActor<AT66FountainOfLifeInteractable>(
				AT66FountainOfLifeInteractable::StaticClass(), SpawnLoc, SpawnRotation, SpawnParams);
			break;

		case EGuaranteedStartInteractable::Chest:
			if (AT66ChestInteractable* Chest = World->SpawnActor<AT66ChestInteractable>(
				AT66ChestInteractable::StaticClass(), SpawnLoc, SpawnRotation, SpawnParams))
			{
				Chest->bIsMimic = false;
				Chest->SetRarity(ET66Rarity::Yellow);
				SpawnedActor = Chest;
			}
			break;

		case EGuaranteedStartInteractable::Wheel:
			if (AT66WheelSpinInteractable* Wheel = World->SpawnActor<AT66WheelSpinInteractable>(
				AT66WheelSpinInteractable::StaticClass(), SpawnLoc, SpawnRotation, SpawnParams))
			{
				const FT66RarityWeights Weights = PlayerExperience
					? PlayerExperience->GetDifficultyWheelRarityWeights(Difficulty)
					: FT66RarityWeights{};
				const bool bWheelRarityReplayable = (RngSub && PlayerExperience);
				const ET66Rarity WheelRarity = RollWeightedRarity(Weights);
				const int32 WheelRarityDrawIndex = bWheelRarityReplayable ? RngSub->GetLastRunDrawIndex() : INDEX_NONE;
				const int32 WheelRarityPreDrawSeed = bWheelRarityReplayable ? RngSub->GetLastRunPreDrawSeed() : 0;
				Wheel->SetRarity(WheelRarity);
				if (RunState)
				{
					RunState->RecordLuckQualityRarity(FName(TEXT("WheelRarity")), WheelRarity, WheelRarityDrawIndex, WheelRarityPreDrawSeed, bWheelRarityReplayable ? &Weights : nullptr);
				}
				SpawnedActor = Wheel;
			}
			break;

		case EGuaranteedStartInteractable::LootBag:
			if (AT66LootBagPickup* LootBag = World->SpawnActor<AT66LootBagPickup>(
				AT66LootBagPickup::StaticClass(), SpawnLoc, SpawnRotation, SpawnParams))
			{
				const FT66RarityWeights Weights = PlayerExperience
					? PlayerExperience->GetDifficultyEnemyLootBagRarityWeights(Difficulty)
					: FT66RarityWeights{};
				const bool bBagRarityReplayable = (RngSub && PlayerExperience);
				const ET66Rarity BagRarity = RollWeightedRarity(Weights);
				LootBag->SetLootRarity(BagRarity);
				LootBag->SetItemID(T66GI->GetRandomItemIDForLootRarityFromStream(BagRarity, Rng));
				if (RunState)
				{
					RunState->RecordLuckQualityRarity(
						FName(TEXT("GuaranteedStartLootBagRarity")),
						BagRarity,
						bBagRarityReplayable ? RngSub->GetLastRunDrawIndex() : INDEX_NONE,
						bBagRarityReplayable ? RngSub->GetLastRunPreDrawSeed() : 0,
						bBagRarityReplayable ? &Weights : nullptr);
				}
				SpawnedActor = LootBag;
			}
			break;

		case EGuaranteedStartInteractable::Crate:
			if (AT66CrateInteractable* Crate = World->SpawnActor<AT66CrateInteractable>(
				AT66CrateInteractable::StaticClass(), SpawnLoc, SpawnRotation, SpawnParams))
			{
				const FT66RarityWeights Weights = PlayerExperience
					? PlayerExperience->GetDifficultyCrateRarityWeights(Difficulty)
					: FT66RarityWeights{};
				const bool bCrateRarityReplayable = (RngSub && PlayerExperience);
				const ET66Rarity CrateRarity = RollWeightedRarity(Weights);
				const int32 CrateRarityDrawIndex = bCrateRarityReplayable ? RngSub->GetLastRunDrawIndex() : INDEX_NONE;
				const int32 CrateRarityPreDrawSeed = bCrateRarityReplayable ? RngSub->GetLastRunPreDrawSeed() : 0;
				Crate->SetRarity(CrateRarity);
				if (RunState)
				{
					RunState->RecordLuckQualityRarity(FName(TEXT("CrateRarity")), CrateRarity, CrateRarityDrawIndex, CrateRarityPreDrawSeed, bCrateRarityReplayable ? &Weights : nullptr);
				}
				SpawnedActor = Crate;
			}
			break;

		case EGuaranteedStartInteractable::ArcadeTruck:
			SpawnedActor = World->SpawnActor<AT66ArcadeTruckInteractable>(
				AT66ArcadeTruckInteractable::StaticClass(), SpawnLoc, SpawnRotation, SpawnParams);
			break;

		case EGuaranteedStartInteractable::ArcadeMachine:
			SpawnedActor = World->SpawnActor<AT66ArcadeMachineInteractable>(
				AT66ArcadeMachineInteractable::StaticClass(), SpawnLoc, SpawnRotation, SpawnParams);
			break;
		}

		if (SpawnedActor)
		{
			SpawnedActor->Tags.AddUnique(Tag);
			T66RememberTaggedActor(SpawnedActor, Tag);
		}
	}
}

void AT66GameMode::SpawnWorldInteractablesForStage()
{
	if (bWorldInteractablesSpawnedForStage) return;

	UWorld* World = GetWorld();
	if (!World) return;

	UGameInstance* GI = GetGameInstance();
	UT66RunStateSubsystem* RunState = GI ? GI->GetSubsystem<UT66RunStateSubsystem>() : nullptr;
	if (!RunState) return;
	UT66RngSubsystem* RngSub = GI ? GI->GetSubsystem<UT66RngSubsystem>() : nullptr;
	UT66GameInstance* T66GI = GetT66GameInstance();

	// Run-level seed so positions change every time "Enter the Tribulation" or PIE is started (like procedural terrain).
	const int32 RunSeed = T66EnsureRunSeed(T66GI);
	const int32 StageNum = RunState->GetCurrentStage();
	FRandomStream Rng(RunSeed + StageNum * 1337 + 42);
	const bool bUsingMainMapTerrain = T66UsesMainMapTerrainStage(World);
	const bool bTowerLayout = IsUsingTowerMainMapLayout();
	if (bTowerLayout && IsBossRushFinaleStage())
	{
		bWorldInteractablesSpawnedForStage = true;
		return;
	}
	TArray<int32> TowerGameplayFloorNumbers;
	TMap<int32, int32> TowerChestCountByFloor;
	TMap<int32, int32> TowerCrateCountByFloor;
	if (bTowerLayout)
	{
		FRandomStream TowerCountRng(RunSeed + StageNum * 1901 + 39);
		for (const T66TowerMapTerrain::FFloor& Floor : CachedTowerMainMapLayout.Floors)
		{
			if (!Floor.bGameplayFloor
				|| Floor.FloorNumber < CachedTowerMainMapLayout.FirstGameplayFloorNumber
				|| Floor.FloorNumber > CachedTowerMainMapLayout.LastGameplayFloorNumber)
			{
				continue;
			}

			TowerGameplayFloorNumbers.Add(Floor.FloorNumber);
			TowerChestCountByFloor.Add(Floor.FloorNumber, TowerCountRng.RandRange(1, 3));
			TowerCrateCountByFloor.Add(Floor.FloorNumber, TowerCountRng.RandRange(1, 3));
		}
		TowerGameplayFloorNumbers.Sort();
	}

	FT66MapPreset MainMapPreset;
	T66MainMapTerrain::FSettings MainMapSettings;
	float MainHalfExtent = 50000.f;
	float TraceStartZ = 8000.f;
	float TraceEndZ = -16000.f;
	if (bUsingMainMapTerrain && !bTowerLayout)
	{
		MainMapPreset = T66BuildMainMapPreset(T66GI);
		MainMapSettings = T66MainMapTerrain::MakeSettings(MainMapPreset);
		MainHalfExtent = FMath::Max(0.0f, MainMapSettings.HalfExtent - MainMapSettings.CellSize * 1.25f);
		TraceStartZ = T66MainMapTerrain::GetTraceZ(MainMapPreset);
		TraceEndZ = T66MainMapTerrain::GetLowestCollisionBottomZ(MainMapPreset) - MainMapSettings.StepHeight;
	}

	static constexpr float SpawnZ = 220.f;
	static constexpr float MinDistBetweenInteractables = 900.f;
	static constexpr float SafeBubbleMargin = 250.f;

	TArray<FVector> UsedLocs;

	// No-spawn zones: keep gameplay spawns out of reserved traversal spaces and special arenas.
	auto IsInsideNoSpawnZone = [&](const FVector& L) -> bool
	{
		if (bTowerLayout)
		{
			return false;
		}

		if (bUsingMainMapTerrain)
		{
			const float RoomReserveRadius = MainMapSettings.CellSize * T66MainMapRoomReserveRadiusCells;
			const float CorridorReserveRadius = MainMapSettings.CellSize * T66MainMapCorridorReserveRadiusCells;
			if (!MainMapStartAreaCenterSurfaceLocation.IsNearlyZero()
				&& FVector::DistSquared2D(L, MainMapStartAreaCenterSurfaceLocation) < FMath::Square(RoomReserveRadius))
			{
				return true;
			}
			if (!MainMapBossAreaCenterSurfaceLocation.IsNearlyZero()
				&& FVector::DistSquared2D(L, MainMapBossAreaCenterSurfaceLocation) < FMath::Square(RoomReserveRadius))
			{
				return true;
			}
			if (!MainMapStartPathSurfaceLocation.IsNearlyZero()
				&& FVector::DistSquared2D(L, MainMapStartPathSurfaceLocation) < FMath::Square(CorridorReserveRadius))
			{
				return true;
			}
			if (!MainMapStartAnchorSurfaceLocation.IsNearlyZero()
				&& FVector::DistSquared2D(L, MainMapStartAnchorSurfaceLocation) < FMath::Square(CorridorReserveRadius))
			{
				return true;
			}
			if (!MainMapBossAnchorSurfaceLocation.IsNearlyZero()
				&& FVector::DistSquared2D(L, MainMapBossAnchorSurfaceLocation) < FMath::Square(CorridorReserveRadius))
			{
				return true;
			}
			return false;
		}

		if (T66GameplayLayout::IsInsideReservedTraversalZone2D(L, 455.f))
		{
			return true;
		}

		static constexpr float ArenaHalf = 9091.f;
		static constexpr float ArenaMargin = 682.f;
		static constexpr float TutorialArenaHalf = 9091.f;
		struct FArena2D { float X; float Y; float Half; };
		static constexpr FArena2D Arenas[] = {
			{      0.f, 61364.f, TutorialArenaHalf }, // Tutorial
		};
		for (const FArena2D& A : Arenas)
		{
			if (FMath::Abs(L.X - A.X) <= (A.Half + ArenaMargin) && FMath::Abs(L.Y - A.Y) <= (A.Half + ArenaMargin))
			{
				return true;
			}
		}

		return false;
	};

	auto IsSameTowerFloor = [&](const FVector& A, const FVector& B) -> bool
	{
		if (!bTowerLayout)
		{
			return true;
		}

		const int32 FloorA = GetTowerFloorIndexForLocation(A);
		const int32 FloorB = GetTowerFloorIndexForLocation(B);
		return FloorA != INDEX_NONE && FloorA == FloorB;
	};

	auto ResolveTowerFloorForActor = [&](const AActor* Actor) -> int32
	{
		if (!bTowerLayout || !Actor)
		{
			return INDEX_NONE;
		}

		const int32 TaggedFloor = T66ReadTowerFloorTag(Actor);
		if (TaggedFloor != INDEX_NONE)
		{
			return TaggedFloor;
		}

		return GetTowerFloorIndexForLocation(Actor->GetActorLocation());
	};

	auto IsGoodLoc = [&](const FVector& L) -> bool
	{
		if (IsInsideNoSpawnZone(L))
		{
			return false;
		}
		static constexpr float CasinoKeepClearRadius = 2200.f;
		if (!bUsingMainMapTerrain && FVector::DistSquared2D(L, FVector::ZeroVector) < (CasinoKeepClearRadius * CasinoKeepClearRadius))
		{
			return false;
		}
		for (const FVector& U : UsedLocs)
		{
			if (!IsSameTowerFloor(L, U))
			{
				continue;
			}
			if (FVector::DistSquared2D(L, U) < (MinDistBetweenInteractables * MinDistBetweenInteractables))
			{
				return false;
			}
		}
		UT66ActorRegistrySubsystem* Registry = World ? World->GetSubsystem<UT66ActorRegistrySubsystem>() : nullptr;
		const TArray<TWeakObjectPtr<AT66HouseNPCBase>>& NPCs = Registry ? Registry->GetNPCs() : TArray<TWeakObjectPtr<AT66HouseNPCBase>>();
		for (const TWeakObjectPtr<AT66HouseNPCBase>& WeakNPC : NPCs)
		{
			const AT66HouseNPCBase* NPC = WeakNPC.Get();
			if (!NPC) continue;
			if (bTowerLayout)
			{
				const int32 CandidateFloor = GetTowerFloorIndexForLocation(L);
				const int32 NPCFloor = ResolveTowerFloorForActor(NPC);
				if (CandidateFloor == INDEX_NONE || NPCFloor == INDEX_NONE || CandidateFloor != NPCFloor)
				{
					continue;
				}
			}
			else if (!IsSameTowerFloor(L, NPC->GetActorLocation()))
			{
				continue;
			}
			const float R = NPC->GetSafeZoneRadius() + SafeBubbleMargin;
			if (FVector::DistSquared2D(L, NPC->GetActorLocation()) < (R * R))
			{
				return false;
			}
		}
		const TArray<TWeakObjectPtr<AT66CasinoInteractable>>& Casinos = Registry ? Registry->GetCasinos() : TArray<TWeakObjectPtr<AT66CasinoInteractable>>();
		for (const TWeakObjectPtr<AT66CasinoInteractable>& WeakCasino : Casinos)
		{
			const AT66CasinoInteractable* Casino = WeakCasino.Get();
			if (!Casino) continue;
			if (bTowerLayout)
			{
				const int32 CandidateFloor = GetTowerFloorIndexForLocation(L);
				const int32 CasinoFloor = ResolveTowerFloorForActor(Casino);
				if (CandidateFloor == INDEX_NONE || CasinoFloor == INDEX_NONE || CandidateFloor != CasinoFloor)
				{
					continue;
				}
			}
			else if (!IsSameTowerFloor(L, Casino->GetActorLocation()))
			{
				continue;
			}
			const float R = Casino->GetSafeZoneRadius() + SafeBubbleMargin;
			if (FVector::DistSquared2D(L, Casino->GetActorLocation()) < (R * R))
			{
				return false;
			}
		}
		return true;
	};

	struct FSpawnHitResult
	{
		FVector Loc = FVector::ZeroVector;
		bool bFound = false;
	};
	auto FindSpawnLoc = [&]() -> FSpawnHitResult
	{
		for (int32 Try = 0; Try < 40; ++Try)
		{
			FVector Loc = FVector::ZeroVector;
			if (bTowerLayout)
			{
				if (!T66TowerMapTerrain::TryGetRandomGameplaySurfaceLocation(World, CachedTowerMainMapLayout, Rng, Loc))
				{
					continue;
				}
			}
			else
			{
				const float X = Rng.FRandRange(-MainHalfExtent, MainHalfExtent);
				const float Y = Rng.FRandRange(-MainHalfExtent, MainHalfExtent);
				Loc = FVector(X, Y, SpawnZ);

				FHitResult Hit;
				const FVector Start(X, Y, TraceStartZ);
				const FVector End(X, Y, TraceEndZ);
				if (!World->LineTraceSingleByChannel(Hit, Start, End, ECC_WorldStatic))
				{
					continue;
				}
				if (!T66GameplayLayout::IsValidGameplayGroundNormal(Hit.ImpactNormal))
				{
					continue;
				}
				Loc = Hit.ImpactPoint;
			}

			if (IsGoodLoc(Loc))
			{
				return { Loc, true };
			}
		}
		return {};
	};

	auto SpawnOne = [&](UClass* Cls) -> AActor*
	{
		FActorSpawnParameters P;
		P.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
		const FSpawnHitResult HitResult = FindSpawnLoc();
		if (!HitResult.bFound)
		{
			return nullptr;
		}
		AActor* A = World->SpawnActor<AActor>(Cls, HitResult.Loc, FRotator::ZeroRotator, P);
		if (A)
		{
			if (bTowerLayout)
			{
				const int32 FloorNumber = GetTowerFloorIndexForLocation(HitResult.Loc);
				if (FloorNumber != INDEX_NONE)
				{
					T66TrySnapActorToTowerFloor(World, A, CachedTowerMainMapLayout, FloorNumber, HitResult.Loc);
					T66AssignTowerFloorTag(A, FloorNumber);
				}
			}
			UsedLocs.Add(HitResult.Loc);
		}
		return A;
	};

	if (RngSub)
	{
		RngSub->UpdateLuckStat(RunState->GetEffectiveLuckBiasStat());
	}

	const UT66RngTuningConfig* Tuning = RngSub ? RngSub->GetTuning() : nullptr;
	UT66PlayerExperienceSubSystem* PlayerExperience = T66GI ? T66GI->GetSubsystem<UT66PlayerExperienceSubSystem>() : nullptr;
	const ET66Difficulty Difficulty = T66GI ? T66GI->SelectedDifficulty : ET66Difficulty::Easy;
	const FT66IntRange ChestCountRange = PlayerExperience
		? PlayerExperience->GetDifficultyChestCountRange(Difficulty)
		: FT66IntRange{ 4, 10 };
	const FT66IntRange WheelCountRange = T66EnableWheelSpinSpawns
		? (PlayerExperience
			? PlayerExperience->GetDifficultyWheelCountRange(Difficulty)
			: FT66IntRange{ 5, 11 })
		: FT66IntRange{ 0, 0 };
	const FT66IntRange CrateCountRange = PlayerExperience
		? PlayerExperience->GetDifficultyCrateCountRange(Difficulty)
		: FT66IntRange{ 3, 6 };

	// Luck-affected counts use central tuning. Locations are still stage-seeded (not luck-affected).
	int32 CountFountains = 0;
	int32 FountainsDrawIndex = INDEX_NONE;
	int32 FountainsPreDrawSeed = 0;
	TSet<int32> TowerFountainFloorNumbers;
	if (bTowerLayout)
	{
		static constexpr float TowerFountainChancePerFloor = 0.40f;
		FRandomStream TowerFountainRng(RunSeed + StageNum * 1901 + 38);
		FountainsPreDrawSeed = RunSeed + StageNum * 1901 + 38;
		for (const int32 FloorNumber : TowerGameplayFloorNumbers)
		{
			if (TowerFountainRng.GetFraction() < TowerFountainChancePerFloor)
			{
				TowerFountainFloorNumbers.Add(FloorNumber);
			}
		}
		CountFountains = TowerFountainFloorNumbers.Num();
	}
	else
	{
		CountFountains = (RngSub && Tuning) ? RngSub->RollIntRangeBiased(Tuning->TreesPerStage, Rng) : Rng.RandRange(2, 5);
		FountainsDrawIndex = (RngSub && Tuning) ? RngSub->GetLastRunDrawIndex() : INDEX_NONE;
		FountainsPreDrawSeed = (RngSub && Tuning) ? RngSub->GetLastRunPreDrawSeed() : 0;
	}
	int32 CountChests = 0;
	int32 ChestsDrawIndex = INDEX_NONE;
	int32 ChestsPreDrawSeed = 0;
	if (bTowerLayout)
	{
		for (const int32 FloorNumber : TowerGameplayFloorNumbers)
		{
			CountChests += TowerChestCountByFloor.FindRef(FloorNumber);
		}
		ChestsPreDrawSeed = RunSeed + StageNum * 1901 + 39;
	}
	else
	{
		CountChests = RngSub ? RngSub->RollIntRangeBiased(ChestCountRange, Rng) : Rng.RandRange(FMath::Min(ChestCountRange.Min, ChestCountRange.Max), FMath::Max(ChestCountRange.Min, ChestCountRange.Max));
		ChestsDrawIndex = RngSub ? RngSub->GetLastRunDrawIndex() : INDEX_NONE;
		ChestsPreDrawSeed = RngSub ? RngSub->GetLastRunPreDrawSeed() : 0;
	}
	int32 CountWheels = 0;
	int32 WheelsDrawIndex = INDEX_NONE;
	int32 WheelsPreDrawSeed = 0;
	if (!bTowerLayout)
	{
		CountWheels = RngSub ? RngSub->RollIntRangeBiased(WheelCountRange, Rng) : Rng.RandRange(FMath::Min(WheelCountRange.Min, WheelCountRange.Max), FMath::Max(WheelCountRange.Min, WheelCountRange.Max));
		WheelsDrawIndex = RngSub ? RngSub->GetLastRunDrawIndex() : INDEX_NONE;
		WheelsPreDrawSeed = RngSub ? RngSub->GetLastRunPreDrawSeed() : 0;
	}
	int32 CountCrates = 0;
	int32 CratesDrawIndex = INDEX_NONE;
	int32 CratesPreDrawSeed = 0;
	if (bTowerLayout)
	{
		for (const int32 FloorNumber : TowerGameplayFloorNumbers)
		{
			CountCrates += TowerCrateCountByFloor.FindRef(FloorNumber);
		}
		CratesPreDrawSeed = RunSeed + StageNum * 1901 + 40;
	}
	else
	{
		CountCrates = RngSub ? RngSub->RollIntRangeBiased(CrateCountRange, Rng) : Rng.RandRange(FMath::Min(CrateCountRange.Min, CrateCountRange.Max), FMath::Max(CrateCountRange.Min, CrateCountRange.Max));
		CratesDrawIndex = RngSub ? RngSub->GetLastRunDrawIndex() : INDEX_NONE;
		CratesPreDrawSeed = RngSub ? RngSub->GetLastRunPreDrawSeed() : 0;
	}

	// Luck Rating tracking (quantity).
	if (RunState)
	{
		const int32 FountainsMin = bTowerLayout ? 0 : (Tuning ? Tuning->TreesPerStage.Min : 2);
		const int32 FountainsMax = bTowerLayout ? TowerGameplayFloorNumbers.Num() : (Tuning ? Tuning->TreesPerStage.Max : 5);
		const int32 ChestsMin = bTowerLayout ? TowerGameplayFloorNumbers.Num() : FMath::Min(ChestCountRange.Min, ChestCountRange.Max);
		const int32 ChestsMax = bTowerLayout ? (TowerGameplayFloorNumbers.Num() * 3) : FMath::Max(ChestCountRange.Min, ChestCountRange.Max);
		const int32 WheelsMin = bTowerLayout ? 0 : FMath::Min(WheelCountRange.Min, WheelCountRange.Max);
		const int32 WheelsMax = bTowerLayout ? 0 : FMath::Max(WheelCountRange.Min, WheelCountRange.Max);
		const int32 CratesMin = bTowerLayout ? TowerGameplayFloorNumbers.Num() : FMath::Min(CrateCountRange.Min, CrateCountRange.Max);
		const int32 CratesMax = bTowerLayout ? (TowerGameplayFloorNumbers.Num() * 3) : FMath::Max(CrateCountRange.Min, CrateCountRange.Max);
		RunState->RecordLuckQuantityRoll(FName(TEXT("FountainsPerStage")), CountFountains, FountainsMin, FountainsMax, FountainsDrawIndex, FountainsPreDrawSeed);
		RunState->RecordLuckQuantityRoll(FName(TEXT("ChestsPerStage")), CountChests, ChestsMin, ChestsMax, ChestsDrawIndex, ChestsPreDrawSeed);
		RunState->RecordLuckQuantityRoll(FName(TEXT("WheelsPerStage")), CountWheels, WheelsMin, WheelsMax, WheelsDrawIndex, WheelsPreDrawSeed);
		RunState->RecordLuckQuantityRoll(FName(TEXT("CratesPerStage")), CountCrates, CratesMin, CratesMax, CratesDrawIndex, CratesPreDrawSeed);
	}

	const int32 CountTotems = bTowerLayout ? TowerGameplayFloorNumbers.Num() : 0;
	int32 RemainingFountains = CountFountains;
	int32 RemainingChests = CountChests;
	int32 RemainingWheels = CountWheels;
	int32 RemainingCrates = CountCrates;
	int32 RemainingTotems = CountTotems;

	auto ConfigureChest = [&](AT66ChestInteractable* Chest)
	{
		if (!Chest)
		{
			return;
		}

		const float ChestMimicChance = PlayerExperience ? PlayerExperience->GetDifficultyChestMimicChance(Difficulty) : 0.20f;
		Chest->bIsMimic = RngSub ? RngSub->RollChance01(ChestMimicChance) : (Rng.GetFraction() < ChestMimicChance);
		Chest->SetRarity(ET66Rarity::Yellow);
		if (RunState)
		{
			RunState->RecordLuckQuantityBool(
				FName(TEXT("ChestMimicAvoided")),
				!Chest->bIsMimic,
				1.f - FMath::Clamp(ChestMimicChance, 0.f, 1.f));
		}
	};

	auto ConfigureWheel = [&](AT66WheelSpinInteractable* Wheel)
	{
		if (!Wheel)
		{
			return;
		}

		const FT66RarityWeights Weights = PlayerExperience
			? PlayerExperience->GetDifficultyWheelRarityWeights(Difficulty)
			: FT66RarityWeights{};
		const bool bWheelRarityReplayable = (RngSub && PlayerExperience);
		const ET66Rarity Rarity = bWheelRarityReplayable ? RngSub->RollRarityWeighted(Weights, Rng) : FT66RarityUtil::RollDefaultRarity(Rng);
		const int32 WheelRarityDrawIndex = bWheelRarityReplayable ? RngSub->GetLastRunDrawIndex() : INDEX_NONE;
		const int32 WheelRarityPreDrawSeed = bWheelRarityReplayable ? RngSub->GetLastRunPreDrawSeed() : 0;
		Wheel->SetRarity(Rarity);
		if (RunState)
		{
			RunState->RecordLuckQualityRarity(FName(TEXT("WheelRarity")), Rarity, WheelRarityDrawIndex, WheelRarityPreDrawSeed, bWheelRarityReplayable ? &Weights : nullptr);
		}
	};

	auto ConfigureCrate = [&](AT66CrateInteractable* Crate)
	{
		if (!Crate)
		{
			return;
		}

		const FT66RarityWeights Weights = PlayerExperience
			? PlayerExperience->GetDifficultyCrateRarityWeights(Difficulty)
			: FT66RarityWeights{};
		const bool bCrateRarityReplayable = (RngSub && PlayerExperience);
		const ET66Rarity Rarity = bCrateRarityReplayable ? RngSub->RollRarityWeighted(Weights, Rng) : FT66RarityUtil::RollDefaultRarity(Rng);
		const int32 CrateRarityDrawIndex = bCrateRarityReplayable ? RngSub->GetLastRunDrawIndex() : INDEX_NONE;
		const int32 CrateRarityPreDrawSeed = bCrateRarityReplayable ? RngSub->GetLastRunPreDrawSeed() : 0;
		Crate->SetRarity(Rarity);
		if (RunState)
		{
			RunState->RecordLuckQualityRarity(FName(TEXT("CrateRarity")), Rarity, CrateRarityDrawIndex, CrateRarityPreDrawSeed, bCrateRarityReplayable ? &Weights : nullptr);
		}
	};

	auto ConfigureTotem = [&](AT66DifficultyTotem* Totem)
	{
		if (Totem)
		{
			Totem->SetRarity(FT66RarityUtil::RollDefaultRarity(Rng));
		}
	};

	auto FindExistingTaggedActor = [&](const FName Tag) -> AActor*
	{
		return T66FindTaggedActor(World, Tag);
	};

	if (bTowerLayout)
	{
		FRandomStream TowerFloorRng(RunSeed + StageNum * 1901 + 77);
		TArray<int32> GameplayFloorNumbers = TowerGameplayFloorNumbers;
		const int32 QuickReviveFloorNumber = CachedTowerMainMapLayout.FirstGameplayFloorNumber;
		TSet<FName> ExistingTowerOccupantTags;
		for (TActorIterator<AActor> It(World); It; ++It)
		{
			if (AActor* ExistingActor = *It)
			{
				for (const FName& ExistingTag : ExistingActor->Tags)
				{
					ExistingTowerOccupantTags.Add(ExistingTag);
				}
			}
		}

		auto ResnapTowerActorToFloor = [&](AActor* Actor, const int32 FloorNumber) -> bool
		{
			if (!Actor)
			{
				return false;
			}

			T66TrySnapActorToTowerFloor(World, Actor, CachedTowerMainMapLayout, FloorNumber, Actor->GetActorLocation());
			T66AssignTowerFloorTag(Actor, FloorNumber);
			const int32 ResolvedFloor = GetTowerFloorIndexForLocation(Actor->GetActorLocation());
			return ResolvedFloor == INDEX_NONE || ResolvedFloor == FloorNumber;
		};

		auto TryFindTowerFloorLocation = [&](const int32 FloorNumber, const int32 SeedOffset, const float EdgePadding, const float HolePadding, FVector& OutLocation) -> bool
		{
			FRandomStream FloorRng(RunSeed + StageNum * 1901 + SeedOffset + FloorNumber * 53);
			for (int32 Attempt = 0; Attempt < 24; ++Attempt)
			{
				if (!T66TowerMapTerrain::TryGetRandomSurfaceLocationOnFloor(
					World,
					CachedTowerMainMapLayout,
					FloorNumber,
					FloorRng,
					OutLocation,
					EdgePadding,
					HolePadding))
				{
					continue;
				}

				if (GetTowerFloorIndexForLocation(OutLocation) != FloorNumber)
				{
					continue;
				}

				if (IsGoodLoc(OutLocation))
				{
					return true;
				}
			}

			return false;
		};

		FActorSpawnParameters OccupantSpawnParams;
		OccupantSpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
		auto SpawnTowerActorOnFloor = [&](UClass* Cls, const int32 FloorNumber, const int32 SeedOffset, const float EdgePadding, const float HolePadding) -> AActor*
		{
			for (int32 SpawnAttempt = 0; SpawnAttempt < 5; ++SpawnAttempt)
			{
				FVector SpawnLoc = FVector::ZeroVector;
				if (!TryFindTowerFloorLocation(FloorNumber, SeedOffset + (SpawnAttempt * 97), EdgePadding, HolePadding, SpawnLoc))
				{
					continue;
				}

				AActor* SpawnedActor = World->SpawnActor<AActor>(Cls, SpawnLoc, FRotator::ZeroRotator, OccupantSpawnParams);
				if (!SpawnedActor)
				{
					continue;
				}

				if (!ResnapTowerActorToFloor(SpawnedActor, FloorNumber))
				{
					SpawnedActor->Destroy();
					continue;
				}

				UsedLocs.Add(SpawnedActor->GetActorLocation());
				return SpawnedActor;
			}

			return nullptr;
		};

		int32 GuaranteedUtilityFountains = 0;
		for (const int32 FloorNumber : GameplayFloorNumbers)
		{
			const int32 FloorChestCount = TowerChestCountByFloor.FindRef(FloorNumber);
			for (int32 ChestIndex = 0; ChestIndex < FloorChestCount; ++ChestIndex)
			{
				for (int32 SpawnAttempt = 0; SpawnAttempt < 4; ++SpawnAttempt)
				{
					if (AT66ChestInteractable* Chest = Cast<AT66ChestInteractable>(SpawnTowerActorOnFloor(
						AT66ChestInteractable::StaticClass(),
						FloorNumber,
						7600 + (ChestIndex * 41) + (SpawnAttempt * 211),
						1100.f,
						1200.f)))
					{
						ConfigureChest(Chest);
						if (!ResnapTowerActorToFloor(Chest, FloorNumber))
						{
							Chest->Destroy();
							continue;
						}
						break;
					}
				}
			}

			const int32 FloorCrateCount = TowerCrateCountByFloor.FindRef(FloorNumber);
			for (int32 CrateIndex = 0; CrateIndex < FloorCrateCount; ++CrateIndex)
			{
				for (int32 SpawnAttempt = 0; SpawnAttempt < 4; ++SpawnAttempt)
				{
					if (AT66CrateInteractable* Crate = Cast<AT66CrateInteractable>(SpawnTowerActorOnFloor(
						AT66CrateInteractable::StaticClass(),
						FloorNumber,
						7700 + (CrateIndex * 41) + (SpawnAttempt * 211),
						1100.f,
						1200.f)))
					{
						ConfigureCrate(Crate);
						if (!ResnapTowerActorToFloor(Crate, FloorNumber))
						{
							Crate->Destroy();
							continue;
						}
						break;
					}
				}
			}

			if (TowerFountainFloorNumbers.Contains(FloorNumber))
			{
				if (SpawnTowerActorOnFloor(AT66FountainOfLifeInteractable::StaticClass(), FloorNumber, 7800, 1200.f, 1300.f))
				{
					++GuaranteedUtilityFountains;
				}
			}
		}

		RemainingChests = 0;
		RemainingCrates = 0;
		RemainingWheels = 0;
		RemainingFountains = FMath::Max(0, RemainingFountains - GuaranteedUtilityFountains);
		RemainingTotems = CountTotems;

		for (const int32 FloorNumber : GameplayFloorNumbers)
		{
			const FName CasinoTag(*FString::Printf(TEXT("T66_Tower_Casino_%02d"), FloorNumber));
			if (!ExistingTowerOccupantTags.Contains(CasinoTag))
			{
				if (RunState)
				{
					RunState->RecordLuckQuantityBool(
						FName(TEXT("TowerCasinoFloorSpawned")),
						true,
						1.f,
						INDEX_NONE,
						RunSeed + StageNum * 1901 + 5100 + FloorNumber * 53);
				}
				FVector CasinoLoc = FVector::ZeroVector;
				if (TryFindTowerFloorLocation(FloorNumber, 5100, 1800.f, 2200.f, CasinoLoc))
				{
					if (AT66CasinoInteractable* Casino = World->SpawnActor<AT66CasinoInteractable>(
						AT66CasinoInteractable::StaticClass(), CasinoLoc, FRotator::ZeroRotator, OccupantSpawnParams))
					{
						T66TrySnapActorToTowerFloor(World, Casino, CachedTowerMainMapLayout, FloorNumber, CasinoLoc);
						T66AssignTowerFloorTag(Casino, FloorNumber);
						Casino->ConfigureCompactTowerVariant();
						Casino->Tags.AddUnique(CasinoTag);
						ExistingTowerOccupantTags.Add(CasinoTag);
						T66RememberTaggedActor(Casino, CasinoTag);
						UsedLocs.Add(CasinoLoc);
					}
				}
			}
		}

		if (QuickReviveFloorNumber != INDEX_NONE)
		{
			const FName QuickReviveTag(*FString::Printf(TEXT("T66_Tower_QuickRevive_%02d"), QuickReviveFloorNumber));
			if (!ExistingTowerOccupantTags.Contains(QuickReviveTag))
			{
				if (RunState)
				{
					RunState->RecordLuckQuantityBool(
						FName(TEXT("TowerQuickReviveFloorSpawned")),
						true,
						1.f,
						INDEX_NONE,
						RunSeed + StageNum * 1901 + 6900 + QuickReviveFloorNumber * 53);
				}

				FVector QuickReviveLoc = FVector::ZeroVector;
				if (TryFindTowerFloorLocation(QuickReviveFloorNumber, 6900, 1300.f, 1700.f, QuickReviveLoc))
				{
					if (AT66QuickReviveVendingMachine* QuickReviveMachine = World->SpawnActor<AT66QuickReviveVendingMachine>(
						AT66QuickReviveVendingMachine::StaticClass(), QuickReviveLoc, FRotator::ZeroRotator, OccupantSpawnParams))
					{
						T66TrySnapActorToTowerFloor(World, QuickReviveMachine, CachedTowerMainMapLayout, QuickReviveFloorNumber, QuickReviveLoc);
						T66AssignTowerFloorTag(QuickReviveMachine, QuickReviveFloorNumber);
						QuickReviveMachine->Tags.AddUnique(QuickReviveTag);
						ExistingTowerOccupantTags.Add(QuickReviveTag);
						T66RememberTaggedActor(QuickReviveMachine, QuickReviveTag);
						UsedLocs.Add(QuickReviveLoc);
					}
				}
			}
		}

		const FName SaintTag(TEXT("T66_Tower_Saint"));
		if (!ExistingTowerOccupantTags.Contains(SaintTag))
		{
			for (int32 Index = GameplayFloorNumbers.Num() - 1; Index > 0; --Index)
			{
				const int32 SwapIndex = TowerFloorRng.RandRange(0, Index);
				if (SwapIndex != Index)
				{
					GameplayFloorNumbers.Swap(Index, SwapIndex);
				}
			}

			for (const int32 FloorNumber : GameplayFloorNumbers)
			{
				FVector SaintLoc = FVector::ZeroVector;
				if (!TryFindTowerFloorLocation(FloorNumber, 7100, 1800.f, 2200.f, SaintLoc))
				{
					continue;
				}

				if (AT66SaintNPC* Saint = World->SpawnActor<AT66SaintNPC>(
					AT66SaintNPC::StaticClass(), SaintLoc, FRotator::ZeroRotator, OccupantSpawnParams))
				{
					const FName SaintFloorTag(*FString::Printf(TEXT("T66_Tower_Saint_%02d"), FloorNumber));
					T66TrySnapActorToTowerFloor(World, Saint, CachedTowerMainMapLayout, FloorNumber, SaintLoc);
					T66AssignTowerFloorTag(Saint, FloorNumber);
					Saint->Tags.AddUnique(SaintTag);
					Saint->Tags.AddUnique(SaintFloorTag);
					ExistingTowerOccupantTags.Add(SaintTag);
					ExistingTowerOccupantTags.Add(SaintFloorTag);
					T66RememberTaggedActor(Saint, SaintTag);
					T66RememberTaggedActor(Saint, SaintFloorTag);
					UsedLocs.Add(SaintLoc);
				}
				break;
			}
		}

		for (const int32 FloorNumber : GameplayFloorNumbers)
		{
			if (RemainingChests > 0)
			{
				if (AT66ChestInteractable* Chest = Cast<AT66ChestInteractable>(SpawnTowerActorOnFloor(AT66ChestInteractable::StaticClass(), FloorNumber, 8100, 1300.f, 1700.f)))
				{
					ConfigureChest(Chest);
					if (!ResnapTowerActorToFloor(Chest, FloorNumber))
					{
						Chest->Destroy();
						continue;
					}
					--RemainingChests;
				}
			}

			if (RemainingWheels > 0)
			{
				if (AT66WheelSpinInteractable* Wheel = Cast<AT66WheelSpinInteractable>(SpawnTowerActorOnFloor(AT66WheelSpinInteractable::StaticClass(), FloorNumber, 8200, 1300.f, 1700.f)))
				{
					ConfigureWheel(Wheel);
					if (!ResnapTowerActorToFloor(Wheel, FloorNumber))
					{
						Wheel->Destroy();
						continue;
					}
					--RemainingWheels;
					continue;
				}
			}

			if (RemainingCrates > 0)
			{
				if (AT66CrateInteractable* Crate = Cast<AT66CrateInteractable>(SpawnTowerActorOnFloor(AT66CrateInteractable::StaticClass(), FloorNumber, 8300, 1300.f, 1700.f)))
				{
					ConfigureCrate(Crate);
					if (!ResnapTowerActorToFloor(Crate, FloorNumber))
					{
						Crate->Destroy();
						continue;
					}
					--RemainingCrates;
					continue;
				}
			}

			if (RemainingFountains > 0)
			{
				if (SpawnTowerActorOnFloor(AT66FountainOfLifeInteractable::StaticClass(), FloorNumber, 8400, 1300.f, 1700.f))
				{
					--RemainingFountains;
					continue;
				}
			}

			if (RemainingTotems > 0
				&& (!PlayerExperience
					|| PlayerExperience->ShouldSpawnDifficultyTotemOnTowerFloor(
						Difficulty,
						IsBossRushFinaleStage(),
						FloorNumber,
						CachedTowerMainMapLayout.FirstGameplayFloorNumber,
						CachedTowerMainMapLayout.LastGameplayFloorNumber)))
			{
				if (AT66DifficultyTotem* Totem = Cast<AT66DifficultyTotem>(SpawnTowerActorOnFloor(AT66DifficultyTotem::StaticClass(), FloorNumber, 8500, 1300.f, 1700.f)))
				{
					ConfigureTotem(Totem);
					if (!ResnapTowerActorToFloor(Totem, FloorNumber))
					{
						Totem->Destroy();
						continue;
					}
					--RemainingTotems;
				}
			}
		}
	}

	for (int32 i = 0; i < RemainingFountains; ++i)
	{
		SpawnOne(AT66FountainOfLifeInteractable::StaticClass());
	}
	for (int32 i = 0; i < RemainingChests; ++i)
	{
		if (AT66ChestInteractable* Chest = Cast<AT66ChestInteractable>(SpawnOne(AT66ChestInteractable::StaticClass())))
		{
			ConfigureChest(Chest);
		}
	}
	for (int32 i = 0; i < RemainingWheels; ++i)
	{
		if (AT66WheelSpinInteractable* Wheel = Cast<AT66WheelSpinInteractable>(SpawnOne(AT66WheelSpinInteractable::StaticClass())))
		{
			ConfigureWheel(Wheel);
		}
	}

	if (!IsLabLevel() && !bUsingMainMapTerrain)
	{
		SpawnModelShowcaseRow();
	}

	// Teleport pads are deprecated and no longer part of stage population.

	for (int32 i = 0; i < RemainingCrates; ++i)
	{
		if (AT66CrateInteractable* Crate = Cast<AT66CrateInteractable>(SpawnOne(AT66CrateInteractable::StaticClass())))
		{
			ConfigureCrate(Crate);
		}
	}

	for (int32 i = 0; i < RemainingTotems; ++i)
	{
		if (AT66DifficultyTotem* Totem = Cast<AT66DifficultyTotem>(SpawnOne(AT66DifficultyTotem::StaticClass())))
		{
			ConfigureTotem(Totem);
		}
	}

	if (bTowerLayout)
	{
		if (UT66TrapSubsystem* TrapSubsystem = World->GetSubsystem<UT66TrapSubsystem>())
		{
			TrapSubsystem->SpawnTowerStageTraps(CachedTowerMainMapLayout, StageNum, Difficulty, RunSeed);
			SyncTowerTrapActivation(true);
		}
	}

	bWorldInteractablesSpawnedForStage = true;
}

void AT66GameMode::SpawnModelShowcaseRow()
{
	if (IsLabLevel())
	{
		return;
	}

	UWorld* World = GetWorld();
	if (!World)
	{
		return;
	}

	struct FShowcaseMeshSpec
	{
		const TCHAR* MeshPath = nullptr;
		FVector Offset = FVector::ZeroVector;
		FRotator Rotation = FRotator::ZeroRotator;
		FVector Scale = FVector(1.f, 1.f, 1.f);
		FName Tag = NAME_None;
	};

	const FVector StartAreaCenter = T66GameplayLayout::GetStartAreaCenter();
	const FShowcaseMeshSpec ShowcaseSpecs[] = {
		{ TEXT("/Game/World/Props/StartAreaDecor/Cow.Cow"),      FVector(1800.f,  1500.f, 0.f), FRotator(0.f, -140.f, 0.f), FVector(2.f, 2.f, 2.f), FName(TEXT("T66StartAreaCow")) },
		{ TEXT("/Game/World/Props/StartAreaDecor/RoboCow.RoboCow"), FVector(1800.f, -1500.f, 0.f), FRotator(0.f,  140.f, 0.f), FVector(2.f, 2.f, 2.f), FName(TEXT("T66StartAreaRoboCow")) },
		{ TEXT("/Game/World/Props/StartAreaDecor/FullBody.FullBody"), FVector(-1800.f,    0.f, 0.f), FRotator(0.f,    0.f, 0.f), FVector(2.f, 2.f, 2.f), FName(TEXT("T66StartAreaFullBody")) },
	};

	auto FindExistingShowcaseActor = [&](const FName Tag) -> AStaticMeshActor*
	{
		for (TActorIterator<AStaticMeshActor> It(World); It; ++It)
		{
			AStaticMeshActor* Existing = *It;
			if (Existing && Existing->ActorHasTag(Tag))
			{
				return Existing;
			}
		}
		return nullptr;
	};

	for (const FShowcaseMeshSpec& Spec : ShowcaseSpecs)
	{
		if (Spec.MeshPath == nullptr || Spec.Tag.IsNone())
		{
			continue;
		}

		AStaticMeshActor* Actor = FindExistingShowcaseActor(Spec.Tag);
		if (!Actor)
		{
			FVector SpawnLoc = StartAreaCenter + Spec.Offset;
			FHitResult Hit;
			const FVector TraceStart = SpawnLoc + FVector(0.f, 0.f, 8000.f);
			const FVector TraceEnd = SpawnLoc - FVector(0.f, 0.f, 16000.f);
			if (World->LineTraceSingleByChannel(Hit, TraceStart, TraceEnd, ECC_WorldStatic))
			{
				SpawnLoc = Hit.ImpactPoint;
			}

			FActorSpawnParameters SpawnParams;
			SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
			Actor = World->SpawnActor<AStaticMeshActor>(AStaticMeshActor::StaticClass(), SpawnLoc, Spec.Rotation, SpawnParams);
			if (!Actor)
			{
				continue;
			}
			Actor->Tags.AddUnique(Spec.Tag);
		}

		TSoftObjectPtr<UStaticMesh> MeshSoft(FSoftObjectPath(Spec.MeshPath));
		UStaticMesh* Mesh = MeshSoft.LoadSynchronous();
		if (!Mesh)
		{
			Actor->Destroy();
			continue;
		}

		T66_SetStaticMeshActorMobility(Actor, EComponentMobility::Movable);
		if (UStaticMeshComponent* MeshComponent = Actor->GetStaticMeshComponent())
		{
			MeshComponent->SetStaticMesh(Mesh);
			MeshComponent->SetCollisionEnabled(ECollisionEnabled::NoCollision);
			MeshComponent->SetGenerateOverlapEvents(false);
			MeshComponent->SetRelativeScale3D(Spec.Scale);
			FT66VisualUtil::GroundMeshToActorOrigin(MeshComponent, Mesh);
		}
		Actor->SetActorRotation(Spec.Rotation);
		T66_SetStaticMeshActorMobility(Actor, EComponentMobility::Static);
	}
}

void AT66GameMode::SpawnIdolAltarForPlayer(AController* Player)
{
	if (IsLabLevel()) return;

	UWorld* World = GetWorld();
	if (!World || !Player) return;

	if (IsValid(IdolAltar))
	{
		return;
	}
	IdolAltar = nullptr;

	UGameInstance* GI = World->GetGameInstance();
	UT66IdolManagerSubsystem* IdolManager = GI ? GI->GetSubsystem<UT66IdolManagerSubsystem>() : nullptr;
	UT66RunStateSubsystem* RunState = GI ? GI->GetSubsystem<UT66RunStateSubsystem>() : nullptr;
	if (!IdolManager)
	{
		return;
	}

	int32 SelectionBudget = 1;
	int32 CatchUpSelections = 0;
	if (IdolManager->HasCatchUpIdolPicksRemaining())
	{
		CatchUpSelections = IdolManager->GetRemainingCatchUpIdolPicks();
		SelectionBudget += CatchUpSelections;
	}

	const int32 CurrentStage = RunState ? RunState->GetCurrentStage() : 1;
	UE_LOG(
		LogT66GameMode,
		Log,
		TEXT("Spawning stage-entry idol altar for stage %d with %d total selections (%d catch-up)."),
		CurrentStage,
		SelectionBudget,
		CatchUpSelections);

	FVector SpawnLoc = FVector::ZeroVector;
	FRotator SpawnRotation = FRotator::ZeroRotator;
	if (T66UsesMainMapTerrainStage(World))
	{
		if (IsUsingTowerMainMapLayout())
		{
			SpawnLoc = CachedTowerMainMapLayout.StartAnchorSurfaceLocation;
		}
		else if (!TryGetMainMapStartPlacementLocation(0.f, 0.f, SpawnLoc))
		{
			return;
		}

		FVector Center = FVector::ZeroVector;
		FVector InwardDirection = FVector::ForwardVector;
		FVector SideDirection = FVector::RightVector;
		float CellSize = 0.f;
		if (TryGetMainMapStartAxes(Center, InwardDirection, SideDirection, CellSize) && !InwardDirection.IsNearlyZero())
		{
			SpawnRotation = (-InwardDirection).Rotation();
		}
	}
	else
	{
		SpawnLoc = T66GameplayLayout::GetStartAreaCenter();

		FHitResult Hit;
		const FVector TraceStart = SpawnLoc + FVector(0.f, 0.f, 3000.f);
		const FVector TraceEnd = SpawnLoc - FVector(0.f, 0.f, 9000.f);
		if (World->LineTraceSingleByChannel(Hit, TraceStart, TraceEnd, ECC_WorldStatic)
			|| World->LineTraceSingleByChannel(Hit, TraceStart, TraceEnd, ECC_Visibility))
		{
			SpawnLoc = Hit.ImpactPoint;
		}
	}

	FActorSpawnParameters SpawnParams;
	SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
	IdolAltar = World->SpawnActor<AT66IdolAltar>(AT66IdolAltar::StaticClass(), SpawnLoc, SpawnRotation, SpawnParams);
	if (IdolAltar)
	{
		if (IsUsingTowerMainMapLayout())
		{
			T66TrySnapActorToTowerFloor(World, IdolAltar, CachedTowerMainMapLayout, CachedTowerMainMapLayout.StartFloorNumber, SpawnLoc);
			T66AssignTowerFloorTag(IdolAltar, CachedTowerMainMapLayout.StartFloorNumber);
		}
		else
		{
			TrySnapActorToTerrainAtLocation(IdolAltar, SpawnLoc);
		}

		if (APawn* PlayerPawn = Player->GetPawn())
		{
			T66FaceActorTowardLocation2D(IdolAltar, PlayerPawn->GetActorLocation());
			if (RunState && RunState->GetCurrentStage() <= 1)
			{
				FRotator FacingRotation = PlayerPawn->GetActorRotation();
				if (T66TryBuildFacingRotation2D(PlayerPawn->GetActorLocation(), IdolAltar->GetActorLocation(), FacingRotation))
				{
					T66SyncPawnAndControllerRotation(PlayerPawn, Player, FacingRotation);
				}
			}
		}

		IdolAltar->RemainingSelections = FMath::Max(1, SelectionBudget);
		IdolAltar->CatchUpSelectionsRemaining = FMath::Max(0, CatchUpSelections);
		TowerIdolSelectionsAtStageStart = IdolAltar->RemainingSelections;
		if (IsUsingTowerMainMapLayout())
		{
			SyncTowerMiasmaSourceAnchor(CachedTowerMainMapLayout.StartFloorNumber, IdolAltar->GetActorLocation());
		}
	}
}

void AT66GameMode::SpawnIdolVFXTestTargets()
{
	if (!bSpawnIdolVFXTestTargetsAtStageStart || IsLabLevel())
	{
		return;
	}

	UWorld* World = GetWorld();
	if (!World)
	{
		return;
	}

	for (AT66EnemyBase* ExistingTarget : IdolVFXTestTargets)
	{
		if (ExistingTarget)
		{
			ExistingTarget->Destroy();
		}
	}
	IdolVFXTestTargets.Reset();

	TArray<FVector> SpawnLocations;
	SpawnLocations.Reserve(3);

	APawn* HeroPawn = UGameplayStatics::GetPlayerPawn(World, 0);
	const FVector HeroLocation = HeroPawn ? HeroPawn->GetActorLocation() : FVector::ZeroVector;
	FVector ForwardDirection = HeroPawn ? HeroPawn->GetActorForwardVector().GetSafeNormal2D() : FVector::ZeroVector;
	FVector SideDirection = FVector::RightVector;
	float CellSize = 400.f;
	FVector ThresholdMidpoint = FVector::ZeroVector;
	bool bHasMainMapThreshold = false;
	if (T66UsesMainMapTerrainStage(World))
	{
		FVector Center = FVector::ZeroVector;
		FVector InwardDirection = FVector::ForwardVector;
		if (TryGetMainMapStartAxes(Center, InwardDirection, SideDirection, CellSize))
		{
			ForwardDirection = InwardDirection.GetSafeNormal2D();
			if (!MainMapStartAnchorSurfaceLocation.IsNearlyZero() && !MainMapStartPathSurfaceLocation.IsNearlyZero())
			{
				ThresholdMidpoint = (MainMapStartAnchorSurfaceLocation + MainMapStartPathSurfaceLocation) * 0.5f;
				bHasMainMapThreshold = true;
			}
		}
	}
	if (ForwardDirection.IsNearlyZero() && IdolAltar && !HeroLocation.IsNearlyZero())
	{
		ForwardDirection = (IdolAltar->GetActorLocation() - HeroLocation).GetSafeNormal2D();
	}
	if (ForwardDirection.IsNearlyZero())
	{
		ForwardDirection = FVector::ForwardVector;
	}

	SideDirection = FVector::CrossProduct(FVector::UpVector, ForwardDirection).GetSafeNormal();
	if (SideDirection.IsNearlyZero())
	{
		SideDirection = FVector::RightVector;
	}

	const FVector StartCenter = !MainMapStartAreaCenterSurfaceLocation.IsNearlyZero()
		? MainMapStartAreaCenterSurfaceLocation
		: T66GameplayLayout::GetStartAreaCenter(DefaultSpawnHeight);
	const FVector AnchorCenter = IdolAltar ? IdolAltar->GetActorLocation() : StartCenter;
	FVector FrontCenter = AnchorCenter + ForwardDirection * 650.f + FVector(0.f, 0.f, 180.f);
	if (bHasMainMapThreshold)
	{
		const float SafeSideOffset = FMath::Max(140.f, CellSize * 0.40f);
		FrontCenter = ThresholdMidpoint - ForwardDirection * SafeSideOffset + FVector(0.f, 0.f, 180.f);
	}
	static constexpr float TargetSideSpacing = 340.f;
	SpawnLocations.Add(FrontCenter - SideDirection * TargetSideSpacing);
	SpawnLocations.Add(FrontCenter);
	SpawnLocations.Add(FrontCenter + SideDirection * TargetSideSpacing);

	static const FName TargetIds[] = {
		FName(TEXT("Cow")),
		FName(TEXT("Pig")),
		FName(TEXT("Goat"))
	};

	for (int32 Index = 0; Index < UE_ARRAY_COUNT(TargetIds) && SpawnLocations.IsValidIndex(Index); ++Index)
	{
		FTransform SpawnTransform(FRotator::ZeroRotator, SpawnLocations[Index]);
		FActorSpawnParameters SpawnParams;
		SpawnParams.Owner = this;
		SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

		AT66EnemyBase* TargetEnemy = World->SpawnActorDeferred<AT66EnemyBase>(
			AT66EnemyBase::StaticClass(),
			SpawnTransform,
			this,
			nullptr,
			ESpawnActorCollisionHandlingMethod::AlwaysSpawn);
		if (!TargetEnemy)
		{
			continue;
		}

		TargetEnemy->CharacterVisualID = TargetIds[Index];
		TargetEnemy->MobID = TargetIds[Index];
		TargetEnemy->MaxHP = 20000;
		TargetEnemy->CurrentHP = 20000;
		TargetEnemy->TouchDamageHearts = 0;
		TargetEnemy->PointValue = 0;
		TargetEnemy->XPValue = 0;
		TargetEnemy->bDropsLoot = false;
		TargetEnemy->Armor = 0.f;

		UGameplayStatics::FinishSpawningActor(TargetEnemy, SpawnTransform);
		TargetEnemy->ConfigureAsMob(TargetIds[Index]);
		TrySnapActorToTerrainAtLocation(TargetEnemy, SpawnLocations[Index]);

		if (UCharacterMovementComponent* Movement = TargetEnemy->GetCharacterMovement())
		{
			Movement->MaxWalkSpeed = 0.f;
			Movement->StopMovementImmediately();
			Movement->DisableMovement();
			Movement->SetMovementMode(MOVE_None);
		}

		if (!HeroLocation.IsNearlyZero())
		{
			FVector Facing = HeroLocation - TargetEnemy->GetActorLocation();
			Facing.Z = 0.f;
			if (!Facing.Normalize())
			{
				Facing = FVector::BackwardVector;
			}
			TargetEnemy->SetActorRotation(Facing.Rotation());
		}

		TargetEnemy->CurrentHP = TargetEnemy->MaxHP;
		TargetEnemy->UpdateHealthBar();
		TargetEnemy->Tags.AddUnique(FName(TEXT("IdolVFXTestTarget")));
		UE_LOG(LogT66GameMode, Log, TEXT("Spawned idol VFX test target %s at %s"), *TargetIds[Index].ToString(), *TargetEnemy->GetActorLocation().ToCompactString());
		IdolVFXTestTargets.Add(TargetEnemy);
	}
}

void AT66GameMode::SpawnIdolAltarAtLocation(const FVector& Location)
{
	UWorld* World = GetWorld();
	if (!World) return;
	if (IsValid(IdolAltar)) return;
	IdolAltar = nullptr;

	// Place near boss death, but offset so it doesn't overlap the Stage Gate.
	FVector SpawnLoc = Location + FVector(420.f, 260.f, 0.f);

	// Trace down so altar sits on the ground.
	FHitResult Hit;
	const FVector TraceStart = SpawnLoc + FVector(0.f, 0.f, 3000.f);
	const FVector TraceEnd = SpawnLoc - FVector(0.f, 0.f, 9000.f);
	if (World->LineTraceSingleByChannel(Hit, TraceStart, TraceEnd, ECC_WorldStatic))
	{
		SpawnLoc = Hit.ImpactPoint;
	}

	FActorSpawnParameters SpawnParams;
	SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
	IdolAltar = World->SpawnActor<AT66IdolAltar>(AT66IdolAltar::StaticClass(), SpawnLoc, FRotator::ZeroRotator, SpawnParams);
	if (IdolAltar)
	{
		IdolAltar->RemainingSelections = 1;
		IdolAltar->CatchUpSelectionsRemaining = 0;
	}
}

void AT66GameMode::SpawnStageCatchUpPlatformAndInteractables()
{
	UWorld* World = GetWorld();
	if (!World) return;

	UT66GameInstance* T66GI = GetT66GameInstance();
	UT66RunStateSubsystem* RunState = T66GI ? T66GI->GetSubsystem<UT66RunStateSubsystem>() : nullptr;
	if (!T66GI || !RunState) return;
	UT66PlayerExperienceSubSystem* PlayerExperience = T66GI->GetSubsystem<UT66PlayerExperienceSubSystem>();

	RunState->SetInStageCatchUp(true);

	const int32 DiffIndex = FMath::Max(0, static_cast<int32>(T66GI->SelectedDifficulty));
	static constexpr int32 DefaultStartStages[] = { 1, 6, 11, 16, 21 };
	const int32 FallbackStartStage = DefaultStartStages[FMath::Clamp(DiffIndex, 0, UE_ARRAY_COUNT(DefaultStartStages) - 1)];
	const int32 StartStage = PlayerExperience
		? PlayerExperience->GetDifficultyStartStage(T66GI->SelectedDifficulty)
		: FallbackStartStage;

	const int32 GoldAmount = PlayerExperience
		? PlayerExperience->GetDifficultyStartGoldBonus(T66GI->SelectedDifficulty)
		: 200 * DiffIndex;
	const int32 LootBags = PlayerExperience
		? PlayerExperience->GetDifficultyStartLootBags(T66GI->SelectedDifficulty)
		: 2 + (DiffIndex * 2);

	const FVector PlatformCenter(-45455.f, 23636.f, -50.f);

	{
		FActorSpawnParameters P;
		P.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

		AT66StageCatchUpGoldInteractable* Gold = World->SpawnActor<AT66StageCatchUpGoldInteractable>(
			AT66StageCatchUpGoldInteractable::StaticClass(),
			PlatformCenter + FVector(-260.f, -120.f, 120.f),
			FRotator::ZeroRotator,
			P);
		if (Gold)
		{
			Gold->GoldAmount = GoldAmount;
		}

		AT66StageCatchUpLootInteractable* Loot = World->SpawnActor<AT66StageCatchUpLootInteractable>(
			AT66StageCatchUpLootInteractable::StaticClass(),
			PlatformCenter + FVector(-260.f, 220.f, 120.f),
			FRotator::ZeroRotator,
			P);
		if (Loot)
		{
			Loot->LootBagCount = LootBags;
		}

		AT66StageCatchUpGate* Gate = World->SpawnActor<AT66StageCatchUpGate>(
			AT66StageCatchUpGate::StaticClass(),
			PlatformCenter + FVector(520.f, 0.f, 200.f),
			FRotator::ZeroRotator,
			P);
		if (Gate)
		{
			Gate->TargetStage = StartStage;
		}
	}
}
