// Copyright Tribulation 66. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Core/T66WorldRuntimeProofTypes.h"
#include "Gameplay/T66CombatTargetTypes.h"
#include "Subsystems/WorldSubsystem.h"
#include "UObject/ObjectKey.h"
#include "T66OutgoingTravelerPoolSubsystem.generated.h"

class UNiagaraComponent;
class UNiagaraSystem;
class USceneComponent;
enum class ET66TemporaryProjectileShape : uint8;

USTRUCT(BlueprintType)
struct FT66OutgoingTravelerHandle
{
	GENERATED_BODY()

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "T66|Outgoing Travelers")
	int32 SlotIndex = INDEX_NONE;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "T66|Outgoing Travelers")
	int32 Generation = 0;

	bool IsValid() const { return SlotIndex != INDEX_NONE && Generation != 0; }
	void Reset()
	{
		SlotIndex = INDEX_NONE;
		Generation = 0;
	}
};

USTRUCT(BlueprintType)
struct FT66OutgoingTravelerVisualState
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "T66|Outgoing Travelers")
	FVector Position = FVector::ZeroVector;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "T66|Outgoing Travelers")
	FQuat Rotation = FQuat::Identity;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "T66|Outgoing Travelers")
	FVector Scale = FVector::OneVector;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "T66|Outgoing Travelers")
	FLinearColor Color = FLinearColor::White;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "T66|Outgoing Travelers")
	int32 MeshIndex = 0;
};

USTRUCT(BlueprintType)
struct FT66OutgoingTravelerFireParams
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "T66|Outgoing Travelers")
	FVector StartPosition = FVector::ZeroVector;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "T66|Outgoing Travelers")
	FVector TargetPosition = FVector::ZeroVector;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "T66|Outgoing Travelers")
	FVector TargetOffset = FVector::ZeroVector;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "T66|Outgoing Travelers")
	FT66CombatTargetHandle TargetHandle;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "T66|Outgoing Travelers")
	FName ProfileID = NAME_None;

	// Optional data-facing selector for the single Niagara pool's per-instance visual slots.
	// NAME_None keeps the temporary projectile ProfileID shape fallback.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "T66|Outgoing Travelers")
	FName TravelerVisualProfileID = NAME_None;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "T66|Outgoing Travelers")
	FLinearColor Color = FLinearColor::White;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "T66|Outgoing Travelers")
	float ScaleMultiplier = 1.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "T66|Outgoing Travelers")
	float Speed = 2400.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "T66|Outgoing Travelers")
	float LifetimeSeconds = 10.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "T66|Outgoing Travelers")
	float ArrivalRadius = 30.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "T66|Outgoing Travelers")
	int32 DamageAmount = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "T66|Outgoing Travelers")
	FName DamageSourceID = NAME_None;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "T66|Outgoing Travelers")
	FName EventType = NAME_None;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "T66|Outgoing Travelers")
	bool bTrackTarget = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "T66|Outgoing Travelers")
	bool bEnableArrivalCollision = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "T66|Outgoing Travelers")
	bool bApplyDamageOnArrival = false;
};

USTRUCT(BlueprintType)
struct FT66OutgoingTravelerArrivalEvent
{
	GENERATED_BODY()

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "T66|Outgoing Travelers")
	FT66OutgoingTravelerHandle Handle;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "T66|Outgoing Travelers")
	FT66CombatTargetHandle ResolvedTargetHandle;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "T66|Outgoing Travelers")
	FVector ArrivalPosition = FVector::ZeroVector;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "T66|Outgoing Travelers")
	FVector LastKnownTargetPosition = FVector::ZeroVector;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "T66|Outgoing Travelers")
	bool bHitLiveTarget = false;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "T66|Outgoing Travelers")
	bool bTargetLostOrDead = false;
};

DECLARE_DELEGATE_OneParam(FT66OutgoingTravelerArrivalCallback, const FT66OutgoingTravelerArrivalEvent&);

USTRUCT(BlueprintType)
struct FT66OutgoingTravelerPoolDiagnostics
{
	GENERATED_BODY()

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "T66|Outgoing Travelers")
	int32 LiveCount = 0;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "T66|Outgoing Travelers")
	int32 PeakLiveCount = 0;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "T66|Outgoing Travelers")
	int32 FiredTotal = 0;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "T66|Outgoing Travelers")
	int32 CanceledTotal = 0;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "T66|Outgoing Travelers")
	int32 DroppedTotal = 0;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "T66|Outgoing Travelers")
	int32 SimulatedFiredTotal = 0;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "T66|Outgoing Travelers")
	int32 SimulatedArrivedTotal = 0;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "T66|Outgoing Travelers")
	int32 SimulatedExpiredTotal = 0;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "T66|Outgoing Travelers")
	int32 UploadCount = 0;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "T66|Outgoing Travelers")
	int32 SimulationTickCount = 0;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "T66|Outgoing Travelers")
	int32 TargetSnapshotBuildCount = 0;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "T66|Outgoing Travelers")
	int32 LastTargetSnapshotCount = 0;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "T66|Outgoing Travelers")
	int32 ArrivalCheckTotal = 0;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "T66|Outgoing Travelers")
	int32 ArrivalDamageAppliedTotal = 0;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "T66|Outgoing Travelers")
	int32 ArrivalCallbackTotal = 0;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "T66|Outgoing Travelers")
	int32 ArrivalFizzleNoTargetTotal = 0;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "T66|Outgoing Travelers")
	int32 LastUploadedLiveCount = 0;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "T66|Outgoing Travelers")
	int32 LastUploadedVisualRowCount = 0;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "T66|Outgoing Travelers")
	int32 PeakUploadedVisualRowCount = 0;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "T66|Outgoing Travelers")
	int32 LastCarrierRecipeExpandedTravelerCount = 0;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "T66|Outgoing Travelers")
	double LastUploadMs = 0.0;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "T66|Outgoing Travelers")
	double LastPackMs = 0.0;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "T66|Outgoing Travelers")
	double LastNiagaraArrayUploadMs = 0.0;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "T66|Outgoing Travelers")
	double LastSimulationMs = 0.0;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "T66|Outgoing Travelers")
	double LastTargetSnapshotMs = 0.0;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "T66|Outgoing Travelers")
	double LastArrivalCollisionMs = 0.0;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "T66|Outgoing Travelers")
	double AverageUploadMs = 0.0;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "T66|Outgoing Travelers")
	double AveragePackMs = 0.0;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "T66|Outgoing Travelers")
	double AverageNiagaraArrayUploadMs = 0.0;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "T66|Outgoing Travelers")
	double AverageSimulationMs = 0.0;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "T66|Outgoing Travelers")
	double AverageTargetSnapshotMs = 0.0;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "T66|Outgoing Travelers")
	double AverageArrivalCollisionMs = 0.0;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "T66|Outgoing Travelers")
	double MaxUploadMs = 0.0;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "T66|Outgoing Travelers")
	double MaxPackMs = 0.0;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "T66|Outgoing Travelers")
	double MaxNiagaraArrayUploadMs = 0.0;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "T66|Outgoing Travelers")
	double MaxSimulationMs = 0.0;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "T66|Outgoing Travelers")
	double MaxTargetSnapshotMs = 0.0;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "T66|Outgoing Travelers")
	double MaxArrivalCollisionMs = 0.0;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "T66|Outgoing Travelers")
	int32 MainMeshSuppressedTotal = 0;
};

UCLASS()
class T66_API UT66OutgoingTravelerPoolSubsystem : public UTickableWorldSubsystem
{
	GENERATED_BODY()

public:
	static constexpr int32 MaxOutgoingTravelers = 20000;
	static constexpr int32 TemporaryProjectileMeshSlotCount = 4;
	static constexpr int32 TravelerVisualProfileSlotBase = 0;
	static constexpr int32 TravelerVisualProfileSlotCount = 20;
	static constexpr int32 MaxCarrierRecipeParts = 4;
	static constexpr int32 MaxOutgoingTravelerVisualRows = MaxOutgoingTravelers * MaxCarrierRecipeParts;

	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;
	virtual void Tick(float DeltaTime) override;
	virtual TStatId GetStatId() const override;
	virtual bool IsTickable() const override;

	bool FireOutgoingTraveler(const FT66OutgoingTravelerFireParams& FireParams, FT66OutgoingTravelerHandle& OutHandle);
	bool FireOutgoingTraveler(
		const FT66OutgoingTravelerFireParams& FireParams,
		FT66OutgoingTravelerHandle& OutHandle,
		FT66OutgoingTravelerArrivalCallback OnArrived);
	void CancelOutgoingTraveler(FT66OutgoingTravelerHandle& Handle);

	void NoteMainMeshSuppressed();

	int32 GetLiveTravelerCount() const { return Diagnostics.LiveCount; }
	int32 GetPeakLiveTravelerCount() const { return Diagnostics.PeakLiveCount; }
	const FT66OutgoingTravelerPoolDiagnostics& GetDiagnostics() const { return Diagnostics; }

	static int32 GetMeshIndexForTemporaryProjectileShape(ET66TemporaryProjectileShape Shape);
	static int32 GetMeshIndexForTravelerVisualProfileID(FName TravelerVisualProfileID, int32 FallbackMeshIndex);
	static int32 GetCarrierRecipePartCountForTravelerVisualProfileID(FName TravelerVisualProfileID);
	static FString GetCarrierRecipeSignatureForTravelerVisualProfileID(FName TravelerVisualProfileID);
	static void AppendKnownTravelerVisualProfileIDs(TArray<FName>& OutProfileIDs);
	static bool IsEnabled();

#if !UE_BUILD_SHIPPING
	FT66WorldRuntimeDebugSnapshot GetWorldRuntimeDebugSnapshot() const
	{
		FT66WorldRuntimeDebugSnapshot Snapshot;
		Snapshot.SystemName = TEXT("UT66OutgoingTravelerPoolSubsystem");
		Snapshot.AddCounter(TEXT("live_traveler_count"), Diagnostics.LiveCount);
		Snapshot.AddCounter(TEXT("peak_live_traveler_count"), Diagnostics.PeakLiveCount);
		Snapshot.AddCounter(TEXT("slot_capacity"), Slots.Num());
		Snapshot.AddCounter(TEXT("free_slot_count"), FreeSlots.Num());
		Snapshot.AddCounter(TEXT("dense_slot_count"), DenseSlots.Num());
		Snapshot.AddCounter(TEXT("target_snapshot_count"), TargetSnapshot.Num());
		Snapshot.AddCounter(TEXT("target_snapshot_index_count"), TargetSnapshotIndexByActor.Num());
		Snapshot.AddCounter(TEXT("position_upload_count"), PositionUpload.Num());
		Snapshot.AddCounter(TEXT("rotation_upload_count"), RotationUpload.Num());
		Snapshot.AddCounter(TEXT("scale_upload_count"), ScaleUpload.Num());
		Snapshot.AddCounter(TEXT("color_upload_count"), ColorUpload.Num());
		Snapshot.AddCounter(TEXT("mesh_index_upload_count"), MeshIndexUpload.Num());
		Snapshot.AddCounter(TEXT("known_timer_handles"), 0);
		Snapshot.AddCounter(TEXT("known_external_delegate_handles"), 0);
		Snapshot.AddCounter(TEXT("async_load_handles_valid"), 0);
		Snapshot.AddFlag(TEXT("initialized_slots"), bInitializedSlots);
		Snapshot.AddFlag(TEXT("dirty"), bDirty);
		Snapshot.AddFlag(TEXT("manifest_written"), bManifestWritten);
		Snapshot.AddFlag(TEXT("render_host_valid"), RenderHost != nullptr);
		Snapshot.AddFlag(TEXT("render_root_valid"), RenderRoot != nullptr);
		Snapshot.AddFlag(TEXT("niagara_component_valid"), NiagaraComponent != nullptr);
		Snapshot.AddFlag(TEXT("niagara_system_valid"), NiagaraSystem != nullptr);
		Snapshot.AddEvidence(TEXT("timers"), TEXT("No stored timer handles found; traveler simulation is tick-driven."));
		Snapshot.AddEvidence(TEXT("delegates"), TEXT("Arrival callbacks are per-slot delegates and clear on slot release; no external registration handle is stored."));
		Snapshot.AddEvidence(TEXT("async_loads"), TEXT("No async load handle is owned by this subsystem."));
		Snapshot.AddMeasurementGap(TEXT("niagara_component_registered_active_state"));
		return Snapshot;
	}
#endif

private:
	struct FT66OutgoingTravelerSlot
	{
		FT66OutgoingTravelerVisualState State;
		FT66CombatTargetHandle TargetHandle;
		FVector TargetPosition = FVector::ZeroVector;
		FVector TargetOffset = FVector::ZeroVector;
		FObjectKey TargetActorKey;
		FVector LastKnownTargetPosition = FVector::ZeroVector;
		FQuat ProfileRotation = FQuat::Identity;
		float Speed = 0.0f;
		float RemainingLifetimeSeconds = 0.0f;
		float ArrivalRadius = 30.0f;
		int32 DamageAmount = 0;
		int32 Generation = 0;
		int32 DenseIndex = INDEX_NONE;
		int32 SourceTowerFloorNumber = INDEX_NONE;
		FName DamageSourceID = NAME_None;
		FName EventType = NAME_None;
		FName TravelerVisualProfileID = NAME_None;
		FT66OutgoingTravelerArrivalCallback OnArrived;
		bool bActive = false;
		bool bSimulated = false;
		bool bTrackTarget = false;
		bool bHasTargetActorKey = false;
		bool bEnableArrivalCollision = false;
		bool bApplyDamageOnArrival = false;
	};

	struct FT66OutgoingTravelerTargetSnapshotEntry
	{
		TWeakObjectPtr<AActor> Actor;
		FObjectKey ActorKey;
		FVector Location = FVector::ZeroVector;
		FVector AimPoint = FVector::ZeroVector;
		ET66HitZoneType HitZoneType = ET66HitZoneType::Body;
		FName HitZoneName = NAME_None;
		bool bAlive = false;
	};

	enum class EReleaseReason : uint8
	{
		Canceled,
		Arrived,
		Expired
	};

	UPROPERTY(Transient)
	TObjectPtr<AActor> RenderHost = nullptr;

	UPROPERTY(Transient)
	TObjectPtr<USceneComponent> RenderRoot = nullptr;

	UPROPERTY(Transient)
	TObjectPtr<UNiagaraComponent> NiagaraComponent = nullptr;

	UPROPERTY(Transient)
	TObjectPtr<UNiagaraSystem> NiagaraSystem = nullptr;

	TArray<FT66OutgoingTravelerSlot> Slots;
	TArray<int32> FreeSlots;
	TArray<int32> DenseSlots;
	TArray<FT66OutgoingTravelerTargetSnapshotEntry> TargetSnapshot;
	TMap<FObjectKey, int32> TargetSnapshotIndexByActor;

	TArray<FVector> PositionUpload;
	TArray<FQuat> RotationUpload;
	TArray<FVector> ScaleUpload;
	TArray<FLinearColor> ColorUpload;
	TArray<int32> MeshIndexUpload;

	FT66OutgoingTravelerPoolDiagnostics Diagnostics;

	bool bInitializedSlots = false;
	bool bDirty = true;
	bool bManifestWritten = false;
	double FpsSum = 0.0;
	double FpsMin = 0.0;
	int32 FpsSamples = 0;
	double GpuFrameMsSum = 0.0;
	double LastGpuFrameMs = -1.0;
	int32 GpuFrameSamples = 0;
	int32 LastDrawCalls = -1;
	int32 MaxDrawCalls = -1;
	float ManifestSampleStartSeconds = 0.0f;
	float FirstActivityTimeSeconds = 0.0f;
	FString ManifestPath;

	void InitializeSlots();
	bool IsHandleCurrent(const FT66OutgoingTravelerHandle& Handle) const;
	bool AllocateSlot(const FT66OutgoingTravelerVisualState& InitialState, FT66OutgoingTravelerHandle& OutHandle);
	void ReleaseSlot(int32 SlotIndex, EReleaseReason Reason);
	bool EnsureNiagaraComponent();
	void BuildTargetSnapshot();
	void AddTargetSnapshotEntry(AActor* Actor);
	const FT66OutgoingTravelerTargetSnapshotEntry* FindTargetSnapshotEntry(const FT66OutgoingTravelerSlot& Slot) const;
	FT66OutgoingTravelerArrivalEvent MakeArrivalEvent(
		const FT66OutgoingTravelerHandle& Handle,
		const FT66OutgoingTravelerSlot& Slot,
		const FT66OutgoingTravelerTargetSnapshotEntry* SnapshotEntry,
		const FVector& ArrivalPosition) const;
	void ResolveTravelerArrival(
		int32 SlotIndex,
		const FT66OutgoingTravelerTargetSnapshotEntry* SnapshotEntry,
		const FVector& ArrivalPosition,
		uint64& InOutArrivalCollisionCycles);
	bool ApplyArrivalDamage(const FT66OutgoingTravelerSlot& Slot, const FT66OutgoingTravelerTargetSnapshotEntry* SnapshotEntry);
	void TickSimulatedTravelers(float DeltaTime);
	void AppendVisualRowsForSlot(const FT66OutgoingTravelerSlot& Slot);
	void UploadLiveState();
	void SampleProofMetrics(float DeltaTime);
	void WriteProofManifest(const TCHAR* Reason);
};
