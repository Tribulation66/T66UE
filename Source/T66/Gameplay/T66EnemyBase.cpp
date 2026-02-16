// Copyright Tribulation 66. All Rights Reserved.

#include "Gameplay/T66EnemyBase.h"
#include "Gameplay/T66EnemyDirector.h"
#include "Gameplay/T66EnemyAIController.h"
#include "Gameplay/T66LootBagPickup.h"
#include "Gameplay/T66HeroBase.h"
#include "Gameplay/T66HouseNPCBase.h"
#include "Core/T66CharacterVisualSubsystem.h"
#include "Core/T66AchievementsSubsystem.h"
#include "Core/T66RunStateSubsystem.h"
#include "Core/T66DamageLogSubsystem.h"
#include "Core/T66LagTrackerSubsystem.h"
#include "Core/T66ActorRegistrySubsystem.h"
#include "Core/T66EnemyPoolSubsystem.h"
#include "Core/T66FloatingCombatTextSubsystem.h"
#include "Core/T66GameInstance.h"
#include "Core/T66Rarity.h"
#include "Core/T66RngSubsystem.h"
#include "UI/T66EnemyHealthBarWidget.h"
#include "Gameplay/T66VisualUtil.h"
#include "Components/CapsuleComponent.h"
#include "Components/WidgetComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Engine/StaticMesh.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/World.h"
// [GOLD] EngineUtils.h removed — no more TActorIterator in this file (using ActorRegistry instead).

AT66EnemyBase::AT66EnemyBase()
{
	PrimaryActorTick.bCanEverTick = true;
	AIControllerClass = AT66EnemyAIController::StaticClass();
	AutoPossessAI = EAutoPossessAI::PlacedInWorldOrSpawned;
	CurrentHP = MaxHP;

	if (UCharacterMovementComponent* Move = GetCharacterMovement())
	{
		Move->MaxWalkSpeed = 350.f;
		Move->bOrientRotationToMovement = true;
		Move->RotationRate = FRotator(0.f, 720.f, 0.f);
	}
	bUseControllerRotationYaw = false;

	UCapsuleComponent* Capsule = GetCapsuleComponent();
	if (Capsule)
	{
		Capsule->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);
		Capsule->SetCollisionResponseToChannel(ECC_WorldDynamic, ECR_Overlap); // so projectile overlaps and hits
		// Allow click-to-lock trace (screen-space hit test) to hit enemies.
		Capsule->SetCollisionResponseToChannel(ECC_Visibility, ECR_Block);
	}

	VisualMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("VisualMesh"));
	VisualMesh->SetupAttachment(RootComponent);
	VisualMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	// Align primitive mesh to ground when capsule is grounded:
	// capsule half-height~88, cylinder half-height=50 => relative Z = 50 - 88 = -38.
	VisualMesh->SetRelativeLocation(FVector(0.f, 0.f, -38.f));
	if (UStaticMesh* Sphere = FT66VisualUtil::GetBasicShapeSphere())
	{
		VisualMesh->SetStaticMesh(Sphere);
		VisualMesh->SetRelativeScale3D(FVector(0.85f, 0.85f, 0.85f));
		FT66VisualUtil::ApplyT66Color(VisualMesh, this, FLinearColor(0.90f, 0.25f, 0.20f, 1.f)); // regular enemy default
	}

	HealthBarWidget = CreateDefaultSubobject<UWidgetComponent>(TEXT("HealthBarWidget"));
	HealthBarWidget->SetupAttachment(RootComponent);
	HealthBarWidget->SetRelativeLocation(FVector(0.f, 0.f, 155.f));
	HealthBarWidget->SetWidgetSpace(EWidgetSpace::Screen);
	HealthBarWidget->SetDrawAtDesiredSize(true);
	// Height includes space for lock indicator above the bar.
	HealthBarWidget->SetDrawSize(FVector2D(120.f, 28.f));

	// Prepare built-in SkeletalMeshComponent for imported models.
	if (USkeletalMeshComponent* Skel = GetMesh())
	{
		Skel->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		Skel->SetVisibility(false, true); // shown only when a character visual mapping exists
	}
}

void AT66EnemyBase::ConfigureAsMob(FName InMobID)
{
	MobID = InMobID;
	// Stage mobs can optionally map MobID -> imported skeletal mesh via DT_CharacterVisuals.
	// If no row exists, we safely fall back to placeholder visuals.
	CharacterVisualID = MobID;

	if (!VisualMesh) return;

	// Stable per-mob visuals: mesh shape + HSV color from MobID hash.
	const uint32 H = GetTypeHash(MobID);
	const int32 Shape = static_cast<int32>(H % 4u);

	UStaticMesh* StaticShapeMesh = nullptr;
	switch (Shape)
	{
		case 0: StaticShapeMesh = FT66VisualUtil::GetBasicShapeSphere(); break;
		case 1: StaticShapeMesh = FT66VisualUtil::GetBasicShapeCube(); break;
		case 2: StaticShapeMesh = FT66VisualUtil::GetBasicShapeCylinder(); break;
		default: StaticShapeMesh = FT66VisualUtil::GetBasicShapeCone(); break;
	}
	if (StaticShapeMesh)
	{
		VisualMesh->SetStaticMesh(StaticShapeMesh);
	}

	// Hue derived from hash; keep saturation/value high for clarity.
	const float Hue01 = static_cast<float>((H / 7u) % 360u) / 360.f;
	const FLinearColor C = FLinearColor::MakeFromHSV8(
		static_cast<uint8>(Hue01 * 255.f),
		210,
		240
	);
	FT66VisualUtil::ApplyT66Color(VisualMesh, this, C);

	// Reasonable default scale per shape.
	switch (Shape)
	{
		case 1: VisualMesh->SetRelativeScale3D(FVector(0.75f, 0.75f, 0.75f)); break; // cube
		case 2: VisualMesh->SetRelativeScale3D(FVector(0.70f, 0.70f, 0.95f)); break; // cylinder
		case 3: VisualMesh->SetRelativeScale3D(FVector(0.80f, 0.80f, 0.95f)); break; // cone
		default: VisualMesh->SetRelativeScale3D(FVector(0.85f, 0.85f, 0.85f)); break; // sphere
	}
}

void AT66EnemyBase::ApplyMiniBossMultipliers(float HPScalar, float DamageScalar, float ScaleScalar)
{
	bIsMiniBoss = true;
	MiniBossHPScalarApplied = FMath::Max(0.1f, HPScalar);
	MiniBossDamageScalarApplied = FMath::Max(0.1f, DamageScalar);
	MiniBossScaleScalarApplied = FMath::Clamp(ScaleScalar, 1.0f, 4.0f);

	// Stats: scale current (already difficulty-scaled) tuning.
	MaxHP = FMath::Max(1, FMath::RoundToInt(static_cast<float>(MaxHP) * MiniBossHPScalarApplied));
	CurrentHP = MaxHP;
	TouchDamageHearts = FMath::Max(1, FMath::RoundToInt(static_cast<float>(TouchDamageHearts) * MiniBossDamageScalarApplied));
	UpdateHealthBar();

	// Visual: scale the whole actor (capsule + mesh) so it's obviously a mini-boss.
	SetActorScale3D(FVector(MiniBossScaleScalarApplied, MiniBossScaleScalarApplied, MiniBossScaleScalarApplied));
}

void AT66EnemyBase::BeginPlay()
{
	Super::BeginPlay();

	// [GOLD] Register with the central actor registry (replaces TActorIterator world scans).
	if (UWorld* W = GetWorld())
	{
		if (UT66ActorRegistrySubsystem* Registry = W->GetSubsystem<UT66ActorRegistrySubsystem>())
		{
			Registry->RegisterEnemy(this);
		}
	}

	if (!bBaseTuningInitialized)
	{
		BaseMaxHP = MaxHP;
		BaseTouchDamageHearts = TouchDamageHearts;
		BasePointValue = PointValue;
		bBaseTuningInitialized = true;
	}
	// Ensure HP is valid on spawn (in case difficulty scaled before BeginPlay).
	CurrentHP = FMath::Clamp(CurrentHP, 1, MaxHP);

	// Ensure the widget exists and is the correct class.
	if (HealthBarWidget)
	{
		HealthBarWidget->SetWidgetClass(UT66EnemyHealthBarWidget::StaticClass());
		HealthBarWidget->InitWidget();
		HealthBarWidget->SetHiddenInGame(false, true);
		HealthBarWidget->SetVisibility(true, true);
	}
	UpdateHealthBar();

	UCapsuleComponent* Capsule = GetCapsuleComponent();
	if (Capsule)
	{
		Capsule->OnComponentBeginOverlap.AddDynamic(this, &AT66EnemyBase::OnCapsuleBeginOverlap);
	}

	// Fail-safe: always make sure the placeholder is visible unless we successfully apply a skeletal mesh.
	if (VisualMesh)
	{
		VisualMesh->SetHiddenInGame(false, true);
		VisualMesh->SetVisibility(true, true);
	}

	// Per request: RegularEnemy + Unique enemy use simple placeholder visuals (no FBX).
	const bool bForcePlaceholder = CharacterVisualID.IsNone() || (CharacterVisualID == FName(TEXT("RegularEnemy")));
	if (bForcePlaceholder)
	{
		if (USkeletalMeshComponent* Skel = GetMesh())
		{
			Skel->SetHiddenInGame(true, true);
			Skel->SetVisibility(false, true);
		}
#if !UE_BUILD_SHIPPING
		static int32 LoggedEnemies = 0;
		if (LoggedEnemies < 12)
		{
			++LoggedEnemies;
			UE_LOG(LogTemp, Warning, TEXT("EnemyVisuals: %s VisualID=%s UsingPlaceholder=1"), *GetName(), *CharacterVisualID.ToString());
		}
#endif
		return;
	}

	// Apply imported character mesh if available (data-driven).
	bUsingCharacterVisual = false;
	if (UWorld* World = GetWorld())
	{
		if (UGameInstance* GI = World->GetGameInstance())
		{
			if (UT66CharacterVisualSubsystem* Visuals = GI->GetSubsystem<UT66CharacterVisualSubsystem>())
			{
				bUsingCharacterVisual = Visuals->ApplyCharacterVisual(CharacterVisualID, GetMesh(), VisualMesh, true);
				if (USkeletalMeshComponent* Skel = GetMesh())
				{
					// If we didn't apply a skeletal mesh, keep the character mesh hidden and use the cylinder.
					if (!bUsingCharacterVisual || !Skel->GetSkeletalMeshAsset())
					{
						bUsingCharacterVisual = false;
						Skel->SetHiddenInGame(true, true);
						Skel->SetVisibility(false, true);
						if (VisualMesh)
						{
							VisualMesh->SetHiddenInGame(false, true);
							VisualMesh->SetVisibility(true, true);
						}
					}
				}
			}
		}
	}

#if !UE_BUILD_SHIPPING
	// Lightweight visibility diagnostics (helps track "enemies are invisible" reports).
	static int32 LoggedEnemies = 0;
	if (LoggedEnemies < 12)
	{
		++LoggedEnemies;
		const USkeletalMeshComponent* Skel = GetMesh();
		const TCHAR* SkelName = (Skel && Skel->GetSkeletalMeshAsset()) ? *Skel->GetSkeletalMeshAsset()->GetName() : TEXT("None");
		UE_LOG(LogTemp, Warning, TEXT("EnemyVisuals: %s VisualID=%s UsingSkel=%d SkelAsset=%s SkelVisible=%d SkelHidden=%d PlaceholderVisible=%d PlaceholderHidden=%d"),
			*GetName(),
			*CharacterVisualID.ToString(),
			bUsingCharacterVisual ? 1 : 0,
			SkelName,
			Skel ? (Skel->IsVisible() ? 1 : 0) : 0,
			Skel ? (Skel->bHiddenInGame ? 1 : 0) : 0,
			VisualMesh ? (VisualMesh->IsVisible() ? 1 : 0) : 0,
			VisualMesh ? (VisualMesh->bHiddenInGame ? 1 : 0) : 0
		);
	}
#endif
}

void AT66EnemyBase::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	// [GOLD] Unregister from the actor registry.
	if (UWorld* W = GetWorld())
	{
		if (UT66ActorRegistrySubsystem* Registry = W->GetSubsystem<UT66ActorRegistrySubsystem>())
		{
			Registry->UnregisterEnemy(this);
		}
	}
	Super::EndPlay(EndPlayReason);
}

void AT66EnemyBase::ApplyStageScaling(int32 Stage)
{
	const int32 ClampedStage = FMath::Clamp(Stage, 1, 999);
	const float Multiplier = FMath::Pow(1.1f, static_cast<float>(ClampedStage - 1));

	BaseMaxHP = FMath::Max(1, FMath::RoundToInt(50.f * Multiplier));
	BaseArmor = 0.1f * Multiplier;
	MaxHP = BaseMaxHP;
	Armor = FMath::Clamp(BaseArmor, 0.f, 0.95f);
	CurrentHP = MaxHP;

	if (!bBaseTuningInitialized)
	{
		BaseTouchDamageHearts = TouchDamageHearts;
		BasePointValue = PointValue;
		bBaseTuningInitialized = true;
	}
	bStageScalingApplied = true;
	UpdateHealthBar();
}

void AT66EnemyBase::ApplyDifficultyScalar(float Scalar)
{
	if (!bBaseTuningInitialized)
	{
		BaseMaxHP = MaxHP;
		BaseTouchDamageHearts = TouchDamageHearts;
		BasePointValue = PointValue;
		BaseArmor = Armor;
		bBaseTuningInitialized = true;
	}

	const float ClampedScalar = FMath::Clamp(Scalar, 1.0f, 99.0f);

	// When stage scaling is used, only scale touch damage and point value; HP and Armor stay stage-based.
	if (!bStageScalingApplied)
	{
		const float PrevMax = static_cast<float>(FMath::Max(1, MaxHP));
		const float PrevCur = static_cast<float>(FMath::Clamp(CurrentHP, 0, MaxHP));
		const float Pct = FMath::Clamp(PrevCur / PrevMax, 0.f, 1.f);
		MaxHP = FMath::Max(1, FMath::RoundToInt(static_cast<float>(BaseMaxHP) * ClampedScalar));
		Armor = FMath::Clamp(static_cast<float>(BaseArmor) * ClampedScalar, 0.f, 0.95f);
		CurrentHP = FMath::Clamp(FMath::RoundToInt(static_cast<float>(MaxHP) * Pct), 1, MaxHP);
	}

	TouchDamageHearts = FMath::Max(1, FMath::RoundToInt(static_cast<float>(BaseTouchDamageHearts) * ClampedScalar));

	// Re-apply mini-boss multipliers if this enemy was promoted.
	if (bIsMiniBoss)
	{
		MaxHP = FMath::Max(1, FMath::RoundToInt(static_cast<float>(MaxHP) * FMath::Max(0.1f, MiniBossHPScalarApplied)));
		CurrentHP = MaxHP;
		TouchDamageHearts = FMath::Max(1, FMath::RoundToInt(static_cast<float>(TouchDamageHearts) * FMath::Max(0.1f, MiniBossDamageScalarApplied)));
		SetActorScale3D(FVector(MiniBossScaleScalarApplied, MiniBossScaleScalarApplied, MiniBossScaleScalarApplied));
	}

	UpdateHealthBar();
}

void AT66EnemyBase::ApplyDifficultyTier(int32 Tier)
{
	// Tier 0 = 1.0x, Tier 1 = 1.1x, Tier 2 = 1.2x, ...
	const int32 ClampedTier = FMath::Max(0, Tier);
	const float Scalar = 1.0f + (0.1f * static_cast<float>(ClampedTier));
	ApplyDifficultyScalar(Scalar);
}

void AT66EnemyBase::ResetForReuse(const FVector& NewLocation, AT66EnemyDirector* NewDirector)
{
	UE_LOG(LogTemp, Verbose, TEXT("[GOLD] EnemyPool: ResetForReuse %s at (%.0f, %.0f, %.0f)"),
		*GetName(), NewLocation.X, NewLocation.Y, NewLocation.Z);

	// Restore base state for reuse.
	OwningDirector = NewDirector;
	CurrentHP = MaxHP;
	bIsConfused = false;
	ConfusionSecondsRemaining = 0.f;
	ArmorDebuffAmount = 0.f;
	ArmorDebuffSecondsRemaining = 0.f;
	bIsMiniBoss = false;
	MiniBossHPScalarApplied = 1.0f;
	MiniBossDamageScalarApplied = 1.0f;
	MiniBossScaleScalarApplied = 1.0f;
	SetActorScale3D(FVector::OneVector);
	LastTouchDamageTime = -9999.f;
	CachedPlayerPawn = nullptr;
	CachedWanderDir = FVector::ZeroVector;
	WanderDirRefreshAccum = 0.f;
	bCachedInsideSafeZone = false;
	SafeZoneCheckAccumSeconds = 0.f;
	bRisingFromGround = false;
	RiseElapsed = 0.f;

	SetActorLocation(NewLocation);
	SetActorHiddenInGame(false);
	SetActorEnableCollision(true);
	SetActorTickEnabled(true);

	if (UCharacterMovementComponent* Move = GetCharacterMovement())
	{
		Move->SetComponentTickEnabled(true);
		Move->SetMovementMode(MOVE_Walking);
	}

	// Re-register with the actor registry (we unregistered when released to pool).
	if (UWorld* W = GetWorld())
	{
		if (UT66ActorRegistrySubsystem* Registry = W->GetSubsystem<UT66ActorRegistrySubsystem>())
		{
			Registry->RegisterEnemy(this);
		}
	}

	UpdateHealthBar();
}

void AT66EnemyBase::StartRiseFromGround(float TargetGroundZ)
{
	RiseTargetZ = TargetGroundZ;
	RiseStartZ = TargetGroundZ - BuryDepth;
	RiseElapsed = 0.f;
	bRisingFromGround = true;

	FVector Loc = GetActorLocation();
	Loc.Z = RiseStartZ;
	SetActorLocation(Loc);

	if (UCapsuleComponent* Cap = GetCapsuleComponent())
	{
		Cap->SetCollisionResponseToChannel(ECC_Pawn, ECR_Ignore);
	}
}

void AT66EnemyBase::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);
	if (CurrentHP <= 0) return;

	// Rise-from-ground animation: lerp Z up, then re-enable collision.
	if (bRisingFromGround)
	{
		RiseElapsed += DeltaSeconds;
		const float Alpha = FMath::Clamp(RiseElapsed / RiseDuration, 0.f, 1.f);
		FVector Loc = GetActorLocation();
		Loc.Z = FMath::Lerp(RiseStartZ, RiseTargetZ, Alpha);
		SetActorLocation(Loc);

		if (Alpha >= 1.f)
		{
			bRisingFromGround = false;
			if (UCapsuleComponent* Cap = GetCapsuleComponent())
			{
				Cap->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);
			}
		}
		return;
	}

	// [GOLD] Cache the player pawn once; re-resolve only if the weak ref went stale.
	APawn* PlayerPawn = CachedPlayerPawn.Get();
	if (!PlayerPawn)
	{
		PlayerPawn = UGameplayStatics::GetPlayerPawn(this, 0);
		CachedPlayerPawn = PlayerPawn;
		UE_LOG(LogTemp, Verbose, TEXT("[GOLD] PlayerPawnCache: enemy %s resolved pawn %s"),
			*GetName(), PlayerPawn ? *PlayerPawn->GetName() : TEXT("null"));
	}
	if (!PlayerPawn) return;

	UCharacterMovementComponent* Move = GetCharacterMovement();
	if (!Move) return;

	// Safe zone rule: enemies cannot enter NPC safe bubbles.
	// Perf: evaluate bubble membership at a low frequency (not every Tick).
	SafeZoneCheckAccumSeconds += DeltaSeconds;
	if (SafeZoneCheckAccumSeconds >= SafeZoneCheckIntervalSeconds)
	{
		SafeZoneCheckAccumSeconds = 0.f;
		bCachedInsideSafeZone = false;
		CachedSafeZoneEscapeDir = FVector::ZeroVector;

		// [GOLD] Use the actor registry instead of TActorIterator for safe-zone NPC lookup.
		UT66ActorRegistrySubsystem* Registry = GetWorld() ? GetWorld()->GetSubsystem<UT66ActorRegistrySubsystem>() : nullptr;
		const TArray<TWeakObjectPtr<AT66HouseNPCBase>>& RegisteredNPCs = Registry ? Registry->GetNPCs() : TArray<TWeakObjectPtr<AT66HouseNPCBase>>();

		for (const TWeakObjectPtr<AT66HouseNPCBase>& WeakNPC : RegisteredNPCs)
		{
			AT66HouseNPCBase* NPC = WeakNPC.Get();
			if (!NPC) continue;

			const FVector N = NPC->GetActorLocation();
			FVector ToEnemy = GetActorLocation() - N;
			ToEnemy.Z = 0.f;
			const float Dist = ToEnemy.Size();
			if (Dist < NPC->GetSafeZoneRadius())
			{
				bCachedInsideSafeZone = true;
				if (Dist > 1.f)
				{
					ToEnemy /= Dist;
					CachedSafeZoneEscapeDir = ToEnemy;
				}
				break;
			}
		}
	}

	if (bCachedInsideSafeZone)
	{
		if (!CachedSafeZoneEscapeDir.IsNearlyZero())
		{
			AddMovementInput(CachedSafeZoneEscapeDir, 1.f);
		}
		return;
	}

	// Leash: if far from player, teleport to a position near the player (throttled).
	if (LeashMaxDistance > 0.f)
	{
		LeashCheckAccumSeconds += DeltaSeconds;
		if (LeashCheckAccumSeconds >= LeashCheckIntervalSeconds)
		{
			LeashCheckAccumSeconds = 0.f;
			const float Dist2D = FVector::Dist2D(GetActorLocation(), PlayerPawn->GetActorLocation());
			if (Dist2D > LeashMaxDistance)
			{
				UWorld* World = GetWorld();
				UT66ActorRegistrySubsystem* Registry = World ? World->GetSubsystem<UT66ActorRegistrySubsystem>() : nullptr;
				auto IsInAnySafeZone2D = [&](const FVector& Loc) -> bool
				{
					if (!Registry) return false;
					for (const TWeakObjectPtr<AT66HouseNPCBase>& WeakNPC : Registry->GetNPCs())
					{
						AT66HouseNPCBase* NPC = WeakNPC.Get();
						if (!NPC) continue;
						const float R = NPC->GetSafeZoneRadius();
						if (FVector::DistSquared2D(Loc, NPC->GetActorLocation()) < (R * R)) return true;
					}
					return false;
				};
				FVector PlayerLoc = PlayerPawn->GetActorLocation();
				static constexpr float LeashSpawnMin = 400.f;
				static constexpr float LeashSpawnMax = 800.f;
				FVector NewLoc = PlayerLoc;
				for (int32 Try = 0; Try < 6; ++Try)
				{
					const float Angle = FMath::FRandRange(0.f, 2.f * PI);
					const float Dist = FMath::FRandRange(LeashSpawnMin, LeashSpawnMax);
					NewLoc = PlayerLoc + FVector(FMath::Cos(Angle) * Dist, FMath::Sin(Angle) * Dist, 0.f);
					if (!IsInAnySafeZone2D(NewLoc)) break;
				}
				if (IsInAnySafeZone2D(NewLoc) && Registry)
				{
					static constexpr float SafeZonePushMargin = 50.f;
					for (const TWeakObjectPtr<AT66HouseNPCBase>& WeakNPC : Registry->GetNPCs())
					{
						AT66HouseNPCBase* NPC = WeakNPC.Get();
						if (!NPC) continue;
						const float R = NPC->GetSafeZoneRadius();
						FVector ToSpawn = NewLoc - NPC->GetActorLocation();
						ToSpawn.Z = 0.f;
						const float Dist2DToNpc = ToSpawn.Size();
						if (Dist2DToNpc < R && Dist2DToNpc > 1.f)
						{
							const FVector Dir = ToSpawn / Dist2DToNpc;
							NewLoc = NPC->GetActorLocation() + FVector(Dir.X, Dir.Y, 0.f) * (R + SafeZonePushMargin);
							NewLoc.Z = PlayerLoc.Z;
							break;
						}
						if (Dist2DToNpc <= 1.f)
						{
							FVector Dir(FMath::FRandRange(-1.f, 1.f), FMath::FRandRange(-1.f, 1.f), 0.f);
							Dir.Z = 0.f;
							if (Dir.Normalize())
							{
								NewLoc = NPC->GetActorLocation() + Dir * (R + SafeZonePushMargin);
								NewLoc.Z = PlayerLoc.Z;
							}
							break;
						}
					}
				}
				FHitResult Hit;
				if (World && World->LineTraceSingleByChannel(Hit, NewLoc + FVector(0.f, 0.f, 500.f), NewLoc - FVector(0.f, 0.f, 1000.f), ECC_WorldStatic))
				{
					static constexpr float CapsuleHalfHeight = 88.f;
					NewLoc = Hit.ImpactPoint + FVector(0.f, 0.f, CapsuleHalfHeight);
				}
				else
				{
					NewLoc.Z = PlayerLoc.Z;
				}
				SetActorLocation(NewLoc);
			}
		}
	}

	// Tick armor debuff timer.
	if (ArmorDebuffSecondsRemaining > 0.f)
	{
		ArmorDebuffSecondsRemaining = FMath::Max(0.f, ArmorDebuffSecondsRemaining - DeltaSeconds);
		if (ArmorDebuffSecondsRemaining <= 0.f)
		{
			ArmorDebuffAmount = 0.f;
		}
	}

	// Tick confusion timer.
	if (bIsConfused)
	{
		ConfusionSecondsRemaining = FMath::Max(0.f, ConfusionSecondsRemaining - DeltaSeconds);
		if (ConfusionSecondsRemaining <= 0.f)
		{
			bIsConfused = false;
		}
		else
		{
			// [GOLD] Confused enemies wander randomly instead of chasing.
			// Refresh wander direction once per second (not every frame — avoids RNG + normalize per tick).
			WanderDirRefreshAccum += DeltaSeconds;
			if (WanderDirRefreshAccum >= WanderDirRefreshInterval || CachedWanderDir.IsNearlyZero())
			{
				WanderDirRefreshAccum = 0.f;
				CachedWanderDir = FVector(FMath::FRandRange(-1.f, 1.f), FMath::FRandRange(-1.f, 1.f), 0.f).GetSafeNormal();
			}
			if (!CachedWanderDir.IsNearlyZero())
			{
				AddMovementInput(CachedWanderDir, 0.5f);
			}
			return; // Skip normal AI
		}
	}

	FVector ToPlayer = PlayerPawn->GetActorLocation() - GetActorLocation();
	ToPlayer.Z = 0.f;
	float Len = ToPlayer.Size();
	if (Len > 10.f)
	{
		ToPlayer /= Len;
		if (bRunAwayFromPlayer)
		{
			AddMovementInput(-ToPlayer, 1.f);
		}
		else
		{
			AddMovementInput(ToPlayer, 1.f);
		}
	}
}

void AT66EnemyBase::OnCapsuleBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
	UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if (!OtherActor) return;
	AT66HeroBase* Hero = Cast<AT66HeroBase>(OtherActor);
	if (!Hero) return;
	if (Hero->IsInSafeZone()) return;

	UWorld* World = GetWorld();
	if (!World) return;
	UGameInstance* GI = World->GetGameInstance();
	if (!GI) return;
	UT66RunStateSubsystem* RunState = GI->GetSubsystem<UT66RunStateSubsystem>();
	if (!RunState) return;

	const float Now = static_cast<float>(World->GetTimeSeconds());
	if (Now - LastTouchDamageTime < TouchDamageCooldown) return;

	LastTouchDamageTime = Now;
	const int32 DamageHP = 5;
	RunState->ApplyDamage(DamageHP, this);
}

bool AT66EnemyBase::TakeDamageFromHero(int32 Damage, FName DamageSourceID, FName EventType)
{
	if (Damage <= 0 || CurrentHP <= 0) return false;

	// Apply enemy armor (damage reduction). Debuffs temporarily lower armor; can go negative for bonus damage.
	const float EffectiveArmor = FMath::Clamp(Armor - ArmorDebuffAmount, -0.5f, 0.95f);
	const int32 ReducedDamage = FMath::Max(1, FMath::RoundToInt(static_cast<float>(Damage) * (1.f - EffectiveArmor)));

	const FName SourceID = DamageSourceID.IsNone() ? UT66DamageLogSubsystem::SourceID_AutoAttack : DamageSourceID;
	if (UWorld* World = GetWorld())
	{
		if (UGameInstance* GI = World->GetGameInstance())
		{
			if (UT66DamageLogSubsystem* DamageLog = GI->GetSubsystem<UT66DamageLogSubsystem>())
			{
				DamageLog->RecordDamageDealt(SourceID, ReducedDamage);
			}
			if (UT66FloatingCombatTextSubsystem* FloatingText = GI->GetSubsystem<UT66FloatingCombatTextSubsystem>())
			{
				FloatingText->ShowDamageNumber(this, ReducedDamage, EventType);
			}
		}
	}
	CurrentHP = FMath::Max(0, CurrentHP - ReducedDamage);
	UpdateHealthBar();
	if (CurrentHP <= 0)
	{
		if (UWorld* World = GetWorld())
			if (UGameInstance* GI = World->GetGameInstance())
				if (UT66RunStateSubsystem* RunState = GI->GetSubsystem<UT66RunStateSubsystem>())
					RunState->NotifyEnemyKilledByHero();
		OnDeath();
		return true;
	}
	return false;
}

void AT66EnemyBase::ApplyConfusion(float DurationSeconds)
{
	const float Dur = FMath::Clamp(DurationSeconds, 0.f, 10.f);
	if (Dur <= 0.f) return;
	bIsConfused = true;
	ConfusionSecondsRemaining = FMath::Max(ConfusionSecondsRemaining, Dur);
}

void AT66EnemyBase::ApplyArmorDebuff(float ReductionAmount, float DurationSeconds)
{
	const float Amt = FMath::Clamp(ReductionAmount, 0.f, 1.f);
	const float Dur = FMath::Clamp(DurationSeconds, 0.f, 30.f);
	if (Amt <= 0.f || Dur <= 0.f) return;
	ArmorDebuffAmount = FMath::Max(ArmorDebuffAmount, Amt);
	ArmorDebuffSecondsRemaining = FMath::Max(ArmorDebuffSecondsRemaining, Dur);
}

void AT66EnemyBase::UpdateHealthBar()
{
	if (!HealthBarWidget) return;
	UT66EnemyHealthBarWidget* W = Cast<UT66EnemyHealthBarWidget>(HealthBarWidget->GetUserWidgetObject());
	if (!W) return;
	const float Pct = (MaxHP <= 0) ? 0.f : (static_cast<float>(FMath::Clamp(CurrentHP, 0, MaxHP)) / static_cast<float>(MaxHP));
	W->SetHealthPercent(Pct);
}

void AT66EnemyBase::SetLockedIndicator(bool bLocked)
{
	if (!HealthBarWidget) return;
	UT66EnemyHealthBarWidget* W = Cast<UT66EnemyHealthBarWidget>(HealthBarWidget->GetUserWidgetObject());
	if (!W) return;
	W->SetLocked(bLocked);
}

void AT66EnemyBase::OnDeath()
{
	UWorld* World = GetWorld();
	UGameInstance* GI = World ? World->GetGameInstance() : nullptr;
	UT66RunStateSubsystem* RunState = GI ? GI->GetSubsystem<UT66RunStateSubsystem>() : nullptr;
	UT66AchievementsSubsystem* Achievements = GI ? GI->GetSubsystem<UT66AchievementsSubsystem>() : nullptr;
	if (RunState)
	{
		const float Scalar = RunState->GetDifficultyScalar();
		const int32 AwardPoints = FMath::Max(0, FMath::RoundToInt(static_cast<float>(PointValue) * Scalar));
		RunState->AddScore(AwardPoints);
		RunState->AddHeroXP(XPValue);
		RunState->AddStructuredEvent(ET66RunEventType::EnemyKilled, FString::Printf(TEXT("PointValue=%d"), AwardPoints));
	}
	if (Achievements)
	{
		Achievements->NotifyEnemyKilled(1);
		// Lab unlock: mark this enemy type as unlocked for The Lab.
		if (!CharacterVisualID.IsNone())
		{
			Achievements->AddLabUnlockedEnemy(CharacterVisualID);
		}
	}

	if (OwningDirector)
	{
		OwningDirector->NotifyEnemyDied(this);
	}

	if (!World) return;
	UT66GameInstance* T66GI = Cast<UT66GameInstance>(World->GetGameInstance());
	if (bDropsLoot && T66GI)
	{
		// Loot bag drop chance + rarity are now driven by the central RNG subsystem (Luck-biased).
		UT66RngSubsystem* RngSub = GI ? GI->GetSubsystem<UT66RngSubsystem>() : nullptr;
		if (RngSub && RunState)
		{
			RngSub->UpdateLuckStat(RunState->GetLuckStat());
		}

		FRandomStream LocalRng(FPlatformTime::Cycles());
		FRandomStream& Stream = RngSub ? RngSub->GetRunStream() : LocalRng;

		const UT66RngTuningConfig* Tuning = RngSub ? RngSub->GetTuning() : nullptr;
		const float BaseDropChance = Tuning ? Tuning->LootBagDropChanceBase : 0.10f;
		const float DropChance = RngSub ? RngSub->BiasChance01(BaseDropChance) : FMath::Clamp(BaseDropChance, 0.f, 1.f);
		const bool bDroppedBag = (Stream.GetFraction() <= DropChance);
		if (RunState)
		{
			RunState->RecordLuckQuantityBool(FName(TEXT("EnemyLootBagDrop")), bDroppedBag);
		}
		if (!bDroppedBag)
		{
			// [GOLD] Return to enemy pool instead of destroying (reduces GC pressure).
			if (UT66EnemyPoolSubsystem* Pool = World->GetSubsystem<UT66EnemyPoolSubsystem>())
			{
				// Unregister from registry before pooling.
				if (UT66ActorRegistrySubsystem* Registry = World->GetSubsystem<UT66ActorRegistrySubsystem>())
				{
					Registry->UnregisterEnemy(this);
				}
				Pool->Release(this);
				UE_LOG(LogTemp, Verbose, TEXT("[GOLD] EnemyPool: enemy %s returned to pool (no loot drop)"), *GetName());
			}
			else
			{
				// Fallback: deferred destruction.
				SetActorHiddenInGame(true);
				SetActorEnableCollision(false);
				SetLifeSpan(0.1f);
			}
			return;
		}

		const FT66RarityWeights Weights = Tuning ? Tuning->LootBagRarityBase : FT66RarityWeights{};
		ET66Rarity BagRarity = RngSub ? RngSub->RollRarityWeighted(Weights, Stream) : FT66RarityUtil::RollDefaultRarity(Stream);
		if (bIsMiniBoss)
		{
			// Mini-bosses should skew toward higher tier loot bags.
			// Simple bias: roll twice (luck-weighted) and take the better rarity.
			const ET66Rarity R2 = RngSub ? RngSub->RollRarityWeighted(Weights, Stream) : FT66RarityUtil::RollDefaultRarity(Stream);
			BagRarity = (static_cast<uint8>(R2) > static_cast<uint8>(BagRarity)) ? R2 : BagRarity;
		}
		if (RunState)
		{
			RunState->RecordLuckQualityRarity(FName(TEXT("EnemyLootBagRarity")), BagRarity);
		}
		const FName ItemID = T66GI->GetRandomItemIDForLootRarity(BagRarity);
		FActorSpawnParameters SpawnParams;
		SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
		AT66LootBagPickup* Loot = World->SpawnActor<AT66LootBagPickup>(AT66LootBagPickup::StaticClass(), GetActorLocation(), FRotator::ZeroRotator, SpawnParams);
		if (Loot)
		{
			Loot->SetLootRarity(BagRarity);
			Loot->SetItemID(ItemID);
		}
	}

	// [GOLD] Return to enemy pool instead of destroying.
	if (UT66EnemyPoolSubsystem* Pool = World->GetSubsystem<UT66EnemyPoolSubsystem>())
	{
		if (UT66ActorRegistrySubsystem* Registry = World->GetSubsystem<UT66ActorRegistrySubsystem>())
		{
			Registry->UnregisterEnemy(this);
		}
		Pool->Release(this);
		UE_LOG(LogTemp, Verbose, TEXT("[GOLD] EnemyPool: enemy %s returned to pool (after loot)"), *GetName());
	}
	else
	{
		// Fallback: deferred destruction.
		SetActorHiddenInGame(true);
		SetActorEnableCollision(false);
		SetLifeSpan(0.1f);
	}
}
