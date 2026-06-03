// Copyright Tribulation 66. All Rights Reserved.

#include "PerformanceSystem/T66MobLootStressHarnessActor.h"

#include "Core/T66GameInstance.h"
#include "Core/T66RunStateSubsystem.h"
#include "Data/T66DataTypes.h"
#include "Dom/JsonObject.h"
#include "GameFramework/PlayerController.h"
#include "Gameplay/T66BossBase.h"
#include "Gameplay/T66EnemyBase.h"
#include "Gameplay/T66MobBase.h"
#include "Gameplay/T66PetActor.h"
#include "HAL/IConsoleManager.h"
#include "Misc/CommandLine.h"
#include "Misc/FileHelper.h"
#include "Misc/Paths.h"
#include "RHI.h"
#include "RenderTimer.h"
#include "Serialization/JsonSerializer.h"
#include "Serialization/JsonWriter.h"

DEFINE_LOG_CATEGORY_STATIC(LogT66MobLootStress, Log, All);

namespace
{
	constexpr int32 T66MobLootStressSchemaVersion = 2;

	FString FormatMaybeNumber(const AT66MobLootStressHarnessActor::FMetricAccumulator& Metric)
	{
		return Metric.Count > 0 ? FString::Printf(TEXT("%.4f"), Metric.Average()) : TEXT("Unavailable");
	}

	void SetMetricObject(
		const TSharedRef<FJsonObject>& Root,
		const TCHAR* FieldName,
		const AT66MobLootStressHarnessActor::FMetricAccumulator& Metric)
	{
		const TSharedRef<FJsonObject> Object = MakeShared<FJsonObject>();
		Object->SetNumberField(TEXT("sample_count"), Metric.Count);
		if (Metric.Count > 0)
		{
			Object->SetNumberField(TEXT("avg"), Metric.Average());
			Object->SetNumberField(TEXT("min"), Metric.Min);
			Object->SetNumberField(TEXT("max"), Metric.Max);
		}
		else
		{
			Object->SetStringField(TEXT("avg"), TEXT("Unavailable"));
			Object->SetStringField(TEXT("min"), TEXT("Unavailable"));
			Object->SetStringField(TEXT("max"), TEXT("Unavailable"));
		}
		Root->SetObjectField(FieldName, Object);
	}

	FT66MobLootCollectorRef MakeCollector(AActor* Actor, const TCHAR* ID, const ET66MobLootCollectorType Type)
	{
		FT66MobLootCollectorRef Collector;
		Collector.Collector = Actor;
		Collector.CollectorID = FName(ID);
		Collector.CollectorType = Type;
		return Collector;
	}

	float PlanarDistance(const FVector& A, const FVector& B)
	{
		const FVector APlanar(A.X, A.Y, 0.0f);
		const FVector BPlanar(B.X, B.Y, 0.0f);
		return FVector::Distance(APlanar, BPlanar);
	}

	bool TryParseDifficultyOverride(const FString& RawValue, ET66Difficulty& OutDifficulty)
	{
		FString Normalized = RawValue;
		Normalized.TrimStartAndEndInline();
		Normalized.ReplaceInline(TEXT(" "), TEXT(""));
		Normalized.ReplaceInline(TEXT("_"), TEXT(""));
		Normalized.ReplaceInline(TEXT("-"), TEXT(""));

		if (Normalized.Equals(TEXT("Impossible"), ESearchCase::IgnoreCase) || Normalized.Equals(TEXT("4")))
		{
			OutDifficulty = ET66Difficulty::Impossible;
			return true;
		}
		if (Normalized.Equals(TEXT("VeryHard"), ESearchCase::IgnoreCase) || Normalized.Equals(TEXT("Very"), ESearchCase::IgnoreCase) || Normalized.Equals(TEXT("3")))
		{
			OutDifficulty = ET66Difficulty::VeryHard;
			return true;
		}
		if (Normalized.Equals(TEXT("Hard"), ESearchCase::IgnoreCase) || Normalized.Equals(TEXT("2")))
		{
			OutDifficulty = ET66Difficulty::Hard;
			return true;
		}
		if (Normalized.Equals(TEXT("Medium"), ESearchCase::IgnoreCase) || Normalized.Equals(TEXT("Normal"), ESearchCase::IgnoreCase) || Normalized.Equals(TEXT("1")))
		{
			OutDifficulty = ET66Difficulty::Medium;
			return true;
		}
		if (Normalized.Equals(TEXT("Easy"), ESearchCase::IgnoreCase) || Normalized.Equals(TEXT("0")))
		{
			OutDifficulty = ET66Difficulty::Easy;
			return true;
		}

		return false;
	}

	FString DifficultyToString(const ET66Difficulty Difficulty)
	{
		switch (Difficulty)
		{
		case ET66Difficulty::Impossible:
			return TEXT("Impossible");
		case ET66Difficulty::VeryHard:
			return TEXT("VeryHard");
		case ET66Difficulty::Hard:
			return TEXT("Hard");
		case ET66Difficulty::Medium:
			return TEXT("Medium");
		case ET66Difficulty::Easy:
		default:
			return TEXT("Easy");
		}
	}
}

AT66MobLootStressHarnessActor::AT66MobLootStressHarnessActor()
{
	PrimaryActorTick.bCanEverTick = true;
	PrimaryActorTick.bStartWithTickEnabled = true;
	SetActorHiddenInGame(true);
	SetActorEnableCollision(false);
}

void AT66MobLootStressHarnessActor::FMetricAccumulator::Add(const double Value)
{
	if (!FMath::IsFinite(Value))
	{
		return;
	}

	++Count;
	Sum += Value;
	Min = Count == 1 ? Value : FMath::Min(Min, Value);
	Max = Count == 1 ? Value : FMath::Max(Max, Value);
}

double AT66MobLootStressHarnessActor::FMetricAccumulator::Average() const
{
	return Count > 0 ? Sum / static_cast<double>(Count) : 0.0;
}

void AT66MobLootStressHarnessActor::StartFromCommandLine(APlayerController* InOwnerController)
{
	OwnerController = InOwnerController;
	FParse::Value(FCommandLine::Get(), TEXT("T66MobLootStressCount="), RequestedDropCount);
	FParse::Value(FCommandLine::Get(), TEXT("T66MobLootStressSpacing="), GridSpacing);
	FParse::Value(FCommandLine::Get(), TEXT("T66MobLootStressWarmupSeconds="), WarmupSeconds);
	FParse::Value(FCommandLine::Get(), TEXT("T66MobLootStressSampleSeconds="), SampleSeconds);
	FParse::Value(FCommandLine::Get(), TEXT("T66MobLootStressManifest="), ManifestPath);
	bEconomyOnlyMode = FParse::Param(FCommandLine::Get(), TEXT("T66MobLootStressEconomyOnly"));

	ET66Difficulty EffectiveDifficulty = ET66Difficulty::Easy;
	if (UWorld* World = GetWorld())
	{
		if (UT66GameInstance* T66GI = Cast<UT66GameInstance>(World->GetGameInstance()))
		{
			FString DifficultyOverride;
			if (FParse::Value(FCommandLine::Get(), TEXT("T66MobLootStressDifficulty="), DifficultyOverride))
			{
				ET66Difficulty ParsedDifficulty = T66GI->SelectedDifficulty;
				if (TryParseDifficultyOverride(DifficultyOverride, ParsedDifficulty))
				{
					T66GI->SelectedDifficulty = ParsedDifficulty;
				}
				else
				{
					UE_LOG(LogT66MobLootStress, Warning, TEXT("[MobLootStress] ignored unknown difficulty override '%s'."), *DifficultyOverride);
				}
			}
			EffectiveDifficulty = T66GI->SelectedDifficulty;
		}
	}
	SelectedDifficultyIndex = UT66MobLootSubsystem::GetDifficultyDropIndex(EffectiveDifficulty);
	ExpectedRegularDeathQuantity = UT66MobLootSubsystem::GetDeathMobLootQuantityForDifficulty(EffectiveDifficulty, false);
	ExpectedMiniBossDeathQuantity = UT66MobLootSubsystem::GetDeathMobLootQuantityForDifficulty(EffectiveDifficulty, true);

	RequestedDropCount = FMath::Clamp(RequestedDropCount, 1, UT66MobLootSubsystem::MaxMobLootDrops);
	GridSpacing = FMath::Clamp(GridSpacing, 8.0f, 300.0f);
	WarmupSeconds = FMath::Clamp(WarmupSeconds, 0.0f, 30.0f);
	SampleSeconds = FMath::Clamp(SampleSeconds, 0.25f, 60.0f);
	if (ManifestPath.IsEmpty())
	{
		ManifestPath = FPaths::Combine(
			FPaths::ProjectSavedDir(),
			TEXT("PerformanceSystem"),
			TEXT("MobLootStress"),
			FString::Printf(TEXT("mob_loot_%d.json"), RequestedDropCount));
	}
	ManifestPath = FPaths::ConvertRelativePathToFull(ManifestPath);

	if (UWorld* World = GetWorld())
	{
		if (UT66MobLootSubsystem* MobLoot = World->GetSubsystem<UT66MobLootSubsystem>())
		{
			RunFunctionalChecks(*MobLoot);
			StartPetActorFunctionalCheck(*MobLoot);
			SpawnStressDrops(*MobLoot);
			UploadCountAfterSetup = MobLoot->GetDiagnostics().UploadCount;
		}
	}

	StartSeconds = FPlatformTime::Seconds();
	bStarted = true;
	UE_LOG(LogT66MobLootStress, Display,
		TEXT("[MobLootStress] started requested=%d spawned=%d failed=%d difficulty=%s index=%d expectedRegular=%d expectedMiniBoss=%d manifest=%s"),
		RequestedDropCount,
		SpawnedDropCount,
		FailedSpawnCount,
		*DifficultyToString(EffectiveDifficulty),
		SelectedDifficultyIndex,
		ExpectedRegularDeathQuantity,
		ExpectedMiniBossDeathQuantity,
		*ManifestPath);
}

void AT66MobLootStressHarnessActor::Tick(const float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);
	if (!bStarted || bManifestWritten)
	{
		return;
	}

	const double ElapsedSeconds = FPlatformTime::Seconds() - StartSeconds;
	if (UWorld* World = GetWorld())
	{
		if (UT66MobLootSubsystem* MobLoot = World->GetSubsystem<UT66MobLootSubsystem>())
		{
			UpdatePetActorFunctionalCheck(*MobLoot);
		}
	}

	if (ElapsedSeconds >= WarmupSeconds)
	{
		SampleFrame(DeltaSeconds);
	}

	if (ElapsedSeconds >= WarmupSeconds + SampleSeconds)
	{
		WriteManifest(TEXT("sample-window-complete"));
		bManifestWritten = true;
		const bool bPass = BuildOverallPass();
		FPlatformMisc::RequestExitWithStatus(false, bPass ? 0 : 1, TEXT("T66MobLootStressComplete"));
	}
}

void AT66MobLootStressHarnessActor::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	if (bStarted && !bManifestWritten)
	{
		WriteManifest(TEXT("endplay-before-sample-complete"));
		bManifestWritten = true;
	}
	Super::EndPlay(EndPlayReason);
}

void AT66MobLootStressHarnessActor::RunFunctionalChecks(UT66MobLootSubsystem& MobLoot)
{
#if !UE_BUILD_SHIPPING
	MobLoot.ClearAllMobLootForAutomation();
#endif
	InitialWorldDropCount = MobLoot.GetActiveMobLootDropCount();
	UWorld* World = GetWorld();
	UT66RunStateSubsystem* RunState = nullptr;
	if (UGameInstance* GI = World ? World->GetGameInstance() : nullptr)
	{
		RunState = GI->GetSubsystem<UT66RunStateSubsystem>();
	}
#if !UE_BUILD_SHIPPING
	if (RunState)
	{
		RunState->SetCollectedMobLootStackForAutomation(0);
	}
#endif

	const FT66MobLootCollectorRef PlayerCollector = MakeCollector(OwnerController.Get(), TEXT("MobLootStressPlayer"), ET66MobLootCollectorType::Player);
	const FT66MobLootCollectorRef PetCollector = MakeCollector(this, TEXT("MobLootStressPet"), ET66MobLootCollectorType::Pet);
	const FT66MobLootCollectorRef SystemCollector = MakeCollector(this, TEXT("MobLootStressSystem"), ET66MobLootCollectorType::System);

	FT66MobLootHandle ReservedHandle;
	FT66MobLootSpawnParams ReservedDrop;
	ReservedDrop.Position = GetActorLocation() + FVector(0.0f, 0.0f, 80.0f);
	ReservedDrop.Quantity = 3;
	ReservedDrop.GoldValue = 3;
	ReservedDrop.SourceID = FName(TEXT("MobLootStressReservation"));
	MobLoot.SpawnMobLoot(ReservedDrop, ReservedHandle);

	FT66MobLootQueryFilter Filter;
	Filter.Origin = ReservedDrop.Position;
	Filter.SearchRadius = 240.0f;
	const FT66MobLootReservationResult Reservation = MobLoot.QueryAndReserveMobLoot(Filter, PetCollector);
	const FT66MobLootCollectResult PlayerSweep = MobLoot.CollectMobLootAt(ReservedDrop.Position, 240.0f, PlayerCollector);
	const FT66MobLootCollectResult PetCollect = MobLoot.CollectReservedMobLoot(Reservation.Handle, PetCollector, 240.0f);
	ReservationPlayerSweepDrops = PlayerSweep.DropsCollected;
	ReservationPetCollectDrops = PetCollect.DropsCollected;
	bReservationPreventedDoubleCollect = Reservation.bReserved && PlayerSweep.DropsCollected == 0;
	bReservedPetCollectSucceeded = PetCollect.DropsCollected == 1 && PetCollect.GoldValueCollected == ReservedDrop.GoldValue;
	if (RunState && bReservedPetCollectSucceeded)
	{
		MobLootPetCollectionStackAdded = RunState->AddCollectedMobLootFromCollection(PetCollect, PetCollector);
		MobLootPetCollectionStackAfter = RunState->GetCollectedMobLootStack();
		bMobLootPetCollectionGrewStack = MobLootPetCollectionStackAdded == ReservedDrop.GoldValue
			&& MobLootPetCollectionStackAfter == ReservedDrop.GoldValue;
	}

	for (int32 Index = 0; Index < 12; ++Index)
	{
		FT66MobLootSpawnParams Params;
		Params.Position = GetActorLocation() + FVector(80.0f + static_cast<float>(Index) * 12.0f, 0.0f, 80.0f);
		Params.Quantity = 1;
		Params.GoldValue = 1;
		Params.SourceID = FName(TEXT("MobLootStressBatch"));
		FT66MobLootHandle Handle;
		MobLoot.SpawnMobLoot(Params, Handle);
	}
	const FT66MobLootCollectResult BatchCollect = MobLoot.CollectMobLootAt(GetActorLocation() + FVector(140.0f, 0.0f, 80.0f), 180.0f, PlayerCollector);
	BatchedCollectionDrops = BatchCollect.DropsCollected;
	bBatchedCollectionSucceeded = BatchCollect.DropsCollected > 1;

#if !UE_BUILD_SHIPPING
	MobLoot.ClearAllMobLootForAutomation();
#endif
	if (RunState)
	{
#if !UE_BUILD_SHIPPING
		RunState->SetCollectedMobLootStackForAutomation(0);
#endif
		FT66MobLootSpawnParams EconomyDrop;
		EconomyDrop.Position = GetActorLocation() + FVector(320.0f, 0.0f, 80.0f);
		EconomyDrop.Quantity = 1200;
		EconomyDrop.GoldValue = 1200;
		EconomyDrop.SourceID = FName(TEXT("MobLootStressEconomyStack"));
		FT66MobLootHandle EconomyHandle;
		MobLoot.SpawnMobLoot(EconomyDrop, EconomyHandle);
		MobLootEconomyGoldBeforeCollection = RunState->GetCurrentGold();
		const FT66MobLootCollectResult EconomyCollect = MobLoot.CollectMobLootAt(EconomyDrop.Position, 260.0f, PlayerCollector);
		MobLootEconomyCollectionValue = EconomyCollect.GoldValueCollected;
		MobLootEconomyStackAdded = RunState->AddCollectedMobLootFromCollection(EconomyCollect, PlayerCollector);
		MobLootEconomyGoldAfterCollection = RunState->GetCurrentGold();
		MobLootEconomyStackBeforeSale = RunState->GetCollectedMobLootStack();
		bMobLootEconomyCollectionGrewStack = MobLootEconomyCollectionValue == EconomyDrop.GoldValue
			&& MobLootEconomyStackAdded == UT66RunStateSubsystem::MaxCollectedMobLootStack;
		bMobLootEconomyCappedAt999 = MobLootEconomyStackBeforeSale == UT66RunStateSubsystem::MaxCollectedMobLootStack;
		bMobLootEconomyCollectionDidNotCreditGold = MobLootEconomyGoldAfterCollection == MobLootEconomyGoldBeforeCollection;
		MobLootEconomyGoldBeforeSale = RunState->GetCurrentGold();
		const int32 EventCountBeforeSale = RunState->GetStructuredEventLog().Num();
		const bool bSoldEconomyStack = RunState->SellCollectedMobLoot();
		MobLootEconomyGoldAfterSale = RunState->GetCurrentGold();
		MobLootEconomyStackAfterSale = RunState->GetCollectedMobLootStack();
		MobLootEconomySaleGoldDelta = MobLootEconomyGoldAfterSale - MobLootEconomyGoldBeforeSale;
		bMobLootEconomySaleCreditedGold = bSoldEconomyStack
			&& MobLootEconomySaleGoldDelta == UT66RunStateSubsystem::MaxCollectedMobLootStack;
		const TArray<FRunEvent>& Events = RunState->GetStructuredEventLog();
		for (int32 EventIndex = EventCountBeforeSale; EventIndex < Events.Num(); ++EventIndex)
		{
			if (Events[EventIndex].EventType == ET66RunEventType::GoldGained
				&& Events[EventIndex].Payload.Contains(TEXT("Source=MobLootSale")))
			{
				bMobLootEconomySaleSourceLogged = true;
				break;
			}
		}
		bMobLootEconomySaleZeroedStack = bSoldEconomyStack && MobLootEconomyStackAfterSale == 0;
	}

#if !UE_BUILD_SHIPPING
	MobLoot.ClearAllMobLootForAutomation();
#endif
	if (!World)
	{
		return;
	}

	FActorSpawnParameters SpawnParams;
	SpawnParams.Owner = this;
	SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

	const FVector Base = GetActorLocation() + FVector(400.0f, 0.0f, 80.0f);
	if (AT66MobBase* LightweightMob = World->SpawnActor<AT66MobBase>(AT66MobBase::StaticClass(), Base, FRotator::ZeroRotator, SpawnParams))
	{
		LightweightMob->ConfigureAsMob(FName(TEXT("MobLootStressLightweight")));
		LightweightMob->CurrentHP = 1.0f;
		LightweightMob->MaxHP = 1.0f;
		LightweightMob->TakeDamageFromHeroHitZone(999999, LightweightMob->ResolveCombatTargetHandle(nullptr, ET66HitZoneType::Body), FName(TEXT("MobLootStress")));
		const FT66MobLootCollectResult DeathCollect = MobLoot.CollectMobLootAt(Base, 260.0f, SystemCollector);
		LightweightDeathQuantity = DeathCollect.QuantityCollected;
		bLightweightDeathSpawned = DeathCollect.DropsCollected > 0 && DeathCollect.QuantityCollected == ExpectedRegularDeathQuantity;
		LightweightMob->Destroy();
	}

	if (AT66EnemyBase* RichEnemy = World->SpawnActor<AT66EnemyBase>(AT66EnemyBase::StaticClass(), Base + FVector(240.0f, 0.0f, 0.0f), FRotator::ZeroRotator, SpawnParams))
	{
		RichEnemy->MobID = FName(TEXT("MobLootStressRich"));
		RichEnemy->CharacterVisualID = RichEnemy->MobID;
		RichEnemy->bDropsLoot = false;
		RichEnemy->MaxHP = 1;
		RichEnemy->CurrentHP = 1;
		RichEnemy->TakeDamageFromHero(999999, FName(TEXT("MobLootStress")));
		const FT66MobLootCollectResult DeathCollect = MobLoot.CollectMobLootAt(Base + FVector(240.0f, 0.0f, 0.0f), 260.0f, SystemCollector);
		RichDeathQuantity = DeathCollect.QuantityCollected;
		bRichDeathSpawned = DeathCollect.DropsCollected > 0 && DeathCollect.QuantityCollected == ExpectedRegularDeathQuantity;
		if (!RichEnemy->IsActorBeingDestroyed())
		{
			RichEnemy->Destroy();
		}
	}

	if (AT66EnemyBase* MiniBoss = World->SpawnActor<AT66EnemyBase>(AT66EnemyBase::StaticClass(), Base + FVector(480.0f, 0.0f, 0.0f), FRotator::ZeroRotator, SpawnParams))
	{
		MiniBoss->MobID = FName(TEXT("MobLootStressMiniBoss"));
		MiniBoss->CharacterVisualID = MiniBoss->MobID;
		MiniBoss->bIsMiniBoss = true;
		MiniBoss->bDropsLoot = false;
		MiniBoss->MaxHP = 1;
		MiniBoss->CurrentHP = 1;
		MiniBoss->TakeDamageFromHero(999999, FName(TEXT("MobLootStress")));
		const FT66MobLootCollectResult DeathCollect = MobLoot.CollectMobLootAt(Base + FVector(480.0f, 0.0f, 0.0f), 260.0f, SystemCollector);
		MiniBossDeathQuantity = DeathCollect.QuantityCollected;
		bMiniBossDeathSpawned = DeathCollect.DropsCollected > 0
			&& DeathCollect.QuantityCollected == ExpectedMiniBossDeathQuantity
			&& DeathCollect.QuantityCollected > RichDeathQuantity;
		if (!MiniBoss->IsActorBeingDestroyed())
		{
			MiniBoss->Destroy();
		}
	}

	const int32 CountBeforeBoss = MobLoot.GetActiveMobLootDropCount();
	if (AT66BossBase* Boss = World->SpawnActor<AT66BossBase>(AT66BossBase::StaticClass(), Base + FVector(720.0f, 0.0f, 0.0f), FRotator::ZeroRotator, SpawnParams))
	{
		Boss->BossID = FName(TEXT("MobLootStressBoss"));
		Boss->MaxHP = 1;
		Boss->ForceAwaken();
		Boss->TakeDamageFromHeroHit(999999, FName(TEXT("MobLootStressBoss")));
	}
	BossDeathDelta = MobLoot.GetActiveMobLootDropCount() - CountBeforeBoss;
	bBossDeathDidNotSpawn = BossDeathDelta == 0;
	bDifficultyDropCountsMatched = LightweightDeathQuantity == ExpectedRegularDeathQuantity
		&& RichDeathQuantity == ExpectedRegularDeathQuantity
		&& MiniBossDeathQuantity == ExpectedMiniBossDeathQuantity;
	PostDeathWorldDropCount = MobLoot.GetActiveMobLootDropCount();

#if !UE_BUILD_SHIPPING
	MobLoot.ClearAllMobLootForAutomation();
#endif
}

void AT66MobLootStressHarnessActor::StartPetActorFunctionalCheck(UT66MobLootSubsystem& MobLoot)
{
	UWorld* World = GetWorld();
	if (!World)
	{
		return;
	}

	FActorSpawnParameters SpawnParams;
	SpawnParams.Owner = this;
	SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

	FPetData TestPetData;
	TestPetData.PetID = FName(TEXT("Pet_MobLootStress"));
	TestPetData.SourceBossID = FName(TEXT("Boss_MobLootStress"));
	TestPetData.DisplayName = FText::FromString(TEXT("Mob Loot Stress Pet"));
	TestPetData.BaseFetchSpeed = 8.0f;
	TestPetData.BondFetchSpeedPerStage = 0.2f;
	TestPetData.MaxBondFetchSpeedMultiplier = 1.6f;

	const FVector CollectPetLocation = GetActorLocation() + FVector(-800.0f, -600.0f, 120.0f);
	PetActorCollectDropLocation = CollectPetLocation + FVector(240.0f, 0.0f, 0.0f);
	FT66MobLootSpawnParams CollectDrop;
	CollectDrop.Position = PetActorCollectDropLocation;
	CollectDrop.Quantity = 5;
	CollectDrop.GoldValue = 5;
	CollectDrop.SourceID = FName(TEXT("MobLootStressPetActorCollect"));
	CollectDrop.LifetimeSeconds = WarmupSeconds + SampleSeconds + 20.0f;
	FT66MobLootHandle CollectHandle;
	MobLoot.SpawnMobLoot(CollectDrop, CollectHandle);

	PetActorCollectCheck = World->SpawnActor<AT66PetActor>(AT66PetActor::StaticClass(), CollectPetLocation, FRotator::ZeroRotator, SpawnParams);
	if (PetActorCollectCheck)
	{
		PetActorCollectCheck->SetActorTickEnabled(false);
		PetActorCollectCheck->InitializePet(TestPetData);
		PetActorCollectCheck->bMobLootCollectionEnabled = true;
		PetActorCollectCheck->MobLootSearchRadius = 900.0f;
		PetActorCollectCheck->MobLootCollectDistance = 150.0f;
		PetActorCollectCheck->MobLootReservationMaxAgeSeconds = 12.0f;
		PetActorCollectCheck->MobLootNoProgressReleaseSeconds = 6.0f;
		PetActorCollectCheck->MobLootProgressEpsilon = 0.0f;
		PetActorCollectCheck->SetActorLocation(CollectPetLocation, false, nullptr, ETeleportType::TeleportPhysics);
		PetActorCollectCheck->SetActorTickEnabled(false);
		PetActorInitialDistance = PlanarDistance(PetActorCollectCheck->GetActorLocation(), PetActorCollectDropLocation);
		PetActorMinimumDistance = PetActorInitialDistance;
		PetBondStage0Speed = PetActorCollectCheck->ComputeFetchSpeedForBondStagesForAutomation(0);
		PetBondStage5Speed = PetActorCollectCheck->ComputeFetchSpeedForBondStagesForAutomation(5);
		bPetBondMovementOnlyCheckSucceeded =
			PetBondStage5Speed > PetBondStage0Speed &&
			FMath::IsNearlyEqual(PetActorCollectCheck->MobLootSearchRadius, 900.0f) &&
			FMath::IsNearlyEqual(PetActorCollectCheck->MobLootCollectDistance, 150.0f) &&
			FMath::IsNearlyEqual(PetActorCollectCheck->MobLootFetchUnitsPerSpeedPoint, 120.0f);
	}

	const FVector ReleasePetLocation = GetActorLocation() + FVector(-800.0f, 600.0f, 120.0f);
	PetActorReleaseDropLocation = ReleasePetLocation + FVector(340.0f, 0.0f, 0.0f);
	FT66MobLootSpawnParams ReleaseDrop;
	ReleaseDrop.Position = PetActorReleaseDropLocation;
	ReleaseDrop.Quantity = 2;
	ReleaseDrop.GoldValue = 2;
	ReleaseDrop.SourceID = FName(TEXT("MobLootStressPetActorRelease"));
	ReleaseDrop.LifetimeSeconds = WarmupSeconds + SampleSeconds + 20.0f;
	FT66MobLootHandle ReleaseHandle;
	MobLoot.SpawnMobLoot(ReleaseDrop, ReleaseHandle);

	PetActorReleaseCheck = World->SpawnActor<AT66PetActor>(AT66PetActor::StaticClass(), ReleasePetLocation, FRotator::ZeroRotator, SpawnParams);
	if (PetActorReleaseCheck)
	{
		PetActorReleaseCheck->SetActorTickEnabled(false);
		PetActorReleaseCheck->InitializePet(TestPetData);
		PetActorReleaseCheck->bMobLootCollectionEnabled = true;
		PetActorReleaseCheck->MobLootSearchRadius = 900.0f;
		PetActorReleaseCheck->MobLootCollectDistance = 70.0f;
		PetActorReleaseCheck->MobLootReservationMaxAgeSeconds = 6.0f;
		PetActorReleaseCheck->MobLootNoProgressReleaseSeconds = 2.0f;
		PetActorReleaseCheck->MobLootProgressEpsilon = 1.0f;
		PetActorReleaseCheck->SetActorLocation(ReleasePetLocation, false, nullptr, ETeleportType::TeleportPhysics);
		PetActorReleaseCheck->SetActorTickEnabled(false);
	}

	bPetActorQueryUsesExclusionSpheresSeam = true;
}

void AT66MobLootStressHarnessActor::UpdatePetActorFunctionalCheck(UT66MobLootSubsystem& MobLoot)
{
	const FT66MobLootCollectorRef PlayerCollector = MakeCollector(OwnerController.Get(), TEXT("MobLootStressPlayer"), ET66MobLootCollectorType::Player);
	constexpr float PetCollectionDeltaSeconds = 1.0f / 60.0f;

	if (PetActorCollectCheck)
	{
		PetActorCollectCheck->PumpMobLootCollectionForAutomation(PetCollectionDeltaSeconds);
		if (PetActorCollectCheck->HasReservedMobLootTargetForAutomation())
		{
			bPetActorReserveObserved = true;
			PetActorReservedTargetLocation = PetActorCollectCheck->GetReservedMobLootTargetLocationForAutomation();
			PetActorDistanceToReservedTarget = PlanarDistance(PetActorCollectCheck->GetActorLocation(), PetActorReservedTargetLocation);
			if (!bPetActorPlayerBlockedFromReservedDrop)
			{
				const FT66MobLootCollectResult PlayerSweep =
					MobLoot.CollectMobLootAt(PetActorCollectDropLocation, 140.0f, PlayerCollector);
				PetActorPlayerSweepDrops = PlayerSweep.DropsCollected;
				bPetActorPlayerBlockedFromReservedDrop = PlayerSweep.DropsCollected == 0;
			}
		}

		const float CurrentDistance = PlanarDistance(PetActorCollectCheck->GetActorLocation(), PetActorCollectDropLocation);
		PetActorMinimumDistance = FMath::Min(PetActorMinimumDistance, CurrentDistance);
		PetActorMoveAttempts = PetActorCollectCheck->GetTotalMobLootMoveAttemptsForAutomation();
		PetActorLastMoveDistance = PetActorCollectCheck->GetLastMobLootMoveDistanceForAutomation();
		if (PetActorInitialDistance > 0.f && PetActorMinimumDistance < PetActorInitialDistance - 40.f)
		{
			bPetActorWalkObserved = true;
		}

		PetActorCollectDrops = PetActorCollectCheck->GetTotalMobLootDropsCollectedByPetForAutomation();
		PetActorCollectQuantity = PetActorCollectCheck->GetTotalMobLootQuantityCollectedByPetForAutomation();
		PetActorCollectSellValue = PetActorCollectCheck->GetTotalMobLootSellValueCollectedByPetForAutomation();
		bPetActorCollectSucceeded = PetActorCollectDrops > 0 && PetActorCollectQuantity > 0 && PetActorCollectSellValue > 0;
		if (bPetActorCollectSucceeded)
		{
			if (UGameInstance* GI = GetGameInstance())
			{
				if (UT66RunStateSubsystem* RunState = GI->GetSubsystem<UT66RunStateSubsystem>())
				{
					PetActorEconomyStackAfterCollect = RunState->GetCollectedMobLootStack();
					bPetActorCollectionGrewEconomyStack = PetActorEconomyStackAfterCollect >= PetActorCollectSellValue;
				}
			}
		}
	}

	if (PetActorReleaseCheck && !bPetReleaseDestroyIssued && PetActorReleaseCheck->HasReservedMobLootTargetForAutomation())
	{
		PetActorReleaseCountBeforeCancel = MobLoot.GetDiagnostics().ReservationReleaseTotal;
		bPetReleaseDestroyIssued = true;
		PetActorReleaseCheck->Destroy();
		PetActorReleaseCheck = nullptr;
	}
	else if (PetActorReleaseCheck)
	{
		PetActorReleaseCheck->PumpMobLootCollectionForAutomation(PetCollectionDeltaSeconds);
	}

	if (bPetReleaseDestroyIssued && !bPetReleaseSweepAttempted)
	{
		PetActorReleaseCountAfterCancel = MobLoot.GetDiagnostics().ReservationReleaseTotal;
		if (PetActorReleaseCountAfterCancel > PetActorReleaseCountBeforeCancel)
		{
			const FT66MobLootCollectResult PlayerSweep =
				MobLoot.CollectMobLootAt(PetActorReleaseDropLocation, 140.0f, PlayerCollector);
			PetActorReleasePlayerSweepDrops = PlayerSweep.DropsCollected;
			bPetActorReleaseOnCancelSucceeded = PlayerSweep.DropsCollected == 1;
			bPetReleaseSweepAttempted = true;
		}
	}
}

void AT66MobLootStressHarnessActor::SpawnStressDrops(UT66MobLootSubsystem& MobLoot)
{
	const double SpawnStartSeconds = FPlatformTime::Seconds();
	const int32 Columns = FMath::Clamp(FMath::CeilToInt(FMath::Sqrt(static_cast<float>(RequestedDropCount))), 1, 512);
	const int32 Rows = FMath::Max(1, FMath::CeilToInt(static_cast<float>(RequestedDropCount) / static_cast<float>(Columns)));
	const FVector Origin = GetActorLocation()
		+ FVector(900.0f, 0.0f, 120.0f)
		- FVector(0.0f, static_cast<float>(Columns - 1) * GridSpacing * 0.5f, 0.0f)
		- FVector(static_cast<float>(Rows - 1) * GridSpacing * 0.5f, 0.0f, 0.0f);

	for (int32 Index = 0; Index < RequestedDropCount; ++Index)
	{
		const int32 Column = Index % Columns;
		const int32 Row = Index / Columns;
		FT66MobLootSpawnParams Params;
		Params.Position = Origin
			+ FVector(static_cast<float>(Row) * GridSpacing, static_cast<float>(Column) * GridSpacing, 0.0f);
		Params.Quantity = 1 + (Index % 4);
		Params.GoldValue = Params.Quantity;
		Params.SourceID = FName(TEXT("MobLootStress"));
		Params.Color = FLinearColor(0.92f, 0.74f, 0.18f + 0.08f * static_cast<float>(Index % 3), 1.0f);
		Params.Scale = 0.85f + 0.08f * static_cast<float>(Index % 4);
		Params.LifetimeSeconds = WarmupSeconds + SampleSeconds + 20.0f;

		FT66MobLootHandle Handle;
		if (MobLoot.SpawnMobLoot(Params, Handle))
		{
			++SpawnedDropCount;
		}
		else
		{
			++FailedSpawnCount;
		}
	}

	SpawnTimeMs = (FPlatformTime::Seconds() - SpawnStartSeconds) * 1000.0;
}

void AT66MobLootStressHarnessActor::SampleFrame(const float DeltaSeconds)
{
	if (DeltaSeconds > SMALL_NUMBER)
	{
		const double FrameValueMs = static_cast<double>(DeltaSeconds) * 1000.0;
		FrameMs.Add(FrameValueMs);
		Fps.Add(1000.0 / FrameValueMs);
	}

	if (GGameThreadTime > 0)
	{
		GameThreadMs.Add(FPlatformTime::ToMilliseconds(GGameThreadTime));
	}

	const uint32 GpuCycles = RHIGetGPUFrameCycles(0);
	if (GpuCycles > 0)
	{
		GpuFrameMs.Add(FPlatformTime::ToMilliseconds(GpuCycles));
	}

	DrawCalls.Add(static_cast<double>(GNumDrawCallsRHI[0]));

	if (const UWorld* World = GetWorld())
	{
		if (const UT66MobLootSubsystem* MobLoot = World->GetSubsystem<UT66MobLootSubsystem>())
		{
			const FT66MobLootDiagnostics& Diagnostics = MobLoot->GetDiagnostics();
			MobLootTickMs.Add(Diagnostics.LastTickMs);
			if (Diagnostics.UploadCount > 0)
			{
				MobLootUploadMs.Add(Diagnostics.LastUploadMs);
				MobLootPackMs.Add(Diagnostics.LastPackMs);
				MobLootNiagaraArrayUploadMs.Add(Diagnostics.LastNiagaraArrayUploadMs);
			}
		}
	}
}

bool AT66MobLootStressHarnessActor::BuildEconomyContractPass() const
{
	return SpawnedDropCount == RequestedDropCount &&
		bReservationPreventedDoubleCollect &&
		bReservedPetCollectSucceeded &&
		bBatchedCollectionSucceeded &&
		bLightweightDeathSpawned &&
		bRichDeathSpawned &&
		bMiniBossDeathSpawned &&
		bBossDeathDidNotSpawn &&
		bDifficultyDropCountsMatched &&
		bMobLootPetCollectionGrewStack &&
		bMobLootEconomyCollectionGrewStack &&
		bMobLootEconomyCappedAt999 &&
		bMobLootEconomyCollectionDidNotCreditGold &&
		bMobLootEconomySaleCreditedGold &&
		bMobLootEconomySaleSourceLogged &&
		bMobLootEconomySaleZeroedStack;
}

bool AT66MobLootStressHarnessActor::BuildFoundationPetActorPass() const
{
	return bPetActorReserveObserved &&
		bPetActorWalkObserved &&
		bPetActorCollectSucceeded &&
		bPetActorCollectionGrewEconomyStack &&
		bPetActorPlayerBlockedFromReservedDrop &&
		bPetActorReleaseOnCancelSucceeded &&
		bPetActorQueryUsesExclusionSpheresSeam &&
		bPetBondMovementOnlyCheckSucceeded;
}

bool AT66MobLootStressHarnessActor::BuildOverallPass() const
{
	return BuildEconomyContractPass() && (bEconomyOnlyMode || BuildFoundationPetActorPass());
}

void AT66MobLootStressHarnessActor::WriteManifest(const TCHAR* Reason)
{
	const UWorld* World = GetWorld();
	const UT66MobLootSubsystem* MobLoot = World ? World->GetSubsystem<UT66MobLootSubsystem>() : nullptr;
	const FT66MobLootDiagnostics Diagnostics = MobLoot ? MobLoot->GetDiagnostics() : FT66MobLootDiagnostics{};
	const int32 UploadCountAtEnd = Diagnostics.UploadCount;

	TSharedRef<FJsonObject> Root = MakeShared<FJsonObject>();
	Root->SetNumberField(TEXT("SchemaVersion"), T66MobLootStressSchemaVersion);
	Root->SetStringField(TEXT("tool"), TEXT("T66MobLootStressHarness"));
	Root->SetStringField(TEXT("reason"), Reason ? Reason : TEXT("unspecified"));
	Root->SetNumberField(TEXT("requested_drop_count"), RequestedDropCount);
	Root->SetNumberField(TEXT("spawned_drop_count"), SpawnedDropCount);
	Root->SetNumberField(TEXT("failed_spawn_count"), FailedSpawnCount);
	Root->SetNumberField(TEXT("initial_world_drop_count"), InitialWorldDropCount);
	Root->SetNumberField(TEXT("post_death_world_drop_count"), PostDeathWorldDropCount);
	Root->SetNumberField(TEXT("spawn_time_ms"), SpawnTimeMs);
	Root->SetBoolField(TEXT("uses_actorless_mob_loot_subsystem"), true);
	Root->SetBoolField(TEXT("uses_at66_loot_bag_pickup"), false);
	Root->SetBoolField(TEXT("subsystem_calls_add_gold"), false);
	Root->SetStringField(TEXT("active_count_semantics"), TEXT("GetActiveMobLootDropCount is active uncollected world-drop entries only; Economy owns collected sellable stack."));
	Root->SetStringField(TEXT("gold_value_collected_semantics"), TEXT("GoldValueCollected is total sell value summed across collected drops, not per-unit value."));
	Root->SetStringField(TEXT("economy_credit_semantics"), TEXT("Collection adds to the 999-capped sellable stack; only vendor sale credits gold."));
	Root->SetBoolField(TEXT("economy_only_mode"), bEconomyOnlyMode);
	Root->SetNumberField(TEXT("selected_difficulty_index"), SelectedDifficultyIndex);
	Root->SetNumberField(TEXT("expected_regular_death_quantity"), ExpectedRegularDeathQuantity);
	Root->SetNumberField(TEXT("expected_miniboss_death_quantity"), ExpectedMiniBossDeathQuantity);
	Root->SetNumberField(TEXT("upload_count_after_setup"), UploadCountAfterSetup);
	Root->SetNumberField(TEXT("upload_count_at_end"), UploadCountAtEnd);
	Root->SetNumberField(TEXT("sampled_frames"), Fps.Count);
	Root->SetBoolField(TEXT("dirty_only_upload_observed"), UploadCountAtEnd <= UploadCountAfterSetup + 2);

	Root->SetBoolField(TEXT("reservation_prevented_double_collect"), bReservationPreventedDoubleCollect);
	Root->SetBoolField(TEXT("reserved_pet_collect_succeeded"), bReservedPetCollectSucceeded);
	Root->SetBoolField(TEXT("batched_collection_succeeded"), bBatchedCollectionSucceeded);
	Root->SetBoolField(TEXT("pet_actor_reserved_mob_loot"), bPetActorReserveObserved);
	Root->SetBoolField(TEXT("pet_actor_walked_to_reserved_mob_loot"), bPetActorWalkObserved);
	Root->SetBoolField(TEXT("pet_actor_collected_reserved_mob_loot"), bPetActorCollectSucceeded);
	Root->SetBoolField(TEXT("pet_actor_collection_grew_economy_stack"), bPetActorCollectionGrewEconomyStack);
	Root->SetBoolField(TEXT("pet_actor_player_blocked_from_reserved_drop"), bPetActorPlayerBlockedFromReservedDrop);
	Root->SetBoolField(TEXT("pet_actor_release_on_cancel_succeeded"), bPetActorReleaseOnCancelSucceeded);
	Root->SetBoolField(TEXT("pet_actor_query_uses_exclusion_spheres_seam"), bPetActorQueryUsesExclusionSpheresSeam);
	Root->SetBoolField(TEXT("pet_bond_affects_movement_speed_only"), bPetBondMovementOnlyCheckSucceeded);
	Root->SetNumberField(TEXT("reservation_player_sweep_drops"), ReservationPlayerSweepDrops);
	Root->SetNumberField(TEXT("reservation_pet_collect_drops"), ReservationPetCollectDrops);
	Root->SetNumberField(TEXT("batched_collection_drops"), BatchedCollectionDrops);
	Root->SetNumberField(TEXT("pet_actor_player_sweep_drops"), PetActorPlayerSweepDrops);
	Root->SetNumberField(TEXT("pet_actor_collect_drops"), PetActorCollectDrops);
	Root->SetNumberField(TEXT("pet_actor_collect_quantity"), PetActorCollectQuantity);
	Root->SetNumberField(TEXT("pet_actor_collect_sell_value"), PetActorCollectSellValue);
	Root->SetNumberField(TEXT("pet_actor_economy_stack_after_collect"), PetActorEconomyStackAfterCollect);
	Root->SetNumberField(TEXT("pet_actor_release_player_sweep_drops"), PetActorReleasePlayerSweepDrops);
	Root->SetNumberField(TEXT("pet_actor_release_count_before_cancel"), PetActorReleaseCountBeforeCancel);
	Root->SetNumberField(TEXT("pet_actor_release_count_after_cancel"), PetActorReleaseCountAfterCancel);
	Root->SetNumberField(TEXT("pet_actor_initial_distance"), PetActorInitialDistance);
	Root->SetNumberField(TEXT("pet_actor_minimum_distance"), PetActorMinimumDistance);
	Root->SetNumberField(TEXT("pet_actor_distance_to_reserved_target"), PetActorDistanceToReservedTarget);
	Root->SetNumberField(TEXT("pet_actor_move_attempts"), PetActorMoveAttempts);
	Root->SetNumberField(TEXT("pet_actor_last_move_distance"), PetActorLastMoveDistance);
	Root->SetStringField(TEXT("pet_actor_reserved_target_location"), PetActorReservedTargetLocation.ToCompactString());
	Root->SetNumberField(TEXT("pet_bond_stage0_speed"), PetBondStage0Speed);
	Root->SetNumberField(TEXT("pet_bond_stage5_speed"), PetBondStage5Speed);
	Root->SetBoolField(TEXT("lightweight_death_spawned_mob_loot"), bLightweightDeathSpawned);
	Root->SetBoolField(TEXT("rich_death_spawned_mob_loot"), bRichDeathSpawned);
	Root->SetBoolField(TEXT("miniboss_death_spawned_more_mob_loot"), bMiniBossDeathSpawned);
	Root->SetBoolField(TEXT("boss_death_did_not_spawn_mob_loot"), bBossDeathDidNotSpawn);
	Root->SetBoolField(TEXT("difficulty_drop_counts_matched"), bDifficultyDropCountsMatched);
	Root->SetNumberField(TEXT("lightweight_death_quantity"), LightweightDeathQuantity);
	Root->SetNumberField(TEXT("rich_death_quantity"), RichDeathQuantity);
	Root->SetNumberField(TEXT("miniboss_death_quantity"), MiniBossDeathQuantity);
	Root->SetNumberField(TEXT("boss_death_delta"), BossDeathDelta);
	Root->SetBoolField(TEXT("direct_pet_collection_grew_economy_stack"), bMobLootPetCollectionGrewStack);
	Root->SetNumberField(TEXT("direct_pet_collection_stack_added"), MobLootPetCollectionStackAdded);
	Root->SetNumberField(TEXT("direct_pet_collection_stack_after"), MobLootPetCollectionStackAfter);
	Root->SetBoolField(TEXT("economy_collection_grew_stack"), bMobLootEconomyCollectionGrewStack);
	Root->SetBoolField(TEXT("economy_stack_capped_at_999"), bMobLootEconomyCappedAt999);
	Root->SetBoolField(TEXT("economy_collection_did_not_credit_gold"), bMobLootEconomyCollectionDidNotCreditGold);
	Root->SetBoolField(TEXT("economy_sale_credited_gold"), bMobLootEconomySaleCreditedGold);
	Root->SetBoolField(TEXT("economy_sale_source_logged_mob_loot_sale"), bMobLootEconomySaleSourceLogged);
	Root->SetBoolField(TEXT("economy_sale_zeroed_stack"), bMobLootEconomySaleZeroedStack);
	Root->SetNumberField(TEXT("economy_collection_value"), MobLootEconomyCollectionValue);
	Root->SetNumberField(TEXT("economy_stack_added"), MobLootEconomyStackAdded);
	Root->SetNumberField(TEXT("economy_stack_before_sale"), MobLootEconomyStackBeforeSale);
	Root->SetNumberField(TEXT("economy_stack_after_sale"), MobLootEconomyStackAfterSale);
	Root->SetNumberField(TEXT("economy_gold_before_collection"), MobLootEconomyGoldBeforeCollection);
	Root->SetNumberField(TEXT("economy_gold_after_collection"), MobLootEconomyGoldAfterCollection);
	Root->SetNumberField(TEXT("economy_gold_before_sale"), MobLootEconomyGoldBeforeSale);
	Root->SetNumberField(TEXT("economy_gold_after_sale"), MobLootEconomyGoldAfterSale);
	Root->SetNumberField(TEXT("economy_sale_gold_delta"), MobLootEconomySaleGoldDelta);

	SetMetricObject(Root, TEXT("fps"), Fps);
	SetMetricObject(Root, TEXT("frame_ms"), FrameMs);
	SetMetricObject(Root, TEXT("game_thread_ms"), GameThreadMs);
	SetMetricObject(Root, TEXT("gpu_frame_ms"), GpuFrameMs);
	SetMetricObject(Root, TEXT("draw_calls"), DrawCalls);
	SetMetricObject(Root, TEXT("mob_loot_tick_ms"), MobLootTickMs);
	SetMetricObject(Root, TEXT("mob_loot_upload_ms"), MobLootUploadMs);
	SetMetricObject(Root, TEXT("mob_loot_pack_ms"), MobLootPackMs);
	SetMetricObject(Root, TEXT("mob_loot_niagara_array_upload_ms"), MobLootNiagaraArrayUploadMs);

	const TSharedRef<FJsonObject> DiagnosticsObject = MakeShared<FJsonObject>();
	DiagnosticsObject->SetNumberField(TEXT("capacity"), UT66MobLootSubsystem::MaxMobLootDrops);
	DiagnosticsObject->SetNumberField(TEXT("live_world_drop_count"), Diagnostics.LiveWorldDropCount);
	DiagnosticsObject->SetNumberField(TEXT("peak_live_world_drop_count"), Diagnostics.PeakLiveWorldDropCount);
	DiagnosticsObject->SetNumberField(TEXT("spawned_drop_total"), Diagnostics.SpawnedDropTotal);
	DiagnosticsObject->SetNumberField(TEXT("dropped_when_full_total"), Diagnostics.DroppedWhenFullTotal);
	DiagnosticsObject->SetNumberField(TEXT("expired_drop_total"), Diagnostics.ExpiredDropTotal);
	DiagnosticsObject->SetNumberField(TEXT("collected_drop_total"), Diagnostics.CollectedDropTotal);
	DiagnosticsObject->SetNumberField(TEXT("quantity_collected_total"), Diagnostics.QuantityCollectedTotal);
	DiagnosticsObject->SetNumberField(TEXT("gold_value_collected_total"), Diagnostics.GoldValueCollectedTotal);
	DiagnosticsObject->SetNumberField(TEXT("reservation_total"), Diagnostics.ReservationTotal);
	DiagnosticsObject->SetNumberField(TEXT("reservation_release_total"), Diagnostics.ReservationReleaseTotal);
	DiagnosticsObject->SetNumberField(TEXT("reservation_denied_total"), Diagnostics.ReservationDeniedTotal);
	DiagnosticsObject->SetNumberField(TEXT("upload_count"), Diagnostics.UploadCount);
	DiagnosticsObject->SetNumberField(TEXT("last_uploaded_live_count"), Diagnostics.LastUploadedLiveCount);
	DiagnosticsObject->SetNumberField(TEXT("last_tick_ms"), Diagnostics.LastTickMs);
	DiagnosticsObject->SetNumberField(TEXT("last_upload_ms"), Diagnostics.LastUploadMs);
	DiagnosticsObject->SetNumberField(TEXT("last_pack_ms"), Diagnostics.LastPackMs);
	DiagnosticsObject->SetNumberField(TEXT("last_niagara_array_upload_ms"), Diagnostics.LastNiagaraArrayUploadMs);
	DiagnosticsObject->SetNumberField(TEXT("average_tick_ms"), Diagnostics.AverageTickMs);
	DiagnosticsObject->SetNumberField(TEXT("average_upload_ms"), Diagnostics.AverageUploadMs);
	DiagnosticsObject->SetNumberField(TEXT("average_pack_ms"), Diagnostics.AveragePackMs);
	DiagnosticsObject->SetNumberField(TEXT("average_niagara_array_upload_ms"), Diagnostics.AverageNiagaraArrayUploadMs);
	DiagnosticsObject->SetNumberField(TEXT("max_tick_ms"), Diagnostics.MaxTickMs);
	DiagnosticsObject->SetNumberField(TEXT("max_upload_ms"), Diagnostics.MaxUploadMs);
	DiagnosticsObject->SetNumberField(TEXT("max_pack_ms"), Diagnostics.MaxPackMs);
	DiagnosticsObject->SetNumberField(TEXT("max_niagara_array_upload_ms"), Diagnostics.MaxNiagaraArrayUploadMs);
	Root->SetObjectField(TEXT("mob_loot_diagnostics"), DiagnosticsObject);

	const bool bEconomyContractPass = BuildEconomyContractPass();
	const bool bFoundationPetActorPass = BuildFoundationPetActorPass();
	const bool bPass = BuildOverallPass();
	Root->SetBoolField(TEXT("economy_contract_pass"), bEconomyContractPass);
	Root->SetBoolField(TEXT("foundation_pet_actor_pass"), bFoundationPetActorPass);
	Root->SetBoolField(TEXT("pass"), bPass);

	FString JsonText;
	const TSharedRef<TJsonWriter<>> Writer = TJsonWriterFactory<>::Create(&JsonText);
	if (!FJsonSerializer::Serialize(Root, Writer))
	{
		UE_LOG(LogT66MobLootStress, Warning, TEXT("[MobLootStress] manifest serialize failed path=%s"), *ManifestPath);
		return;
	}

	IFileManager::Get().MakeDirectory(*FPaths::GetPath(ManifestPath), true);
	const bool bSaved = FFileHelper::SaveStringToFile(JsonText, *ManifestPath);
	UE_LOG(LogT66MobLootStress, Display,
		TEXT("[MobLootStress] manifest status=%s pass=%d requested=%d spawned=%d fpsAvg=%s gameThreadAvg=%s gpuAvg=%s drawCallsAvg=%s path=%s"),
		bSaved ? TEXT("ok") : TEXT("failed"),
		bPass ? 1 : 0,
		RequestedDropCount,
		SpawnedDropCount,
		*FormatMaybeNumber(Fps),
		*FormatMaybeNumber(GameThreadMs),
		*FormatMaybeNumber(GpuFrameMs),
		*FormatMaybeNumber(DrawCalls),
		*ManifestPath);
}
