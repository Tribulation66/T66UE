// Copyright Tribulation 66. All Rights Reserved.

#include "Gameplay/T66Hero1AxeAOEVFXLabActor.h"

#include "Components/SceneComponent.h"
#include "Dom/JsonObject.h"
#include "DynamicRHI.h"
#include "Engine/StaticMesh.h"
#include "HAL/FileManager.h"
#include "Materials/MaterialInterface.h"
#include "Misc/CommandLine.h"
#include "Misc/FileHelper.h"
#include "Misc/Parse.h"
#include "Misc/Paths.h"
#include "NiagaraComponent.h"
#include "NiagaraDataInterfaceArrayFunctionLibrary.h"
#include "NiagaraEmitter.h"
#include "NiagaraEmitterHandle.h"
#include "NiagaraEmitterInstance.h"
#include "NiagaraMeshRendererProperties.h"
#include "NiagaraRendererProperties.h"
#include "NiagaraSystem.h"
#include "NiagaraSystemInstance.h"
#include "NiagaraSystemInstanceController.h"
#include "RHIStats.h"
#include "Serialization/JsonSerializer.h"
#include "Serialization/JsonWriter.h"

namespace
{
	const TCHAR* T66Hero1AxeAOEMeshSlashNiagaraPath =
		TEXT("/Game/VFXLab/Hero1Axe/AOE/NS_Hero1AxeAOE_MeshSlash.NS_Hero1AxeAOE_MeshSlash");
	const FName T66Hero1AxeAOELabTag(TEXT("T66Hero1AxeAOEVFXLab"));

	FString T66DiagBool(const bool bValue)
	{
		return bValue ? TEXT("true") : TEXT("false");
	}

	FString T66DiagObjectPath(const UObject* Object)
	{
		return Object ? Object->GetPathName() : FString(TEXT("<null>"));
	}

	template <typename TEnum>
	FString T66DiagEnumToString(const TEnum Value)
	{
		if (const UEnum* Enum = StaticEnum<TEnum>())
		{
			return Enum->GetNameStringByValue(static_cast<int64>(Value));
		}
		return FString::FromInt(static_cast<int32>(Value));
	}

	TArray<FVector> T66BuildOutgoingTravelerArrayProofPositions(const int32 Count, const float Spacing)
	{
		TArray<FVector> Positions;
		Positions.Reserve(FMath::Max(0, Count));
		if (Count <= 0)
		{
			return Positions;
		}

		const int32 Columns = FMath::Max(1, FMath::CeilToInt(FMath::Sqrt(static_cast<float>(Count) * 2.0f)));
		const int32 Rows = FMath::Max(1, FMath::CeilToInt(static_cast<float>(Count) / static_cast<float>(Columns)));
		const float HalfWidth = static_cast<float>(Columns - 1) * Spacing * 0.5f;
		const float HalfHeight = static_cast<float>(Rows - 1) * Spacing * 0.5f;
		for (int32 Index = 0; Index < Count; ++Index)
		{
			const int32 Column = Index % Columns;
			const int32 Row = Index / Columns;
			const float X = static_cast<float>(Column) * Spacing - HalfWidth;
			const float Y = static_cast<float>(Row) * Spacing - HalfHeight;
			const float Z = FMath::Sin(static_cast<float>(Index) * 0.173f) * 2.0f;
			Positions.Add(FVector(X, Y, Z));
		}
		return Positions;
	}

	struct FT66OutgoingTravelerRuntimeStats
	{
		bool bComponentActive = false;
		bool bComponentVisible = false;
		bool bControllerValid = false;
		bool bInstancePresent = false;
		bool bInstanceReady = false;
		bool bHasGPUEmitters = false;
		int32 RuntimeEmitterCount = 0;
		int32 RuntimeTotalParticlesReported = 0;
		int32 RuntimeTotalParticlesCpuCounted = 0;
		FString RequestedExecutionState = TEXT("Unavailable");
		FString ActualExecutionState = TEXT("Unavailable");
		FString LocalBounds = TEXT("Unavailable");
	};

	FT66OutgoingTravelerRuntimeStats T66CollectOutgoingTravelerRuntimeStats(UNiagaraComponent* NiagaraComponent)
	{
		FT66OutgoingTravelerRuntimeStats Stats;
		if (!NiagaraComponent || !IsInGameThread())
		{
			return Stats;
		}

		Stats.bComponentActive = NiagaraComponent->IsActive();
		Stats.bComponentVisible = NiagaraComponent->IsVisible();
		const FNiagaraSystemInstanceControllerConstPtr Controller = NiagaraComponent->GetSystemInstanceController();
		Stats.bControllerValid = Controller.IsValid() && Controller->IsValid();
		FNiagaraSystemInstance* SystemInstance = Stats.bControllerValid ? Controller->GetSystemInstance_Unsafe() : nullptr;
		Stats.bInstancePresent = SystemInstance != nullptr;
		if (!SystemInstance)
		{
			return Stats;
		}

		Stats.bInstanceReady = SystemInstance->IsReadyToRun();
		Stats.bHasGPUEmitters = SystemInstance->HasGPUEmitters();
		Stats.RequestedExecutionState = T66DiagEnumToString(SystemInstance->GetRequestedExecutionState());
		Stats.ActualExecutionState = T66DiagEnumToString(SystemInstance->GetActualExecutionState());
		Stats.LocalBounds = SystemInstance->GetLocalBounds().ToString();
		const TConstArrayView<FNiagaraEmitterInstanceRef> EmitterInstances = SystemInstance->GetEmitters();
		Stats.RuntimeEmitterCount = EmitterInstances.Num();
		for (const FNiagaraEmitterInstanceRef& EmitterInstanceRef : EmitterInstances)
		{
			const FNiagaraEmitterInstance& EmitterInstance = EmitterInstanceRef.Get();
			Stats.RuntimeTotalParticlesReported += EmitterInstance.GetNumParticles();
			if (EmitterInstance.GetSimTarget() == ENiagaraSimTarget::CPUSim)
			{
				Stats.RuntimeTotalParticlesCpuCounted += EmitterInstance.GetNumParticles();
			}
		}
		return Stats;
	}
}

AT66Hero1AxeAOEVFXLabActor::AT66Hero1AxeAOEVFXLabActor()
{
	PrimaryActorTick.bCanEverTick = true;

	SceneRoot = CreateDefaultSubobject<USceneComponent>(TEXT("SceneRoot"));
	SetRootComponent(SceneRoot);

	SlashNiagaraComponent = CreateDefaultSubobject<UNiagaraComponent>(TEXT("SlashNiagaraComponent"));
	SlashNiagaraComponent->SetupAttachment(SceneRoot);
	SlashNiagaraComponent->SetAutoActivate(false);
	SlashNiagaraComponent->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	SlashNiagaraComponent->SetGenerateOverlapEvents(false);
	SlashNiagaraComponent->SetTranslucentSortPriority(14);
	SlashNiagaraComponent->SetRelativeLocation(FVector(28.0f, 0.0f, 70.0f));
	SlashNiagaraComponent->SetRelativeRotation(FRotator::ZeroRotator);
	SlashNiagaraComponent->SetRelativeScale3D(FVector::OneVector);
	SlashNiagaraComponent->SetCustomTimeDilation(1.0f);

	Tags.AddUnique(T66Hero1AxeAOELabTag);
}

void AT66Hero1AxeAOEVFXLabActor::OnConstruction(const FTransform& Transform)
{
	Super::OnConstruction(Transform);
	ApplyNiagaraSystems();
}

void AT66Hero1AxeAOEVFXLabActor::BeginPlay()
{
	Super::BeginPlay();
	SpawnTimeSeconds = GetWorld() ? GetWorld()->GetTimeSeconds() : 0.0f;
	LastSlashCycleIndex = INDEX_NONE;
	LastDiagnosticLogTimeSeconds = -1000.0f;
	bLoggedDiagnosticMatrix = false;
	OutgoingTravelerArrayProofStartTimeSeconds = SpawnTimeSeconds;
	OutgoingTravelerArrayProofSampleCount = 0;
	OutgoingTravelerArrayProofFpsSum = 0.0;
	OutgoingTravelerArrayProofFpsMin = 0.0;
	OutgoingTravelerArrayProofGpuFrameMsSum = 0.0;
	OutgoingTravelerArrayProofLastGpuFrameMs = -1.0;
	OutgoingTravelerArrayProofGpuFrameSamples = 0;
	OutgoingTravelerArrayProofLastDrawCalls = -1;
	OutgoingTravelerArrayProofMaxDrawCalls = -1;
	bOutgoingTravelerArrayProofManifestWritten = false;
	ApplyNiagaraSystems();
	LogSlashStaticDiagnostics(TEXT("BeginPlayAfterApply"));
	RestartSlashIfNeeded(SpawnTimeSeconds);
}

void AT66Hero1AxeAOEVFXLabActor::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);
	const float CurrentTimeSeconds = GetWorld() ? GetWorld()->GetTimeSeconds() : SpawnTimeSeconds;
	RestartSlashIfNeeded(CurrentTimeSeconds);

	constexpr float DiagnosticLogIntervalSeconds = 0.25f;
	if (CurrentTimeSeconds - LastDiagnosticLogTimeSeconds >= DiagnosticLogIntervalSeconds)
	{
		LastDiagnosticLogTimeSeconds = CurrentTimeSeconds;
		LogSlashRuntimeDiagnostics(TEXT("Tick"), CurrentTimeSeconds);
	}
	SampleOutgoingTravelerArrayProof(DeltaSeconds, CurrentTimeSeconds);
}

void AT66Hero1AxeAOEVFXLabActor::ApplyNiagaraSystems()
{
	if (!SlashNiagaraComponent)
	{
		return;
	}

	FString OverrideNiagaraPath;
	const TCHAR* NiagaraPath = T66Hero1AxeAOEMeshSlashNiagaraPath;
	if (FParse::Value(FCommandLine::Get(), TEXT("T66Hero1AxeAOEOverrideNiagara="), OverrideNiagaraPath) &&
		!OverrideNiagaraPath.IsEmpty())
	{
		NiagaraPath = *OverrideNiagaraPath;
	}
	UNiagaraSystem* MeshSlashSystem = LoadObject<UNiagaraSystem>(nullptr, NiagaraPath);
	SlashNiagaraComponent->SetAsset(MeshSlashSystem);
	float ComponentTimeDilation = 1.0f;
	if (FParse::Value(FCommandLine::Get(), TEXT("T66Hero1AxeAOEComponentTimeDilation="), ComponentTimeDilation))
	{
		ComponentTimeDilation = FMath::Clamp(ComponentTimeDilation, 0.05f, 4.0f);
	}
	SlashNiagaraComponent->SetCustomTimeDilation(ComponentTimeDilation);
	ApplyOutgoingTravelerArrayProof();
	UE_LOG(
		LogTemp,
		Display,
		TEXT("[Hero1AxeAOEDiag] phase=ApplyNiagaraSystems componentTimeDilation=%.3f"),
		ComponentTimeDilation);
	LogSlashStaticDiagnostics(TEXT("ApplyNiagaraSystems"));
}

void AT66Hero1AxeAOEVFXLabActor::ApplyOutgoingTravelerArrayProof()
{
	if (!SlashNiagaraComponent)
	{
		return;
	}

	int32 RequestedCount = 0;
	if (!FParse::Value(FCommandLine::Get(), TEXT("T66OutgoingTravelerArrayProofCount="), RequestedCount))
	{
		return;
	}
	RequestedCount = FMath::Clamp(RequestedCount, 1, 100000);

	float Spacing = 6.0f;
	FParse::Value(FCommandLine::Get(), TEXT("T66OutgoingTravelerArrayProofSpacing="), Spacing);
	Spacing = FMath::Clamp(Spacing, 1.0f, 1000.0f);

	FString ParameterName(TEXT("User.TravelerPositions"));
	FParse::Value(FCommandLine::Get(), TEXT("T66OutgoingTravelerArrayProofParameter="), ParameterName);
	ParameterName.TrimStartAndEndInline();
	ParameterName.TrimQuotesInline();
	if (ParameterName.IsEmpty())
	{
		ParameterName = TEXT("User.TravelerPositions");
	}

	FString ManifestPath;
	FParse::Value(FCommandLine::Get(), TEXT("T66OutgoingTravelerArrayProofManifest="), ManifestPath);
	ManifestPath.TrimStartAndEndInline();
	ManifestPath.TrimQuotesInline();

	SlashNiagaraComponent->SetAutoDestroy(false);
	SlashNiagaraComponent->SetForceSolo(true);

	const TArray<FVector> Positions = T66BuildOutgoingTravelerArrayProofPositions(RequestedCount, Spacing);
	const double UploadStartSeconds = FPlatformTime::Seconds();
	UNiagaraDataInterfaceArrayFunctionLibrary::SetNiagaraArrayVector(
		SlashNiagaraComponent,
		FName(*ParameterName),
		Positions);
	const double UploadSeconds = FPlatformTime::Seconds() - UploadStartSeconds;

	const TArray<FVector> Readback = UNiagaraDataInterfaceArrayFunctionLibrary::GetNiagaraArrayVector(
		SlashNiagaraComponent,
		FName(*ParameterName));

	bOutgoingTravelerArrayProofEnabled = true;
	OutgoingTravelerArrayProofParameter = ParameterName;
	OutgoingTravelerArrayProofManifestPath = ManifestPath;
	OutgoingTravelerArrayProofRequestedCount = RequestedCount;
	OutgoingTravelerArrayProofUploadedCount = Positions.Num();
	OutgoingTravelerArrayProofReadbackCount = Readback.Num();
	OutgoingTravelerArrayProofLastUploadSeconds = UploadSeconds;
	OutgoingTravelerArrayProofFirstPosition = Readback.Num() > 0 ? Readback[0].ToCompactString() : FString();
	OutgoingTravelerArrayProofLastPosition = Readback.Num() > 0 ? Readback.Last().ToCompactString() : FString();

	UE_LOG(
		LogTemp,
		Display,
		TEXT("[OutgoingTravelerProof] phase=ApplyArray requested=%d uploaded=%d readback=%d parameter=%s spacing=%.2f uploadMs=%.3f first=%s last=%s asset=%s component=%s"),
		OutgoingTravelerArrayProofRequestedCount,
		OutgoingTravelerArrayProofUploadedCount,
		OutgoingTravelerArrayProofReadbackCount,
		*OutgoingTravelerArrayProofParameter,
		Spacing,
		OutgoingTravelerArrayProofLastUploadSeconds * 1000.0,
		*OutgoingTravelerArrayProofFirstPosition,
		*OutgoingTravelerArrayProofLastPosition,
		*T66DiagObjectPath(SlashNiagaraComponent->GetAsset()),
		*GetNameSafe(SlashNiagaraComponent));
}

void AT66Hero1AxeAOEVFXLabActor::RestartSlashIfNeeded(const float TimeSeconds)
{
	if (!SlashNiagaraComponent || !SlashNiagaraComponent->GetAsset())
	{
		return;
	}

	float CycleDuration = 1.18f;
	if (FParse::Value(FCommandLine::Get(), TEXT("T66Hero1AxeAOECycleDuration="), CycleDuration))
	{
		CycleDuration = FMath::Clamp(CycleDuration, 0.2f, 8.0f);
	}
	const float Elapsed = FMath::Max(0.0f, TimeSeconds - SpawnTimeSeconds + 0.08f);
	const int32 SlashCycleIndex = FMath::FloorToInt(Elapsed / CycleDuration);
	if (SlashCycleIndex == LastSlashCycleIndex)
	{
		return;
	}

	LastSlashCycleIndex = SlashCycleIndex;
	SlashNiagaraComponent->DeactivateImmediate();
	ApplyOutgoingTravelerArrayProof();
	if (bOutgoingTravelerArrayProofEnabled)
	{
		SlashNiagaraComponent->ReinitializeSystem();
		ApplyOutgoingTravelerArrayProof();
	}
	SlashNiagaraComponent->Activate(true);
	int32 ManualWarmupTicks = 0;
	if (FParse::Value(FCommandLine::Get(), TEXT("T66Hero1AxeAOEManualWarmupTicks="), ManualWarmupTicks))
	{
		ManualWarmupTicks = FMath::Clamp(ManualWarmupTicks, 0, 240);
	}
	if (ManualWarmupTicks > 0)
	{
		float ManualWarmupDelta = 1.0f / 60.0f;
		if (FParse::Value(FCommandLine::Get(), TEXT("T66Hero1AxeAOEManualWarmupDelta="), ManualWarmupDelta))
		{
			ManualWarmupDelta = FMath::Clamp(ManualWarmupDelta, 1.0f / 240.0f, 1.0f / 10.0f);
		}
		SlashNiagaraComponent->AdvanceSimulation(ManualWarmupTicks, ManualWarmupDelta);
		UE_LOG(
			LogTemp,
			Display,
			TEXT("[Hero1AxeAOEDiag] phase=ManualWarmup ticks=%d delta=%.4f"),
			ManualWarmupTicks,
			ManualWarmupDelta);
	}
	LogSlashRuntimeDiagnostics(TEXT("Activate"), TimeSeconds);
}

void AT66Hero1AxeAOEVFXLabActor::SampleOutgoingTravelerArrayProof(const float DeltaSeconds, const float TimeSeconds)
{
	if (!bOutgoingTravelerArrayProofEnabled)
	{
		return;
	}

	if (DeltaSeconds > SMALL_NUMBER)
	{
		const double Fps = 1.0 / static_cast<double>(DeltaSeconds);
		OutgoingTravelerArrayProofFpsSum += Fps;
		OutgoingTravelerArrayProofFpsMin = OutgoingTravelerArrayProofFpsMin <= 0.0
			? Fps
			: FMath::Min(OutgoingTravelerArrayProofFpsMin, Fps);
		++OutgoingTravelerArrayProofSampleCount;
	}

	const uint32 GpuCycles = RHIGetGPUFrameCycles(0);
	if (GpuCycles > 0)
	{
		OutgoingTravelerArrayProofLastGpuFrameMs = FPlatformTime::ToMilliseconds(GpuCycles);
		OutgoingTravelerArrayProofGpuFrameMsSum += OutgoingTravelerArrayProofLastGpuFrameMs;
		++OutgoingTravelerArrayProofGpuFrameSamples;
	}

	OutgoingTravelerArrayProofLastDrawCalls = GNumDrawCallsRHI[0];
	OutgoingTravelerArrayProofMaxDrawCalls = FMath::Max(
		OutgoingTravelerArrayProofMaxDrawCalls,
		OutgoingTravelerArrayProofLastDrawCalls);

	if (!bOutgoingTravelerArrayProofManifestWritten &&
		!OutgoingTravelerArrayProofManifestPath.IsEmpty() &&
		OutgoingTravelerArrayProofSampleCount >= 12 &&
		TimeSeconds - OutgoingTravelerArrayProofStartTimeSeconds >= 1.0f)
	{
		const FT66OutgoingTravelerRuntimeStats RuntimeStats =
			T66CollectOutgoingTravelerRuntimeStats(SlashNiagaraComponent);
		if (RuntimeStats.bComponentActive &&
			RuntimeStats.RuntimeEmitterCount == 1 &&
			RuntimeStats.RuntimeTotalParticlesReported >= OutgoingTravelerArrayProofRequestedCount)
		{
			WriteOutgoingTravelerArrayProofManifest(TEXT("sample-window-complete"));
			bOutgoingTravelerArrayProofManifestWritten = true;
		}
		else if (TimeSeconds - OutgoingTravelerArrayProofStartTimeSeconds >= 12.0f)
		{
			WriteOutgoingTravelerArrayProofManifest(TEXT("sample-window-timeout-runtime-not-ready"));
			bOutgoingTravelerArrayProofManifestWritten = true;
		}
	}
}

void AT66Hero1AxeAOEVFXLabActor::WriteOutgoingTravelerArrayProofManifest(const TCHAR* Reason) const
{
	if (OutgoingTravelerArrayProofManifestPath.IsEmpty())
	{
		return;
	}

	const FT66OutgoingTravelerRuntimeStats RuntimeStats = T66CollectOutgoingTravelerRuntimeStats(SlashNiagaraComponent);
	const UNiagaraSystem* System = SlashNiagaraComponent ? SlashNiagaraComponent->GetAsset() : nullptr;
	const int32 SystemEmitterHandleCount = System ? System->GetEmitterHandles().Num() : 0;
	const double RoughFpsAverage = OutgoingTravelerArrayProofSampleCount > 0
		? OutgoingTravelerArrayProofFpsSum / static_cast<double>(OutgoingTravelerArrayProofSampleCount)
		: 0.0;
	const double RoughGpuMsAverage = OutgoingTravelerArrayProofGpuFrameSamples > 0
		? OutgoingTravelerArrayProofGpuFrameMsSum / static_cast<double>(OutgoingTravelerArrayProofGpuFrameSamples)
		: -1.0;

	TSharedRef<FJsonObject> Root = MakeShared<FJsonObject>();
	Root->SetStringField(TEXT("tool"), TEXT("T66OutgoingTravelerArrayProofGameplayLab"));
	Root->SetStringField(TEXT("phase"), TEXT("Phase 0"));
	Root->SetStringField(TEXT("reason"), Reason ? FString(Reason) : FString());
	Root->SetStringField(TEXT("command_line"), FCommandLine::Get());
	Root->SetStringField(TEXT("system_path"), T66DiagObjectPath(System));
	Root->SetStringField(TEXT("component_name"), GetNameSafe(SlashNiagaraComponent));
	Root->SetNumberField(TEXT("canonical_active_outgoing_travelers"), 5000);
	Root->SetNumberField(TEXT("array_proof_requested_count"), OutgoingTravelerArrayProofRequestedCount);
	Root->SetNumberField(TEXT("array_proof_uploaded_count"), OutgoingTravelerArrayProofUploadedCount);
	Root->SetNumberField(TEXT("array_proof_readback_count"), OutgoingTravelerArrayProofReadbackCount);
	Root->SetStringField(TEXT("array_proof_parameter"), OutgoingTravelerArrayProofParameter);
	Root->SetNumberField(TEXT("array_proof_last_upload_seconds"), OutgoingTravelerArrayProofLastUploadSeconds);
	Root->SetStringField(TEXT("array_proof_first_position"), OutgoingTravelerArrayProofFirstPosition);
	Root->SetStringField(TEXT("array_proof_last_position"), OutgoingTravelerArrayProofLastPosition);
	Root->SetNumberField(TEXT("niagara_components_spawned"), 1);
	Root->SetBoolField(TEXT("single_persistent_niagara_component"), true);
	Root->SetStringField(TEXT("integration_scope"), TEXT("capture-only lab actor; no projectile manager, combat, damage, or target lookup path"));
	Root->SetStringField(TEXT("global_max_projectiles_status"), TEXT("unchanged by this proof"));
	Root->SetStringField(TEXT("phase1_status"), TEXT("not_started"));
	Root->SetBoolField(TEXT("component_active"), RuntimeStats.bComponentActive);
	Root->SetBoolField(TEXT("component_visible"), RuntimeStats.bComponentVisible);
	Root->SetBoolField(TEXT("controller_valid"), RuntimeStats.bControllerValid);
	Root->SetBoolField(TEXT("system_instance_present"), RuntimeStats.bInstancePresent);
	Root->SetBoolField(TEXT("system_instance_ready"), RuntimeStats.bInstanceReady);
	Root->SetBoolField(TEXT("system_instance_has_gpu_emitters"), RuntimeStats.bHasGPUEmitters);
	Root->SetNumberField(TEXT("system_emitter_handle_count"), SystemEmitterHandleCount);
	Root->SetNumberField(TEXT("runtime_emitter_count"), RuntimeStats.RuntimeEmitterCount);
	Root->SetNumberField(TEXT("runtime_total_particles_reported"), RuntimeStats.RuntimeTotalParticlesReported);
	Root->SetNumberField(TEXT("runtime_total_particles_cpu_counted"), RuntimeStats.RuntimeTotalParticlesCpuCounted);
	Root->SetStringField(
		TEXT("runtime_particle_count_note"),
		RuntimeStats.bHasGPUEmitters
			? TEXT("GPU emitter count is reported by Niagara runtime diagnostics; CPU particle readback is not used for the proof.")
			: TEXT("CPU emitter count is counted directly from Niagara runtime emitter instances."));
	Root->SetStringField(TEXT("requested_execution_state"), RuntimeStats.RequestedExecutionState);
	Root->SetStringField(TEXT("actual_execution_state"), RuntimeStats.ActualExecutionState);
	Root->SetStringField(TEXT("local_bounds"), RuntimeStats.LocalBounds);
	Root->SetNumberField(TEXT("rough_fps_average_from_tick_delta"), RoughFpsAverage);
	Root->SetNumberField(TEXT("rough_fps_min_from_tick_delta"), OutgoingTravelerArrayProofFpsMin);
	Root->SetStringField(
		TEXT("rough_gpu_frame_time_ms_average"),
		RoughGpuMsAverage >= 0.0 ? FString::Printf(TEXT("%.4f"), RoughGpuMsAverage) : TEXT("Unavailable"));
	Root->SetStringField(
		TEXT("rough_gpu_frame_time_ms_last"),
		OutgoingTravelerArrayProofLastGpuFrameMs >= 0.0 ? FString::Printf(TEXT("%.4f"), OutgoingTravelerArrayProofLastGpuFrameMs) : TEXT("Unavailable"));
	Root->SetStringField(
		TEXT("rough_draw_calls_rhi_last"),
		OutgoingTravelerArrayProofLastDrawCalls >= 0 ? FString::FromInt(OutgoingTravelerArrayProofLastDrawCalls) : TEXT("Unavailable"));
	Root->SetStringField(
		TEXT("rough_draw_calls_rhi_max"),
		OutgoingTravelerArrayProofMaxDrawCalls >= 0 ? FString::FromInt(OutgoingTravelerArrayProofMaxDrawCalls) : TEXT("Unavailable"));
	Root->SetStringField(TEXT("rough_vram"), TEXT("Unavailable"));
	Root->SetStringField(TEXT("rough_thermals"), TEXT("Unavailable"));

	FString Serialized;
	const TSharedRef<TJsonWriter<>> Writer = TJsonWriterFactory<>::Create(&Serialized);
	if (!FJsonSerializer::Serialize(Root, Writer))
	{
		UE_LOG(LogTemp, Warning, TEXT("[OutgoingTravelerProof] phase=ManifestWrite status=serialize-failed path=%s"), *OutgoingTravelerArrayProofManifestPath);
		return;
	}

	IFileManager::Get().MakeDirectory(*FPaths::GetPath(OutgoingTravelerArrayProofManifestPath), true);
	const bool bSaved = FFileHelper::SaveStringToFile(
		Serialized,
		*OutgoingTravelerArrayProofManifestPath,
		FFileHelper::EEncodingOptions::ForceUTF8WithoutBOM);
	const TCHAR* SaveStatus = bSaved ? TEXT("saved") : TEXT("failed");
	const FString RoughGpuMsText = RoughGpuMsAverage >= 0.0 ? FString::Printf(TEXT("%.4f"), RoughGpuMsAverage) : TEXT("Unavailable");
	const FString DrawCallsMaxText = OutgoingTravelerArrayProofMaxDrawCalls >= 0 ? FString::FromInt(OutgoingTravelerArrayProofMaxDrawCalls) : TEXT("Unavailable");
	if (bSaved)
	{
		UE_LOG(
			LogTemp,
			Display,
			TEXT("[OutgoingTravelerProof] phase=ManifestWrite status=%s path=%s runtimeEmitters=%d runtimeParticlesReported=%d runtimeParticlesCpu=%d roughFpsAvg=%.2f roughGpuMsAvg=%s drawCallsMax=%s"),
			SaveStatus,
			*OutgoingTravelerArrayProofManifestPath,
			RuntimeStats.RuntimeEmitterCount,
			RuntimeStats.RuntimeTotalParticlesReported,
			RuntimeStats.RuntimeTotalParticlesCpuCounted,
			RoughFpsAverage,
			*RoughGpuMsText,
			*DrawCallsMaxText);
	}
	else
	{
		UE_LOG(
			LogTemp,
			Warning,
			TEXT("[OutgoingTravelerProof] phase=ManifestWrite status=%s path=%s runtimeEmitters=%d runtimeParticlesReported=%d runtimeParticlesCpu=%d roughFpsAvg=%.2f roughGpuMsAvg=%s drawCallsMax=%s"),
			SaveStatus,
			*OutgoingTravelerArrayProofManifestPath,
			RuntimeStats.RuntimeEmitterCount,
			RuntimeStats.RuntimeTotalParticlesReported,
			RuntimeStats.RuntimeTotalParticlesCpuCounted,
			RoughFpsAverage,
			*RoughGpuMsText,
			*DrawCallsMaxText);
	}
}

void AT66Hero1AxeAOEVFXLabActor::LogSlashStaticDiagnostics(const TCHAR* Phase) const
{
	if (!SlashNiagaraComponent)
	{
		UE_LOG(LogTemp, Display, TEXT("[Hero1AxeAOEDiag] phase=%s component=<null>"), Phase);
		return;
	}

	UNiagaraSystem* System = SlashNiagaraComponent->GetAsset();
	UE_LOG(
		LogTemp,
		Display,
		TEXT("[Hero1AxeAOEDiag] phase=%s asset=%s systemReady=%s componentActive=%s componentVisible=%s relLoc=%s relRot=%s relScale=%s"),
		Phase,
		*T66DiagObjectPath(System),
		System ? *T66DiagBool(System->IsReadyToRun()) : TEXT("false"),
		*T66DiagBool(SlashNiagaraComponent->IsActive()),
		*T66DiagBool(SlashNiagaraComponent->IsVisible()),
		*SlashNiagaraComponent->GetRelativeLocation().ToString(),
		*SlashNiagaraComponent->GetRelativeRotation().ToString(),
		*SlashNiagaraComponent->GetRelativeScale3D().ToString());

	if (!System)
	{
		UE_LOG(LogTemp, Display, TEXT("[Hero1AxeAOEDiag] phase=%s conclusion=component-asset-null activation cannot render"), Phase);
		return;
	}

	const TArray<FNiagaraEmitterHandle>& EmitterHandles = System->GetEmitterHandles();
	UE_LOG(LogTemp, Display, TEXT("[Hero1AxeAOEDiag] phase=%s emitterHandleCount=%d"), Phase, EmitterHandles.Num());
	if (EmitterHandles.Num() == 0)
	{
		UE_LOG(LogTemp, Display, TEXT("[Hero1AxeAOEDiag] phase=%s conclusion=emitter-serialization-or-system-build-failure"), Phase);
	}

	for (int32 EmitterIndex = 0; EmitterIndex < EmitterHandles.Num(); ++EmitterIndex)
	{
		const FNiagaraEmitterHandle& EmitterHandle = EmitterHandles[EmitterIndex];
		const FVersionedNiagaraEmitterData* EmitterData = EmitterHandle.GetEmitterData();
		const TArray<UNiagaraRendererProperties*>* Renderers = EmitterData ? &EmitterData->GetRenderers() : nullptr;
		UE_LOG(
			LogTemp,
			Display,
			TEXT("[Hero1AxeAOEDiag] phase=%s emitter[%d] name=%s valid=%s enabled=%s data=%s dataReady=%s simTarget=%s rendererCount=%d"),
			Phase,
			EmitterIndex,
			*EmitterHandle.GetName().ToString(),
			*T66DiagBool(EmitterHandle.IsValid()),
			*T66DiagBool(EmitterHandle.GetIsEnabled()),
			EmitterData ? TEXT("present") : TEXT("<null>"),
			EmitterData ? *T66DiagBool(EmitterData->IsReadyToRun()) : TEXT("false"),
			EmitterData ? *T66DiagEnumToString(EmitterData->SimTarget) : TEXT("<unknown>"),
			Renderers ? Renderers->Num() : 0);

		if (!Renderers)
		{
			continue;
		}

		for (int32 RendererIndex = 0; RendererIndex < Renderers->Num(); ++RendererIndex)
		{
			const UNiagaraRendererProperties* Renderer = (*Renderers)[RendererIndex];
			const UNiagaraMeshRendererProperties* MeshRenderer = Cast<UNiagaraMeshRendererProperties>(Renderer);
			FString MeshPath(TEXT("<not-mesh-renderer>"));
			FString MaterialPath(TEXT("<none>"));
			FString FacingMode(TEXT("<n/a>"));
			FString SourceMode(TEXT("<n/a>"));
			int32 MeshCount = 0;
			int32 OverrideMaterialCount = 0;

			if (MeshRenderer)
			{
				MeshCount = MeshRenderer->Meshes.Num();
				OverrideMaterialCount = MeshRenderer->OverrideMaterials.Num();
				if (MeshRenderer->Meshes.Num() > 0)
				{
					MeshPath = T66DiagObjectPath(MeshRenderer->Meshes[0].Mesh.Get());
				}
				if (MeshRenderer->OverrideMaterials.Num() > 0)
				{
					MaterialPath = T66DiagObjectPath(MeshRenderer->OverrideMaterials[0].ExplicitMat.Get());
				}
				FacingMode = T66DiagEnumToString(MeshRenderer->FacingMode);
				SourceMode = T66DiagEnumToString(MeshRenderer->SourceMode);
			}

			UE_LOG(
				LogTemp,
				Display,
				TEXT("[Hero1AxeAOEDiag] phase=%s emitter[%d].renderer[%d] class=%s enabled=%s simSupported=%s meshCount=%d mesh=%s overrideMaterialCount=%d material=%s facingMode=%s sourceMode=%s"),
				Phase,
				EmitterIndex,
				RendererIndex,
				Renderer ? *Renderer->GetClass()->GetName() : TEXT("<null>"),
				Renderer ? *T66DiagBool(Renderer->GetIsEnabled()) : TEXT("false"),
				(Renderer && EmitterData) ? *T66DiagBool(Renderer->IsSimTargetSupported(EmitterData->SimTarget)) : TEXT("false"),
				MeshCount,
				*MeshPath,
				OverrideMaterialCount,
				*MaterialPath,
				*FacingMode,
				*SourceMode);
		}
	}
}

void AT66Hero1AxeAOEVFXLabActor::LogSlashRuntimeDiagnostics(const TCHAR* Phase, const float TimeSeconds) const
{
	if (!SlashNiagaraComponent)
	{
		UE_LOG(LogTemp, Display, TEXT("[Hero1AxeAOEDiag] phase=%s time=%.3f component=<null>"), Phase, TimeSeconds);
		return;
	}

	if (!bLoggedDiagnosticMatrix)
	{
		bLoggedDiagnosticMatrix = true;
		UE_LOG(LogTemp, Display, TEXT("[Hero1AxeAOEDiag] decisionMatrix emitterHandleCount=0=>saved-system-or-emitter-build-failure rendererMeshOrMaterialNull=>renderer-binding-failure active=false-or-controller-null=>activation-instance-failure particlesGreaterThan0WithValidRendererButNoPixels=>material-orientation-bounds-visibility-issue particlesZeroWithActiveCpuEmitter=>spawn-or-lifetime-failure"));
	}

	if (!IsInGameThread())
	{
		UE_LOG(LogTemp, Display, TEXT("[Hero1AxeAOEDiag] phase=%s time=%.3f skipped=not-game-thread"), Phase, TimeSeconds);
		return;
	}

	UNiagaraSystem* System = SlashNiagaraComponent->GetAsset();
	const FNiagaraSystemInstanceControllerConstPtr Controller = SlashNiagaraComponent->GetSystemInstanceController();
	const bool bControllerValid = Controller.IsValid() && Controller->IsValid();
	FNiagaraSystemInstance* SystemInstance = bControllerValid ? Controller->GetSystemInstance_Unsafe() : nullptr;

	UE_LOG(
		LogTemp,
		Display,
		TEXT("[Hero1AxeAOEDiag] phase=%s time=%.3f asset=%s componentActive=%s componentVisible=%s controllerValid=%s instance=%s cycle=%d"),
		Phase,
		TimeSeconds,
		*T66DiagObjectPath(System),
		*T66DiagBool(SlashNiagaraComponent->IsActive()),
		*T66DiagBool(SlashNiagaraComponent->IsVisible()),
		*T66DiagBool(bControllerValid),
		SystemInstance ? TEXT("present") : TEXT("<null>"),
		LastSlashCycleIndex);

	if (!SystemInstance)
	{
		UE_LOG(LogTemp, Display, TEXT("[Hero1AxeAOEDiag] phase=%s time=%.3f conclusion=activation-instance-failure"), Phase, TimeSeconds);
		return;
	}

	UE_LOG(
		LogTemp,
		Display,
		TEXT("[Hero1AxeAOEDiag] phase=%s time=%.3f instanceReady=%s instanceAge=%.3f instanceTick=%d requestedState=%s actualState=%s localBounds=%s hasGPUEmitters=%s"),
		Phase,
		TimeSeconds,
		*T66DiagBool(SystemInstance->IsReadyToRun()),
		SystemInstance->GetAge(),
		SystemInstance->GetTickCount(),
		*T66DiagEnumToString(SystemInstance->GetRequestedExecutionState()),
		*T66DiagEnumToString(SystemInstance->GetActualExecutionState()),
		*SystemInstance->GetLocalBounds().ToString(),
		*T66DiagBool(SystemInstance->HasGPUEmitters()));

	const TConstArrayView<FNiagaraEmitterInstanceRef> EmitterInstances = SystemInstance->GetEmitters();
	UE_LOG(LogTemp, Display, TEXT("[Hero1AxeAOEDiag] phase=%s time=%.3f runtimeEmitterCount=%d"), Phase, TimeSeconds, EmitterInstances.Num());

	for (int32 EmitterIndex = 0; EmitterIndex < EmitterInstances.Num(); ++EmitterIndex)
	{
		const FNiagaraEmitterInstance& EmitterInstance = EmitterInstances[EmitterIndex].Get();
		const ENiagaraSimTarget SimTarget = EmitterInstance.GetSimTarget();
		const bool bParticleCountReliable = SimTarget == ENiagaraSimTarget::CPUSim;
		int32 EnabledRendererCount = 0;
		EmitterInstance.ForEachEnabledRenderer(
			[&EnabledRendererCount](const UNiagaraRendererProperties*)
			{
				++EnabledRendererCount;
			});
		// FNiagaraEmitterInstance::IsReadyToRun is deprecated (5.4); report the versioned
		// emitter data's readiness like the asset-side diagnostics above.
		const FVersionedNiagaraEmitterData* RuntimeEmitterData = EmitterInstance.GetEmitterHandle().GetEmitterData();
		UE_LOG(
			LogTemp,
			Display,
			TEXT("[Hero1AxeAOEDiag] phase=%s time=%.3f runtimeEmitter[%d] name=%s simTarget=%s particleCount=%d particleCountReliable=%s execState=%s ready=%s active=%s bounds=%s rendererCount=%d"),
			Phase,
			TimeSeconds,
			EmitterIndex,
			*EmitterInstance.GetEmitterHandle().GetName().ToString(),
			*T66DiagEnumToString(SimTarget),
			EmitterInstance.GetNumParticles(),
			*T66DiagBool(bParticleCountReliable),
			*T66DiagEnumToString(EmitterInstance.GetExecutionState()),
			*T66DiagBool(RuntimeEmitterData && RuntimeEmitterData->IsReadyToRun()),
			*T66DiagBool(EmitterInstance.IsActive()),
			*EmitterInstance.GetBounds().ToString(),
			EnabledRendererCount);
	}
}
