// Copyright Tribulation 66. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Gameplay/T66MobLootSubsystem.h"
#include "T66MobLootStressHarnessActor.generated.h"

class APlayerController;
class AT66PetActor;

UCLASS(NotPlaceable)
class T66_API AT66MobLootStressHarnessActor : public AActor
{
	GENERATED_BODY()

public:
	AT66MobLootStressHarnessActor();

	void StartFromCommandLine(APlayerController* InOwnerController);

	struct FMetricAccumulator
	{
		int32 Count = 0;
		double Sum = 0.0;
		double Min = 0.0;
		double Max = 0.0;

		void Add(double Value);
		double Average() const;
	};

protected:
	virtual void Tick(float DeltaSeconds) override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

private:
	UPROPERTY(Transient)
	TObjectPtr<APlayerController> OwnerController = nullptr;

	FString ManifestPath;
	int32 RequestedDropCount = 5000;
	int32 SpawnedDropCount = 0;
	int32 FailedSpawnCount = 0;
	int32 InitialWorldDropCount = 0;
	int32 PostDeathWorldDropCount = 0;
	int32 UploadCountAfterSetup = 0;
	float GridSpacing = 34.0f;
	float WarmupSeconds = 0.75f;
	float SampleSeconds = 2.5f;
	bool bStarted = false;
	bool bManifestWritten = false;
	bool bEconomyOnlyMode = false;
	double StartSeconds = 0.0;
	double SpawnTimeMs = 0.0;

	bool bReservationPreventedDoubleCollect = false;
	bool bReservedPetCollectSucceeded = false;
	bool bBatchedCollectionSucceeded = false;
	bool bLightweightDeathSpawned = false;
	bool bRichDeathSpawned = false;
	bool bMiniBossDeathSpawned = false;
	bool bBossDeathDidNotSpawn = false;
	bool bDifficultyDropCountsMatched = false;
	bool bMobLootPetCollectionGrewStack = false;
	bool bMobLootEconomyCollectionGrewStack = false;
	bool bMobLootEconomyCappedAt999 = false;
	bool bMobLootEconomyCollectionDidNotCreditGold = false;
	bool bMobLootEconomySaleCreditedGold = false;
	bool bMobLootEconomySaleSourceLogged = false;
	bool bMobLootEconomySaleZeroedStack = false;
	bool bPetActorReserveObserved = false;
	bool bPetActorWalkObserved = false;
	bool bPetActorCollectSucceeded = false;
	bool bPetActorCollectionGrewEconomyStack = false;
	bool bPetActorPlayerBlockedFromReservedDrop = false;
	bool bPetActorReleaseOnCancelSucceeded = false;
	bool bPetActorQueryUsesExclusionSpheresSeam = false;
	bool bPetBondMovementOnlyCheckSucceeded = false;
	int32 ReservationPlayerSweepDrops = 0;
	int32 ReservationPetCollectDrops = 0;
	int32 BatchedCollectionDrops = 0;
	int32 SelectedDifficultyIndex = 0;
	int32 ExpectedRegularDeathQuantity = 1;
	int32 ExpectedMiniBossDeathQuantity = 4;
	int32 LightweightDeathQuantity = 0;
	int32 RichDeathQuantity = 0;
	int32 MiniBossDeathQuantity = 0;
	int32 BossDeathDelta = 0;
	int32 MobLootPetCollectionStackAdded = 0;
	int32 MobLootPetCollectionStackAfter = 0;
	int32 MobLootEconomyCollectionValue = 0;
	int32 MobLootEconomyStackAdded = 0;
	int32 MobLootEconomyStackBeforeSale = 0;
	int32 MobLootEconomyStackAfterSale = 0;
	int32 MobLootEconomyGoldBeforeCollection = 0;
	int32 MobLootEconomyGoldAfterCollection = 0;
	int32 MobLootEconomyGoldBeforeSale = 0;
	int32 MobLootEconomyGoldAfterSale = 0;
	int32 MobLootEconomySaleGoldDelta = 0;
	int32 PetActorPlayerSweepDrops = INDEX_NONE;
	int32 PetActorCollectDrops = 0;
	int32 PetActorCollectQuantity = 0;
	int32 PetActorCollectSellValue = 0;
	int32 PetActorEconomyStackAfterCollect = 0;
	int32 PetActorReleasePlayerSweepDrops = 0;
	int32 PetActorReleaseCountBeforeCancel = 0;
	int32 PetActorReleaseCountAfterCancel = 0;
	float PetActorInitialDistance = 0.f;
	float PetActorMinimumDistance = 0.f;
	float PetActorDistanceToReservedTarget = 0.f;
	float PetActorLastMoveDistance = 0.f;
	float PetBondStage0Speed = 0.f;
	float PetBondStage5Speed = 0.f;
	int32 PetActorMoveAttempts = 0;
	bool bPetReleaseDestroyIssued = false;
	bool bPetReleaseSweepAttempted = false;
	FVector PetActorCollectDropLocation = FVector::ZeroVector;
	FVector PetActorReleaseDropLocation = FVector::ZeroVector;
	FVector PetActorReservedTargetLocation = FVector::ZeroVector;

	UPROPERTY(Transient)
	TObjectPtr<AT66PetActor> PetActorCollectCheck = nullptr;

	UPROPERTY(Transient)
	TObjectPtr<AT66PetActor> PetActorReleaseCheck = nullptr;

	FMetricAccumulator FrameMs;
	FMetricAccumulator Fps;
	FMetricAccumulator GameThreadMs;
	FMetricAccumulator GpuFrameMs;
	FMetricAccumulator DrawCalls;
	FMetricAccumulator MobLootTickMs;
	FMetricAccumulator MobLootUploadMs;
	FMetricAccumulator MobLootPackMs;
	FMetricAccumulator MobLootNiagaraArrayUploadMs;

	void RunFunctionalChecks(UT66MobLootSubsystem& MobLoot);
	void StartPetActorFunctionalCheck(UT66MobLootSubsystem& MobLoot);
	void UpdatePetActorFunctionalCheck(UT66MobLootSubsystem& MobLoot);
	void SpawnStressDrops(UT66MobLootSubsystem& MobLoot);
	void SampleFrame(float DeltaSeconds);
	bool BuildEconomyContractPass() const;
	bool BuildFoundationPetActorPass() const;
	bool BuildOverallPass() const;
	void WriteManifest(const TCHAR* Reason);
};
