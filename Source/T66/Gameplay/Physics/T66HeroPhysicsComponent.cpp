#include "T66/Gameplay/Physics/T66HeroPhysicsComponent.h"

#include "Animation/AnimationAsset.h"
#include "Components/CapsuleComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Core/T66RunStateSubsystem.h"
#include "Engine/GameInstance.h"
#include "Engine/World.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Gameplay/T66CombatComponent.h"
#include "PhysicsEngine/BodyInstance.h"
#include "PhysicsEngine/PhysicsAsset.h"
#include "PhysicsEngine/SkeletalBodySetup.h"
#include "Gameplay/T66HeroBase.h"

DEFINE_LOG_CATEGORY_STATIC(LogT66HeroPhysics, Log, All);

static TAutoConsoleVariable<int32> CVarT66HeroPhysicsEnableActiveRagdoll(
	TEXT("t66.HeroPhysics.EnableActiveRagdoll"),
	1,
	TEXT("Enables Hero 1 Chad hit-triggered ragdoll reactions."),
	ECVF_Default);

static TAutoConsoleVariable<int32> CVarT66HeroPhysicsDebugLog(
	TEXT("t66.HeroPhysics.DebugLog"),
	0,
	TEXT("Logs Hero 1 Chad ragdoll runtime samples."),
	ECVF_Default);

static TAutoConsoleVariable<float> CVarT66HeroPhysicsDebugLogInterval(
	TEXT("t66.HeroPhysics.DebugLogInterval"),
	0.10f,
	TEXT("Minimum seconds between Hero 1 Chad ragdoll debug samples."),
	ECVF_Default);

static TAutoConsoleVariable<float> CVarT66HeroPhysicsJumpRecoverySecondsPerPress(
	TEXT("t66.HeroPhysics.JumpRecoverySecondsPerPress"),
	0.20f,
	TEXT("Seconds of ragdoll recovery credit granted for each accepted jump press while Hero 1 is ragdolled."),
	ECVF_Default);

static TAutoConsoleVariable<float> CVarT66HeroPhysicsJumpRecoveryMinInterval(
	TEXT("t66.HeroPhysics.JumpRecoveryMinInterval"),
	0.06f,
	TEXT("Minimum seconds between accepted ragdoll jump-recovery presses."),
	ECVF_Default);

static TAutoConsoleVariable<float> CVarT66HeroPhysicsJumpRecoveryMinRagdollSeconds(
	TEXT("t66.HeroPhysics.JumpRecoveryMinRagdollSeconds"),
	0.45f,
	TEXT("Minimum ragdoll seconds before jump-mash recovery can force get-up."),
	ECVF_Default);

static TAutoConsoleVariable<float> CVarT66HeroPhysicsRagdollDurationScaleAtZeroPercent(
	TEXT("t66.HeroPhysics.RagdollDurationScaleAtZeroPercent"),
	0.33333334f,
	TEXT("Multiplier applied to Hero 1 ragdoll/get-up duration at 0 percent damage. With the default 3s profile this is a 1s baseline."),
	ECVF_Default);

static TAutoConsoleVariable<float> CVarT66HeroPhysicsRagdollDurationFullPercent(
	TEXT("t66.HeroPhysics.RagdollDurationFullPercent"),
	80.0f,
	TEXT("Damage percent where Hero 1 ragdoll/get-up duration reaches the unscaled profile duration."),
	ECVF_Default);

namespace
{
	constexpr TCHAR GetUpFrontPath[] = TEXT("/Game/Characters/Heroes/Hero_1/Chad/FriendSlopRaw/PhysicsFirst/AM_Hero_1_Chad_PhysicsFirst_GetUp_Front.AM_Hero_1_Chad_PhysicsFirst_GetUp_Front");
	constexpr TCHAR GetUpBackPath[] = TEXT("/Game/Characters/Heroes/Hero_1/Chad/FriendSlopRaw/PhysicsFirst/AM_Hero_1_Chad_PhysicsFirst_GetUp_Back.AM_Hero_1_Chad_PhysicsFirst_GetUp_Back");

	const TCHAR* StateName(ET66HeroPhysicsRuntimeState State)
	{
		switch (State)
		{
		case ET66HeroPhysicsRuntimeState::Normal:
			return TEXT("Normal");
		case ET66HeroPhysicsRuntimeState::Ragdoll:
			return TEXT("Ragdoll");
		case ET66HeroPhysicsRuntimeState::GettingUp:
			return TEXT("GettingUp");
		default:
			return TEXT("Unknown");
		}
	}

	bool IsValidBodyInstance(const FBodyInstance* BodyInstance)
	{
		return BodyInstance && BodyInstance->IsValidBodyInstance();
	}

	int32 CountSimulatingBodies(const USkeletalMeshComponent* Mesh)
	{
		if (!Mesh)
		{
			return 0;
		}

		int32 Count = 0;
		for (const FBodyInstance* BodyInstance : Mesh->Bodies)
		{
			if (IsValidBodyInstance(BodyInstance) && BodyInstance->IsInstanceSimulatingPhysics())
			{
				++Count;
			}
		}
		return Count;
	}

	FString DescribePhysicsBodies(const UPhysicsAsset* PhysicsAsset)
	{
		if (!PhysicsAsset)
		{
			return TEXT("None");
		}

		TArray<FString> BodyNames;
		BodyNames.Reserve(PhysicsAsset->SkeletalBodySetups.Num());
		for (const USkeletalBodySetup* BodySetup : PhysicsAsset->SkeletalBodySetups)
		{
			if (BodySetup)
			{
				BodyNames.Add(BodySetup->BoneName.ToString());
			}
		}
		return FString::Join(BodyNames, TEXT(","));
	}

	FName FirstExistingPhysicsBody(const USkeletalMeshComponent* Mesh, const TArrayView<const FName> Candidates)
	{
		const UPhysicsAsset* PhysicsAsset = Mesh ? Mesh->GetPhysicsAsset() : nullptr;
		if (!Mesh || !PhysicsAsset)
		{
			return NAME_None;
		}

		for (const FName Candidate : Candidates)
		{
			if (!Candidate.IsNone()
				&& Mesh->GetBoneIndex(Candidate) != INDEX_NONE
				&& PhysicsAsset->FindBodyIndex(Candidate) != INDEX_NONE)
			{
				return Candidate;
			}
		}

		if (PhysicsAsset->SkeletalBodySetups.Num() > 0 && PhysicsAsset->SkeletalBodySetups[0])
		{
			return PhysicsAsset->SkeletalBodySetups[0]->BoneName;
		}

		return NAME_None;
	}
}

UT66HeroPhysicsComponent::UT66HeroPhysicsComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
	PrimaryComponentTick.bStartWithTickEnabled = false;
}

void UT66HeroPhysicsComponent::BeginPlay()
{
	Super::BeginPlay();
	SetComponentTickEnabled(false);
}

void UT66HeroPhysicsComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	ShutdownActiveRagdoll();
	Super::EndPlay(EndPlayReason);
}

void UT66HeroPhysicsComponent::InitializeForHero(AT66HeroBase* InHero, USkeletalMeshComponent* InMesh)
{
	CachedHero = InHero;
	ActiveMesh = InMesh ? InMesh : (InHero ? InHero->GetMesh() : nullptr);

	if (bInitialized)
	{
		ShutdownActiveRagdoll();
		CachedHero = InHero;
		ActiveMesh = InMesh ? InMesh : (InHero ? InHero->GetMesh() : nullptr);
	}

	TryInitializeActiveRagdoll();
}

void UT66HeroPhysicsComponent::ShutdownActiveRagdoll()
{
	if (bGameplaySuppressed)
	{
		ApplyGameplaySuppression(false);
	}

	if (ActiveMesh && CachedHero)
	{
		ConfigureNormalMesh();
	}

	RuntimeState = ET66HeroPhysicsRuntimeState::Normal;
	bInitialized = false;
	RagdollElapsedSeconds = 0.0f;
	RagdollSettleHeldSeconds = 0.0f;
	RagdollJumpRecoveryCreditSeconds = 0.0f;
	RagdollJumpRecoveryAcceptedPresses = 0;
	GettingUpElapsedSeconds = 0.0f;
	LastAppliedImpulseMagnitude = 0.0f;
	LastJumpRecoveryInputTimeSeconds = -1000.0f;
	RagdollDurationDamagePercent = -1.0f;
	RagdollDurationScaleStartPercent = 0.0f;
	RagdollDurationScaleFullPercent = -1.0f;
	SetComponentTickEnabled(false);
}

bool UT66HeroPhysicsComponent::TryInitializeActiveRagdoll()
{
	if (CVarT66HeroPhysicsEnableActiveRagdoll.GetValueOnGameThread() == 0)
	{
		UE_LOG(LogT66HeroPhysics, Log, TEXT("Init Skipped Reason=CVarDisabled"));
		SetComponentTickEnabled(false);
		return false;
	}

	AT66HeroBase* Hero = CachedHero.Get();
	USkeletalMeshComponent* Mesh = ActiveMesh.Get();
	if (!Hero || !Mesh)
	{
		UE_LOG(LogT66HeroPhysics, Warning, TEXT("Init Failed Reason=MissingHeroOrMesh Hero=%s Mesh=%s"),
			Hero ? *Hero->GetName() : TEXT("None"),
			Mesh ? *Mesh->GetName() : TEXT("None"));
		SetComponentTickEnabled(false);
		return false;
	}

	if (!Profile.bEnabled)
	{
		UE_LOG(LogT66HeroPhysics, Log, TEXT("Init Skipped Hero=%s Reason=ProfileDisabled"), *Hero->GetName());
		SetComponentTickEnabled(false);
		return false;
	}

	if (Profile.bHero1ChadOnly && !T66BodyTypeAliases::IsChad(Hero->BodyType))
	{
		UE_LOG(LogT66HeroPhysics, Log, TEXT("Init Skipped Hero=%s BodyType=%s Reason=Hero1ChadOnly"),
			*Hero->GetName(),
			*StaticEnum<ET66BodyType>()->GetNameStringByValue(static_cast<int64>(Hero->BodyType)));
		SetComponentTickEnabled(false);
		return false;
	}

	if (!Mesh->GetPhysicsAsset())
	{
		UE_LOG(LogT66HeroPhysics, Warning, TEXT("Init Failed Hero=%s Reason=MissingPhysicsAsset Mesh=%s"),
			*Hero->GetName(),
			*GetNameSafe(Mesh->GetSkeletalMeshAsset()));
		SetComponentTickEnabled(false);
		return false;
	}

	if (!ResolveRequiredBodies())
	{
		SetComponentTickEnabled(false);
		return false;
	}

	DefaultMeshRelativeTransform = Mesh->GetRelativeTransform();
	bHasDefaultMeshRelativeTransform = true;
	ConfigureNormalMesh();

	bInitialized = true;
	RuntimeState = ET66HeroPhysicsRuntimeState::Normal;
	StateStartWorldTimeSeconds = GetWorld() ? GetWorld()->GetTimeSeconds() : 0.0f;
	LastDebugLogTimeSeconds = -1000.0f;
	SetComponentTickEnabled(false);

	UE_LOG(LogT66HeroPhysics, Log,
		TEXT("Init OK Hero=%s Mesh=%s PhysicsAsset=%s Model=HitTriggeredFullRagdoll NormalMeshSim=0 MeshCollision=NoCollision CapsuleAuthority=1 PelvisBody=%s"),
		*Hero->GetName(),
		*Mesh->GetName(),
		*GetNameSafe(Mesh->GetPhysicsAsset()),
		*ResolvedPelvisBodyName.ToString());

	return true;
}

bool UT66HeroPhysicsComponent::ResolveRequiredBodies()
{
	USkeletalMeshComponent* Mesh = ActiveMesh.Get();
	if (!Mesh)
	{
		return false;
	}

	static const FName CommonCenterBodies[] =
	{
		FName(TEXT("pelvis")),
		FName(TEXT("hips")),
		FName(TEXT("Hips")),
		FName(TEXT("mixamorig:Hips")),
		FName(TEXT("spine_01")),
		FName(TEXT("spine_02")),
		FName(TEXT("spine_03")),
		FName(TEXT("DEF-spine")),
		FName(TEXT("DEF-spine.001")),
		FName(TEXT("DEF-spine.002")),
		FName(TEXT("ORG-spine")),
		FName(TEXT("root"))
	};

	TArray<FName> PelvisSearch;
	PelvisSearch.Reserve(UE_ARRAY_COUNT(CommonCenterBodies) + 1);
	PelvisSearch.Add(Profile.PelvisBodyName);
	for (const FName Candidate : CommonCenterBodies)
	{
		PelvisSearch.Add(Candidate);
	}

	TArray<FName> SimulationRootSearch;
	SimulationRootSearch.Reserve(UE_ARRAY_COUNT(CommonCenterBodies) + 1);
	SimulationRootSearch.Add(Profile.SimulationRootBodyName);
	for (const FName Candidate : CommonCenterBodies)
	{
		SimulationRootSearch.Add(Candidate);
	}

	ResolvedPelvisBodyName = FirstExistingPhysicsBody(Mesh, MakeArrayView(PelvisSearch));
	ResolvedSimulationRootBodyName = FirstExistingPhysicsBody(Mesh, MakeArrayView(SimulationRootSearch));

	const UPhysicsAsset* PhysicsAsset = Mesh->GetPhysicsAsset();
	if (!PhysicsAsset || ResolvedPelvisBodyName.IsNone() || PhysicsAsset->FindBodyIndex(ResolvedPelvisBodyName) == INDEX_NONE)
	{
		UE_LOG(LogT66HeroPhysics, Warning, TEXT("Init Failed Hero=%s Reason=MissingPelvisBody Requested=%s Resolved=%s PhysicsAsset=%s AvailableBodies=%s"),
			*GetNameSafe(CachedHero.Get()),
			*Profile.PelvisBodyName.ToString(),
			*ResolvedPelvisBodyName.ToString(),
			*GetNameSafe(PhysicsAsset),
			*DescribePhysicsBodies(PhysicsAsset));
		return false;
	}

	if (ResolvedSimulationRootBodyName.IsNone() || PhysicsAsset->FindBodyIndex(ResolvedSimulationRootBodyName) == INDEX_NONE)
	{
		UE_LOG(LogT66HeroPhysics, Warning, TEXT("Init Failed Hero=%s Reason=MissingSimulationRootBody Requested=%s Resolved=%s PhysicsAsset=%s AvailableBodies=%s"),
			*GetNameSafe(CachedHero.Get()),
			*Profile.SimulationRootBodyName.ToString(),
			*ResolvedSimulationRootBodyName.ToString(),
			*GetNameSafe(PhysicsAsset),
			*DescribePhysicsBodies(PhysicsAsset));
		return false;
	}

	UE_LOG(LogT66HeroPhysics, Log, TEXT("Bodies Resolved Hero=%s RequestedPelvis=%s ResolvedPelvis=%s RequestedSimRoot=%s ResolvedSimRoot=%s AvailableBodies=%s"),
		*GetNameSafe(CachedHero.Get()),
		*Profile.PelvisBodyName.ToString(),
		*ResolvedPelvisBodyName.ToString(),
		*Profile.SimulationRootBodyName.ToString(),
		*ResolvedSimulationRootBodyName.ToString(),
		*DescribePhysicsBodies(PhysicsAsset));

	return true;
}

void UT66HeroPhysicsComponent::ConfigureNormalMesh()
{
	AT66HeroBase* Hero = CachedHero.Get();
	USkeletalMeshComponent* Mesh = ActiveMesh.Get();
	if (!Hero || !Mesh)
	{
		return;
	}

	UCapsuleComponent* Capsule = Hero->GetCapsuleComponent();
	if (Capsule)
	{
		Capsule->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
		Capsule->SetCollisionProfileName(TEXT("Pawn"));
	}

	Mesh->SetAllBodiesPhysicsBlendWeight(0.0f, false);
	Mesh->SetAllBodiesSimulatePhysics(false);
	Mesh->SetSimulatePhysics(false);
	Mesh->SetAllUseCCD(false);
	Mesh->bBlendPhysics = false;
	Mesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	Mesh->SetCollisionObjectType(ECC_PhysicsBody);
	Mesh->SetCollisionResponseToAllChannels(ECR_Ignore);
	Mesh->KinematicBonesUpdateType = EKinematicBonesUpdateToPhysics::SkipSimulatingBones;
	Mesh->SetPlayRate(1.0f);

	if (Capsule && Mesh->GetAttachParent() != Capsule)
	{
		Mesh->AttachToComponent(Capsule, FAttachmentTransformRules::KeepWorldTransform);
	}

	if (bHasDefaultMeshRelativeTransform)
	{
		Mesh->SetRelativeTransform(DefaultMeshRelativeTransform);
	}
}

void UT66HeroPhysicsComponent::ConfigureRagdollMesh()
{
	USkeletalMeshComponent* Mesh = ActiveMesh.Get();
	if (!Mesh)
	{
		return;
	}

	Mesh->PhysicsTransformUpdateMode = EPhysicsTransformUpdateMode::SimulationUpatesComponentTransform;
	Mesh->KinematicBonesUpdateType = EKinematicBonesUpdateToPhysics::SkipSimulatingBones;
	Mesh->SetCollisionObjectType(ECC_PhysicsBody);
	Mesh->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	Mesh->SetCollisionResponseToAllChannels(ECR_Ignore);
	Mesh->SetCollisionResponseToChannel(ECC_WorldStatic, ECR_Block);
	Mesh->SetCollisionResponseToChannel(ECC_WorldDynamic, ECR_Block);
	Mesh->SetCollisionResponseToChannel(ECC_PhysicsBody, ECR_Block);
	Mesh->SetCollisionResponseToChannel(ECC_Pawn, ECR_Ignore);
	Mesh->SetCollisionResponseToChannel(ECC_Camera, ECR_Ignore);
	Mesh->SetEnableGravity(true);
	Mesh->bBlendPhysics = true;
	Mesh->SetAllUseCCD(true);
	Mesh->SetSimulatePhysics(true);
	Mesh->SetAllBodiesSimulatePhysics(true);
	Mesh->SetAllBodiesPhysicsBlendWeight(1.0f, false);
	Mesh->WakeAllRigidBodies();
}

bool UT66HeroPhysicsComponent::ApplyPhysicsReaction(
	const FVector& RequestedVelocityChange,
	const FVector& WorldHitLocation,
	FName SourceTag,
	const float LaunchScale,
	const float DurationDamagePercent,
	const float DurationScaleStartPercent,
	const float DurationScaleFullPercent)
{
	if (!bInitialized && !TryInitializeActiveRagdoll())
	{
		return false;
	}

	const UWorld* World = GetWorld();
	const float Now = World ? World->GetTimeSeconds() : 0.0f;
	if (RuntimeState == ET66HeroPhysicsRuntimeState::Ragdoll)
	{
		const FVector LaunchVelocity = BuildLaunchVelocity(RequestedVelocityChange, LaunchScale);
		RagdollElapsedSeconds = 0.0f;
		RagdollSettleHeldSeconds = 0.0f;
		RagdollJumpRecoveryCreditSeconds = 0.0f;
		RagdollJumpRecoveryAcceptedPresses = 0;
		RagdollDurationDamagePercent = DurationDamagePercent >= 0.0f ? DurationDamagePercent : GetCurrentHeroDamagePercent();
		RagdollDurationScaleStartPercent = FMath::Max(0.0f, DurationScaleStartPercent);
		RagdollDurationScaleFullPercent = DurationScaleFullPercent;
		LastJumpRecoveryInputTimeSeconds = -1000.0f;
		ApplyLaunchToBodies(LaunchVelocity, WorldHitLocation);
		LastReactionTimeSeconds = Now;
		UE_LOG(LogT66HeroPhysics, Log, TEXT("Reaction Applied=1 Hero=%s State=%s Source=%s Reason=AlreadyRagdollActive LaunchScale=%.2f RequestedVelocityChange=%s AppliedLaunch=%s ImpulseMagnitude=%.2f RecoveryReset=1"),
			*GetNameSafe(CachedHero.Get()),
			StateName(RuntimeState),
			*SourceTag.ToString(),
			FMath::Max(0.1f, LaunchScale),
			*RequestedVelocityChange.ToCompactString(),
			*LaunchVelocity.ToCompactString(),
			LastAppliedImpulseMagnitude);
		return true;
	}
	if (RuntimeState == ET66HeroPhysicsRuntimeState::GettingUp)
	{
		UE_LOG(LogT66HeroPhysics, Log, TEXT("Reaction Applied=1 Hero=%s State=%s Source=%s Reason=RestartRagdollFromGetUp LaunchScale=%.2f"),
			*GetNameSafe(CachedHero.Get()),
			StateName(RuntimeState),
			*SourceTag.ToString(),
			FMath::Max(0.1f, LaunchScale));
		return EnterRagdoll(RequestedVelocityChange, WorldHitLocation, SourceTag, LaunchScale, DurationDamagePercent, DurationScaleStartPercent, DurationScaleFullPercent);
	}

	if ((Now - LastReactionTimeSeconds) < Profile.ReactionCooldownSeconds)
	{
		UE_LOG(LogT66HeroPhysics, Log, TEXT("Reaction Ignored Hero=%s State=%s Source=%s Reason=Cooldown Remaining=%.3f"),
			*GetNameSafe(CachedHero.Get()),
			StateName(RuntimeState),
			*SourceTag.ToString(),
			Profile.ReactionCooldownSeconds - (Now - LastReactionTimeSeconds));
		return true;
	}

	return EnterRagdoll(RequestedVelocityChange, WorldHitLocation, SourceTag, LaunchScale, DurationDamagePercent, DurationScaleStartPercent, DurationScaleFullPercent);
}

bool UT66HeroPhysicsComponent::EnterRagdoll(
	const FVector& RequestedVelocityChange,
	const FVector& WorldHitLocation,
	FName SourceTag,
	const float LaunchScale,
	const float DurationDamagePercent,
	const float DurationScaleStartPercent,
	const float DurationScaleFullPercent)
{
	AT66HeroBase* Hero = CachedHero.Get();
	USkeletalMeshComponent* Mesh = ActiveMesh.Get();
	if (!Hero || !Mesh)
	{
		return false;
	}

	const FVector LaunchVelocity = BuildLaunchVelocity(RequestedVelocityChange, LaunchScale);

	LastReactionTimeSeconds = GetWorld() ? GetWorld()->GetTimeSeconds() : 0.0f;
	LastAppliedImpulseMagnitude = 0.0f;
	RagdollElapsedSeconds = 0.0f;
	RagdollSettleHeldSeconds = 0.0f;
	RagdollJumpRecoveryCreditSeconds = 0.0f;
	RagdollJumpRecoveryAcceptedPresses = 0;
	GettingUpElapsedSeconds = 0.0f;
	GetUpDurationSeconds = 0.6f;
	CurrentGetUpAnimation = nullptr;
	LastJumpRecoveryInputTimeSeconds = -1000.0f;
	RagdollDurationDamagePercent = DurationDamagePercent >= 0.0f ? DurationDamagePercent : GetCurrentHeroDamagePercent();
	RagdollDurationScaleStartPercent = FMath::Max(0.0f, DurationScaleStartPercent);
	RagdollDurationScaleFullPercent = DurationScaleFullPercent;

	ApplyGameplaySuppression(true);

	if (UCharacterMovementComponent* Movement = Hero->GetCharacterMovement())
	{
		Movement->StopMovementImmediately();
		Movement->DisableMovement();
		Movement->SetMovementMode(MOVE_None);
	}

	if (UCapsuleComponent* Capsule = Hero->GetCapsuleComponent())
	{
		Capsule->SetCollisionProfileName(TEXT("Pawn"));
		Capsule->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
		Capsule->SetGenerateOverlapEvents(true);
	}

	Mesh->DetachFromComponent(FDetachmentTransformRules::KeepWorldTransform);
	ConfigureRagdollMesh();
	ApplyLaunchToBodies(LaunchVelocity, WorldHitLocation);

	SetRuntimeState(ET66HeroPhysicsRuntimeState::Ragdoll, SourceTag);
	SetComponentTickEnabled(true);

	FVector PelvisLocation = FVector::ZeroVector;
	FVector PelvisVelocity = FVector::ZeroVector;
	ResolvePelvisWorld(PelvisLocation, &PelvisVelocity, nullptr);

	UE_LOG(LogT66HeroPhysics, Log,
		TEXT("Reaction Applied=1 Hero=%s Source=%s State=Ragdoll LaunchScale=%.2f DurationDamagePercent=%.1f DurationScaleStart=%.1f DurationScaleFull=%.1f UncreditedMax=%.3f RequestedVelocityChange=%s AppliedLaunch=%s Hit=%s PelvisWorld=%s SimBodies=%d ImpulseMagnitude=%.2f"),
		*Hero->GetName(),
		*SourceTag.ToString(),
		FMath::Max(0.1f, LaunchScale),
		RagdollDurationDamagePercent,
		RagdollDurationScaleStartPercent,
		RagdollDurationScaleFullPercent,
		ComputeHealthScaledRagdollMaxSeconds(),
		*RequestedVelocityChange.ToCompactString(),
		*LaunchVelocity.ToCompactString(),
		*WorldHitLocation.ToCompactString(),
		*PelvisLocation.ToCompactString(),
		CountSimulatingBodies(Mesh),
		LastAppliedImpulseMagnitude);

	return true;
}

float UT66HeroPhysicsComponent::GetCurrentHeroDamagePercent() const
{
	const AT66HeroBase* Hero = CachedHero.Get();
	const UWorld* World = Hero ? Hero->GetWorld() : GetWorld();
	const UGameInstance* GameInstance = World ? World->GetGameInstance() : nullptr;
	const UT66RunStateSubsystem* RunState = GameInstance ? GameInstance->GetSubsystem<UT66RunStateSubsystem>() : nullptr;
	return RunState ? FMath::Max(0.0f, RunState->GetHeroDamagePercent()) : 0.0f;
}

float UT66HeroPhysicsComponent::ComputeDamagePercentDurationScale() const
{
	const float ScaleAtZeroPercent = FMath::Clamp(
		CVarT66HeroPhysicsRagdollDurationScaleAtZeroPercent.GetValueOnGameThread(),
		0.01f,
		1.0f);
	const float FullDurationPercent = FMath::Max(
		1.0f,
		CVarT66HeroPhysicsRagdollDurationFullPercent.GetValueOnGameThread());
	const float ScaleStartPercent = FMath::Clamp(RagdollDurationScaleStartPercent, 0.0f, 99.0f);
	const float ScaleFullPercent = RagdollDurationScaleFullPercent > ScaleStartPercent
		? FMath::Clamp(RagdollDurationScaleFullPercent, ScaleStartPercent + 1.0f, 100.0f)
		: FullDurationPercent;
	const float DurationDamagePercent = RagdollDurationDamagePercent >= 0.0f ? RagdollDurationDamagePercent : GetCurrentHeroDamagePercent();
	const float Alpha = FMath::Clamp(
		(DurationDamagePercent - ScaleStartPercent) / FMath::Max(1.0f, ScaleFullPercent - ScaleStartPercent),
		0.0f,
		1.0f);
	return FMath::Lerp(ScaleAtZeroPercent, 1.0f, Alpha);
}

float UT66HeroPhysicsComponent::ComputeHealthScaledRagdollMaxSeconds() const
{
	return FMath::Max(0.0f, Profile.RagdollMaxSeconds) * ComputeDamagePercentDurationScale();
}

float UT66HeroPhysicsComponent::ComputeEffectiveRagdollMaxSeconds() const
{
	const float HealthScaledMaxSeconds = ComputeHealthScaledRagdollMaxSeconds();
	const float MinRagdollSeconds = FMath::Clamp(
		CVarT66HeroPhysicsJumpRecoveryMinRagdollSeconds.GetValueOnGameThread(),
		0.0f,
		HealthScaledMaxSeconds);
	return FMath::Max(MinRagdollSeconds, HealthScaledMaxSeconds - RagdollJumpRecoveryCreditSeconds);
}

float UT66HeroPhysicsComponent::ComputeEffectiveRagdollSettleHoldSeconds() const
{
	return FMath::Max(0.01f, Profile.RagdollSettleHoldSeconds - RagdollJumpRecoveryCreditSeconds);
}

bool UT66HeroPhysicsComponent::GetRagdollRecoveryUIState(FT66HeroRagdollRecoveryUIState& OutState) const
{
	OutState = FT66HeroRagdollRecoveryUIState();
	if (RuntimeState != ET66HeroPhysicsRuntimeState::Ragdoll)
	{
		return false;
	}

	const float UncreditedMaxSeconds = ComputeHealthScaledRagdollMaxSeconds();
	const float EffectiveMaxSeconds = ComputeEffectiveRagdollMaxSeconds();
	const float EffectiveSettleHoldSeconds = ComputeEffectiveRagdollSettleHoldSeconds();
	const float TimeProgress01 = UncreditedMaxSeconds > KINDA_SMALL_NUMBER
		? (RagdollElapsedSeconds + RagdollJumpRecoveryCreditSeconds) / UncreditedMaxSeconds
		: 1.0f;
	const float SettleProgress01 = EffectiveSettleHoldSeconds > KINDA_SMALL_NUMBER
		? RagdollSettleHeldSeconds / EffectiveSettleHoldSeconds
		: 0.0f;

	OutState.bVisible = true;
	OutState.Progress01 = FMath::Clamp(FMath::Max(TimeProgress01, SettleProgress01), 0.0f, 1.0f);
	OutState.ElapsedSeconds = RagdollElapsedSeconds;
	OutState.RemainingSeconds = FMath::Max(0.0f, EffectiveMaxSeconds - RagdollElapsedSeconds);
	OutState.EffectiveMaxSeconds = EffectiveMaxSeconds;
	OutState.UncreditedMaxSeconds = UncreditedMaxSeconds;
	OutState.CreditSeconds = RagdollJumpRecoveryCreditSeconds;
	OutState.AcceptedJumpPresses = RagdollJumpRecoveryAcceptedPresses;
	return true;
}

bool UT66HeroPhysicsComponent::NotifyJumpRecoveryInput()
{
	if (RuntimeState != ET66HeroPhysicsRuntimeState::Ragdoll)
	{
		return false;
	}

	const UWorld* World = GetWorld();
	const float Now = World ? World->GetTimeSeconds() : 0.0f;
	const float MinIntervalSeconds = FMath::Max(0.0f, CVarT66HeroPhysicsJumpRecoveryMinInterval.GetValueOnGameThread());
	if ((Now - LastJumpRecoveryInputTimeSeconds) < MinIntervalSeconds)
	{
		UE_LOG(LogT66HeroPhysics, Log, TEXT("JumpRecoveryInput Applied=0 Hero=%s State=%s Reason=Debounced Remaining=%.3f Credit=%.3f"),
			*GetNameSafe(CachedHero.Get()),
			StateName(RuntimeState),
			MinIntervalSeconds - (Now - LastJumpRecoveryInputTimeSeconds),
			RagdollJumpRecoveryCreditSeconds);
		return true;
	}

	LastJumpRecoveryInputTimeSeconds = Now;

	const float CreditPerPress = FMath::Max(0.0f, CVarT66HeroPhysicsJumpRecoverySecondsPerPress.GetValueOnGameThread());
	const float UncreditedMaxSeconds = ComputeHealthScaledRagdollMaxSeconds();
	const float MinRagdollSeconds = FMath::Clamp(
		CVarT66HeroPhysicsJumpRecoveryMinRagdollSeconds.GetValueOnGameThread(),
		0.0f,
		UncreditedMaxSeconds);
	const float MaxCredit = FMath::Max(0.0f, UncreditedMaxSeconds - MinRagdollSeconds);
	const float PreviousCredit = RagdollJumpRecoveryCreditSeconds;
	RagdollJumpRecoveryCreditSeconds = FMath::Clamp(RagdollJumpRecoveryCreditSeconds + CreditPerPress, 0.0f, MaxCredit);
	if (RagdollJumpRecoveryCreditSeconds > PreviousCredit + KINDA_SMALL_NUMBER)
	{
		++RagdollJumpRecoveryAcceptedPresses;
	}

	const float EffectiveMaxSeconds = ComputeEffectiveRagdollMaxSeconds();
	const float EffectiveSettleHoldSeconds = ComputeEffectiveRagdollSettleHoldSeconds();
	UE_LOG(LogT66HeroPhysics, Log, TEXT("JumpRecoveryInput Applied=1 Hero=%s State=%s DurationDamagePercent=%.1f CurrentDamagePercent=%.1f UncreditedMax=%.3f CreditBefore=%.3f CreditAfter=%.3f CreditPerPress=%.3f RagdollElapsed=%.3f EffectiveMax=%.3f EffectiveSettleHold=%.3f AcceptedPresses=%d"),
		*GetNameSafe(CachedHero.Get()),
		StateName(RuntimeState),
		RagdollDurationDamagePercent,
		GetCurrentHeroDamagePercent(),
		UncreditedMaxSeconds,
		PreviousCredit,
		RagdollJumpRecoveryCreditSeconds,
		CreditPerPress,
		RagdollElapsedSeconds,
		EffectiveMaxSeconds,
		EffectiveSettleHoldSeconds,
		RagdollJumpRecoveryAcceptedPresses);

	return true;
}

FVector UT66HeroPhysicsComponent::BuildLaunchVelocity(const FVector& RequestedVelocityChange, const float LaunchScale) const
{
	AT66HeroBase* Hero = CachedHero.Get();
	FVector Horizontal = RequestedVelocityChange;
	Horizontal.Z = 0.0f;

	FVector HorizontalDirection = Horizontal.GetSafeNormal();
	if (HorizontalDirection.IsNearlyZero())
	{
		HorizontalDirection = Hero ? Hero->GetActorForwardVector().GetSafeNormal2D() : FVector::ForwardVector;
		if (HorizontalDirection.IsNearlyZero())
		{
			HorizontalDirection = FVector::ForwardVector;
		}
	}

	const float ClampedLaunchScale = FMath::Max(0.1f, LaunchScale);
	const float EffectiveLaunchSpeedMax = FMath::Max(1.f, Profile.RagdollLaunchSpeedMax) * ClampedLaunchScale;
	const float EffectiveLaunchUpSpeed = FMath::Max(1.f, Profile.RagdollLaunchUpSpeed) * ClampedLaunchScale;
	const float HorizontalSpeed = FMath::Min(Horizontal.Size(), EffectiveLaunchSpeedMax);
	const float RequestedUpSpeed = FMath::Max(0.0f, RequestedVelocityChange.Z);
	const float UpSpeed = FMath::Clamp(
		FMath::Max(RequestedUpSpeed, EffectiveLaunchUpSpeed),
		0.0f,
		EffectiveLaunchUpSpeed * 3.0f);
	return HorizontalDirection * HorizontalSpeed + FVector::UpVector * UpSpeed;
}

void UT66HeroPhysicsComponent::ApplyLaunchToBodies(const FVector& LaunchVelocity, const FVector& WorldHitLocation)
{
	USkeletalMeshComponent* Mesh = ActiveMesh.Get();
	if (!Mesh)
	{
		return;
	}

	for (FBodyInstance* BodyInstance : Mesh->Bodies)
	{
		if (IsValidBodyInstance(BodyInstance) && BodyInstance->IsInstanceSimulatingPhysics())
		{
			BodyInstance->SetLinearVelocity(LaunchVelocity, false, true);
			BodyInstance->WakeInstance();
		}
	}

	FBodyInstance* PelvisBody = Mesh->GetBodyInstance(ResolvedPelvisBodyName);
	if (IsValidBodyInstance(PelvisBody) && PelvisBody->IsInstanceSimulatingPhysics())
	{
		const float PelvisMass = FMath::Max(1.0f, PelvisBody->GetBodyMass());
		const FVector Impulse = LaunchVelocity * PelvisMass;
		FVector ImpulseLocation = WorldHitLocation;
		if (ImpulseLocation.ContainsNaN() || ImpulseLocation.IsNearlyZero())
		{
			ImpulseLocation = PelvisBody->GetUnrealWorldTransform().GetLocation() + FVector(30.0f, 0.0f, 40.0f);
		}

		PelvisBody->AddImpulseAtPosition(Impulse, ImpulseLocation);
		LastAppliedImpulseMagnitude = Impulse.Size();
	}
}

void UT66HeroPhysicsComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	if (!bInitialized)
	{
		SetComponentTickEnabled(false);
		return;
	}

	switch (RuntimeState)
	{
	case ET66HeroPhysicsRuntimeState::Ragdoll:
		UpdateRagdoll(DeltaTime);
		break;
	case ET66HeroPhysicsRuntimeState::GettingUp:
		UpdateGettingUp(DeltaTime);
		break;
	case ET66HeroPhysicsRuntimeState::Normal:
	default:
		SetComponentTickEnabled(false);
		return;
	}

	EmitRuntimeSample(DeltaTime);
}

void UT66HeroPhysicsComponent::UpdateRagdoll(float DeltaTime)
{
	FVector PelvisLocation = FVector::ZeroVector;
	FVector PelvisVelocity = FVector::ZeroVector;
	FTransform PelvisTransform = FTransform::Identity;
	if (!ResolvePelvisWorld(PelvisLocation, &PelvisVelocity, &PelvisTransform))
	{
		EnterGettingUp(TEXT("MissingPelvisDuringRagdoll"));
		return;
	}

	RagdollElapsedSeconds += DeltaTime;
	MoveActorXYToPelvis(PelvisLocation);

	const float PelvisSpeed = PelvisVelocity.Size();
	if (PelvisSpeed < Profile.RagdollSettleSpeed)
	{
		RagdollSettleHeldSeconds += DeltaTime;
	}
	else
	{
		RagdollSettleHeldSeconds = 0.0f;
	}

	const float EffectiveSettleHoldSeconds = ComputeEffectiveRagdollSettleHoldSeconds();
	const float EffectiveMaxSeconds = ComputeEffectiveRagdollMaxSeconds();
	if (RagdollSettleHeldSeconds >= EffectiveSettleHoldSeconds)
	{
		EnterGettingUp(TEXT("Settled"));
		return;
	}

	if (RagdollElapsedSeconds >= EffectiveMaxSeconds)
	{
		EnterGettingUp(RagdollJumpRecoveryCreditSeconds > 0.0f ? TEXT("JumpRecovery") : TEXT("MaxTime"));
	}
}

void UT66HeroPhysicsComponent::EnterGettingUp(FName Reason)
{
	AT66HeroBase* Hero = CachedHero.Get();
	USkeletalMeshComponent* Mesh = ActiveMesh.Get();
	if (!Hero || !Mesh)
	{
		return;
	}

	FVector PelvisLocation = Mesh->GetComponentLocation();
	FVector PelvisVelocity = FVector::ZeroVector;
	FTransform PelvisTransform = Mesh->GetComponentTransform();
	ResolvePelvisWorld(PelvisLocation, &PelvisVelocity, &PelvisTransform);

	const bool bUseFaceDownGetUp = IsFaceDown(PelvisTransform);
	PlaceCapsuleForGetUp(PelvisLocation);

	Mesh->SetAllBodiesPhysicsBlendWeight(1.0f, false);
	Mesh->SetAllBodiesSimulatePhysics(false);
	Mesh->SetSimulatePhysics(false);
	Mesh->SetAllUseCCD(false);
	Mesh->bBlendPhysics = true;
	Mesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	if (UCapsuleComponent* Capsule = Hero->GetCapsuleComponent())
	{
		Mesh->AttachToComponent(Capsule, FAttachmentTransformRules::KeepWorldTransform);
	}

	if (bHasDefaultMeshRelativeTransform)
	{
		Mesh->SetRelativeTransform(DefaultMeshRelativeTransform);
	}

	CurrentGetUpAnimation = ResolveGetUpAnimation(bUseFaceDownGetUp);
	const float RawGetUpDurationSeconds = CurrentGetUpAnimation ? FMath::Max(CurrentGetUpAnimation->GetPlayLength(), Profile.RagdollBlendOutSeconds) : 0.6f;
	GetUpDurationSeconds = FMath::Max(Profile.RagdollBlendOutSeconds, RawGetUpDurationSeconds * ComputeDamagePercentDurationScale());
	GettingUpElapsedSeconds = 0.0f;

	if (CurrentGetUpAnimation)
	{
		Mesh->PlayAnimation(CurrentGetUpAnimation, false);
		if (GetUpDurationSeconds > KINDA_SMALL_NUMBER)
		{
			Mesh->SetPlayRate(RawGetUpDurationSeconds / GetUpDurationSeconds);
		}
	}

	SetRuntimeState(ET66HeroPhysicsRuntimeState::GettingUp, Reason);

	UE_LOG(LogT66HeroPhysics, Log,
		TEXT("GetUp Enter Hero=%s Reason=%s FaceDown=%d Animation=%s DurationDamagePercent=%.1f CurrentDamagePercent=%.1f RawDuration=%.3f Duration=%.3f PelvisWorld=%s Capsule=%s"),
		*Hero->GetName(),
		*Reason.ToString(),
		bUseFaceDownGetUp ? 1 : 0,
		*GetNameSafe(CurrentGetUpAnimation.Get()),
		RagdollDurationDamagePercent,
		GetCurrentHeroDamagePercent(),
		RawGetUpDurationSeconds,
		GetUpDurationSeconds,
		*PelvisLocation.ToCompactString(),
		*Hero->GetActorLocation().ToCompactString());
}

void UT66HeroPhysicsComponent::UpdateGettingUp(float DeltaTime)
{
	GettingUpElapsedSeconds += DeltaTime;

	const float BlendOutSeconds = FMath::Max(0.01f, Profile.RagdollBlendOutSeconds);
	const float BlendAlpha = FMath::Clamp(GettingUpElapsedSeconds / BlendOutSeconds, 0.0f, 1.0f);
	const float BlendWeight = 1.0f - BlendAlpha;
	if (ActiveMesh)
	{
		ActiveMesh->SetAllBodiesPhysicsBlendWeight(BlendWeight, false);
	}

	if (GettingUpElapsedSeconds >= GetUpDurationSeconds)
	{
		FinishGetUp();
	}
}

void UT66HeroPhysicsComponent::FinishGetUp()
{
	AT66HeroBase* Hero = CachedHero.Get();
	if (!Hero)
	{
		return;
	}

	ConfigureNormalMesh();

	if (UCharacterMovementComponent* Movement = Hero->GetCharacterMovement())
	{
		Movement->SetMovementMode(MOVE_Walking);
	}

	ApplyGameplaySuppression(false);

	CurrentGetUpAnimation = nullptr;
	RagdollElapsedSeconds = 0.0f;
	RagdollSettleHeldSeconds = 0.0f;
	RagdollJumpRecoveryCreditSeconds = 0.0f;
	RagdollJumpRecoveryAcceptedPresses = 0;
	GettingUpElapsedSeconds = 0.0f;
	LastAppliedImpulseMagnitude = 0.0f;
	LastJumpRecoveryInputTimeSeconds = -1000.0f;
	RagdollDurationDamagePercent = -1.0f;
	RagdollDurationScaleStartPercent = 0.0f;
	RagdollDurationScaleFullPercent = -1.0f;

	SetRuntimeState(ET66HeroPhysicsRuntimeState::Normal, TEXT("GetUpComplete"));
	SetComponentTickEnabled(false);

	UE_LOG(LogT66HeroPhysics, Log, TEXT("GetUp Complete Hero=%s State=Normal Capsule=%s"),
		*Hero->GetName(),
		*Hero->GetActorLocation().ToCompactString());
}

void UT66HeroPhysicsComponent::ApplyGameplaySuppression(bool bSuppress)
{
	AT66HeroBase* Hero = CachedHero.Get();
	if (!Hero)
	{
		bGameplaySuppressed = bSuppress;
		return;
	}

	const bool bWasGameplaySuppressed = bGameplaySuppressed;
	bGameplaySuppressed = bSuppress;
	if (Hero->CombatComponent)
	{
		if (bSuppress)
		{
			if (!bWasGameplaySuppressed)
			{
				bPreImpactAutoAttackSuppressed = Hero->CombatComponent->IsAutoAttackSuppressed();
			}
			Hero->CombatComponent->SetAutoAttackSuppressed(true);
		}
		else if (bWasGameplaySuppressed)
		{
			Hero->CombatComponent->SetAutoAttackSuppressed(bPreImpactAutoAttackSuppressed);
			bPreImpactAutoAttackSuppressed = false;
		}
	}

	if (AController* Controller = Hero->GetController())
	{
		if (bSuppress)
		{
			if (!bAppliedMoveInputSuppression)
			{
				Controller->SetIgnoreMoveInput(true);
				bAppliedMoveInputSuppression = true;
			}
		}
		else
		{
			if (bAppliedMoveInputSuppression)
			{
				Controller->SetIgnoreMoveInput(false);
				bAppliedMoveInputSuppression = false;
			}
		}
	}

	UE_LOG(LogT66HeroPhysics, Log, TEXT("Suppression Hero=%s Suppressed=%d"),
		*Hero->GetName(),
		bSuppress ? 1 : 0);
}

void UT66HeroPhysicsComponent::SetRuntimeState(ET66HeroPhysicsRuntimeState NewState, FName Reason)
{
	if (RuntimeState == NewState)
	{
		return;
	}

	const ET66HeroPhysicsRuntimeState OldState = RuntimeState;
	RuntimeState = NewState;
	StateStartWorldTimeSeconds = GetWorld() ? GetWorld()->GetTimeSeconds() : 0.0f;
	LastDebugLogTimeSeconds = -1000.0f;

	UE_LOG(LogT66HeroPhysics, Log, TEXT("StateChange Hero=%s From=%s To=%s Reason=%s"),
		*GetNameSafe(CachedHero.Get()),
		StateName(OldState),
		StateName(NewState),
		*Reason.ToString());
}

bool UT66HeroPhysicsComponent::ResolvePelvisWorld(FVector& OutLocation, FVector* OutVelocity, FTransform* OutTransform) const
{
	const USkeletalMeshComponent* Mesh = ActiveMesh.Get();
	if (!Mesh)
	{
		return false;
	}

	const FBodyInstance* PelvisBody = Mesh->GetBodyInstance(ResolvedPelvisBodyName);
	if (IsValidBodyInstance(PelvisBody))
	{
		const FTransform PelvisTransform = PelvisBody->GetUnrealWorldTransform();
		OutLocation = PelvisTransform.GetLocation();
		if (OutVelocity)
		{
			*OutVelocity = PelvisBody->GetUnrealWorldVelocity();
		}
		if (OutTransform)
		{
			*OutTransform = PelvisTransform;
		}
		return true;
	}

	OutLocation = Mesh->GetSocketLocation(ResolvedPelvisBodyName);
	if (OutVelocity)
	{
		*OutVelocity = FVector::ZeroVector;
	}
	if (OutTransform)
	{
		*OutTransform = Mesh->GetSocketTransform(ResolvedPelvisBodyName, RTS_World);
	}
	return true;
}

float UT66HeroPhysicsComponent::ComputePelvisCapsuleDistance(const FVector& PelvisWorldLocation) const
{
	const AT66HeroBase* Hero = CachedHero.Get();
	const UCapsuleComponent* Capsule = Hero ? Hero->GetCapsuleComponent() : nullptr;
	return Capsule ? FVector::Dist(PelvisWorldLocation, Capsule->GetComponentLocation()) : 0.0f;
}

void UT66HeroPhysicsComponent::MoveActorXYToPelvis(const FVector& PelvisWorldLocation)
{
	AT66HeroBase* Hero = CachedHero.Get();
	if (!Hero)
	{
		return;
	}

	FVector NewActorLocation = Hero->GetActorLocation();
	NewActorLocation.X = PelvisWorldLocation.X;
	NewActorLocation.Y = PelvisWorldLocation.Y;
	Hero->SetActorLocation(NewActorLocation, false, nullptr, ETeleportType::TeleportPhysics);
}

void UT66HeroPhysicsComponent::PlaceCapsuleForGetUp(const FVector& PelvisWorldLocation)
{
	AT66HeroBase* Hero = CachedHero.Get();
	if (!Hero)
	{
		return;
	}

	UCapsuleComponent* Capsule = Hero->GetCapsuleComponent();
	const float HalfHeight = Capsule ? Capsule->GetScaledCapsuleHalfHeight() : 88.0f;
	FVector FloorLocation = PelvisWorldLocation;
	FString FloorSource = TEXT("FallbackPelvis");
	bool bFoundFloor = false;

	if (UWorld* World = GetWorld())
	{
		const float TraceStartLift = FMath::Clamp(HalfHeight * 0.5f, 30.0f, 80.0f);
		const FVector TraceStart = PelvisWorldLocation + FVector::UpVector * TraceStartLift;
		const FVector TraceEnd = PelvisWorldLocation - FVector::UpVector * Profile.FloorTraceDownDistance;
		FCollisionQueryParams Params(SCENE_QUERY_STAT(T66HeroPhysicsGetUpFloor), false);
		Params.AddIgnoredActor(Hero);

		FHitResult Hit;
		if (World->LineTraceSingleByChannel(Hit, TraceStart, TraceEnd, ECC_WorldStatic, Params))
		{
			FloorLocation = Hit.ImpactPoint;
			FloorSource = GetNameSafe(Hit.GetComponent());
			bFoundFloor = true;
		}
	}

	const FVector TargetLocation(PelvisWorldLocation.X, PelvisWorldLocation.Y, FloorLocation.Z + HalfHeight);
	const FRotator TargetRotation(0.0f, Hero->GetActorRotation().Yaw, 0.0f);
	Hero->SetActorLocationAndRotation(TargetLocation, TargetRotation, false, nullptr, ETeleportType::TeleportPhysics);

	if (Capsule)
	{
		Capsule->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
		Capsule->SetCollisionProfileName(TEXT("Pawn"));
	}

	UE_LOG(LogT66HeroPhysics, Log, TEXT("GetUp CapsulePlaced Hero=%s FoundFloor=%d PelvisWorld=%s Floor=%s Target=%s FloorSource=%s"),
		*Hero->GetName(),
		bFoundFloor ? 1 : 0,
		*PelvisWorldLocation.ToCompactString(),
		*FloorLocation.ToCompactString(),
		*TargetLocation.ToCompactString(),
		*FloorSource);
}

bool UT66HeroPhysicsComponent::IsFaceDown(const FTransform& PelvisWorldTransform) const
{
	const FVector BodyUp = PelvisWorldTransform.GetUnitAxis(EAxis::Z);
	return FVector::DotProduct(BodyUp, FVector::UpVector) < 0.0f;
}

UAnimationAsset* UT66HeroPhysicsComponent::ResolveGetUpAnimation(bool bFaceDown) const
{
	const TCHAR* AssetPath = bFaceDown ? GetUpFrontPath : GetUpBackPath;
	return LoadObject<UAnimationAsset>(nullptr, AssetPath);
}

void UT66HeroPhysicsComponent::EmitRuntimeSample(float DeltaTime)
{
	if (CVarT66HeroPhysicsDebugLog.GetValueOnGameThread() == 0)
	{
		return;
	}

	const UWorld* World = GetWorld();
	const float Now = World ? World->GetTimeSeconds() : 0.0f;
	const float Interval = FMath::Max(0.01f, CVarT66HeroPhysicsDebugLogInterval.GetValueOnGameThread());
	if ((Now - LastDebugLogTimeSeconds) < Interval)
	{
		return;
	}
	LastDebugLogTimeSeconds = Now;

	FVector PelvisLocation = FVector::ZeroVector;
	FVector PelvisVelocity = FVector::ZeroVector;
	ResolvePelvisWorld(PelvisLocation, &PelvisVelocity, nullptr);

	const AT66HeroBase* Hero = CachedHero.Get();
	const UCapsuleComponent* Capsule = Hero ? Hero->GetCapsuleComponent() : nullptr;
	const USkeletalMeshComponent* Mesh = ActiveMesh.Get();
	const UCharacterMovementComponent* Movement = Hero ? Hero->GetCharacterMovement() : nullptr;
	const float PelvisCapsuleDistance = ComputePelvisCapsuleDistance(PelvisLocation);

	UE_LOG(LogT66HeroPhysics, Log,
		TEXT("DebugSample Time=%.3f Delta=%.3f State=%s MovementMode=%s CapsuleWorld=%s CapsuleVelocity=%s PelvisWorld=%s PelvisWorldLinearVelocity=%s PelvisWorldSpeed=%.2f MeshWorld=%s PelvisCapsuleWorldDistance=%.2f LastImpulseMagnitude=%.2f JumpRecoveryCredit=%.3f SimBodies=%d"),
		Now,
		DeltaTime,
		StateName(RuntimeState),
		Movement ? *Movement->GetMovementName() : TEXT("None"),
		Capsule ? *Capsule->GetComponentLocation().ToCompactString() : TEXT("None"),
		Movement ? *Movement->Velocity.ToCompactString() : TEXT("None"),
		*PelvisLocation.ToCompactString(),
		*PelvisVelocity.ToCompactString(),
		PelvisVelocity.Size(),
		Mesh ? *Mesh->GetComponentLocation().ToCompactString() : TEXT("None"),
		PelvisCapsuleDistance,
		LastAppliedImpulseMagnitude,
		RagdollJumpRecoveryCreditSeconds,
		CountSimulatingBodies(Mesh));
}
