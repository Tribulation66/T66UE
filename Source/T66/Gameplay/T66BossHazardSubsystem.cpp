// Copyright Tribulation 66. All Rights Reserved.

#include "Gameplay/T66BossHazardSubsystem.h"

#include "Core/T66GameInstance.h"
#include "Core/T66RunStateSubsystem.h"
#include "Gameplay/T66HeroBase.h"
#include "Gameplay/T66GameMode.h"
#include "Gameplay/T66VisualUtil.h"
#include "Components/HierarchicalInstancedStaticMeshComponent.h"
#include "Components/SceneComponent.h"
#include "Engine/StaticMesh.h"
#include "Engine/World.h"
#include "GameFramework/Actor.h"
#include "GameFramework/PlayerController.h"
#include "HAL/FileManager.h"
#include "HAL/IConsoleManager.h"
#include "HAL/PlatformMisc.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "Misc/App.h"
#include "Misc/CommandLine.h"
#include "Misc/FileHelper.h"
#include "Misc/Parse.h"
#include "Misc/Paths.h"

DEFINE_LOG_CATEGORY_STATIC(LogT66BossHazardSubsystem, Log, All);

namespace
{
double T66BossHazardCyclesToMicroseconds(const uint64 Cycles)
{
	return FPlatformTime::ToMilliseconds64(Cycles) * 1000.0;
}

const FName T66BossHazardShape_Circle(TEXT("Circle"));
const FName T66BossHazardShape_Box(TEXT("Box"));
const FName T66BossHazardDefaultVisualProfile(TEXT("BossHazard.Visual.Default"));

FString T66BossHazardMakeBucketKey(const FT66BossHazardDefinitionData& Definition, const bool bTelegraph)
{
	const FName VisualProfileID = Definition.VisualProfileID.IsNone() ? T66BossHazardDefaultVisualProfile : Definition.VisualProfileID;
	return FString::Printf(
		TEXT("%s|%s|%s"),
		*VisualProfileID.ToString(),
		*Definition.ShapeType.ToString(),
		bTelegraph ? TEXT("Telegraph") : TEXT("Active"));
}

FLinearColor T66BossHazardPhaseColor(const FT66BossHazardDefinitionData& Definition, const bool bTelegraph)
{
	return bTelegraph
		? FLinearColor(Definition.TelegraphR, Definition.TelegraphG, Definition.TelegraphB, 0.55f)
		: FLinearColor(Definition.ActiveR, Definition.ActiveG, Definition.ActiveB, 0.82f);
}

#if !UE_BUILD_SHIPPING
const TCHAR* T66BossHazardBoolString(const bool bValue)
{
	return bValue ? TEXT("true") : TEXT("false");
}

FString T66BossHazardJsonEscape(FString Value)
{
	Value.ReplaceInline(TEXT("\\"), TEXT("\\\\"));
	Value.ReplaceInline(TEXT("\""), TEXT("\\\""));
	Value.ReplaceInline(TEXT("\r"), TEXT("\\r"));
	Value.ReplaceInline(TEXT("\n"), TEXT("\\n"));
	return Value;
}

FString T66BossHazardFloatArrayJson(const TArray<float>& Values)
{
	FString Result(TEXT("["));
	for (int32 Index = 0; Index < Values.Num(); ++Index)
	{
		if (Index > 0)
		{
			Result += TEXT(",");
		}
		Result += FString::Printf(TEXT("%.3f"), Values[Index]);
	}
	Result += TEXT("]");
	return Result;
}
#endif
}

void FT66BossHazardDiagnostics::Reset(const FString& Reason)
{
	++ResetCount;
	HazardsSpawned = 0;
	HazardsActivePeak = 0;
	HazardsExpired = 0;
	DroppedByCap = 0;
	DroppedMissingDefinition = 0;
	DamageTicks = 0;
	DamageApplications = 0;
	ManagerTickMaxUs = 0.0;
	ManagerTickTotalUs = 0.0;
	ManagerTickSamples = 0;
	HISMUpdateMaxUs = 0.0;
	HISMUpdateTotalUs = 0.0;
	HISMUpdateSamples = 0;
	LastResetReason = Reason;
}

double FT66BossHazardDiagnostics::GetManagerTickAvgUs() const
{
	return ManagerTickSamples > 0 ? ManagerTickTotalUs / static_cast<double>(ManagerTickSamples) : 0.0;
}

double FT66BossHazardDiagnostics::GetHISMUpdateAvgUs() const
{
	return HISMUpdateSamples > 0 ? HISMUpdateTotalUs / static_cast<double>(HISMUpdateSamples) : 0.0;
}

void UT66BossHazardSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);
	Diagnostics.Reset(TEXT("Initialize"));
	bInitialized = true;
	bShuttingDown = false;
}

void UT66BossHazardSubsystem::Deinitialize()
{
	bShuttingDown = true;
	for (TObjectPtr<UHierarchicalInstancedStaticMeshComponent>& Component : RenderComponents)
	{
		if (Component)
		{
			Component->DestroyComponent();
			Component = nullptr;
		}
	}
	RenderComponents.Reset();
	RenderBuckets.Reset();
	RenderBucketByKey.Reset();
	if (RenderHost)
	{
		RenderHost->Destroy();
		RenderHost = nullptr;
	}
	RenderRoot = nullptr;
	ActiveHazards.Reset();
	bInitialized = false;
	Super::Deinitialize();
}

bool UT66BossHazardSubsystem::SpawnBossHazard(const FT66BossHazardSpawnParams& Params)
{
	FT66BossHazardDefinitionData Definition;
	if (!ResolveHazardDefinition(Params.HazardID, Definition))
	{
		++Diagnostics.DroppedMissingDefinition;
		return false;
	}

	const int32 MaxActiveCount = FMath::Max(1, Definition.MaxActiveCount);
	if (CountActiveHazardsForID(Definition.HazardID) >= MaxActiveCount)
	{
		++Diagnostics.DroppedByCap;
		return false;
	}

	FT66BossHazardEntry& Hazard = ActiveHazards.AddDefaulted_GetRef();
	Hazard.Definition = Definition;
	Hazard.SourceActor = Params.SourceActor;
	Hazard.SourceID = Params.SourceID;
	Hazard.Location = Params.Location;
	Hazard.Rotation = Params.Rotation;
	Hazard.RadiusScale = FMath::Max(0.05f, Params.RadiusScale);
	Hazard.TelegraphScale = FMath::Max(0.05f, Params.TelegraphScale);
	Hazard.VisualScaleMultiplier = FMath::Max(0.05f, Params.VisualScaleMultiplier);
	Hazard.AgeSeconds = 0.f;
	Hazard.NextDamageSeconds = FMath::Max(0.f, Definition.TelegraphSeconds);
	Hazard.DamageHP = Params.DamageOverrideHP >= 0 ? Params.DamageOverrideHP : Definition.DamageHP;

	++Diagnostics.HazardsSpawned;
	Diagnostics.HazardsActivePeak = FMath::Max(Diagnostics.HazardsActivePeak, ActiveHazards.Num());
	return true;
}

void UT66BossHazardSubsystem::ResetBossHazardDiagnostics(const TCHAR* Reason)
{
	Diagnostics.Reset(Reason ? FString(Reason) : FString(TEXT("Manual")));
}

void UT66BossHazardSubsystem::EmitBossHazardSummary(const TCHAR* Reason, const bool bTerminal)
{
	UE_LOG(
		LogT66BossHazardSubsystem,
		Display,
		TEXT("[BossHazardSummary] Reason=%s Terminal=%d Active=%d Spawned=%d ActivePeak=%d Expired=%d DroppedByCap=%d DroppedMissingDefinition=%d DamageTicks=%d DamageApplications=%d RenderBuckets=%d ManagerTickMaxUs=%.1f ManagerTickAvgUs=%.1f HISMUpdateMaxUs=%.1f HISMUpdateAvgUs=%.1f"),
		Reason ? Reason : TEXT("None"),
		bTerminal ? 1 : 0,
		GetActiveHazardCount(),
		Diagnostics.HazardsSpawned,
		Diagnostics.HazardsActivePeak,
		Diagnostics.HazardsExpired,
		Diagnostics.DroppedByCap,
		Diagnostics.DroppedMissingDefinition,
		Diagnostics.DamageTicks,
		Diagnostics.DamageApplications,
		RenderBuckets.Num(),
		Diagnostics.ManagerTickMaxUs,
		Diagnostics.GetManagerTickAvgUs(),
		Diagnostics.HISMUpdateMaxUs,
		Diagnostics.GetHISMUpdateAvgUs());
}

void UT66BossHazardSubsystem::Tick(const float DeltaTime)
{
	if (!IsTickable() || DeltaTime <= 0.f)
	{
		return;
	}

	const uint64 TickStartCycles = FPlatformTime::Cycles64();
	for (int32 Index = ActiveHazards.Num() - 1; Index >= 0; --Index)
	{
		FT66BossHazardEntry& Hazard = ActiveHazards[Index];
		Hazard.AgeSeconds += DeltaTime;

		const float ActiveStart = FMath::Max(0.f, Hazard.Definition.TelegraphSeconds);
		const float ActiveEnd = ActiveStart + FMath::Max(0.f, Hazard.Definition.ActiveSeconds);
		const float TotalLifetime = ActiveEnd + FMath::Max(0.f, Hazard.Definition.LingerSeconds);
		if (Hazard.AgeSeconds >= TotalLifetime)
		{
			ActiveHazards.RemoveAtSwap(Index, 1, EAllowShrinking::No);
			++Diagnostics.HazardsExpired;
			continue;
		}

		if (Hazard.DamageHP > 0
			&& Hazard.AgeSeconds >= ActiveStart
			&& Hazard.AgeSeconds <= ActiveEnd
			&& Hazard.AgeSeconds >= Hazard.NextDamageSeconds)
		{
			ApplyHazardDamage(Hazard);
			Hazard.NextDamageSeconds = Hazard.AgeSeconds + FMath::Max(0.05f, Hazard.Definition.DamageCadenceSeconds);
		}
	}

	FlushRenderInstances();

	const double TickUs = T66BossHazardCyclesToMicroseconds(FPlatformTime::Cycles64() - TickStartCycles);
	Diagnostics.ManagerTickMaxUs = FMath::Max(Diagnostics.ManagerTickMaxUs, TickUs);
	Diagnostics.ManagerTickTotalUs += TickUs;
	++Diagnostics.ManagerTickSamples;

#if !UE_BUILD_SHIPPING
	TickBossHazardDamageProof();
#endif
}

TStatId UT66BossHazardSubsystem::GetStatId() const
{
	RETURN_QUICK_DECLARE_CYCLE_STAT(UT66BossHazardSubsystem, STATGROUP_Tickables);
}

bool UT66BossHazardSubsystem::IsTickable() const
{
	return bInitialized && !bShuttingDown && ActiveHazards.Num() > 0 && GetWorld() && GetWorld()->IsGameWorld();
}

bool UT66BossHazardSubsystem::IsTickableWhenPaused() const
{
	return false;
}

bool UT66BossHazardSubsystem::IsTickableInEditor() const
{
	return false;
}

UWorld* UT66BossHazardSubsystem::GetTickableGameObjectWorld() const
{
	return GetWorld();
}

bool UT66BossHazardSubsystem::ResolveHazardDefinition(const FName HazardID, FT66BossHazardDefinitionData& OutDefinition) const
{
	const UWorld* World = GetWorld();
	UGameInstance* GI = World ? World->GetGameInstance() : nullptr;
	UT66GameInstance* T66GI = Cast<UT66GameInstance>(GI);
	return T66GI && T66GI->GetBossHazardDefinitionData(HazardID, OutDefinition);
}

int32 UT66BossHazardSubsystem::CountActiveHazardsForID(const FName HazardID) const
{
	int32 Count = 0;
	for (const FT66BossHazardEntry& Hazard : ActiveHazards)
	{
		if (Hazard.Definition.HazardID == HazardID)
		{
			++Count;
		}
	}
	return Count;
}

bool UT66BossHazardSubsystem::EnsureRenderResources()
{
	UWorld* World = GetWorld();
	if (!World || !World->IsGameWorld())
	{
		return false;
	}

	if (!RenderHost)
	{
		FActorSpawnParameters SpawnParams;
		SpawnParams.ObjectFlags |= RF_Transient;
		SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
		RenderHost = World->SpawnActor<AActor>(AActor::StaticClass(), FVector::ZeroVector, FRotator::ZeroRotator, SpawnParams);
		if (!RenderHost)
		{
			return false;
		}
		RenderHost->SetActorHiddenInGame(false);

		RenderRoot = NewObject<USceneComponent>(RenderHost, TEXT("BossHazardRenderRoot"));
		if (!RenderRoot)
		{
			return false;
		}
		RenderRoot->SetMobility(EComponentMobility::Movable);
		RenderRoot->RegisterComponent();
		RenderHost->SetRootComponent(RenderRoot);
	}

	return true;
}

int32 UT66BossHazardSubsystem::FindOrCreateRenderBucket(const FT66BossHazardDefinitionData& Definition, const bool bTelegraph)
{
	const FString Key = T66BossHazardMakeBucketKey(Definition, bTelegraph);
	if (const int32* Existing = RenderBucketByKey.Find(Key))
	{
		return *Existing;
	}

	if (!EnsureRenderResources() || !RenderHost || !RenderRoot)
	{
		return INDEX_NONE;
	}

	UStaticMesh* Mesh = Definition.ShapeType == T66BossHazardShape_Box
		? FT66VisualUtil::GetBasicShapeCube()
		: FT66VisualUtil::GetBasicShapeCylinder();
	if (!Mesh)
	{
		return INDEX_NONE;
	}

	UHierarchicalInstancedStaticMeshComponent* Component = NewObject<UHierarchicalInstancedStaticMeshComponent>(RenderHost);
	if (!Component)
	{
		return INDEX_NONE;
	}
	Component->SetMobility(EComponentMobility::Movable);
	Component->SetupAttachment(RenderRoot);
	Component->SetStaticMesh(Mesh);
	Component->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	Component->SetGenerateOverlapEvents(false);
	Component->SetCanEverAffectNavigation(false);
	Component->CastShadow = false;
	Component->bCastDynamicShadow = false;
	Component->bCastStaticShadow = false;
	Component->SetReceivesDecals(false);
	if (UMaterialInterface* BaseMaterial = FT66VisualUtil::GetFlatColorMaterial())
	{
		if (UMaterialInstanceDynamic* Material = UMaterialInstanceDynamic::Create(BaseMaterial, Component))
		{
			FT66VisualUtil::ConfigureFlatColorMaterial(Material, T66BossHazardPhaseColor(Definition, bTelegraph));
			Component->SetMaterial(0, Material);
		}
	}
	Component->RegisterComponent();

	FT66BossHazardRenderBucket& Bucket = RenderBuckets.AddDefaulted_GetRef();
	Bucket.Component = Component;
	Bucket.VisualProfileID = Definition.VisualProfileID.IsNone() ? T66BossHazardDefaultVisualProfile : Definition.VisualProfileID;
	Bucket.ShapeType = Definition.ShapeType;
	Bucket.bTelegraph = bTelegraph;
	Bucket.Color = T66BossHazardPhaseColor(Definition, bTelegraph);

	const int32 BucketIndex = RenderBuckets.Num() - 1;
	RenderComponents.Add(Component);
	RenderBucketByKey.Add(Key, BucketIndex);
	return BucketIndex;
}

void UT66BossHazardSubsystem::FlushRenderInstances()
{
	const uint64 StartCycles = FPlatformTime::Cycles64();
	for (FT66BossHazardRenderBucket& Bucket : RenderBuckets)
	{
		if (Bucket.Component)
		{
			Bucket.Component->ClearInstances();
		}
	}

	for (const FT66BossHazardEntry& Hazard : ActiveHazards)
	{
		const float ActiveStart = FMath::Max(0.f, Hazard.Definition.TelegraphSeconds);
		const float ActiveEnd = ActiveStart + FMath::Max(0.f, Hazard.Definition.ActiveSeconds);
		const float TotalLifetime = ActiveEnd + FMath::Max(0.f, Hazard.Definition.LingerSeconds);
		const bool bTelegraph = Hazard.AgeSeconds < ActiveStart;
		const float PhaseDuration = bTelegraph
			? FMath::Max(0.05f, ActiveStart)
			: FMath::Max(0.05f, TotalLifetime - ActiveStart);
		const float PhaseStart = bTelegraph ? 0.f : ActiveStart;
		const float PhaseAlpha = FMath::Clamp((Hazard.AgeSeconds - PhaseStart) / PhaseDuration, 0.f, 1.f);

		const int32 BucketIndex = FindOrCreateRenderBucket(Hazard.Definition, bTelegraph);
		if (!RenderBuckets.IsValidIndex(BucketIndex) || !RenderBuckets[BucketIndex].Component)
		{
			continue;
		}

		FTransform InstanceTransform(Hazard.Rotation, Hazard.Location, GetHazardVisualScale(Hazard, bTelegraph, PhaseAlpha));
		RenderBuckets[BucketIndex].Component->AddInstance(InstanceTransform, true);
	}

	const double FlushUs = T66BossHazardCyclesToMicroseconds(FPlatformTime::Cycles64() - StartCycles);
	Diagnostics.HISMUpdateMaxUs = FMath::Max(Diagnostics.HISMUpdateMaxUs, FlushUs);
	Diagnostics.HISMUpdateTotalUs += FlushUs;
	++Diagnostics.HISMUpdateSamples;
}

void UT66BossHazardSubsystem::ApplyHazardDamage(const FT66BossHazardEntry& Hazard)
{
	++Diagnostics.DamageTicks;
	UWorld* World = GetWorld();
	UGameInstance* GI = World ? World->GetGameInstance() : nullptr;
	UT66RunStateSubsystem* RunState = GI ? GI->GetSubsystem<UT66RunStateSubsystem>() : nullptr;
	if (!World || !RunState)
	{
		return;
	}

	for (FConstPlayerControllerIterator It = World->GetPlayerControllerIterator(); It; ++It)
	{
		const APlayerController* Controller = It->Get();
		const APawn* Pawn = Controller ? Controller->GetPawn() : nullptr;
		const AT66HeroBase* Hero = Cast<AT66HeroBase>(Pawn);
		if (!Hero || Hero->IsInSafeZone() || !IsHeroInsideHazard(Hero, Hazard))
		{
			continue;
		}

		if (RunState->ApplyDamage(Hazard.DamageHP, Hazard.SourceActor.Get(), FName(TEXT("BossHazard")), Hazard.SourceActor.Get()))
		{
			++Diagnostics.DamageApplications;
		}
	}
}

bool UT66BossHazardSubsystem::IsHeroInsideHazard(const AT66HeroBase* Hero, const FT66BossHazardEntry& Hazard) const
{
	if (!Hero)
	{
		return false;
	}

	const FVector Delta = Hero->GetActorLocation() - Hazard.Location;
	if (Hazard.Definition.ShapeType == T66BossHazardShape_Box)
	{
		const FVector Local = Hazard.Rotation.UnrotateVector(Delta);
		return FMath::Abs(Local.X) <= FMath::Max(1.f, Hazard.Definition.BoxExtentX)
			&& FMath::Abs(Local.Y) <= FMath::Max(1.f, Hazard.Definition.BoxExtentY)
			&& FMath::Abs(Local.Z) <= FMath::Max(1.f, Hazard.Definition.BoxExtentZ);
	}

	const float Radius = FMath::Max(1.f, Hazard.Definition.Radius * Hazard.RadiusScale);
	return FVector::DistSquared2D(Hero->GetActorLocation(), Hazard.Location) <= FMath::Square(Radius);
}

FVector UT66BossHazardSubsystem::GetHazardVisualScale(const FT66BossHazardEntry& Hazard, const bool bTelegraph, const float PhaseAlpha) const
{
	const FVector AuthoredScale(
		FMath::Max(0.01f, Hazard.Definition.VisualScaleX),
		FMath::Max(0.01f, Hazard.Definition.VisualScaleY),
		FMath::Max(0.01f, Hazard.Definition.VisualScaleZ));
	const float TelegraphMultiplier = bTelegraph
		? FMath::Lerp(0.35f, FMath::Max(0.05f, Hazard.TelegraphScale), PhaseAlpha)
		: 1.f;

	if (Hazard.Definition.ShapeType == T66BossHazardShape_Box)
	{
		const FVector BoxScale(
			FMath::Max(1.f, Hazard.Definition.BoxExtentX) / 50.f,
			FMath::Max(1.f, Hazard.Definition.BoxExtentY) / 50.f,
			FMath::Max(1.f, Hazard.Definition.BoxExtentZ) / 50.f);
		return BoxScale * AuthoredScale * Hazard.VisualScaleMultiplier * TelegraphMultiplier;
	}

	const float RadiusScale = FMath::Max(1.f, Hazard.Definition.Radius * Hazard.RadiusScale) / 50.f;
	return FVector(RadiusScale, RadiusScale, 0.025f) * AuthoredScale * Hazard.VisualScaleMultiplier * TelegraphMultiplier;
}

#if !UE_BUILD_SHIPPING
bool UT66BossHazardSubsystem::RunBossHazardDefinitionProof()
{
	ResetBossHazardDiagnostics(TEXT("BossHazardDefinitionProof"));
	ActiveHazards.Reset();

	int32 RequestedCount = 256;
	FParse::Value(FCommandLine::Get(), TEXT("T66BossHazardProofCount="), RequestedCount);
	RequestedCount = FMath::Clamp(RequestedCount, 1, 512);

	const int32 Columns = FMath::Max(1, FMath::CeilToInt(FMath::Sqrt(static_cast<float>(RequestedCount))));
	const float Spacing = 420.f;
	const FVector Origin(-static_cast<float>(Columns) * Spacing * 0.5f, -static_cast<float>(Columns) * Spacing * 0.5f, 45.f);

	int32 Spawned = 0;
	for (int32 Index = 0; Index < RequestedCount; ++Index)
	{
		const int32 X = Index % Columns;
		const int32 Y = Index / Columns;
		FT66BossHazardSpawnParams Params;
		Params.HazardID = FName(TEXT("BossHazard.SlimePatch"));
		Params.SourceID = FName(TEXT("BossHazardDefinitionProof"));
		Params.Location = Origin + FVector(static_cast<float>(X) * Spacing, static_cast<float>(Y) * Spacing, 45.f);
		Params.Rotation = FRotator::ZeroRotator;
		Params.DamageOverrideHP = 0;
		if (SpawnBossHazard(Params))
		{
			++Spawned;
		}
	}

	for (int32 Step = 0; Step < 90; ++Step)
	{
		Tick(1.f / 60.f);
	}

	const bool bPass =
		Spawned == RequestedCount
		&& Diagnostics.HazardsSpawned == RequestedCount
		&& Diagnostics.HazardsActivePeak >= RequestedCount
		&& Diagnostics.DroppedByCap == 0
		&& Diagnostics.DroppedMissingDefinition == 0
		&& Diagnostics.HISMUpdateSamples > 0
		&& RenderBuckets.Num() > 0;

	UE_LOG(
		LogT66BossHazardSubsystem,
		Display,
		TEXT("[BossHazardDefinitionProofSummary] Terminal=1 Requested=%d Spawned=%d ActivePeak=%d Active=%d DroppedByCap=%d DroppedMissingDefinition=%d DamageTicks=%d DamageApplications=%d RenderBuckets=%d ManagerTickMaxUs=%.1f ManagerTickAvgUs=%.1f HISMUpdateMaxUs=%.1f HISMUpdateAvgUs=%.1f Pass=%d"),
		RequestedCount,
		Spawned,
		Diagnostics.HazardsActivePeak,
		GetActiveHazardCount(),
		Diagnostics.DroppedByCap,
		Diagnostics.DroppedMissingDefinition,
		Diagnostics.DamageTicks,
		Diagnostics.DamageApplications,
		RenderBuckets.Num(),
		Diagnostics.ManagerTickMaxUs,
		Diagnostics.GetManagerTickAvgUs(),
		Diagnostics.HISMUpdateMaxUs,
		Diagnostics.GetHISMUpdateAvgUs(),
		bPass ? 1 : 0);
	EmitBossHazardSummary(TEXT("BossHazardDefinitionProof"), true);
	return bPass;
}

bool UT66BossHazardSubsystem::StartBossHazardDamageProof()
{
	if (DamageProof.bRunning)
	{
		return false;
	}

	DamageProof = FT66BossHazardDamageProofRuntime();
	DamageProof.bRunning = true;
	DamageProof.HazardID = FName(TEXT("BossHazard.SlimePatch"));

	FString HazardIDString;
	if (FParse::Value(FCommandLine::Get(), TEXT("T66BossHazardDamageProofHazardID="), HazardIDString)
		&& !HazardIDString.TrimStartAndEnd().IsEmpty())
	{
		DamageProof.HazardID = FName(*HazardIDString.TrimStartAndEnd());
	}

	if (!FParse::Value(FCommandLine::Get(), TEXT("T66BossHazardDamageProofManifest="), DamageProof.ManifestPath)
		|| DamageProof.ManifestPath.TrimStartAndEnd().IsEmpty())
	{
		DamageProof.ManifestPath = FPaths::Combine(FPaths::ProjectSavedDir(), TEXT("Codex/BossHazardDamageProof/hazard_damage_manifest.json"));
	}
	DamageProof.ManifestPath = FPaths::ConvertRelativePathToFull(DamageProof.ManifestPath);

	if (!ResolveHazardDefinition(DamageProof.HazardID, DamageProof.Definition))
	{
		CompleteBossHazardDamageProof(false, FString::Printf(TEXT("Missing hazard definition %s"), *DamageProof.HazardID.ToString()));
		return true;
	}

	UWorld* World = GetWorld();
	UGameInstance* GI = World ? World->GetGameInstance() : nullptr;
	DamageProof.RunState = GI ? GI->GetSubsystem<UT66RunStateSubsystem>() : nullptr;

	APlayerController* PC = World ? World->GetFirstPlayerController() : nullptr;
	DamageProof.Hero = PC ? Cast<AT66HeroBase>(PC->GetPawn()) : nullptr;
	if (!World || !DamageProof.RunState.IsValid() || !DamageProof.Hero.IsValid())
	{
		CompleteBossHazardDamageProof(false, TEXT("Missing world, run state, or possessed AT66HeroBase"));
		return true;
	}

	DamageProof.InsideLocation = FVector(2400.f, 2400.f, 90.f);
	if (DamageProof.Hero.IsValid())
	{
		const FVector CurrentHeroLocation = DamageProof.Hero->GetActorLocation();
		DamageProof.InsideLocation.Z = CurrentHeroLocation.Z;
	}

	const float ExitDistance = DamageProof.Definition.ShapeType == T66BossHazardShape_Box
		? FMath::Max(DamageProof.Definition.BoxExtentX, DamageProof.Definition.BoxExtentY) + 600.f
		: FMath::Max(1.f, DamageProof.Definition.Radius) + 600.f;
	DamageProof.OutsideLocation = DamageProof.InsideLocation + FVector(ExitDistance, 0.f, 0.f);

	UE_LOG(
		LogT66BossHazardSubsystem,
		Display,
		TEXT("[BossHazardDamageProof] Started HazardID=%s Manifest=%s DamageHP=%d Cadence=%.3f Telegraph=%.3f Active=%.3f Linger=%.3f"),
		*DamageProof.HazardID.ToString(),
		*DamageProof.ManifestPath,
		DamageProof.Definition.DamageHP,
		DamageProof.Definition.DamageCadenceSeconds,
		DamageProof.Definition.TelegraphSeconds,
		DamageProof.Definition.ActiveSeconds,
		DamageProof.Definition.LingerSeconds);

	FApp::SetFixedDeltaTime(1.0 / 60.0);
	FApp::SetUseFixedTimeStep(true);
	BeginBossHazardDamageProofPhase(ET66BossHazardDamageProofPhase::InsideCadence);
	return true;
}

void UT66BossHazardSubsystem::BeginBossHazardDamageProofPhase(const ET66BossHazardDamageProofPhase Phase)
{
	UWorld* World = GetWorld();
	AT66HeroBase* Hero = DamageProof.Hero.Get();
	UT66RunStateSubsystem* RunState = DamageProof.RunState.Get();
	if (!World || !Hero || !RunState)
	{
		CompleteBossHazardDamageProof(false, TEXT("Missing proof world, hero, or run state at phase start"));
		return;
	}

	ActiveHazards.Reset();
	for (FT66BossHazardRenderBucket& Bucket : RenderBuckets)
	{
		if (Bucket.Component)
		{
			Bucket.Component->ClearInstances();
		}
	}
	ResetBossHazardDiagnostics(Phase == ET66BossHazardDamageProofPhase::InsideCadence
		? TEXT("BossHazardDamageProof.InsideCadence")
		: TEXT("BossHazardDamageProof.ExitStopsDamage"));

	DamageProof.Phase = Phase;
	FT66BossHazardDamageProofPhaseResult& Result = Phase == ET66BossHazardDamageProofPhase::InsideCadence
		? DamageProof.InsideCadence
		: DamageProof.ExitStopsDamage;
	Result = FT66BossHazardDamageProofPhaseResult();
	Result.PhaseName = Phase == ET66BossHazardDamageProofPhase::InsideCadence
		? TEXT("InsideCadence")
		: TEXT("ExitStopsDamage");
	Result.bStarted = true;
	Result.StartWorldTime = static_cast<float>(World->GetTimeSeconds());
	Result.ExpectedApplications = BuildBossHazardDamageProofExpectations(Result.ExpectedApplicationAges, Result.ExpectedDamageTicks);
	Result.ExpectedHPDrop = static_cast<float>(Result.ExpectedApplications * FMath::Max(0, DamageProof.Definition.DamageHP));

	Hero->SetActorLocation(DamageProof.InsideLocation, false, nullptr, ETeleportType::TeleportPhysics);
	if (Hero->IsInSafeZone())
	{
		Hero->AddSafeZoneOverlap(-1000);
	}
	Result.bSafeZoneAtSpawn = Hero->IsInSafeZone();

	RunState->ApplyAutomationHeroHPOverride(1000.f, TEXT("BossHazardDamageProof"));
	RunState->AutomationResetDamageInvuln();
	Result.StartHP = RunState->GetCurrentHP();
	Result.EvasionChance = RunState->GetEvasionChance01();
	Result.ArmorReduction = RunState->GetArmorReduction01();
	Result.bSaintBlessingActive = RunState->IsSaintBlessingActive();
	if (const AT66GameMode* GameMode = World->GetAuthGameMode<AT66GameMode>())
	{
		Result.bBackroomsChallengeActive = GameMode->IsBackroomsChallengeActive();
	}
	Result.bCleanHeroState =
		FMath::IsNearlyZero(Result.EvasionChance, 0.0001f)
		&& FMath::IsNearlyZero(Result.ArmorReduction, 0.0001f)
		&& !Result.bSaintBlessingActive
		&& !Result.bBackroomsChallengeActive;

	FT66BossHazardSpawnParams Params;
	Params.HazardID = DamageProof.HazardID;
	Params.SourceID = FName(TEXT("BossHazardDamageProof"));
	Params.Location = DamageProof.InsideLocation;
	Params.Rotation = FRotator::ZeroRotator;
	Result.bDamageOverrideApplied = Params.DamageOverrideHP != INDEX_NONE;
	Result.bSpawned = SpawnBossHazard(Params);

	if (!Result.bSpawned || ActiveHazards.Num() != 1)
	{
		Result.FailureReason = TEXT("Failed to spawn exactly one proof hazard");
		CompleteBossHazardDamageProof(false, Result.FailureReason);
		return;
	}

	Result.bInsideAtSpawn = IsHeroInsideHazard(Hero, ActiveHazards[0]);
	if (Result.bSafeZoneAtSpawn || !Result.bInsideAtSpawn)
	{
		Result.FailureReason = FString::Printf(
			TEXT("Hero setup invalid SafeZone=%d Inside=%d"),
			Result.bSafeZoneAtSpawn ? 1 : 0,
			Result.bInsideAtSpawn ? 1 : 0);
		CompleteBossHazardDamageProof(false, Result.FailureReason);
		return;
	}
}

void UT66BossHazardSubsystem::TickBossHazardDamageProof()
{
	if (!DamageProof.bRunning || DamageProof.bExitRequested)
	{
		return;
	}

	UWorld* World = GetWorld();
	AT66HeroBase* Hero = DamageProof.Hero.Get();
	UT66RunStateSubsystem* RunState = DamageProof.RunState.Get();
	if (!World || !Hero || !RunState)
	{
		CompleteBossHazardDamageProof(false, TEXT("Missing proof world, hero, or run state during tick"));
		return;
	}

	FT66BossHazardDamageProofPhaseResult* Result = nullptr;
	if (DamageProof.Phase == ET66BossHazardDamageProofPhase::InsideCadence)
	{
		Result = &DamageProof.InsideCadence;
	}
	else if (DamageProof.Phase == ET66BossHazardDamageProofPhase::ExitStopsDamage)
	{
		Result = &DamageProof.ExitStopsDamage;
	}
	else
	{
		return;
	}

	const int32 PreviousObservedApplications = Result->ObservedApplicationAges.Num();
	while (Result->ObservedApplicationAges.Num() < Diagnostics.DamageApplications)
	{
		const float HazardAge = ActiveHazards.Num() > 0 ? ActiveHazards[0].AgeSeconds : DamageProof.Definition.TelegraphSeconds + DamageProof.Definition.ActiveSeconds;
		Result->ObservedApplicationAges.Add(HazardAge);
		Result->ObservedApplicationWorldTimes.Add(static_cast<float>(World->GetTimeSeconds()));
		Result->ObservedHPAfterApplications.Add(RunState->GetCurrentHP());
	}

	if (DamageProof.Phase == ET66BossHazardDamageProofPhase::ExitStopsDamage
		&& Result->ObservedApplicationAges.Num() > PreviousObservedApplications
		&& Result->ApplicationsAtExit <= 0)
	{
		Result->ApplicationsAtExit = Diagnostics.DamageApplications;
		Result->DamageTicksAtExit = Diagnostics.DamageTicks;
		Result->HPAtExit = RunState->GetCurrentHP();
		Result->ExitAgeSeconds = ActiveHazards.Num() > 0 ? ActiveHazards[0].AgeSeconds : 0.f;
		Hero->SetActorLocation(DamageProof.OutsideLocation, false, nullptr, ETeleportType::TeleportPhysics);
		Result->ExitDistance2D = FVector::Dist2D(Hero->GetActorLocation(), DamageProof.InsideLocation);
		Result->bInsideAfterExit = ActiveHazards.Num() > 0 && IsHeroInsideHazard(Hero, ActiveHazards[0]);
	}

	const bool bPhaseHazardExpired = ActiveHazards.Num() == 0;
	if (!bPhaseHazardExpired)
	{
		return;
	}

	Result->bCompleted = true;
	Result->EndWorldTime = static_cast<float>(World->GetTimeSeconds());
	Result->EndHP = RunState->GetCurrentHP();
	Result->DamageTicks = Diagnostics.DamageTicks;
	Result->DamageApplications = Diagnostics.DamageApplications;

	if (DamageProof.Phase == ET66BossHazardDamageProofPhase::InsideCadence)
	{
		const float ExpectedEndHP = Result->StartHP - Result->ExpectedHPDrop;
		bool bApplicationAgesMatch = Result->ObservedApplicationAges.Num() == Result->ExpectedApplicationAges.Num();
		if (bApplicationAgesMatch)
		{
			for (int32 Index = 0; Index < Result->ExpectedApplicationAges.Num(); ++Index)
			{
				if (!FMath::IsNearlyEqual(Result->ObservedApplicationAges[Index], Result->ExpectedApplicationAges[Index], 0.12f))
				{
					bApplicationAgesMatch = false;
					break;
				}
			}
		}

		Result->bPass =
			Result->bSpawned
			&& Result->bInsideAtSpawn
			&& !Result->bSafeZoneAtSpawn
			&& Result->bCleanHeroState
			&& !Result->bDamageOverrideApplied
			&& Result->DamageTicks == Result->ExpectedDamageTicks
			&& Result->DamageApplications == Result->ExpectedApplications
			&& FMath::IsNearlyEqual(Result->EndHP, ExpectedEndHP, 0.1f)
			&& bApplicationAgesMatch;
		if (!Result->bPass)
		{
			Result->FailureReason = FString::Printf(
				TEXT("Inside cadence assertion failed Ticks=%d/%d Apps=%d/%d EndHP=%.1f ExpectedHP=%.1f AgesMatch=%d CleanHero=%d DamageOverride=%d"),
				Result->DamageTicks,
				Result->ExpectedDamageTicks,
				Result->DamageApplications,
				Result->ExpectedApplications,
				Result->EndHP,
				ExpectedEndHP,
				bApplicationAgesMatch ? 1 : 0,
				Result->bCleanHeroState ? 1 : 0,
				Result->bDamageOverrideApplied ? 1 : 0);
			CompleteBossHazardDamageProof(false, Result->FailureReason);
			return;
		}

		BeginBossHazardDamageProofPhase(ET66BossHazardDamageProofPhase::ExitStopsDamage);
		return;
	}

	Result->PostExitDamageTicks = FMath::Max(0, Result->DamageTicks - Result->DamageTicksAtExit);
	Result->PostExitDamageApplications = FMath::Max(0, Result->DamageApplications - Result->ApplicationsAtExit);
	const float ExpectedExitHP = Result->StartHP - static_cast<float>(FMath::Max(0, DamageProof.Definition.DamageHP));
	Result->ExpectedHPDrop = static_cast<float>(FMath::Max(0, DamageProof.Definition.DamageHP));
	Result->bPass =
		Result->bSpawned
		&& Result->bInsideAtSpawn
		&& !Result->bSafeZoneAtSpawn
		&& Result->bCleanHeroState
		&& !Result->bDamageOverrideApplied
		&& Result->ApplicationsAtExit == 1
		&& Result->DamageApplications == 1
		&& Result->PostExitDamageTicks > 0
		&& Result->PostExitDamageApplications == 0
		&& !Result->bInsideAfterExit
		&& FMath::IsNearlyEqual(Result->HPAtExit, ExpectedExitHP, 0.1f)
		&& FMath::IsNearlyEqual(Result->EndHP, Result->HPAtExit, 0.1f);
	if (!Result->bPass)
	{
		Result->FailureReason = FString::Printf(
			TEXT("Exit assertion failed AppsAtExit=%d AppsFinal=%d PostExitTicks=%d PostExitApps=%d InsideAfterExit=%d HPAtExit=%.1f EndHP=%.1f CleanHero=%d DamageOverride=%d"),
			Result->ApplicationsAtExit,
			Result->DamageApplications,
			Result->PostExitDamageTicks,
			Result->PostExitDamageApplications,
			Result->bInsideAfterExit ? 1 : 0,
			Result->HPAtExit,
			Result->EndHP,
			Result->bCleanHeroState ? 1 : 0,
			Result->bDamageOverrideApplied ? 1 : 0);
		CompleteBossHazardDamageProof(false, Result->FailureReason);
		return;
	}

	CompleteBossHazardDamageProof(DamageProof.InsideCadence.bPass && DamageProof.ExitStopsDamage.bPass, TEXT("BossHazardDamageProofComplete"));
}

void UT66BossHazardSubsystem::CompleteBossHazardDamageProof(const bool bPass, const FString& Reason)
{
	if (DamageProof.bExitRequested)
	{
		return;
	}

	DamageProof.bRunning = false;
	DamageProof.bExitRequested = true;
	DamageProof.Phase = ET66BossHazardDamageProofPhase::Complete;
	DamageProof.CompletionReason = Reason;
	WriteBossHazardDamageProofManifest(bPass, Reason);
	FApp::SetUseFixedTimeStep(false);

	UE_LOG(
		LogT66BossHazardSubsystem,
		Display,
		TEXT("[BossHazardDamageProofSummary] Terminal=1 HazardID=%s InsidePass=%d ExitPass=%d Pass=%d Reason=%s Manifest=%s"),
		*DamageProof.HazardID.ToString(),
		DamageProof.InsideCadence.bPass ? 1 : 0,
		DamageProof.ExitStopsDamage.bPass ? 1 : 0,
		bPass ? 1 : 0,
		*Reason,
		*DamageProof.ManifestPath);

	ActiveHazards.Reset();
	EmitBossHazardSummary(TEXT("BossHazardDamageProof"), true);
	FPlatformMisc::RequestExitWithStatus(false, bPass ? 0 : 1, TEXT("BossHazardDamageProofComplete"));
}

void UT66BossHazardSubsystem::WriteBossHazardDamageProofManifest(const bool bPass, const FString& Reason) const
{
	IFileManager::Get().MakeDirectory(*FPaths::GetPath(DamageProof.ManifestPath), true);

	FString Json;
	Json += TEXT("{\n");
	Json += FString::Printf(TEXT("  \"pass\": %s,\n"), T66BossHazardBoolString(bPass));
	Json += FString::Printf(TEXT("  \"reason\": \"%s\",\n"), *T66BossHazardJsonEscape(Reason));
	Json += FString::Printf(TEXT("  \"hazard_id\": \"%s\",\n"), *T66BossHazardJsonEscape(DamageProof.HazardID.ToString()));
	Json += FString::Printf(TEXT("  \"shape_type\": \"%s\",\n"), *T66BossHazardJsonEscape(DamageProof.Definition.ShapeType.ToString()));
	Json += FString::Printf(TEXT("  \"damage_hp_per_application\": %d,\n"), DamageProof.Definition.DamageHP);
	Json += FString::Printf(
		TEXT("  \"damage_override_applied\": %s,\n"),
		T66BossHazardBoolString(DamageProof.InsideCadence.bDamageOverrideApplied || DamageProof.ExitStopsDamage.bDamageOverrideApplied));
	Json += TEXT("  \"phase_hp_resets_are_independent\": true,\n");
	Json += FString::Printf(TEXT("  \"authored_damage_cadence_seconds\": %.3f,\n"), DamageProof.Definition.DamageCadenceSeconds);
	Json += FString::Printf(TEXT("  \"hero_invulnerability_seconds\": %.3f,\n"), UT66RunStateSubsystem::DefaultInvulnDurationSeconds);
	Json += FString::Printf(TEXT("  \"telegraph_seconds\": %.3f,\n"), DamageProof.Definition.TelegraphSeconds);
	Json += FString::Printf(TEXT("  \"active_seconds\": %.3f,\n"), DamageProof.Definition.ActiveSeconds);
	Json += FString::Printf(TEXT("  \"linger_seconds\": %.3f,\n"), DamageProof.Definition.LingerSeconds);
	Json += FString::Printf(TEXT("  \"radius\": %.3f,\n"), DamageProof.Definition.Radius);
	Json += TEXT("  \"phases\": {\n");
	Json += TEXT("    \"inside_cadence\": ");
	AppendBossHazardDamageProofPhaseJson(Json, DamageProof.InsideCadence);
	Json += TEXT(",\n");
	Json += TEXT("    \"exit_stops_damage\": ");
	AppendBossHazardDamageProofPhaseJson(Json, DamageProof.ExitStopsDamage);
	Json += TEXT("\n  }\n");
	Json += TEXT("}\n");

	if (!FFileHelper::SaveStringToFile(Json, *DamageProof.ManifestPath))
	{
		UE_LOG(LogT66BossHazardSubsystem, Warning, TEXT("[BossHazardDamageProof] Failed to write manifest: %s"), *DamageProof.ManifestPath);
	}
}

void UT66BossHazardSubsystem::AppendBossHazardDamageProofPhaseJson(FString& OutJson, const FT66BossHazardDamageProofPhaseResult& Phase) const
{
	OutJson += TEXT("{\n");
	OutJson += FString::Printf(TEXT("      \"phase_name\": \"%s\",\n"), *T66BossHazardJsonEscape(Phase.PhaseName));
	OutJson += FString::Printf(TEXT("      \"started\": %s,\n"), T66BossHazardBoolString(Phase.bStarted));
	OutJson += FString::Printf(TEXT("      \"spawned\": %s,\n"), T66BossHazardBoolString(Phase.bSpawned));
	OutJson += FString::Printf(TEXT("      \"completed\": %s,\n"), T66BossHazardBoolString(Phase.bCompleted));
	OutJson += FString::Printf(TEXT("      \"pass\": %s,\n"), T66BossHazardBoolString(Phase.bPass));
	OutJson += FString::Printf(TEXT("      \"safe_zone_at_spawn\": %s,\n"), T66BossHazardBoolString(Phase.bSafeZoneAtSpawn));
	OutJson += FString::Printf(TEXT("      \"inside_at_spawn\": %s,\n"), T66BossHazardBoolString(Phase.bInsideAtSpawn));
	OutJson += FString::Printf(TEXT("      \"inside_after_exit\": %s,\n"), T66BossHazardBoolString(Phase.bInsideAfterExit));
	OutJson += FString::Printf(TEXT("      \"clean_hero_state\": %s,\n"), T66BossHazardBoolString(Phase.bCleanHeroState));
	OutJson += FString::Printf(TEXT("      \"evasion_chance\": %.3f,\n"), Phase.EvasionChance);
	OutJson += FString::Printf(TEXT("      \"armor_reduction\": %.3f,\n"), Phase.ArmorReduction);
	OutJson += FString::Printf(TEXT("      \"saint_blessing_active\": %s,\n"), T66BossHazardBoolString(Phase.bSaintBlessingActive));
	OutJson += FString::Printf(TEXT("      \"backrooms_challenge_active\": %s,\n"), T66BossHazardBoolString(Phase.bBackroomsChallengeActive));
	OutJson += FString::Printf(TEXT("      \"damage_override_applied\": %s,\n"), T66BossHazardBoolString(Phase.bDamageOverrideApplied));
	OutJson += FString::Printf(TEXT("      \"start_hp\": %.3f,\n"), Phase.StartHP);
	OutJson += FString::Printf(TEXT("      \"end_hp\": %.3f,\n"), Phase.EndHP);
	OutJson += FString::Printf(TEXT("      \"hp_at_exit\": %.3f,\n"), Phase.HPAtExit);
	OutJson += FString::Printf(TEXT("      \"exit_age_seconds\": %.3f,\n"), Phase.ExitAgeSeconds);
	OutJson += FString::Printf(TEXT("      \"exit_distance_2d\": %.3f,\n"), Phase.ExitDistance2D);
	OutJson += FString::Printf(TEXT("      \"damage_ticks\": %d,\n"), Phase.DamageTicks);
	OutJson += FString::Printf(TEXT("      \"damage_applications\": %d,\n"), Phase.DamageApplications);
	OutJson += FString::Printf(TEXT("      \"applications_at_exit\": %d,\n"), Phase.ApplicationsAtExit);
	OutJson += FString::Printf(TEXT("      \"post_exit_damage_ticks\": %d,\n"), Phase.PostExitDamageTicks);
	OutJson += FString::Printf(TEXT("      \"post_exit_damage_applications\": %d,\n"), Phase.PostExitDamageApplications);
	OutJson += FString::Printf(TEXT("      \"expected_damage_ticks\": %d,\n"), Phase.ExpectedDamageTicks);
	OutJson += FString::Printf(TEXT("      \"expected_applications\": %d,\n"), Phase.ExpectedApplications);
	OutJson += FString::Printf(TEXT("      \"expected_hp_drop\": %.3f,\n"), Phase.ExpectedHPDrop);
	OutJson += FString::Printf(TEXT("      \"expected_application_ages\": %s,\n"), *T66BossHazardFloatArrayJson(Phase.ExpectedApplicationAges));
	OutJson += FString::Printf(TEXT("      \"observed_application_ages\": %s,\n"), *T66BossHazardFloatArrayJson(Phase.ObservedApplicationAges));
	OutJson += FString::Printf(TEXT("      \"observed_application_world_times\": %s,\n"), *T66BossHazardFloatArrayJson(Phase.ObservedApplicationWorldTimes));
	OutJson += FString::Printf(TEXT("      \"observed_hp_after_applications\": %s,\n"), *T66BossHazardFloatArrayJson(Phase.ObservedHPAfterApplications));
	OutJson += FString::Printf(TEXT("      \"failure_reason\": \"%s\"\n"), *T66BossHazardJsonEscape(Phase.FailureReason));
	OutJson += TEXT("    }");
}

int32 UT66BossHazardSubsystem::BuildBossHazardDamageProofExpectations(TArray<float>& OutExpectedApplicationAges, int32& OutExpectedDamageTicks) const
{
	OutExpectedApplicationAges.Reset();
	OutExpectedDamageTicks = 0;

	const float ActiveStart = FMath::Max(0.f, DamageProof.Definition.TelegraphSeconds);
	const float ActiveEnd = ActiveStart + FMath::Max(0.f, DamageProof.Definition.ActiveSeconds);
	const float Cadence = FMath::Max(0.05f, DamageProof.Definition.DamageCadenceSeconds);
	float LastApplicationAge = -100000.f;
	for (float AttemptAge = ActiveStart; AttemptAge <= ActiveEnd + KINDA_SMALL_NUMBER; AttemptAge += Cadence)
	{
		++OutExpectedDamageTicks;
		if (AttemptAge - LastApplicationAge >= UT66RunStateSubsystem::DefaultInvulnDurationSeconds - KINDA_SMALL_NUMBER)
		{
			OutExpectedApplicationAges.Add(AttemptAge);
			LastApplicationAge = AttemptAge;
		}
	}

	return OutExpectedApplicationAges.Num();
}
#endif
