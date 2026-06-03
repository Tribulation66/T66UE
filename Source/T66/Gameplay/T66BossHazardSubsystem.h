// Copyright Tribulation 66. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Data/T66DataTypes.h"
#include "Subsystems/WorldSubsystem.h"
#include "Tickable.h"
#include "T66BossHazardSubsystem.generated.h"

class AActor;
class AT66HeroBase;
class UT66RunStateSubsystem;
class UHierarchicalInstancedStaticMeshComponent;
class USceneComponent;

struct FT66BossHazardSpawnParams
{
	AActor* SourceActor = nullptr;
	FName SourceID = NAME_None;
	FName HazardID = NAME_None;
	FVector Location = FVector::ZeroVector;
	FRotator Rotation = FRotator::ZeroRotator;
	float RadiusScale = 1.f;
	float TelegraphScale = 1.f;
	float VisualScaleMultiplier = 1.f;
	int32 DamageOverrideHP = INDEX_NONE;
};

struct FT66BossHazardDiagnostics
{
	int32 ResetCount = 0;
	int32 HazardsSpawned = 0;
	int32 HazardsActivePeak = 0;
	int32 HazardsExpired = 0;
	int32 DroppedByCap = 0;
	int32 DroppedMissingDefinition = 0;
	int32 DamageTicks = 0;
	int32 DamageApplications = 0;
	double ManagerTickMaxUs = 0.0;
	double ManagerTickTotalUs = 0.0;
	int32 ManagerTickSamples = 0;
	double HISMUpdateMaxUs = 0.0;
	double HISMUpdateTotalUs = 0.0;
	int32 HISMUpdateSamples = 0;
	FString LastResetReason;

	void Reset(const FString& Reason);
	double GetManagerTickAvgUs() const;
	double GetHISMUpdateAvgUs() const;
};

UCLASS()
class T66_API UT66BossHazardSubsystem : public UWorldSubsystem, public FTickableGameObject
{
	GENERATED_BODY()

public:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;

	bool SpawnBossHazard(const FT66BossHazardSpawnParams& Params);
	int32 GetActiveHazardCount() const { return ActiveHazards.Num(); }
	const FT66BossHazardDiagnostics& GetDiagnostics() const { return Diagnostics; }
	void ResetBossHazardDiagnostics(const TCHAR* Reason);
	void EmitBossHazardSummary(const TCHAR* Reason, bool bTerminal);

#if !UE_BUILD_SHIPPING
	bool RunBossHazardDefinitionProof();
	bool StartBossHazardDamageProof();
#endif

	virtual void Tick(float DeltaTime) override;
	virtual TStatId GetStatId() const override;
	virtual bool IsTickable() const override;
	virtual bool IsTickableWhenPaused() const override;
	virtual bool IsTickableInEditor() const override;
	virtual UWorld* GetTickableGameObjectWorld() const override;

private:
	struct FT66BossHazardEntry
	{
		FT66BossHazardDefinitionData Definition;
		TWeakObjectPtr<AActor> SourceActor;
		FName SourceID = NAME_None;
		FVector Location = FVector::ZeroVector;
		FRotator Rotation = FRotator::ZeroRotator;
		float RadiusScale = 1.f;
		float TelegraphScale = 1.f;
		float VisualScaleMultiplier = 1.f;
		float AgeSeconds = 0.f;
		float NextDamageSeconds = 0.f;
		int32 DamageHP = 0;
	};

	struct FT66BossHazardRenderBucket
	{
		TObjectPtr<UHierarchicalInstancedStaticMeshComponent> Component = nullptr;
		FName VisualProfileID = NAME_None;
		FName ShapeType = NAME_None;
		bool bTelegraph = true;
		FLinearColor Color = FLinearColor::White;
	};

#if !UE_BUILD_SHIPPING
	enum class ET66BossHazardDamageProofPhase : uint8
	{
		None,
		InsideCadence,
		ExitStopsDamage,
		Complete
	};

	struct FT66BossHazardDamageProofPhaseResult
	{
		FString PhaseName;
		bool bStarted = false;
		bool bSpawned = false;
		bool bCompleted = false;
		bool bPass = false;
		bool bSafeZoneAtSpawn = false;
		bool bInsideAtSpawn = false;
		bool bInsideAfterExit = false;
		bool bCleanHeroState = false;
		bool bSaintBlessingActive = false;
		bool bBackroomsChallengeActive = false;
		bool bDamageOverrideApplied = false;
		float StartWorldTime = 0.f;
		float EndWorldTime = 0.f;
		float StartHP = 0.f;
		float EndHP = 0.f;
		float HPAtExit = 0.f;
		float ExitAgeSeconds = 0.f;
		float ExitDistance2D = 0.f;
		float EvasionChance = 0.f;
		float ArmorReduction = 0.f;
		int32 DamageTicks = 0;
		int32 DamageApplications = 0;
		int32 ApplicationsAtExit = 0;
		int32 DamageTicksAtExit = 0;
		int32 PostExitDamageTicks = 0;
		int32 PostExitDamageApplications = 0;
		int32 ExpectedDamageTicks = 0;
		int32 ExpectedApplications = 0;
		float ExpectedHPDrop = 0.f;
		TArray<float> ExpectedApplicationAges;
		TArray<float> ObservedApplicationAges;
		TArray<float> ObservedApplicationWorldTimes;
		TArray<float> ObservedHPAfterApplications;
		FString FailureReason;
	};

	struct FT66BossHazardDamageProofRuntime
	{
		bool bRunning = false;
		bool bExitRequested = false;
		ET66BossHazardDamageProofPhase Phase = ET66BossHazardDamageProofPhase::None;
		FString ManifestPath;
		FName HazardID = NAME_None;
		FT66BossHazardDefinitionData Definition;
		FVector InsideLocation = FVector::ZeroVector;
		FVector OutsideLocation = FVector::ZeroVector;
		TWeakObjectPtr<AT66HeroBase> Hero;
		TWeakObjectPtr<UT66RunStateSubsystem> RunState;
		FT66BossHazardDamageProofPhaseResult InsideCadence;
		FT66BossHazardDamageProofPhaseResult ExitStopsDamage;
		FString CompletionReason;
	};
#endif

	UPROPERTY(Transient)
	TObjectPtr<AActor> RenderHost = nullptr;

	UPROPERTY(Transient)
	TObjectPtr<USceneComponent> RenderRoot = nullptr;

	UPROPERTY(Transient)
	TArray<TObjectPtr<UHierarchicalInstancedStaticMeshComponent>> RenderComponents;

	TArray<FT66BossHazardEntry> ActiveHazards;
	TArray<FT66BossHazardRenderBucket> RenderBuckets;
	TMap<FString, int32> RenderBucketByKey;
	FT66BossHazardDiagnostics Diagnostics;
	bool bInitialized = false;
	bool bShuttingDown = false;

	bool ResolveHazardDefinition(FName HazardID, FT66BossHazardDefinitionData& OutDefinition) const;
	int32 CountActiveHazardsForID(FName HazardID) const;
	bool EnsureRenderResources();
	int32 FindOrCreateRenderBucket(const FT66BossHazardDefinitionData& Definition, bool bTelegraph);
	void FlushRenderInstances();
	void ApplyHazardDamage(const FT66BossHazardEntry& Hazard);
	bool IsHeroInsideHazard(const class AT66HeroBase* Hero, const FT66BossHazardEntry& Hazard) const;
	FVector GetHazardVisualScale(const FT66BossHazardEntry& Hazard, bool bTelegraph, float PhaseAlpha) const;

#if !UE_BUILD_SHIPPING
	FT66BossHazardDamageProofRuntime DamageProof;
	void BeginBossHazardDamageProofPhase(ET66BossHazardDamageProofPhase Phase);
	void TickBossHazardDamageProof();
	void CompleteBossHazardDamageProof(bool bPass, const FString& Reason);
	void WriteBossHazardDamageProofManifest(bool bPass, const FString& Reason) const;
	void AppendBossHazardDamageProofPhaseJson(FString& OutJson, const FT66BossHazardDamageProofPhaseResult& Phase) const;
	int32 BuildBossHazardDamageProofExpectations(TArray<float>& OutExpectedApplicationAges, int32& OutExpectedDamageTicks) const;
#endif
};
