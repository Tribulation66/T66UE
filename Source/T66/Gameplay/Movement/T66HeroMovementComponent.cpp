// Copyright Tribulation 66. All Rights Reserved.

#include "Gameplay/Movement/T66HeroMovementComponent.h"

#include "Gameplay/T66HeroBase.h"
#include "Core/T66AudioSubsystem.h"
#include "Core/T66HeroSpeedSubsystem.h"
#include "Core/T66RunStateSubsystem.h"
#include "CollisionQueryParams.h"
#include "Components/CapsuleComponent.h"
#include "Components/PrimitiveComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/Controller.h"
#include "Engine/World.h"
#include "HAL/IConsoleManager.h"
#include "Misc/CommandLine.h"
#include "Misc/Parse.h"

namespace
{
	constexpr float T66HeroWalkSpeedUnitsPerSpeedPoint = 300.f;
	constexpr float T66HeroMinimumWalkSpeed = 100.f;
	static const FName T66HeroMovementNoSurfaceBounceTag(TEXT("T66_NoSurfaceBounce"));
	static const FName T66TowerCeilingTag(TEXT("T66_Tower_Ceiling"));
	static const FName T66TowerDescentHoleTag(TEXT("T66_Tower_DescentHole"));

	static TAutoConsoleVariable<int32> CVarT66HeroSurfaceBounceEnabled(
		TEXT("t66.HeroMovement.SurfaceBounceEnabled"),
		1,
		TEXT("Enables movement-owned bouncy ground and wall impulses for the hero capsule. This does not trigger damage or ragdoll."),
		ECVF_Default);

	static TAutoConsoleVariable<int32> CVarT66HeroSurfaceBounceDebugLog(
		TEXT("t66.HeroMovement.SurfaceBounceDebugLog"),
		0,
		TEXT("Logs hero surface-bounce launches for automation proof and tuning."),
		ECVF_Default);

	static TAutoConsoleVariable<float> CVarT66HeroSurfaceBounceGroundRestitution(
		TEXT("t66.HeroMovement.SurfaceBounceGroundRestitution"),
		0.55f,
		TEXT("Fraction of landing impact speed returned as vertical launch for bouncy ground."),
		ECVF_Default);

	static TAutoConsoleVariable<float> CVarT66HeroSurfaceBounceGroundMinImpactSpeed(
		TEXT("t66.HeroMovement.SurfaceBounceGroundMinImpactSpeed"),
		520.0f,
		TEXT("Minimum downward landing speed required before bouncy ground launches the hero."),
		ECVF_Default);

	static TAutoConsoleVariable<float> CVarT66HeroSurfaceBounceGroundMinFallHeight(
		TEXT("t66.HeroMovement.SurfaceBounceGroundMinFallHeight"),
		70.0f,
		TEXT("Minimum airborne peak-to-landing height required before bouncy ground launches the hero."),
		ECVF_Default);

	static TAutoConsoleVariable<float> CVarT66HeroSurfaceBounceGroundMinLaunchZ(
		TEXT("t66.HeroMovement.SurfaceBounceGroundMinLaunchZ"),
		260.0f,
		TEXT("Minimum vertical launch velocity for an accepted bouncy-ground landing."),
		ECVF_Default);

	static TAutoConsoleVariable<float> CVarT66HeroSurfaceBounceGroundMaxLaunchZ(
		TEXT("t66.HeroMovement.SurfaceBounceGroundMaxLaunchZ"),
		1650.0f,
		TEXT("Maximum vertical launch velocity for bouncy-ground landing restitution."),
		ECVF_Default);

	static TAutoConsoleVariable<float> CVarT66HeroSurfaceBounceGroundCooldown(
		TEXT("t66.HeroMovement.SurfaceBounceGroundCooldown"),
		0.12f,
		TEXT("Minimum seconds between accepted ground landing bounces."),
		ECVF_Default);

	static TAutoConsoleVariable<float> CVarT66HeroSurfaceBounceWallHorizontal(
		TEXT("t66.HeroMovement.SurfaceBounceWallHorizontal"),
		2200.0f,
		TEXT("Minimum horizontal launch speed applied away from bouncy walls."),
		ECVF_Default);

	static TAutoConsoleVariable<float> CVarT66HeroSurfaceBounceWallZ(
		TEXT("t66.HeroMovement.SurfaceBounceWallZ"),
		420.0f,
		TEXT("Vertical launch velocity applied with wall bounces."),
		ECVF_Default);

	static TAutoConsoleVariable<float> CVarT66HeroSurfaceBounceWallCooldown(
		TEXT("t66.HeroMovement.SurfaceBounceWallCooldown"),
		0.24f,
		TEXT("Minimum seconds between wall bounces."),
		ECVF_Default);

	static TAutoConsoleVariable<float> CVarT66HeroSurfaceBounceWallTraceDistance(
		TEXT("t66.HeroMovement.SurfaceBounceWallTraceDistance"),
		50.0f,
		TEXT("Forward sweep distance used to detect bouncy wall contact. Exact-collision contract: ")
		TEXT("the sweep sphere (capsule radius * 0.85) must reach barely past the 34uu capsule skin — ")
		TEXT("the old 220 default bounced the hero ~2 body widths before visual contact."),
		ECVF_Default);

	static TAutoConsoleVariable<float> CVarT66HeroSurfaceBounceMinSpeed(
		TEXT("t66.HeroMovement.SurfaceBounceMinSpeed"),
		160.0f,
		TEXT("Minimum 2D movement speed that counts as moving for automatic surface bounces."),
		ECVF_Default);

	float T66ResolveWalkSpeedFromSpeedStat(const int32 SpeedStat)
	{
		return FMath::Max(1, SpeedStat) * T66HeroWalkSpeedUnitsPerSpeedPoint;
	}

	bool T66IsHeroMovementQACapture()
	{
#if !UE_BUILD_SHIPPING
		FString CaptureMode;
		return FParse::Value(FCommandLine::Get(), TEXT("T66GameplayAutoCapture="), CaptureMode)
			&& (CaptureMode.Equals(TEXT("heromovementqa"), ESearchCase::IgnoreCase)
				|| CaptureMode.Equals(TEXT("herolocomotionqa"), ESearchCase::IgnoreCase)
				|| CaptureMode.Equals(TEXT("herosurfacebounceqa"), ESearchCase::IgnoreCase));
#else
		return false;
#endif
	}

	bool T66HitHasTag(const FHitResult& Hit, const FName Tag)
	{
		const AActor* HitActor = Hit.GetActor();
		const UPrimitiveComponent* HitComponent = Hit.GetComponent();
		return (HitActor && HitActor->ActorHasTag(Tag))
			|| (HitComponent && HitComponent->ComponentHasTag(Tag));
	}

	bool T66ShouldIgnoreSurfaceBounceWallHit(const FHitResult& Hit)
	{
		// Query-only volumes (interactable TriggerBoxes, NPC safe-zone/interaction bubbles)
		// are WorldDynamic and show up in the object-type sweep, but they cannot physically
		// stop the hero — bouncing off them reads as bouncing off air far outside the mesh.
		// Only surfaces that actually block pawn movement may bounce.
		const UPrimitiveComponent* HitComponent = Hit.GetComponent();
		const ECollisionEnabled::Type CollisionEnabled = HitComponent ? HitComponent->GetCollisionEnabled() : ECollisionEnabled::NoCollision;
		const bool bPhysicallyBlocksPawn =
			(CollisionEnabled == ECollisionEnabled::QueryAndPhysics || CollisionEnabled == ECollisionEnabled::PhysicsOnly)
			&& HitComponent->GetCollisionResponseToChannel(ECC_Pawn) == ECR_Block;
		if (!bPhysicallyBlocksPawn)
		{
			return true;
		}
		return T66HitHasTag(Hit, T66HeroMovementNoSurfaceBounceTag)
			|| T66HitHasTag(Hit, T66TowerCeilingTag)
			|| T66HitHasTag(Hit, T66TowerDescentHoleTag);
	}
}

UT66HeroMovementComponent::UT66HeroMovementComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
	PrimaryComponentTick.bStartWithTickEnabled = true;
}

void UT66HeroMovementComponent::BeginPlay()
{
	Super::BeginPlay();

	CachedHero = Cast<AT66HeroBase>(GetOwner());
	if (UWorld* World = GetWorld())
	{
		if (UGameInstance* GI = World->GetGameInstance())
		{
			CachedRunState = GI->GetSubsystem<UT66RunStateSubsystem>();
			CachedHeroSpeedSubsystem = GI->GetSubsystem<UT66HeroSpeedSubsystem>();
		}
	}

	ApplyDefaultMovementConfig();
	RefreshWalkSpeedFromRunState();
	UpdateAnimationStateBridge();
}

void UT66HeroMovementComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	UpdateSurfaceBounce(DeltaTime);
}

AT66HeroBase* UT66HeroMovementComponent::ResolveHero() const
{
	return CachedHero ? CachedHero.Get() : Cast<AT66HeroBase>(GetOwner());
}

UCharacterMovementComponent* UT66HeroMovementComponent::ResolveCharacterMovement() const
{
	if (AT66HeroBase* Hero = ResolveHero())
	{
		return Hero->GetCharacterMovement();
	}
	return nullptr;
}

float UT66HeroMovementComponent::ResolveCurrentMaxWalkSpeed() const
{
	if (const UCharacterMovementComponent* Movement = ResolveCharacterMovement())
	{
		return FMath::Max(T66HeroMinimumWalkSpeed, Movement->MaxWalkSpeed);
	}

	return FMath::Max(T66HeroMinimumWalkSpeed, BaseWalkSpeed);
}

void UT66HeroMovementComponent::ApplyDefaultMovementConfig()
{
	if (AT66HeroBase* Hero = ResolveHero())
	{
		Hero->JumpMaxCount = 1;
		Hero->JumpMaxHoldTime = MovementTuning.JumpMaxHoldTime;
	}

	if (UCharacterMovementComponent* Movement = ResolveCharacterMovement())
	{
		Movement->MaxWalkSpeed = BaseWalkSpeed;
		Movement->MaxAcceleration = MovementTuning.MaxAcceleration;
		Movement->BrakingDecelerationWalking = MovementTuning.BrakingDecelerationWalking;
		Movement->GroundFriction = MovementTuning.GroundFriction;
		Movement->bUseSeparateBrakingFriction = MovementTuning.bUseSeparateBrakingFriction;
		Movement->BrakingFriction = MovementTuning.BrakingFriction;
		Movement->BrakingFrictionFactor = MovementTuning.BrakingFrictionFactor;
		Movement->JumpZVelocity = MovementTuning.JumpZVelocity;
		Movement->AirControl = MovementTuning.AirControl;
		Movement->GravityScale = MovementTuning.GravityScale;
		Movement->FallingLateralFriction = MovementTuning.FallingLateralFriction;
		Movement->BrakingDecelerationFalling = MovementTuning.BrakingDecelerationFalling;
		Movement->bOrientRotationToMovement = true;
		Movement->RotationRate = FRotator(0.f, MovementTuning.RotationRateYaw, 0.f);
	}
}

void UT66HeroMovementComponent::RefreshWalkSpeedFromRunState()
{
	UCharacterMovementComponent* Movement = ResolveCharacterMovement();
	if (!Movement)
	{
		return;
	}

	float ResolvedBaseWalkSpeed = BaseWalkSpeed;
	float Multiplier = 1.f;
	if (!CachedRunState)
	{
		if (UWorld* World = GetWorld())
		{
			if (UGameInstance* GI = World->GetGameInstance())
			{
				CachedRunState = GI->GetSubsystem<UT66RunStateSubsystem>();
			}
		}
	}

	if (CachedRunState)
	{
		// The foundational Speed stat owns base locomotion speed. MaxSpeed is reserved for future cap semantics.
		ResolvedBaseWalkSpeed = T66ResolveWalkSpeedFromSpeedStat(CachedRunState->GetSpeedStat());
		Multiplier =
			CachedRunState->GetItemMoveSpeedMultiplier() *
			CachedRunState->GetStageMoveSpeedMultiplier() *
			CachedRunState->GetStatusMoveSpeedMultiplier();
	}

	Movement->MaxWalkSpeed = FMath::Clamp(ResolvedBaseWalkSpeed * Multiplier, T66HeroMinimumWalkSpeed, 10000.f);

	if (!CachedHeroSpeedSubsystem)
	{
		if (UWorld* World = GetWorld())
		{
			if (UGameInstance* GI = World->GetGameInstance())
			{
				CachedHeroSpeedSubsystem = GI->GetSubsystem<UT66HeroSpeedSubsystem>();
			}
		}
	}
	if (CachedHeroSpeedSubsystem)
	{
		CachedHeroSpeedSubsystem->SetParams(Movement->MaxWalkSpeed);
	}
}

void UT66HeroMovementComponent::SetHeroBaseWalkSpeed(const float InBaseWalkSpeed)
{
	BaseWalkSpeed = FMath::Max(T66HeroMinimumWalkSpeed, InBaseWalkSpeed);

	RefreshWalkSpeedFromRunState();
}

void UT66HeroMovementComponent::SetHeroBaseSpeedStat(const int32 InBaseSpeedStat)
{
	BaseWalkSpeed = T66ResolveWalkSpeedFromSpeedStat(InBaseSpeedStat);
	RefreshWalkSpeedFromRunState();
}

void UT66HeroMovementComponent::SetMoveInputAxes(const float ForwardValue, const float RightValue)
{
	CachedForwardInput = ForwardValue;
	CachedRightInput = RightValue;
	UpdateAnimationStateBridge();
}

bool UT66HeroMovementComponent::TryJump()
{
	AT66HeroBase* Hero = ResolveHero();
	if (!Hero || !CanUseMovementAbilities() || !Hero->CanJump())
	{
		return false;
	}

	Hero->Jump();
	UT66AudioSubsystem::PlayEventFromWorldContext(this, FName(TEXT("Hero.Movement.Jump")), Hero->GetActorLocation(), Hero);
	return true;
}

void UT66HeroMovementComponent::StopJumping()
{
	if (AT66HeroBase* Hero = ResolveHero())
	{
		Hero->StopJumping();
	}
}

bool UT66HeroMovementComponent::HasMovementInput() const
{
	return FMath::Abs(CachedForwardInput) > 0.1f || FMath::Abs(CachedRightInput) > 0.1f;
}

bool UT66HeroMovementComponent::CanUseMovementAbilities() const
{
	const AT66HeroBase* Hero = ResolveHero();
	if (!Hero)
	{
		return false;
	}

	return !Hero->IsPreviewMode()
		&& !Hero->IsVehicleMounted()
		&& !Hero->IsKnockbackIncapacitated();
}

float UT66HeroMovementComponent::ResolveLeapCooldownSeconds() const
{
	float Cooldown = MovementTuning.LeapCooldownSeconds;
	if (CachedRunState)
	{
		Cooldown *= CachedRunState->GetDashCooldownMultiplier();
	}

	return FMath::Clamp(Cooldown, 0.05f, 10.f);
}

void UT66HeroMovementComponent::UpdateSurfaceBounce(float DeltaTime)
{
	if (CVarT66HeroSurfaceBounceEnabled.GetValueOnGameThread() == 0)
	{
		return;
	}

	AT66HeroBase* Hero = ResolveHero();
	UCharacterMovementComponent* Movement = ResolveCharacterMovement();
	if (!Hero || !Movement || !CanUseMovementAbilities() || Movement->MovementMode == MOVE_None)
	{
		return;
	}

	const UWorld* World = GetWorld();
	const float Now = World ? static_cast<float>(World->GetTimeSeconds()) : 0.0f;
	UpdateSurfaceBounceAirborneState(Hero, Movement, Now);
	TryApplyWallSurfaceBounce(Hero, Movement, Now);
}

bool UT66HeroMovementComponent::WantsSurfaceBounce(const AT66HeroBase* Hero, const UCharacterMovementComponent* Movement) const
{
	if (!Hero || !Movement)
	{
		return false;
	}

	if (HasMovementInput())
	{
		return true;
	}

	const FVector PendingInput = Hero->GetPendingMovementInputVector();
	if (PendingInput.SizeSquared2D() > KINDA_SMALL_NUMBER)
	{
		return true;
	}

	const float MinSpeed = FMath::Max(0.0f, CVarT66HeroSurfaceBounceMinSpeed.GetValueOnGameThread());
	return Movement->Velocity.Size2D() >= MinSpeed;
}

void UT66HeroMovementComponent::UpdateSurfaceBounceAirborneState(AT66HeroBase* Hero, UCharacterMovementComponent* Movement, const float Now)
{
	if (!Hero || !Movement)
	{
		return;
	}

	const bool bAirborne = Movement->IsFalling();
	const FVector HeroLocation = Hero->GetActorLocation();
	if (bAirborne)
	{
		if (!bSurfaceBounceWasAirborne)
		{
			SurfaceBounceAirbornePeakZ = HeroLocation.Z;
		}
		else
		{
			SurfaceBounceAirbornePeakZ = FMath::Max(SurfaceBounceAirbornePeakZ, HeroLocation.Z);
		}

		SurfaceBounceLastAirborneVelocityZ = Movement->Velocity.Z;
		bSurfaceBounceWasAirborne = true;
		return;
	}

	if (bSurfaceBounceWasAirborne && Movement->IsMovingOnGround())
	{
		const float ImpactDownSpeed = FMath::Max(0.0f, -SurfaceBounceLastAirborneVelocityZ);
		const float FallHeight = FMath::Max(0.0f, SurfaceBounceAirbornePeakZ - HeroLocation.Z);
		bSurfaceBounceWasAirborne = false;
		TryApplyGroundLandingSurfaceBounce(Hero, Movement, Now, ImpactDownSpeed, FallHeight);
		return;
	}

	if (!Movement->IsMovingOnGround())
	{
		bSurfaceBounceWasAirborne = false;
	}
}

bool UT66HeroMovementComponent::TryApplyGroundLandingSurfaceBounce(AT66HeroBase* Hero, UCharacterMovementComponent* Movement, const float Now, const float ImpactDownSpeed, const float FallHeight)
{
	if (!Hero || !Movement || !Movement->IsMovingOnGround())
	{
		return false;
	}

	const float CooldownSeconds = FMath::Max(0.01f, CVarT66HeroSurfaceBounceGroundCooldown.GetValueOnGameThread());
	if (Now - LastGroundSurfaceBounceTime < CooldownSeconds)
	{
		return false;
	}

	const float MinFallHeight = FMath::Max(0.0f, CVarT66HeroSurfaceBounceGroundMinFallHeight.GetValueOnGameThread());
	if (FallHeight < MinFallHeight)
	{
		return false;
	}

	const float GravitySpeed = Movement->GetGravityZ() < -KINDA_SMALL_NUMBER
		? FMath::Sqrt(FMath::Max(0.0f, 2.0f * FMath::Abs(Movement->GetGravityZ()) * FallHeight))
		: 0.0f;
	const float EffectiveImpactSpeed = FMath::Max(ImpactDownSpeed, GravitySpeed);
	const float MinImpactSpeed = FMath::Max(0.0f, CVarT66HeroSurfaceBounceGroundMinImpactSpeed.GetValueOnGameThread());
	if (EffectiveImpactSpeed < MinImpactSpeed)
	{
		return false;
	}

	const float Restitution = FMath::Max(0.0f, CVarT66HeroSurfaceBounceGroundRestitution.GetValueOnGameThread());
	if (Restitution <= KINDA_SMALL_NUMBER)
	{
		return false;
	}

	const float MaxLaunchZ = FMath::Max(0.0f, CVarT66HeroSurfaceBounceGroundMaxLaunchZ.GetValueOnGameThread());
	const float MinLaunchZ = FMath::Clamp(CVarT66HeroSurfaceBounceGroundMinLaunchZ.GetValueOnGameThread(), 0.0f, MaxLaunchZ);
	const float LaunchZ = FMath::Clamp(EffectiveImpactSpeed * Restitution, MinLaunchZ, MaxLaunchZ);
	if (LaunchZ <= KINDA_SMALL_NUMBER)
	{
		return false;
	}

	LastGroundSurfaceBounceTime = Now;
	Hero->LaunchCharacter(FVector(0.0f, 0.0f, LaunchZ), false, true);
	if (CVarT66HeroSurfaceBounceDebugLog.GetValueOnGameThread() != 0 || T66IsHeroMovementQACapture())
	{
		UE_LOG(LogTemp, Display, TEXT("[HeroSurfaceBounce] Type=GroundLanding Hero=%s Time=%.3f ImpactDownSpeed=%.1f FallHeight=%.1f LaunchZ=%.1f VelocityBefore=%s Loc=%s"),
			*Hero->GetName(),
			Now,
			ImpactDownSpeed,
			FallHeight,
			LaunchZ,
			*Movement->Velocity.ToCompactString(),
			*Hero->GetActorLocation().ToCompactString());
	}
	return true;
}

bool UT66HeroMovementComponent::TryApplyWallSurfaceBounce(AT66HeroBase* Hero, UCharacterMovementComponent* Movement, const float Now)
{
	if (!WantsSurfaceBounce(Hero, Movement))
	{
		return false;
	}

	const float CooldownSeconds = FMath::Max(0.01f, CVarT66HeroSurfaceBounceWallCooldown.GetValueOnGameThread());
	if (Now - LastWallSurfaceBounceTime < CooldownSeconds)
	{
		return false;
	}

	const FVector Direction = ResolveSurfaceBounceDirection(Hero, Movement);
	if (Direction.IsNearlyZero())
	{
		return false;
	}

	const UCapsuleComponent* Capsule = Hero->GetCapsuleComponent();
	const float Radius = Capsule ? FMath::Max(12.0f, Capsule->GetScaledCapsuleRadius() * 0.85f) : 30.0f;
	const FVector Start = Hero->GetActorLocation();
	const FVector End = Start + Direction * FMath::Max(20.0f, CVarT66HeroSurfaceBounceWallTraceDistance.GetValueOnGameThread());

	FCollisionQueryParams Params(SCENE_QUERY_STAT(T66HeroSurfaceBounceWall), false);
	Params.AddIgnoredActor(Hero);
	FCollisionObjectQueryParams ObjectParams;
	ObjectParams.AddObjectTypesToQuery(ECC_WorldStatic);
	ObjectParams.AddObjectTypesToQuery(ECC_WorldDynamic);

	// Multi-sweep: trigger bubbles overlap the sweep before the real body does — skip the
	// ignored ones and bounce off the first surface that can physically stop the hero.
	TArray<FHitResult> Hits;
	UWorld* World = GetWorld();
	if (!World || !World->SweepMultiByObjectType(Hits, Start, End, FQuat::Identity, ObjectParams, FCollisionShape::MakeSphere(Radius), Params))
	{
		return false;
	}

	const FHitResult* FirstValidHit = nullptr;
	for (const FHitResult& Candidate : Hits)
	{
		if (!T66ShouldIgnoreSurfaceBounceWallHit(Candidate))
		{
			FirstValidHit = &Candidate;
			break;
		}
		if (CVarT66HeroSurfaceBounceDebugLog.GetValueOnGameThread() != 0 || T66IsHeroMovementQACapture())
		{
			UE_LOG(LogTemp, Display, TEXT("[HeroSurfaceBounce] Type=WallIgnored Hero=%s Time=%.3f HitNormal=%s HitActor=%s HitComponent=%s Loc=%s Reason=NoSurfaceBounce"),
				*Hero->GetName(),
				Now,
				*Candidate.ImpactNormal.ToCompactString(),
				*GetNameSafe(Candidate.GetActor()),
				*GetNameSafe(Candidate.GetComponent()),
				*Hero->GetActorLocation().ToCompactString());
		}
	}
	if (!FirstValidHit)
	{
		return false;
	}
	const FHitResult& Hit = *FirstValidHit;

	FVector WallNormal = Hit.ImpactNormal.GetSafeNormal();
	WallNormal.Z = 0.0f;
	if (!WallNormal.Normalize())
	{
		return false;
	}
	if (FMath::Abs(Hit.ImpactNormal.Z) > 0.35f || FVector::DotProduct(Direction, -WallNormal) < 0.15f)
	{
		return false;
	}

	FVector Incoming = Movement->Velocity;
	Incoming.Z = 0.0f;
	if (!Incoming.Normalize())
	{
		Incoming = Direction;
	}
	FVector Reflected = Incoming - (2.0f * FVector::DotProduct(Incoming, WallNormal) * WallNormal);
	Reflected.Z = 0.0f;
	FVector BounceDirection = (Reflected + WallNormal * 0.75f).GetSafeNormal();
	if (BounceDirection.IsNearlyZero())
	{
		BounceDirection = WallNormal;
	}

	const float CurrentSpeed = Movement->Velocity.Size2D();
	const float LaunchHorizontal = FMath::Max(CVarT66HeroSurfaceBounceWallHorizontal.GetValueOnGameThread(), CurrentSpeed * 0.92f);
	const float LaunchZ = FMath::Max(0.0f, CVarT66HeroSurfaceBounceWallZ.GetValueOnGameThread());
	FVector LaunchVelocity = BounceDirection * LaunchHorizontal;
	LaunchVelocity.Z = LaunchZ;

	LastWallSurfaceBounceTime = Now;
	Hero->AddActorWorldOffset(WallNormal * 24.0f, true, nullptr, ETeleportType::TeleportPhysics);
	Hero->LaunchCharacter(LaunchVelocity, true, true);
	if (CVarT66HeroSurfaceBounceDebugLog.GetValueOnGameThread() != 0 || T66IsHeroMovementQACapture())
	{
		UE_LOG(LogTemp, Display, TEXT("[HeroSurfaceBounce] Type=Wall Hero=%s Time=%.3f Launch=%s HitNormal=%s HitActor=%s Loc=%s"),
			*Hero->GetName(),
			Now,
			*LaunchVelocity.ToCompactString(),
			*Hit.ImpactNormal.ToCompactString(),
			*GetNameSafe(Hit.GetActor()),
			*Hero->GetActorLocation().ToCompactString());
	}
	return true;
}

FVector UT66HeroMovementComponent::ResolveSurfaceBounceDirection(const AT66HeroBase* Hero, const UCharacterMovementComponent* Movement) const
{
	if (!Hero || !Movement)
	{
		return FVector::ZeroVector;
	}

	const FVector PendingInput = Hero->GetPendingMovementInputVector().GetSafeNormal2D();
	if (!PendingInput.IsNearlyZero())
	{
		return PendingInput;
	}

	FVector VelocityDirection = Movement->Velocity;
	VelocityDirection.Z = 0.0f;
	if (VelocityDirection.Normalize())
	{
		return VelocityDirection;
	}

	if (const AController* Controller = Hero->GetController())
	{
		FRotator ControlRot = Controller->GetControlRotation();
		ControlRot.Pitch = 0.0f;
		ControlRot.Roll = 0.0f;
		FVector Forward = FRotationMatrix(ControlRot).GetUnitAxis(EAxis::X) * CachedForwardInput;
		FVector Right = FRotationMatrix(ControlRot).GetUnitAxis(EAxis::Y) * CachedRightInput;
		FVector InputDirection = Forward + Right;
		InputDirection.Z = 0.0f;
		if (InputDirection.Normalize())
		{
			return InputDirection;
		}
	}

	return FVector::ZeroVector;
}

void UT66HeroMovementComponent::UpdateAnimationStateBridge() const
{
	if (CachedHeroSpeedSubsystem)
	{
		CachedHeroSpeedSubsystem->Update(0.f, HasMovementInput());
	}
}

bool UT66HeroMovementComponent::TryLeap()
{
	const AT66HeroBase* Hero = ResolveHero();
	return Hero ? TryLeapInWorldDirection(Hero->GetActorForwardVector()) : false;
}

bool UT66HeroMovementComponent::TryRollForward()
{
	return TryLeap();
}

bool UT66HeroMovementComponent::TryLeapInWorldDirection(const FVector& DesiredWorldDirection)
{
	AT66HeroBase* Hero = ResolveHero();
	UWorld* World = GetWorld();
	if (!Hero || !World || !CanUseMovementAbilities())
	{
		return false;
	}

	FVector LeapDirection = DesiredWorldDirection;
	LeapDirection.Z = 0.f;
	if (!LeapDirection.Normalize())
	{
		return false;
	}

	const float Now = static_cast<float>(World->GetTimeSeconds());
	if (Now - LastLeapTime < ResolveLeapCooldownSeconds())
	{
		return false;
	}

	const float LeapHorizontalStrength = FMath::Max(
		MovementTuning.LeapStrength,
		ResolveCurrentMaxWalkSpeed() * MovementTuning.LeapSpeedMultiplierOverWalkSpeed);
	const float LeapUpwardStrength = FMath::Max(
		MovementTuning.LeapUpwardStrength,
		MovementTuning.JumpZVelocity * 0.5f);
	FVector LeapVelocity = LeapDirection * LeapHorizontalStrength;
	LeapVelocity.Z = LeapUpwardStrength;

	LastLeapTime = Now;
	Hero->LaunchCharacter(LeapVelocity, true, true);
	if (T66IsHeroMovementQACapture())
	{
		UE_LOG(LogTemp, Display, TEXT("[HeroMovementQA] Leap launch direction=%s velocity=%s horizontalStrength=%.1f upwardStrength=%.1f cooldown=%.2f."),
			*LeapDirection.ToCompactString(),
			*LeapVelocity.ToCompactString(),
			LeapHorizontalStrength,
			LeapUpwardStrength,
			ResolveLeapCooldownSeconds());
	}
	UT66AudioSubsystem::PlayEventFromWorldContext(this, FName(TEXT("Hero.Movement.Dash")), Hero->GetActorLocation(), Hero);
	return true;
}

bool UT66HeroMovementComponent::TryDashInWorldDirection(const FVector& DesiredWorldDirection)
{
	return TryLeapInWorldDirection(DesiredWorldDirection);
}
