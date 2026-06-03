// Copyright Tribulation 66. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Gameplay/T66OutgoingTravelerPoolSubsystem.h"
#include "T66OutgoingTravelerStressHarnessActor.generated.h"

class APlayerController;
class ACameraActor;
class AT66EnemyBase;
class AT66HeroProjectile;

UCLASS(NotPlaceable)
class T66_API AT66OutgoingTravelerStressHarnessActor : public AActor
{
	GENERATED_BODY()

public:
	AT66OutgoingTravelerStressHarnessActor();

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

	TArray<TWeakObjectPtr<AT66HeroProjectile>> Projectiles;
	TArray<FT66OutgoingTravelerHandle> TravelerHandles;
	TArray<TWeakObjectPtr<AT66EnemyBase>> TargetEnemies;
	TWeakObjectPtr<ACameraActor> ProofCameraActor;

	FString ManifestPath;
	int32 RequestedCount = 500;
	int32 TargetCount = 128;
	int32 SpawnedCount = 0;
	int32 FailedSpawnCount = 0;
	int32 MainMeshVisibleCount = 0;
	int32 MainMeshSuppressedCountAtSpawn = 0;
	int32 ActorlessTravelerCount = 0;
	int32 GridColumns = 0;
	float GridSpacing = 10.0f;
	float SpawnDistance = 2400.0f;
	float TravelerSpeed = 1.0f;
	float VisualScaleMultiplier = 1.0f;
	float ProofCameraDistance = 1800.0f;
	float ProofCameraFOV = 45.0f;
	FVector ProofLaneCenter = FVector(20000.0f, 0.0f, 9000.0f);
	float ProofLaneYaw = 0.0f;
	float ProofLanePitch = -90.0f;
	float WarmupSeconds = 0.75f;
	float SampleSeconds = 2.5f;
	bool bUsePool = true;
	bool bDisableCollisionForRenderIsolation = true;
	bool bUseTargetSnapshot = false;
	bool bEnableArrivalCollision = false;
	bool bBindArrivalCallback = false;
	bool bUseMixedVisualProfiles = false;
	bool bUseProofCamera = false;
	bool bHideHeroForProof = false;
	bool bUseFixedProofLane = true;
	FString VisualProfileFamilyFilter;
	bool bStarted = false;
	bool bManifestWritten = false;
	double StartSeconds = 0.0;
	double SpawnTimeMs = 0.0;
	int32 ArrivalCallbackCount = 0;
	int32 ArrivalCallbackLiveTargetCount = 0;
	int32 ArrivalCallbackTargetLostCount = 0;
	TArray<FName> VisualProfilesUsed;

	FMetricAccumulator FrameMs;
	FMetricAccumulator Fps;
	FMetricAccumulator GameThreadMs;
	FMetricAccumulator GpuFrameMs;
	FMetricAccumulator DrawCalls;
	FMetricAccumulator PoolUploadMs;
	FMetricAccumulator PoolPackMs;
	FMetricAccumulator PoolNiagaraArrayUploadMs;
	FMetricAccumulator PoolSimulationMs;
	FMetricAccumulator PoolTargetSnapshotMs;
	FMetricAccumulator PoolArrivalCollisionMs;

	void SpawnStressTargets(const FVector& Center, const FVector& Right, const FVector& Up);
	void ConfigureProofCamera(const FVector& GridCenter, const FVector& ViewForward);
	void SpawnTravelers();
	void SampleFrame(float DeltaSeconds);
	void WriteManifest(const TCHAR* Reason);
	void DestroyTravelers();
	void DestroyStressTargets();
};
