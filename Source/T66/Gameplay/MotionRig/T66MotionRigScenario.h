// Copyright Tribulation 66. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Gameplay/MotionRig/T66MotionRigTypes.h"
#include "T66MotionRigScenario.generated.h"

class ACameraActor;
class AT66MotionRigPawn;

// Deterministic test driver + telemetry recorder. Scenarios feed synthetic
// input through the same interface the player controller uses, so a capture
// is exactly what a player would have produced with that stick work. Every
// run logs per-tick ground truth to CSV under Saved/MotionRig/ for the
// rubric analysis (Scripts/MotionRig/AnalyzeTelemetry.py).
//
// Console surface (also driven by Scripts/MotionRig/CaptureMotionRig.ps1):
//   t66.MotionRig.RunScenario <walkcircle|jumptriple|dive|impact|full>
//   t66.MotionRig.Camera <side|front|threequarter|chase>
//   t66.MotionRig.Impact <speed>
UCLASS()
class T66_API UT66MotionRigScenario : public UActorComponent
{
	GENERATED_BODY()

public:
	UT66MotionRigScenario();

	// Self-start from the command line so the capture harness needs no new
	// capture modes: -T66MotionRigScenario=full -T66MotionRigCamera=side
	// -T66MotionRigScenarioDelay=4.0
	virtual void BeginPlay() override;

	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	void StartScenario(const FString& ScenarioName);
	void SetFixedCamera(const FString& CameraName);
	bool IsScenarioRunning() const { return bRunning; }

private:
	struct FStep
	{
		float Time = 0.f;
		TFunction<void(AT66MotionRigPawn&)> Action;
		FString Label;
	};

	void BuildScenario(const FString& ScenarioName);
	void FinishScenario();
	void RecordTelemetryRow(AT66MotionRigPawn& Pawn, float DeltaTime);
	AT66MotionRigPawn* GetRigPawn() const;

	TArray<FStep> Steps;
	int32 NextStepIndex = 0;
	float ScenarioClock = 0.f;
	float EndTime = 0.f;
	bool bRunning = false;
	FString ActiveScenarioName;

	TArray<FString> TelemetryRows;

	UPROPERTY()
	TObjectPtr<ACameraActor> FixedCamera;
};
