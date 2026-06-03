// Copyright Tribulation 66. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Data/T66DataTypes.h"
#include "Gameplay/T66WorldSystemsAPI.h"
#include "T66PetActor.generated.h"

class USceneComponent;
class UStaticMeshComponent;
class UT66MobLootSubsystem;

/**
 * One active pet follows the hero during a run.
 * Mob Loot targeting/collection uses Foundation's actorless reservation API.
 */
UCLASS(Blueprintable)
class T66_API AT66PetActor : public AActor
{
	GENERATED_BODY()

public:
	AT66PetActor();

	UPROPERTY(BlueprintReadOnly, Category = "Pet")
	FName PetID = NAME_None;

	UPROPERTY(BlueprintReadOnly, Category = "Pet")
	FPetData PetData;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Visuals")
	TObjectPtr<UStaticMeshComponent> VisualMesh;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Follow")
	FVector FollowOffset = FVector(-95.f, -120.f, 0.f);

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Follow")
	float ReturnFollowSpeed = 8.f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Follow")
	float GroundFollowSpeed = 12.f;

	/** Enables Foundation-side Mob Loot reserve/walk/collect behavior. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Mob Loot")
	bool bMobLootCollectionEnabled = false;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Mob Loot", meta = (ClampMin = "0.0"))
	float MobLootSearchRadius = 2400.f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Mob Loot", meta = (ClampMin = "1.0"))
	float MobLootCollectDistance = 120.f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Mob Loot", meta = (ClampMin = "1.0"))
	float MobLootFetchUnitsPerSpeedPoint = 120.f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Mob Loot", meta = (ClampMin = "0.1"))
	float MobLootReservationMaxAgeSeconds = 8.f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Mob Loot", meta = (ClampMin = "0.1"))
	float MobLootNoProgressReleaseSeconds = 2.5f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Mob Loot", meta = (ClampMin = "0.0"))
	float MobLootProgressEpsilon = 8.f;

	UFUNCTION(BlueprintCallable, Category = "Pet")
	void InitializePet(const FPetData& InData, FName SkinID = NAME_None);

	bool HasReservedMobLootTargetForAutomation() const { return bHasReservedMobLootTarget && ReservedMobLootHandle.IsValid(); }
	int32 GetTotalMobLootDropsCollectedByPetForAutomation() const { return TotalMobLootDropsCollected; }
	int32 GetTotalMobLootQuantityCollectedByPetForAutomation() const { return TotalMobLootQuantityCollected; }
	int32 GetTotalMobLootSellValueCollectedByPetForAutomation() const { return TotalMobLootSellValueCollected; }
	int32 GetTotalMobLootReservationsReleasedForAutomation() const { return TotalMobLootReservationsReleased; }
	float GetCurrentMobLootFetchSpeedForAutomation() const { return CurrentFollowSpeed; }
	FVector GetReservedMobLootTargetLocationForAutomation() const { return ReservedMobLootTargetLocation; }
	int32 GetTotalMobLootMoveAttemptsForAutomation() const { return TotalMobLootMoveAttempts; }
	float GetLastMobLootMoveDistanceForAutomation() const { return LastMobLootMoveDistance; }
	float ComputeFetchSpeedForBondStagesForAutomation(int32 BondStages) const;
	bool PumpMobLootCollectionForAutomation(float DeltaSeconds);

protected:
	virtual void BeginPlay() override;
	virtual void Tick(float DeltaSeconds) override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

	UPROPERTY(VisibleAnywhere, Category = "Visuals")
	TObjectPtr<USceneComponent> Root;

	UPROPERTY(Transient)
	FName ActiveSkinID = FName(TEXT("Default"));

	UPROPERTY(Transient)
	float CurrentFollowSpeed = 8.f;

	UPROPERTY(Transient)
	TWeakObjectPtr<APawn> CachedHeroPawn;

	bool bHasCachedGroundZ = false;
	float CachedGroundZ = 0.f;
	int32 GroundTraceTickCounter = 0;
	static constexpr int32 GroundTraceEveryNTicks = 3;

	FT66MobLootHandle ReservedMobLootHandle;
	FVector ReservedMobLootTargetLocation = FVector::ZeroVector;
	float ReservedMobLootAgeSeconds = 0.f;
	float ReservedMobLootNoProgressSeconds = 0.f;
	float ReservedMobLootLastDistanceSq = TNumericLimits<float>::Max();
	bool bHasReservedMobLootTarget = false;

	FT66MobLootCollectResult LastMobLootCollectResult;
	int32 TotalMobLootDropsCollected = 0;
	int32 TotalMobLootQuantityCollected = 0;
	int32 TotalMobLootSellValueCollected = 0;
	int32 TotalMobLootReservationsReleased = 0;
	int32 TotalMobLootMoveAttempts = 0;
	float LastMobLootMoveDistance = 0.f;

	void ApplyPetVisuals();
	void UpdateMovementTuning();
	bool TryFollowFoundationMobLootTarget(float DeltaSeconds);
	bool TryReserveMobLootTarget(UT66MobLootSubsystem& MobLoot);
	bool MoveTowardReservedMobLootTarget(float DeltaSeconds);
	bool TryCollectReservedMobLoot(UT66MobLootSubsystem& MobLoot);
	void ReleaseReservedMobLootReservation(UT66MobLootSubsystem* MobLoot);
	void ClearReservedMobLootTarget();
	void HandleMobLootCollectedForEconomyStack(const FT66MobLootCollectResult& CollectResult);
	FT66MobLootCollectorRef BuildMobLootCollectorRef() const;
	APawn* ResolveFollowHero() const;
};
