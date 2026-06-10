// Copyright Tribulation 66. All Rights Reserved.

#include "Gameplay/MotionRig/T66MotionRigScenario.h"

#include "Camera/CameraActor.h"
#include "Camera/CameraComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Engine/World.h"
#include "EngineUtils.h"
#include "GameFramework/PlayerController.h"
#include "Gameplay/MotionRig/T66MotionRigPawn.h"
#include "HAL/FileManager.h"
#include "Misc/CommandLine.h"
#include "Misc/FileHelper.h"
#include "Misc/Parse.h"
#include "Misc/Paths.h"
#include "TimerManager.h"

DEFINE_LOG_CATEGORY_STATIC(LogT66MotionRigScenario, Log, All);

namespace
{
	UT66MotionRigScenario* FindScenarioComponent(UWorld* World)
	{
		if (!World)
		{
			return nullptr;
		}
		for (TActorIterator<AT66MotionRigPawn> It(World); It; ++It)
		{
			if (UT66MotionRigScenario* Scenario = It->FindComponentByClass<UT66MotionRigScenario>())
			{
				return Scenario;
			}
		}
		return nullptr;
	}

	FAutoConsoleCommandWithWorldAndArgs GMotionRigRunScenarioCmd(
		TEXT("t66.MotionRig.RunScenario"),
		TEXT("Runs a deterministic MotionRig test scenario: walkcircle | jumptriple | dive | impact | full"),
		FConsoleCommandWithWorldAndArgsDelegate::CreateStatic(
			[](const TArray<FString>& Args, UWorld* World)
			{
				if (UT66MotionRigScenario* Scenario = FindScenarioComponent(World))
				{
					Scenario->StartScenario(Args.Num() > 0 ? Args[0] : TEXT("full"));
				}
				else
				{
					UE_LOG(LogT66MotionRigScenario, Error,
						TEXT("RunScenario: no MotionRig pawn in world (is t66.MotionRig.TestRoom enabled and Hero_1 selected?)"));
				}
			}));

	FAutoConsoleCommandWithWorldAndArgs GMotionRigCameraCmd(
		TEXT("t66.MotionRig.Camera"),
		TEXT("Sets a fixed review camera: side | front | threequarter | chase"),
		FConsoleCommandWithWorldAndArgsDelegate::CreateStatic(
			[](const TArray<FString>& Args, UWorld* World)
			{
				if (UT66MotionRigScenario* Scenario = FindScenarioComponent(World))
				{
					Scenario->SetFixedCamera(Args.Num() > 0 ? Args[0] : TEXT("side"));
				}
			}));

	FAutoConsoleCommandWithWorldAndArgs GMotionRigImpactCmd(
		TEXT("t66.MotionRig.Impact"),
		TEXT("Applies a standardized knockdown impulse to the MotionRig pawn. Arg: speed (cm/s, default 900)."),
		FConsoleCommandWithWorldAndArgsDelegate::CreateStatic(
			[](const TArray<FString>& Args, UWorld* World)
			{
				if (!World)
				{
					return;
				}
				for (TActorIterator<AT66MotionRigPawn> It(World); It; ++It)
				{
					const float Speed = Args.Num() > 0 ? FCString::Atof(*Args[0]) : 900.f;
					const FVector Launch = It->GetActorForwardVector() * -Speed + FVector(0.f, 0.f, Speed * 0.45f);
					It->TriggerKnockdown(Launch);
					return;
				}
			}));
}

UT66MotionRigScenario::UT66MotionRigScenario()
{
	PrimaryComponentTick.bCanEverTick = true;
	PrimaryComponentTick.TickGroup = TG_PostPhysics; // read settled physics state
}

void UT66MotionRigScenario::BeginPlay()
{
	Super::BeginPlay();

	FString ScenarioName;
	if (!FParse::Value(FCommandLine::Get(), TEXT("T66MotionRigScenario="), ScenarioName) || ScenarioName.IsEmpty())
	{
		return;
	}

	FString CameraName = TEXT("side");
	FParse::Value(FCommandLine::Get(), TEXT("T66MotionRigCamera="), CameraName);

	float DelaySeconds = 4.0f;
	FParse::Value(FCommandLine::Get(), TEXT("T66MotionRigScenarioDelay="), DelaySeconds);
	DelaySeconds = FMath::Clamp(DelaySeconds, 0.5f, 30.f);

	FTimerHandle StartTimer;
	GetWorld()->GetTimerManager().SetTimer(
		StartTimer,
		FTimerDelegate::CreateWeakLambda(this, [this, ScenarioName, CameraName]()
		{
			SetFixedCamera(CameraName);
			StartScenario(ScenarioName);
		}),
		DelaySeconds,
		false);
}

AT66MotionRigPawn* UT66MotionRigScenario::GetRigPawn() const
{
	return Cast<AT66MotionRigPawn>(GetOwner());
}

void UT66MotionRigScenario::BuildScenario(const FString& ScenarioName)
{
	Steps.Reset();
	auto Add = [this](const float Time, const FString& Label, TFunction<void(AT66MotionRigPawn&)> Action)
	{
		FStep Step;
		Step.Time = Time;
		Step.Label = Label;
		Step.Action = MoveTemp(Action);
		Steps.Add(MoveTemp(Step));
	};
	auto Axes = [](const float F, const float R)
	{
		return [F, R](AT66MotionRigPawn& Pawn) { Pawn.ScenarioSetMoveAxes(F, R); };
	};

	float T = 0.f;
	const FString Name = ScenarioName.ToLower();

	// Scenarios are absolute-time scripted so identical every run.
	if (Name == TEXT("walkcircle") || Name == TEXT("full"))
	{
		// Straight run, two turns, stop. Exercises cadence, lean, yaw spring.
		Add(T + 0.5f, TEXT("walk fwd"), Axes(1.f, 0.f));
		Add(T + 3.0f, TEXT("turn right"), Axes(1.f, 1.f));
		Add(T + 4.5f, TEXT("turn left"), Axes(1.f, -1.f));
		Add(T + 6.0f, TEXT("walk fwd"), Axes(1.f, 0.f));
		Add(T + 7.5f, TEXT("hard stop"), Axes(0.f, 0.f));
		T += 9.0f;
	}
	if (Name == TEXT("jumptriple") || Name == TEXT("full"))
	{
		Add(T + 0.5f, TEXT("jump 1 (standing)"), [](AT66MotionRigPawn& P) { P.MotionRigJumpPressed(); });
		Add(T + 2.0f, TEXT("walk"), Axes(1.f, 0.f));
		Add(T + 2.6f, TEXT("jump 2 (moving)"), [](AT66MotionRigPawn& P) { P.MotionRigJumpPressed(); });
		Add(T + 4.2f, TEXT("jump 3 (moving)"), [](AT66MotionRigPawn& P) { P.MotionRigJumpPressed(); });
		Add(T + 5.6f, TEXT("stop"), Axes(0.f, 0.f));
		T += 7.0f;
	}
	if (Name == TEXT("dive") || Name == TEXT("full"))
	{
		Add(T + 0.5f, TEXT("walk"), Axes(1.f, 0.f));
		Add(T + 1.4f, TEXT("dive"), [](AT66MotionRigPawn& P) { P.MotionRigDivePressed(); });
		Add(T + 1.45f, TEXT("release axes"), Axes(0.f, 0.f));
		T += 6.0f; // prone slide + recovery time
	}
	if (Name == TEXT("impact") || Name == TEXT("full"))
	{
		Add(T + 0.5f, TEXT("standard impact"), [](AT66MotionRigPawn& P)
		{
			const FVector Launch = P.GetActorForwardVector() * -900.f + FVector(0.f, 0.f, 420.f);
			P.TriggerKnockdown(Launch);
		});
		T += 7.0f; // tumble + settle + recovery
	}

	EndTime = T + 1.0f;
	ActiveScenarioName = Name;
}

void UT66MotionRigScenario::StartScenario(const FString& ScenarioName)
{
	AT66MotionRigPawn* Pawn = GetRigPawn();
	if (!Pawn)
	{
		return;
	}

	BuildScenario(ScenarioName);
	NextStepIndex = 0;
	ScenarioClock = 0.f;
	bRunning = true;
	Pawn->SetScenarioInputOverride(true);

	TelemetryRows.Reset();
	TelemetryRows.Add(TEXT("t,state,grounded,bean_x,bean_y,bean_z,bean_vx,bean_vy,bean_vz,bean_pitch,bean_roll,pelvis_x,pelvis_y,pelvis_z,pelvis_speed,foot_l_speed,foot_r_speed,hand_r_z,head_z"));

	UE_LOG(LogT66MotionRigScenario, Display,
		TEXT("MotionRig scenario '%s' started: %d steps over %.1fs"),
		*ActiveScenarioName, Steps.Num(), EndTime);
}

void UT66MotionRigScenario::TickComponent(
	const float DeltaTime, const ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	if (!bRunning)
	{
		return;
	}

	AT66MotionRigPawn* Pawn = GetRigPawn();
	if (!Pawn)
	{
		bRunning = false;
		return;
	}

	ScenarioClock += DeltaTime;

	// The T66 controller restores its own view target opportunistically —
	// re-assert the review camera while a scenario runs.
	if (FixedCamera)
	{
		if (APlayerController* PC = GetWorld() ? GetWorld()->GetFirstPlayerController() : nullptr)
		{
			if (PC->GetViewTarget() != FixedCamera)
			{
				PC->SetViewTargetWithBlend(FixedCamera, 0.f);
			}
		}
	}

	while (NextStepIndex < Steps.Num() && Steps[NextStepIndex].Time <= ScenarioClock)
	{
		UE_LOG(LogT66MotionRigScenario, Display, TEXT("[%6.2fs] step: %s"),
			ScenarioClock, *Steps[NextStepIndex].Label);
		Steps[NextStepIndex].Action(*Pawn);
		++NextStepIndex;
	}

	RecordTelemetryRow(*Pawn, DeltaTime);

	if (ScenarioClock >= EndTime)
	{
		FinishScenario();
	}
}

void UT66MotionRigScenario::RecordTelemetryRow(AT66MotionRigPawn& Pawn, const float DeltaTime)
{
	const FVector BeanLocation = Pawn.GetBean() ? Pawn.GetBean()->GetComponentLocation() : FVector::ZeroVector;
	const FVector BeanVelocity = Pawn.GetBeanVelocity();
	const FRotator BeanRotation = Pawn.GetBean() ? Pawn.GetBean()->GetComponentRotation() : FRotator::ZeroRotator;

	FVector PelvisLocation = BeanLocation;
	float PelvisSpeed = BeanVelocity.Size();
	float FootLSpeed = 0.f;
	float FootRSpeed = 0.f;
	float HandRZ = 0.f;
	float HeadZ = 0.f;

	if (USkeletalMeshComponent* Mesh = Pawn.GetRigMesh(); Mesh && Mesh->GetSkeletalMeshAsset())
	{
		// Read BODY INSTANCES, not bones: bone transforms depend on the
		// physics→bone blend path, which has its own failure modes; bodies
		// are the simulation ground truth.
		auto BodyZ = [Mesh](const FName Bone) -> float
		{
			const FBodyInstance* Body = Mesh->GetBodyInstance(Bone);
			return Body ? Body->GetUnrealWorldTransform().GetLocation().Z : 0.f;
		};
		if (const FBodyInstance* PelvisBody = Mesh->GetBodyInstance(TEXT("pelvis")))
		{
			PelvisLocation = PelvisBody->GetUnrealWorldTransform().GetLocation();
			PelvisSpeed = PelvisBody->GetUnrealWorldVelocity().Size();
		}
		if (const FBodyInstance* FootL = Mesh->GetBodyInstance(TEXT("foot_l")))
		{
			FootLSpeed = FootL->GetUnrealWorldVelocity().Size();
		}
		if (const FBodyInstance* FootR = Mesh->GetBodyInstance(TEXT("foot_r")))
		{
			FootRSpeed = FootR->GetUnrealWorldVelocity().Size();
		}
		HandRZ = BodyZ(TEXT("hand_r"));
		HeadZ = BodyZ(TEXT("head"));
	}

	TelemetryRows.Add(FString::Printf(
		TEXT("%.4f,%s,%d,%.2f,%.2f,%.2f,%.2f,%.2f,%.2f,%.2f,%.2f,%.2f,%.2f,%.2f,%.2f,%.2f,%.2f,%.2f,%.2f"),
		ScenarioClock,
		T66MotionRigStateName(Pawn.GetMotionState()),
		Pawn.IsBeanGrounded() ? 1 : 0,
		BeanLocation.X, BeanLocation.Y, BeanLocation.Z,
		BeanVelocity.X, BeanVelocity.Y, BeanVelocity.Z,
		BeanRotation.Pitch, BeanRotation.Roll,
		PelvisLocation.X, PelvisLocation.Y, PelvisLocation.Z,
		PelvisSpeed,
		FootLSpeed, FootRSpeed,
		HandRZ, HeadZ));
}

void UT66MotionRigScenario::FinishScenario()
{
	bRunning = false;
	if (AT66MotionRigPawn* Pawn = GetRigPawn())
	{
		Pawn->SetScenarioInputOverride(false);
	}

	const FString Directory = FPaths::Combine(FPaths::ProjectSavedDir(), TEXT("MotionRig"));
	IFileManager::Get().MakeDirectory(*Directory, true);
	const FString Timestamp = FDateTime::Now().ToString(TEXT("%Y%m%d_%H%M%S"));
	const FString FilePath = FPaths::Combine(
		Directory, FString::Printf(TEXT("telemetry_%s_%s.csv"), *ActiveScenarioName, *Timestamp));

	if (FFileHelper::SaveStringArrayToFile(TelemetryRows, *FilePath))
	{
		UE_LOG(LogT66MotionRigScenario, Display,
			TEXT("MotionRig scenario '%s' finished. Telemetry rows=%d -> %s"),
			*ActiveScenarioName, TelemetryRows.Num() - 1, *FilePath);
	}
	else
	{
		UE_LOG(LogT66MotionRigScenario, Error,
			TEXT("MotionRig telemetry write FAILED: %s"), *FilePath);
	}
}

void UT66MotionRigScenario::SetFixedCamera(const FString& CameraName)
{
	AT66MotionRigPawn* Pawn = GetRigPawn();
	UWorld* World = GetWorld();
	if (!Pawn || !World)
	{
		return;
	}

	APlayerController* PC = World->GetFirstPlayerController();
	if (!PC)
	{
		return;
	}

	const FString Name = CameraName.ToLower();
	if (Name == TEXT("chase"))
	{
		PC->SetViewTargetWithBlend(Pawn, 0.f);
		return;
	}

	const FVector Origin = Pawn->GetActorLocation();
	// Close enough to read limbs (a 180cm character at ~3.5m fills ~40% of
	// the frame height) — gait is unjudgeable from the original 6.5m offsets.
	FVector Offset = FVector(0.f, -340.f, 95.f); // side (looking +Y)
	if (Name == TEXT("front"))
	{
		Offset = FVector(340.f, 0.f, 95.f);
	}
	else if (Name == TEXT("threequarter"))
	{
		Offset = FVector(260.f, -260.f, 150.f);
	}

	if (!FixedCamera)
	{
		FActorSpawnParameters Params;
		Params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
		FixedCamera = World->SpawnActor<ACameraActor>(ACameraActor::StaticClass(), Origin + Offset, FRotator::ZeroRotator, Params);
		if (FixedCamera && FixedCamera->GetCameraComponent())
		{
			FixedCamera->GetCameraComponent()->SetConstraintAspectRatio(false);
		}
	}

	if (FixedCamera)
	{
		FixedCamera->SetActorLocation(Origin + Offset);
		FixedCamera->SetActorRotation((Origin + FVector(0.f, 0.f, 60.f) - (Origin + Offset)).Rotation());
		PC->SetViewTargetWithBlend(FixedCamera, 0.f);
	}
}
