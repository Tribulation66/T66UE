// Copyright Tribulation 66. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "T66Hero1AxeAOEVFXLabActor.generated.h"

class UNiagaraComponent;

/**
 * Capture-only Hero 1 axe AOE VFX lab actor.
 *
 * This actor is not damage authority and is not referenced by live combat data.
 * It exists so automation can preview the first half-moon axe slash prototype
 * through an explicit -T66GameplayAutoCapture=hero1axeaoe mode.
 */
UCLASS()
class T66_API AT66Hero1AxeAOEVFXLabActor : public AActor
{
	GENERATED_BODY()

public:
	AT66Hero1AxeAOEVFXLabActor();

	virtual void OnConstruction(const FTransform& Transform) override;
	virtual void BeginPlay() override;
	virtual void Tick(float DeltaSeconds) override;

private:
	UPROPERTY(VisibleAnywhere, Category = "T66|VFX Lab")
	TObjectPtr<USceneComponent> SceneRoot = nullptr;

	UPROPERTY(VisibleAnywhere, Category = "T66|VFX Lab")
	TObjectPtr<UNiagaraComponent> SlashNiagaraComponent = nullptr;

	float SpawnTimeSeconds = 0.0f;
	int32 LastSlashCycleIndex = INDEX_NONE;
	float LastDiagnosticLogTimeSeconds = -1000.0f;
	mutable bool bLoggedDiagnosticMatrix = false;
	bool bOutgoingTravelerArrayProofEnabled = false;
	bool bOutgoingTravelerArrayProofManifestWritten = false;
	FString OutgoingTravelerArrayProofParameter = TEXT("User.TravelerPositions");
	FString OutgoingTravelerArrayProofManifestPath;
	int32 OutgoingTravelerArrayProofRequestedCount = 0;
	int32 OutgoingTravelerArrayProofUploadedCount = 0;
	int32 OutgoingTravelerArrayProofReadbackCount = 0;
	double OutgoingTravelerArrayProofLastUploadSeconds = 0.0;
	FString OutgoingTravelerArrayProofFirstPosition;
	FString OutgoingTravelerArrayProofLastPosition;
	int32 OutgoingTravelerArrayProofSampleCount = 0;
	double OutgoingTravelerArrayProofFpsSum = 0.0;
	double OutgoingTravelerArrayProofFpsMin = 0.0;
	double OutgoingTravelerArrayProofGpuFrameMsSum = 0.0;
	double OutgoingTravelerArrayProofLastGpuFrameMs = -1.0;
	int32 OutgoingTravelerArrayProofGpuFrameSamples = 0;
	int32 OutgoingTravelerArrayProofLastDrawCalls = -1;
	int32 OutgoingTravelerArrayProofMaxDrawCalls = -1;
	float OutgoingTravelerArrayProofStartTimeSeconds = 0.0f;

	void ApplyNiagaraSystems();
	void ApplyOutgoingTravelerArrayProof();
	void RestartSlashIfNeeded(float TimeSeconds);
	void SampleOutgoingTravelerArrayProof(float DeltaSeconds, float TimeSeconds);
	void WriteOutgoingTravelerArrayProofManifest(const TCHAR* Reason) const;
	void LogSlashStaticDiagnostics(const TCHAR* Phase) const;
	void LogSlashRuntimeDiagnostics(const TCHAR* Phase, float TimeSeconds) const;
};
