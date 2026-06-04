// Copyright Tribulation 66. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Gameplay/T66WorldSystemsAPI.h"
#include "Subsystems/WorldSubsystem.h"
#include "UObject/ObjectKey.h"
#include "T66MobLootSubsystem.generated.h"

class UNiagaraComponent;
class UNiagaraSystem;
class USceneComponent;
class UHierarchicalInstancedStaticMeshComponent;
enum class ET66Difficulty : uint8;

USTRUCT(BlueprintType)
struct FT66MobLootDiagnostics
{
	GENERATED_BODY()

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "T66|Mob Loot")
	int32 LiveWorldDropCount = 0;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "T66|Mob Loot")
	int32 PeakLiveWorldDropCount = 0;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "T66|Mob Loot")
	int32 SpawnedDropTotal = 0;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "T66|Mob Loot")
	int32 DroppedWhenFullTotal = 0;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "T66|Mob Loot")
	int32 ExpiredDropTotal = 0;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "T66|Mob Loot")
	int32 CollectedDropTotal = 0;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "T66|Mob Loot")
	int32 QuantityCollectedTotal = 0;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "T66|Mob Loot")
	int32 GoldValueCollectedTotal = 0;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "T66|Mob Loot")
	int32 ReservationTotal = 0;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "T66|Mob Loot")
	int32 ReservationReleaseTotal = 0;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "T66|Mob Loot")
	int32 ReservationDeniedTotal = 0;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "T66|Mob Loot")
	int32 UploadCount = 0;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "T66|Mob Loot")
	int32 LastUploadedLiveCount = 0;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "T66|Mob Loot")
	int32 TickCount = 0;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "T66|Mob Loot")
	double LastTickMs = 0.0;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "T66|Mob Loot")
	double LastUploadMs = 0.0;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "T66|Mob Loot")
	double LastPackMs = 0.0;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "T66|Mob Loot")
	double LastNiagaraArrayUploadMs = 0.0;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "T66|Mob Loot")
	double LastCollectMs = 0.0;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "T66|Mob Loot")
	double LastQueryMs = 0.0;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "T66|Mob Loot")
	double AverageTickMs = 0.0;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "T66|Mob Loot")
	double AverageUploadMs = 0.0;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "T66|Mob Loot")
	double AveragePackMs = 0.0;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "T66|Mob Loot")
	double AverageNiagaraArrayUploadMs = 0.0;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "T66|Mob Loot")
	double MaxTickMs = 0.0;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "T66|Mob Loot")
	double MaxUploadMs = 0.0;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "T66|Mob Loot")
	double MaxPackMs = 0.0;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "T66|Mob Loot")
	double MaxNiagaraArrayUploadMs = 0.0;
};

UCLASS()
class T66_API UT66MobLootSubsystem : public UTickableWorldSubsystem, public IT66MobLootRuntime
{
	GENERATED_BODY()

public:
	static constexpr int32 MaxMobLootDrops = 20000;
	static constexpr int32 MobLootGoldPerUnit = 1;

	static int32 GetDifficultyDropIndex(ET66Difficulty Difficulty);
	static int32 GetDeathMobLootQuantityForDifficulty(ET66Difficulty Difficulty, bool bIsMiniBoss);

	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;
	virtual void Tick(float DeltaTime) override;
	virtual TStatId GetStatId() const override;
	virtual bool IsTickable() const override;

	virtual bool SpawnMobLoot(const FT66MobLootSpawnParams& Params, FT66MobLootHandle& OutHandle) override;
	virtual bool QueryMobLootTarget(const FT66MobLootQueryFilter& Filter, FT66MobLootHandle& OutHandle, FVector& OutPosition) const override;
	virtual FT66MobLootReservationResult QueryAndReserveMobLoot(const FT66MobLootQueryFilter& Filter, const FT66MobLootCollectorRef& Collector) override;
	virtual bool ReserveMobLoot(const FT66MobLootHandle& Handle, const FT66MobLootCollectorRef& Collector, FT66MobLootReservationResult& OutReservation) override;
	virtual bool ReleaseMobLootReservation(const FT66MobLootHandle& Handle, const FT66MobLootCollectorRef& Collector) override;
	virtual FT66MobLootCollectResult CollectReservedMobLoot(const FT66MobLootHandle& Handle, const FT66MobLootCollectorRef& Collector, float MaxDistance = 0.0f) override;
	virtual FT66MobLootCollectResult CollectMobLootAt(const FVector& Center, float Radius, const FT66MobLootCollectorRef& Collector, int32 MaxDrops = INDEX_NONE) override;
	virtual int32 GetActiveMobLootDropCount() const override;
	virtual void PushMobLootRenderArrays(FT66MobLootRenderArrays& OutArrays) const override;

	bool SpawnMobLootFromNonBossDeath(AActor* SourceActor, FName SourceID, bool bIsMiniBoss, FT66MobLootHandle& OutHandle);

	const FT66MobLootDiagnostics& GetDiagnostics() const { return Diagnostics; }
	static bool IsEnabled();

#if !UE_BUILD_SHIPPING
	void ClearAllMobLootForAutomation();
	int32 GetVisibleMobLootInstanceCountForAutomation() const;
#endif

private:
	struct FT66MobLootSlot
	{
		FVector Position = FVector::ZeroVector;
		FName SourceID = NAME_None;
		FLinearColor Color = FLinearColor::White;
		float Scale = 1.0f;
		float RemainingLifetimeSeconds = 0.0f;
		int32 Quantity = 0;
		int32 GoldValue = 0;
		int32 Generation = 0;
		int32 DenseIndex = INDEX_NONE;
		bool bActive = false;
		bool bReserved = false;
		FObjectKey ReservedCollectorKey;
		FName ReservedCollectorID = NAME_None;
		ET66MobLootCollectorType ReservedCollectorType = ET66MobLootCollectorType::System;
	};

	enum class EReleaseReason : uint8
	{
		Collected,
		Expired,
		Cleared
	};

	UPROPERTY(Transient)
	TObjectPtr<AActor> RenderHost = nullptr;

	UPROPERTY(Transient)
	TObjectPtr<USceneComponent> RenderRoot = nullptr;

	UPROPERTY(Transient)
	TObjectPtr<UNiagaraComponent> NiagaraComponent = nullptr;

	UPROPERTY(Transient)
	TObjectPtr<UNiagaraSystem> NiagaraSystem = nullptr;

	UPROPERTY(Transient)
	TObjectPtr<UHierarchicalInstancedStaticMeshComponent> FallbackVisualComponent = nullptr;

	TArray<FT66MobLootSlot> Slots;
	TArray<int32> FreeSlots;
	TArray<int32> DenseSlots;

	TArray<FVector> PositionUpload;
	TArray<float> ScaleUpload;
	TArray<FLinearColor> ColorUpload;
	TArray<int32> QuantityUpload;

	FT66MobLootDiagnostics Diagnostics;
	bool bInitializedSlots = false;
	bool bDirty = true;

	void InitializeSlots();
	bool EnsureNiagaraComponent();
	bool IsHandleCurrent(const FT66MobLootHandle& Handle) const;
	bool IsCollectorMatch(const FT66MobLootSlot& Slot, const FT66MobLootCollectorRef& Collector) const;
	bool IsEligibleForQuery(const FT66MobLootSlot& Slot, const FT66MobLootQueryFilter& Filter, const FT66MobLootCollectorRef* Collector = nullptr) const;
	FT66MobLootReservationResult BuildReservationResult(const FT66MobLootSlot& Slot, const FT66MobLootHandle& Handle, bool bReserved) const;
	void ReleaseSlot(int32 SlotIndex, EReleaseReason Reason, FT66MobLootCollectResult* InOutCollectResult = nullptr);
	void TickExpirations(float DeltaTime);
	void UploadLiveState();
	bool EnsureFallbackVisualComponent();
	void UploadFallbackVisualState();
};
