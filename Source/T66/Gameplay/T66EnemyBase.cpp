// Copyright Tribulation 66. All Rights Reserved.

#include "Gameplay/T66EnemyBase.h"
#include "Gameplay/T66CombatComponent.h"
#include "Gameplay/T66EnemyDirector.h"
#include "Gameplay/T66EnemyAIController.h"
#include "Gameplay/T66CircusInteractable.h"
#include "Gameplay/T66GameMode.h"
#include "Gameplay/T66LootBagPickup.h"
#include "Gameplay/T66HeroBase.h"
#include "Gameplay/T66HouseNPCBase.h"
#include "Gameplay/T66CombatHitZoneComponent.h"
#include "Core/T66CharacterVisualSubsystem.h"
#include "Core/T66AchievementsSubsystem.h"
#include "Core/T66RunStateSubsystem.h"
#include "Core/T66DamageLogSubsystem.h"
#include "Core/T66LagTrackerSubsystem.h"
#include "Core/T66ActorRegistrySubsystem.h"
#include "Core/T66EnemyPoolSubsystem.h"
#include "Core/T66FloatingCombatTextSubsystem.h"
#include "Core/T66GameInstance.h"
#include "Core/T66GameplayLayout.h"
#include "Core/T66PlayerExperienceSubSystem.h"
#include "Core/T66Rarity.h"
#include "Core/T66RngSubsystem.h"
#include "UI/T66EnemyHealthBarWidget.h"
#include "UI/T66EnemyLockWidget.h"
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

DEFINE_LOG_CATEGORY_STATIC(LogT66Enemy, Log, All);

namespace
{
	struct FT66SafeZoneHit
	{
		bool bInside = false;
		FVector Center = FVector::ZeroVector;
		float Radius = 0.f;
		float Penetration = -FLT_MAX;
	};

	void T66ConsiderSafeZoneHit(const FVector& QueryLocation, const FVector& SafeZoneCenter, const float SafeZoneRadius, FT66SafeZoneHit& InOutHit)
	{
		if (SafeZoneRadius <= KINDA_SMALL_NUMBER)
		{
			return;
		}

		const float Dist = FVector::Dist2D(QueryLocation, SafeZoneCenter);
		if (Dist >= SafeZoneRadius)
		{
			return;
		}

		const float Penetration = SafeZoneRadius - Dist;
		if (!InOutHit.bInside || Penetration > InOutHit.Penetration)
		{
			InOutHit.bInside = true;
			InOutHit.Center = SafeZoneCenter;
			InOutHit.Radius = SafeZoneRadius;
			InOutHit.Penetration = Penetration;
		}
	}

	FT66SafeZoneHit T66ResolveSafeZoneHit2D(const UWorld* World, const FVector& QueryLocation)
	{
		FT66SafeZoneHit Result;
		const UT66ActorRegistrySubsystem* Registry = World ? World->GetSubsystem<UT66ActorRegistrySubsystem>() : nullptr;
		const AT66GameMode* GameMode = World ? Cast<AT66GameMode>(World->GetAuthGameMode()) : nullptr;
		const bool bTowerLayout = GameMode && GameMode->IsUsingTowerMainMapLayout();
		const int32 QueryFloorNumber = bTowerLayout ? GameMode->GetTowerFloorIndexForLocation(QueryLocation) : INDEX_NONE;
		if (!Registry)
		{
			return Result;
		}

		for (const TWeakObjectPtr<AT66HouseNPCBase>& WeakNPC : Registry->GetNPCs())
		{
			const AT66HouseNPCBase* NPC = WeakNPC.Get();
			if (!NPC)
			{
				continue;
			}

			if (bTowerLayout && QueryFloorNumber != INDEX_NONE
				&& GameMode->GetTowerFloorIndexForLocation(NPC->GetActorLocation()) != QueryFloorNumber)
			{
				continue;
			}

			T66ConsiderSafeZoneHit(QueryLocation, NPC->GetActorLocation(), NPC->GetSafeZoneRadius(), Result);
		}

		for (const TWeakObjectPtr<AT66CircusInteractable>& WeakCircus : Registry->GetCircuses())
		{
			const AT66CircusInteractable* Circus = WeakCircus.Get();
			if (!Circus)
			{
				continue;
			}

			if (bTowerLayout && QueryFloorNumber != INDEX_NONE
				&& GameMode->GetTowerFloorIndexForLocation(Circus->GetActorLocation()) != QueryFloorNumber)
			{
				continue;
			}

			T66ConsiderSafeZoneHit(QueryLocation, Circus->GetActorLocation(), Circus->GetSafeZoneRadius(), Result);
		}

		return Result;
	}

	APawn* T66ResolveClosestPlayerPawn(const AActor* ContextActor)
	{
		const UWorld* World = ContextActor ? ContextActor->GetWorld() : nullptr;
		if (!World)
		{
			return nullptr;
		}

		const FVector Origin = ContextActor->GetActorLocation();
		APawn* BestPawn = nullptr;
		float BestDistSq = TNumericLimits<float>::Max();
		for (FConstPlayerControllerIterator It = World->GetPlayerControllerIterator(); It; ++It)
		{
			APawn* Pawn = It->Get() ? It->Get()->GetPawn() : nullptr;
			if (!Pawn)
			{
				continue;
			}

			const float DistSq = FVector::DistSquared2D(Origin, Pawn->GetActorLocation());
			if (DistSq < BestDistSq)
			{
				BestDistSq = DistSq;
				BestPawn = Pawn;
			}
		}

		return BestPawn;
	}

	void T66ApplyCharacterDisplacement(ACharacter* Character, const FVector& Origin, float Distance, const bool bTowardOrigin)
	{
		if (!Character || FMath::IsNearlyZero(Distance))
		{
			return;
		}

		FVector Dir = Origin - Character->GetActorLocation();
		Dir.Z = 0.f;
		const float Magnitude = Dir.Size();
		if (Magnitude <= KINDA_SMALL_NUMBER)
		{
			return;
		}

		Dir /= Magnitude;
		const float ClampedDistance = FMath::Clamp(FMath::Abs(Distance), 0.f, bTowardOrigin ? FMath::Max(0.f, Magnitude - 50.f) : FMath::Abs(Distance));
		if (ClampedDistance <= 0.f)
		{
			return;
		}

		const FVector Delta = (bTowardOrigin ? Dir : -Dir) * ClampedDistance;
		Character->SetActorLocation(Character->GetActorLocation() + Delta, true);
		if (UCharacterMovementComponent* Move = Character->GetCharacterMovement())
		{
			Move->StopMovementImmediately();
		}
	}
}

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
		Capsule->SetCollisionResponseToChannel(ECC_Pawn, ECR_Block); // no overlap with hero or other enemies
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
	HealthBarWidget->SetWidgetClass(UT66EnemyHealthBarWidget::StaticClass());

	LockIndicatorWidget = CreateDefaultSubobject<UWidgetComponent>(TEXT("LockIndicatorWidget"));
	LockIndicatorWidget->SetupAttachment(RootComponent);
	LockIndicatorWidget->SetRelativeLocation(FVector(0.f, 0.f, 225.f));
	LockIndicatorWidget->SetWidgetSpace(EWidgetSpace::Screen);
	LockIndicatorWidget->SetDrawAtDesiredSize(true);
	LockIndicatorWidget->SetDrawSize(FVector2D(52.f, 52.f));
	LockIndicatorWidget->SetWidgetClass(UT66EnemyLockWidget::StaticClass());
	LockIndicatorWidget->SetHiddenInGame(true, true);
	LockIndicatorWidget->SetVisibility(false, true);

	BodyHitZone = CreateDefaultSubobject<UT66CombatHitZoneComponent>(TEXT("BodyHitZone"));
	BodyHitZone->SetupAttachment(RootComponent);
	BodyHitZone->HitZoneType = ET66HitZoneType::Body;
	BodyHitZone->SetRelativeLocation(FVector(0.f, 0.f, 64.f));
	BodyHitZone->InitSphereRadius(42.f);

	HeadHitZone = CreateDefaultSubobject<UT66CombatHitZoneComponent>(TEXT("HeadHitZone"));
	HeadHitZone->SetupAttachment(RootComponent);
	HeadHitZone->HitZoneType = ET66HitZoneType::Head;
	HeadHitZone->SetRelativeLocation(FVector(0.f, 0.f, 124.f));
	HeadHitZone->InitSphereRadius(24.f);

	RefreshCombatHitZoneState();

	// Prepare built-in SkeletalMeshComponent for imported models.
	if (USkeletalMeshComponent* Skel = GetMesh())
	{
		Skel->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		Skel->SetVisibility(false, true); // shown only when a character visual mapping exists
	}
}

void AT66EnemyBase::RefreshCombatHitZoneState()
{
	const bool bEnableHitZones = bUsesCombatHitZones;

	if (UCapsuleComponent* Capsule = GetCapsuleComponent())
	{
		Capsule->SetCollisionResponseToChannel(ECC_Visibility, bEnableHitZones ? ECR_Ignore : ECR_Block);
	}

	auto ConfigureZone = [bEnableHitZones](UT66CombatHitZoneComponent* Zone, const ET66HitZoneType ZoneType)
	{
		if (!Zone)
		{
			return;
		}

		Zone->HitZoneType = ZoneType;
		Zone->bTargetable = bEnableHitZones;
		Zone->SetCollisionEnabled(bEnableHitZones ? ECollisionEnabled::QueryOnly : ECollisionEnabled::NoCollision);
		Zone->SetGenerateOverlapEvents(bEnableHitZones);
	};

	ConfigureZone(BodyHitZone, ET66HitZoneType::Body);
	ConfigureZone(HeadHitZone, ET66HitZoneType::Head);
}

ET66HitZoneType AT66EnemyBase::ResolveHitZoneType(const UPrimitiveComponent* HitComponent, const ET66HitZoneType PreferredZone) const
{
	if (bUsesCombatHitZones)
	{
		if (HitComponent && HeadHitZone && HitComponent == HeadHitZone)
		{
			return ET66HitZoneType::Head;
		}

		if (HitComponent && BodyHitZone && HitComponent == BodyHitZone)
		{
			return ET66HitZoneType::Body;
		}

		if (PreferredZone == ET66HitZoneType::Head && HeadHitZone && HeadHitZone->bTargetable)
		{
			return ET66HitZoneType::Head;
		}
	}

	return ET66HitZoneType::Body;
}

float AT66EnemyBase::GetHitZoneDamageMultiplier(const ET66HitZoneType HitZoneType) const
{
	return HitZoneType == ET66HitZoneType::Head
		? FMath::Max(0.1f, HeadDamageMultiplier)
		: FMath::Max(0.1f, BodyDamageMultiplier);
}

bool AT66EnemyBase::SupportsCombatHitZones() const
{
	return bUsesCombatHitZones && BodyHitZone && HeadHitZone;
}

FT66CombatTargetHandle AT66EnemyBase::ResolveCombatTargetHandle(const UPrimitiveComponent* HitComponent, const ET66HitZoneType PreferredZone) const
{
	FT66CombatTargetHandle Handle;
	Handle.Actor = const_cast<AT66EnemyBase*>(this);
	Handle.HitZoneType = ResolveHitZoneType(HitComponent, PreferredZone);

	UT66CombatHitZoneComponent* ZoneComponent = nullptr;
	if (Handle.HitZoneType == ET66HitZoneType::Head && HeadHitZone && HeadHitZone->bTargetable)
	{
		ZoneComponent = HeadHitZone;
	}
	else if (BodyHitZone && BodyHitZone->bTargetable)
	{
		ZoneComponent = BodyHitZone;
		Handle.HitZoneType = ET66HitZoneType::Body;
	}

	if (ZoneComponent)
	{
		Handle = ZoneComponent->MakeTargetHandle();
	}
	else
	{
		Handle.AimPoint = GetActorLocation();
		Handle.HitZoneType = ET66HitZoneType::Body;
	}

	return Handle;
}

FVector AT66EnemyBase::GetAimPointForHitZone(const ET66HitZoneType HitZoneType) const
{
	return ResolveCombatTargetHandle(nullptr, HitZoneType).AimPoint;
}

void AT66EnemyBase::ConfigureAsMob(FName InMobID)
{
	MobID = InMobID;
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

	const float Hue01 = static_cast<float>((H / 7u) % 360u) / 360.f;
	const FLinearColor C = FLinearColor::MakeFromHSV8(
		static_cast<uint8>(Hue01 * 255.f),
		210,
		240
	);
	FT66VisualUtil::ApplyT66Color(VisualMesh, this, C);

	switch (Shape)
	{
		case 1: VisualMesh->SetRelativeScale3D(FVector(0.75f, 0.75f, 0.75f)); break;
		case 2: VisualMesh->SetRelativeScale3D(FVector(0.70f, 0.70f, 0.95f)); break;
		case 3: VisualMesh->SetRelativeScale3D(FVector(0.80f, 0.80f, 0.95f)); break;
		default: VisualMesh->SetRelativeScale3D(FVector(0.85f, 0.85f, 0.85f)); break;
	}

	// Re-apply character visual for pooled (reused) actors whose BeginPlay already ran.
	if (HasActorBegunPlay() && !CharacterVisualID.IsNone() && CharacterVisualID != FName(TEXT("RegularEnemy")))
	{
		if (UWorld* World = GetWorld())
		{
			if (UGameInstance* GI = World->GetGameInstance())
			{
				if (UT66CharacterVisualSubsystem* Visuals = GI->GetSubsystem<UT66CharacterVisualSubsystem>())
				{
					bUsingCharacterVisual = Visuals->ApplyCharacterVisual(CharacterVisualID, GetMesh(), VisualMesh, true);
					if (USkeletalMeshComponent* Skel = GetMesh())
					{
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
	}
}

void AT66EnemyBase::ApplyMiniBossMultipliers(float HPScalar, float DamageScalar, float ScaleScalar)
{
	bIsMiniBoss = true;
	MiniBossHPScalarApplied = FMath::Max(0.1f, HPScalar);
	MiniBossDamageScalarApplied = FMath::Max(0.1f, DamageScalar);
	MiniBossScaleScalarApplied = FMath::Clamp(ScaleScalar, 1.0f, 4.0f);
	RebuildScaledCombatStats(true);
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

	if (UCharacterMovementComponent* Move = GetCharacterMovement())
	{
		Move->SetMovementMode(GetDefaultMovementMode());
		BaseMaxWalkSpeed = Move->MaxWalkSpeed;
	}

	if (!bBaseTuningInitialized)
	{
		BaseMaxHP = MaxHP;
		BaseTouchDamageHearts = TouchDamageHearts;
		BasePointValue = PointValue;
		BaseArmor = Armor;
		bBaseTuningInitialized = true;
	}
	// Ensure HP is valid on spawn (in case difficulty scaled before BeginPlay).
	CurrentHP = FMath::Clamp(CurrentHP, 1, MaxHP);
	ResetFamilyState();

	if (HealthBarWidget)
	{
		HealthBarWidget->InitWidget();
		HealthBarWidget->SetHiddenInGame(true, true);
		HealthBarWidget->SetVisibility(false, true);
	}
	if (LockIndicatorWidget)
	{
		LockIndicatorWidget->InitWidget();
		LockIndicatorWidget->SetHiddenInGame(true, true);
		LockIndicatorWidget->SetVisibility(false, true);
	}

	UCapsuleComponent* Capsule = GetCapsuleComponent();
	if (Capsule)
	{
		Capsule->OnComponentBeginOverlap.AddDynamic(this, &AT66EnemyBase::OnCapsuleBeginOverlap);
	}

	RefreshCombatHitZoneState();

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
			UE_LOG(LogT66Enemy, Verbose, TEXT("EnemyVisuals: %s VisualID=%s UsingPlaceholder=1"), *GetName(), *CharacterVisualID.ToString());
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
		UE_LOG(LogT66Enemy, Verbose, TEXT("EnemyVisuals: %s VisualID=%s UsingSkel=%d SkelAsset=%s SkelVisible=%d SkelHidden=%d PlaceholderVisible=%d PlaceholderHidden=%d"),
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

void AT66EnemyBase::RebuildScaledCombatStats(const bool bResetCurrentHPToMax)
{
	if (!bBaseTuningInitialized)
	{
		BaseMaxHP = MaxHP;
		BaseTouchDamageHearts = TouchDamageHearts;
		BasePointValue = PointValue;
		BaseArmor = Armor;
		bBaseTuningInitialized = true;
	}

	const float PreviousMax = static_cast<float>(FMath::Max(1, MaxHP));
	const float PreviousCurrent = static_cast<float>(FMath::Clamp(CurrentHP, 0, MaxHP));
	const float PreviousHealthPercent = FMath::Clamp(PreviousCurrent / PreviousMax, 0.f, 1.f);

	int32 ResolvedMaxHP = BaseMaxHP;
	float ResolvedArmor = BaseArmor;
	if (!bStageScalingApplied)
	{
		ResolvedMaxHP = FMath::Max(1, FMath::RoundToInt(static_cast<float>(BaseMaxHP) * DifficultyScalarApplied));
		ResolvedArmor = static_cast<float>(BaseArmor) * DifficultyScalarApplied;
	}

	ResolvedMaxHP = FMath::Max(1, FMath::RoundToInt(static_cast<float>(ResolvedMaxHP) * ProgressionEnemyScalarApplied));
	ResolvedMaxHP = FMath::Max(1, FMath::RoundToInt(static_cast<float>(ResolvedMaxHP) * FinaleScalarApplied));
	if (const UT66GameInstance* T66GI = Cast<UT66GameInstance>(GetGameInstance()))
	{
		const float DailyEnemyHpMultiplier = T66GI->GetDailyClimbFloatRuleValue(ET66DailyClimbRuleType::EnemyHpMultiplier, 1.0f);
		if (DailyEnemyHpMultiplier > 0.0f && !FMath::IsNearlyEqual(DailyEnemyHpMultiplier, 1.0f))
		{
			ResolvedMaxHP = FMath::Max(1, FMath::RoundToInt(static_cast<float>(ResolvedMaxHP) * DailyEnemyHpMultiplier));
		}
	}

	int32 ResolvedTouchDamage = FMath::Max(1, FMath::RoundToInt(static_cast<float>(BaseTouchDamageHearts) * DifficultyScalarApplied));
	ResolvedTouchDamage = FMath::Max(1, FMath::RoundToInt(static_cast<float>(ResolvedTouchDamage) * ProgressionEnemyScalarApplied));
	ResolvedTouchDamage = FMath::Max(1, FMath::RoundToInt(static_cast<float>(ResolvedTouchDamage) * FinaleScalarApplied));

	if (bIsMiniBoss)
	{
		ResolvedMaxHP = FMath::Max(1, FMath::RoundToInt(static_cast<float>(ResolvedMaxHP) * MiniBossHPScalarApplied));
		ResolvedTouchDamage = FMath::Max(1, FMath::RoundToInt(static_cast<float>(ResolvedTouchDamage) * MiniBossDamageScalarApplied));
		SetActorScale3D(FVector(MiniBossScaleScalarApplied, MiniBossScaleScalarApplied, MiniBossScaleScalarApplied));
	}
	else
	{
		SetActorScale3D(FVector::OneVector);
	}

	MaxHP = FMath::Max(1, ResolvedMaxHP);
	Armor = FMath::Clamp(ResolvedArmor, 0.f, 0.95f);
	TouchDamageHearts = ResolvedTouchDamage;

	if (bResetCurrentHPToMax)
	{
		CurrentHP = MaxHP;
	}
	else if (CurrentHP > 0)
	{
		CurrentHP = FMath::Clamp(FMath::RoundToInt(static_cast<float>(MaxHP) * PreviousHealthPercent), 1, MaxHP);
	}
	else
	{
		CurrentHP = 0;
	}

	UpdateHealthBar();
}

void AT66EnemyBase::ApplyStageScaling(int32 Stage)
{
	const int32 ClampedStage = FMath::Clamp(Stage, 1, 999);
	const float Multiplier = FMath::Pow(1.1f, static_cast<float>(ClampedStage - 1));

	BaseMaxHP = FMath::Max(1, FMath::RoundToInt(50.f * Multiplier));
	BaseArmor = 0.1f * Multiplier;

	if (!bBaseTuningInitialized)
	{
		BaseTouchDamageHearts = TouchDamageHearts;
		BasePointValue = PointValue;
		BaseArmor = 0.1f * Multiplier;
		bBaseTuningInitialized = true;
	}
	bStageScalingApplied = true;
	RebuildScaledCombatStats(true);
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

	DifficultyScalarApplied = FMath::Clamp(Scalar, 1.0f, 99.0f);
	RebuildScaledCombatStats(false);
}

void AT66EnemyBase::ApplyProgressionEnemyScalar(float Scalar)
{
	ProgressionEnemyScalarApplied = FMath::Clamp(Scalar, 1.0f, 99.0f);
	RebuildScaledCombatStats(false);
}

void AT66EnemyBase::ApplyFinaleScaling(float Scalar)
{
	FinaleScalarApplied = FMath::Clamp(Scalar, 1.0f, 99.0f);
	RebuildScaledCombatStats(false);
}

void AT66EnemyBase::FreezeScoreAwardAtSpawn(const float DifficultyScalar)
{
	UGameInstance* GI = GetGameInstance();
	const UT66PlayerExperienceSubSystem* PlayerExperience = GI ? GI->GetSubsystem<UT66PlayerExperienceSubSystem>() : nullptr;
	const UT66GameInstance* T66GI = Cast<UT66GameInstance>(GI);
	const ET66Difficulty Difficulty = T66GI ? T66GI->SelectedDifficulty : ET66Difficulty::Easy;
	ResolvedScoreAward = PlayerExperience
		? PlayerExperience->ResolveEnemyScoreAtSpawn(Difficulty, BasePointValue, DifficultyScalar)
		: FMath::Max(0, FMath::RoundToInt(static_cast<float>(BasePointValue) * FMath::Max(1.0f, DifficultyScalar)));
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
	UE_LOG(LogT66Enemy, Verbose, TEXT("[GOLD] EnemyPool: ResetForReuse %s at (%.0f, %.0f, %.0f)"),
		*GetName(), NewLocation.X, NewLocation.Y, NewLocation.Z);

	// Restore base state for reuse.
	OwningDirector = NewDirector;
	CurrentHP = MaxHP;
	bIsConfused = false;
	ConfusionSecondsRemaining = 0.f;
	ArmorDebuffAmount = 0.f;
	ArmorDebuffSecondsRemaining = 0.f;
	MoveSlowMultiplier = 1.f;
	MoveSlowSecondsRemaining = 0.f;
	ForcedRunAwaySecondsRemaining = 0.f;
	StunSecondsRemaining = 0.f;
	RootSecondsRemaining = 0.f;
	FreezeSecondsRemaining = 0.f;
	AutoAttackKnockbackSecondsRemaining = 0.f;
	bIsMiniBoss = false;
	MiniBossHPScalarApplied = 1.0f;
	MiniBossDamageScalarApplied = 1.0f;
	MiniBossScaleScalarApplied = 1.0f;
	DifficultyScalarApplied = 1.0f;
	ProgressionEnemyScalarApplied = 1.0f;
	FinaleScalarApplied = 1.0f;
	ResolvedScoreAward = 0;
	SetActorScale3D(FVector::OneVector);
	LastTouchDamageTime = -9999.f;
	CachedPlayerPawn = nullptr;
	CachedWanderDir = FVector::ZeroVector;
	WanderDirRefreshAccum = 0.f;
	bCachedInsideSafeZone = false;
	CachedSafeZoneEscapeDir = FVector::ZeroVector;
	CachedSafeZoneCenter = FVector::ZeroVector;
	CachedSafeZoneRadius = 0.f;
	CachedSafeZoneLoiterDir = FVector::ZeroVector;
	SafeZoneLoiterDirRefreshAccum = 0.f;
	SafeZoneCheckAccumSeconds = 0.f;
	bRisingFromGround = false;
	bEmergingFromWall = false;
	RiseElapsed = 0.f;
	WallEmergeElapsed = 0.f;
	SetLockedIndicator(false);
	RefreshCombatHitZoneState();

	SetActorLocation(NewLocation);
	SetActorHiddenInGame(false);
	SetActorEnableCollision(true);
	SetActorTickEnabled(true);

	if (UCharacterMovementComponent* Move = GetCharacterMovement())
	{
		Move->SetComponentTickEnabled(true);
		Move->SetMovementMode(GetDefaultMovementMode());
	}

	// Re-register with the actor registry (we unregistered when released to pool).
	if (UWorld* W = GetWorld())
	{
		if (UT66ActorRegistrySubsystem* Registry = W->GetSubsystem<UT66ActorRegistrySubsystem>())
		{
			Registry->RegisterEnemy(this);
		}
	}

	ResetFamilyState();
	UpdateHealthBar();
}

void AT66EnemyBase::StartRiseFromGround(float TargetGroundZ)
{
	RiseTargetZ = TargetGroundZ;
	RiseStartZ = TargetGroundZ - BuryDepth;
	RiseElapsed = 0.f;
	bRisingFromGround = true;
	bEmergingFromWall = false;

	FVector Loc = GetActorLocation();
	Loc.Z = RiseStartZ;
	SetActorLocation(Loc);

	if (UCapsuleComponent* Cap = GetCapsuleComponent())
	{
		Cap->SetCollisionResponseToChannel(ECC_Pawn, ECR_Ignore);
	}
}

void AT66EnemyBase::StartEmergeFromWall(const FVector& TargetLocation, const FVector& WallNormal)
{
	FVector InwardNormal = WallNormal;
	InwardNormal.Z = 0.f;
	if (!InwardNormal.Normalize())
	{
		InwardNormal = FVector(1.0f, 0.0f, 0.0f);
	}

	WallEmergeTargetLocation = TargetLocation;
	WallEmergeStartLocation = TargetLocation - (InwardNormal * BuryDepth);
	WallEmergeStartLocation.Z = TargetLocation.Z;
	WallEmergeElapsed = 0.f;
	bEmergingFromWall = true;
	bRisingFromGround = false;
	SetActorLocation(WallEmergeStartLocation);

	if (UCapsuleComponent* Cap = GetCapsuleComponent())
	{
		Cap->SetCollisionResponseToChannel(ECC_Pawn, ECR_Ignore);
	}
}

void AT66EnemyBase::ResetFamilyState()
{
}

void AT66EnemyBase::TickFamilyBehavior(APawn* PlayerPawn, float DeltaSeconds, float Dist2DToPlayer, const bool bShouldRunAwayFromPlayer)
{
	if (!PlayerPawn)
	{
		return;
	}

	FVector ToPlayer = PlayerPawn->GetActorLocation() - GetActorLocation();
	ToPlayer.Z = 0.f;
	if (Dist2DToPlayer > 10.f && ToPlayer.Normalize())
	{
		AddMovementInput(bShouldRunAwayFromPlayer ? -ToPlayer : ToPlayer, 1.f);
	}
}

void AT66EnemyBase::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);
	if (CurrentHP <= 0) return;

	FLagScopedScope LagScope(GetWorld(), TEXT("EnemyBase::Tick"));

	if (bEmergingFromWall)
	{
		WallEmergeElapsed += DeltaSeconds;
		const float Alpha = FMath::Clamp(WallEmergeElapsed / RiseDuration, 0.f, 1.f);
		SetActorLocation(FMath::Lerp(WallEmergeStartLocation, WallEmergeTargetLocation, Alpha));

		if (Alpha >= 1.f)
		{
			bEmergingFromWall = false;
			if (UCapsuleComponent* Cap = GetCapsuleComponent())
			{
				Cap->SetCollisionResponseToChannel(ECC_Pawn, ECR_Block);
			}
		}
		return;
	}

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
				Cap->SetCollisionResponseToChannel(ECC_Pawn, ECR_Block);
			}
		}
		return;
	}

	APawn* PlayerPawn = T66ResolveClosestPlayerPawn(this);
	CachedPlayerPawn = PlayerPawn;
	UE_LOG(LogT66Enemy, Verbose, TEXT("[GOLD] PlayerPawnCache: enemy %s resolved pawn %s"),
		*GetName(), PlayerPawn ? *PlayerPawn->GetName() : TEXT("null"));
	if (!PlayerPawn) return;

	UCharacterMovementComponent* Move = GetCharacterMovement();
	if (!Move) return;

	if (AutoAttackKnockbackSecondsRemaining > 0.f)
	{
		AutoAttackKnockbackSecondsRemaining = FMath::Max(0.f, AutoAttackKnockbackSecondsRemaining - DeltaSeconds);
		if (AutoAttackKnockbackSecondsRemaining <= 0.f)
		{
			Move->StopMovementImmediately();
		}
		return;
	}

	const float Dist2DToPlayer = FVector::Dist2D(GetActorLocation(), PlayerPawn->GetActorLocation());
	const bool bPlayerInsideReservedTraversalZone = T66GameplayLayout::IsInsideReservedTraversalZone2D(
		PlayerPawn->GetActorLocation(),
		220.f);

	// Safe zone rule: enemies cannot enter NPC safe bubbles.
	// Perf: evaluate bubble membership at a low frequency (not every Tick).
	SafeZoneCheckAccumSeconds += DeltaSeconds;
	if (bPlayerInsideReservedTraversalZone)
	{
		bCachedInsideSafeZone = false;
		CachedSafeZoneEscapeDir = FVector::ZeroVector;
		CachedSafeZoneCenter = FVector::ZeroVector;
		CachedSafeZoneRadius = 0.f;
		CachedSafeZoneLoiterDir = FVector::ZeroVector;
		SafeZoneLoiterDirRefreshAccum = 0.f;
	}
	else if (SafeZoneCheckAccumSeconds >= SafeZoneCheckIntervalSeconds)
	{
		SafeZoneCheckAccumSeconds = 0.f;
		bCachedInsideSafeZone = false;
		CachedSafeZoneEscapeDir = FVector::ZeroVector;
		CachedSafeZoneCenter = FVector::ZeroVector;
		CachedSafeZoneRadius = 0.f;

		const FT66SafeZoneHit SafeZoneHit = T66ResolveSafeZoneHit2D(GetWorld(), GetActorLocation());
		if (SafeZoneHit.bInside)
		{
			bCachedInsideSafeZone = true;
			CachedSafeZoneCenter = SafeZoneHit.Center;
			CachedSafeZoneRadius = SafeZoneHit.Radius;

			FVector ToEnemy = GetActorLocation() - SafeZoneHit.Center;
			ToEnemy.Z = 0.f;
			CachedSafeZoneEscapeDir = ToEnemy.GetSafeNormal();
			if (CachedSafeZoneEscapeDir.IsNearlyZero())
			{
				CachedSafeZoneEscapeDir = FVector(1.f, 0.f, 0.f);
			}
		}
		else
		{
			CachedSafeZoneLoiterDir = FVector::ZeroVector;
			SafeZoneLoiterDirRefreshAccum = 0.f;
		}
	}

	if (bCachedInsideSafeZone)
	{
		if (Move)
		{
			Move->MaxWalkSpeed = BaseMaxWalkSpeed * SafeZoneLoiterMoveScale;
		}

		SafeZoneLoiterDirRefreshAccum += DeltaSeconds;
		if (SafeZoneLoiterDirRefreshAccum >= SafeZoneLoiterDirRefreshInterval || CachedSafeZoneLoiterDir.IsNearlyZero())
		{
			SafeZoneLoiterDirRefreshAccum = 0.f;

			FVector Outward = CachedSafeZoneEscapeDir;
			if (Outward.IsNearlyZero())
			{
				Outward = FVector(1.f, 0.f, 0.f);
			}

			const FVector Tangent(-Outward.Y, Outward.X, 0.f);
			FVector RandomDir(FMath::FRandRange(-1.f, 1.f), FMath::FRandRange(-1.f, 1.f), 0.f);
			RandomDir = RandomDir.GetSafeNormal();
			if (RandomDir.IsNearlyZero())
			{
				RandomDir = Tangent;
			}

			const float DistToCenter = FVector::Dist2D(GetActorLocation(), CachedSafeZoneCenter);
			const float DepthAlpha = (CachedSafeZoneRadius > KINDA_SMALL_NUMBER)
				? FMath::Clamp((CachedSafeZoneRadius - DistToCenter) / CachedSafeZoneRadius, 0.f, 1.f)
				: 1.f;
			const float OutwardWeight = FMath::Lerp(0.20f, 0.90f, DepthAlpha);
			const float TangentWeight = FMath::Lerp(0.80f, 0.35f, DepthAlpha);
			const float RandomWeight = 0.45f;
			const float TangentSign = (FMath::FRand() < 0.5f) ? -1.f : 1.f;

			CachedSafeZoneLoiterDir = (Outward * OutwardWeight + Tangent * TangentSign * TangentWeight + RandomDir * RandomWeight).GetSafeNormal();
			if (CachedSafeZoneLoiterDir.IsNearlyZero())
			{
				CachedSafeZoneLoiterDir = Outward;
			}
		}

		if (!CachedSafeZoneLoiterDir.IsNearlyZero())
		{
			AddMovementInput(CachedSafeZoneLoiterDir, 1.f);
		}
		return;
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

	if (MoveSlowSecondsRemaining > 0.f)
	{
		MoveSlowSecondsRemaining = FMath::Max(0.f, MoveSlowSecondsRemaining - DeltaSeconds);
		if (MoveSlowSecondsRemaining <= 0.f)
		{
			MoveSlowMultiplier = 1.f;
		}
	}

	if (ForcedRunAwaySecondsRemaining > 0.f)
	{
		ForcedRunAwaySecondsRemaining = FMath::Max(0.f, ForcedRunAwaySecondsRemaining - DeltaSeconds);
	}

	if (StunSecondsRemaining > 0.f)
	{
		StunSecondsRemaining = FMath::Max(0.f, StunSecondsRemaining - DeltaSeconds);
	}

	if (RootSecondsRemaining > 0.f)
	{
		RootSecondsRemaining = FMath::Max(0.f, RootSecondsRemaining - DeltaSeconds);
	}

	if (FreezeSecondsRemaining > 0.f)
	{
		FreezeSecondsRemaining = FMath::Max(0.f, FreezeSecondsRemaining - DeltaSeconds);
	}

	const bool bShouldRunAwayFromPlayer = bRunAwayFromPlayer || ForcedRunAwaySecondsRemaining > 0.f;

	float FarChaseMultiplier = 1.f;
	if (!bShouldRunAwayFromPlayer && LeashMaxDistance > 0.f && Dist2DToPlayer > LeashMaxDistance)
	{
		const float RampDistance = FMath::Max(1.f, FarChaseRampDistance);
		const float RampAlpha = FMath::Clamp((Dist2DToPlayer - LeashMaxDistance) / RampDistance, 0.f, 1.f);
		FarChaseMultiplier = FMath::Lerp(1.f, FMath::Max(1.f, FarChaseSpeedMultiplier), RampAlpha);
	}
	if (UCharacterMovementComponent* MoveComp = GetCharacterMovement())
	{
		const float ControlMultiplier = (FreezeSecondsRemaining > 0.f) ? 0.f : MoveSlowMultiplier;
		MoveComp->MaxWalkSpeed = BaseMaxWalkSpeed * ControlMultiplier * FarChaseMultiplier;
	}

	if (FreezeSecondsRemaining > 0.f || StunSecondsRemaining > 0.f)
	{
		Move->StopMovementImmediately();
		return;
	}

	if (RootSecondsRemaining > 0.f)
	{
		Move->StopMovementImmediately();
		return;
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

	TickFamilyBehavior(PlayerPawn, DeltaSeconds, Dist2DToPlayer, bShouldRunAwayFromPlayer);
}

void AT66EnemyBase::OnCapsuleBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
	UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if (!OtherActor) return;
	AT66HeroBase* Hero = Cast<AT66HeroBase>(OtherActor);
	if (!Hero) return;
	if (Hero->IsVehicleMounted()) return;
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
	const int32 DamageHP = 20;
	RunState->ApplyDamage(DamageHP, this);
}

bool AT66EnemyBase::ApplyResolvedDamage(int32 Damage, const bool bCreditHeroKill, FName DamageSourceID, FName EventType)
{
	if (Damage <= 0 || CurrentHP <= 0)
	{
		return false;
	}

	const float EffectiveArmor = GetEffectiveArmor();
	const int32 ReducedDamage = FMath::Max(1, FMath::RoundToInt(static_cast<float>(Damage) * (1.f - EffectiveArmor)));
	const FName SourceID = DamageSourceID.IsNone() ? UT66DamageLogSubsystem::SourceID_AutoAttack : DamageSourceID;

	if (UWorld* World = GetWorld())
	{
		if (UGameInstance* GI = World->GetGameInstance())
		{
			if (bCreditHeroKill)
			{
				if (UT66DamageLogSubsystem* DamageLog = GI->GetSubsystem<UT66DamageLogSubsystem>())
				{
					DamageLog->RecordDamageDealt(SourceID, ReducedDamage);
				}
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
		{
			if (bCreditHeroKill)
			{
				if (UGameInstance* GI = World->GetGameInstance())
				{
					if (UT66RunStateSubsystem* RunState = GI->GetSubsystem<UT66RunStateSubsystem>())
					{
						RunState->NotifyEnemyKilledByHero();
					}
				}
			}

			UT66CombatComponent::SpawnDeathBurstAtLocation(World, GetActorLocation(), 16, 60.f);
		}

		OnDeath();
		return true;
	}

	return false;
}

bool AT66EnemyBase::TakeDamageFromHero(int32 Damage, FName DamageSourceID, FName EventType)
{
	return ApplyResolvedDamage(Damage, true, DamageSourceID, EventType);
}

bool AT66EnemyBase::TakeDamageFromHeroHitZone(int32 Damage, const FT66CombatTargetHandle& TargetHandle, FName DamageSourceID, FName EventType)
{
	const ET66HitZoneType HitZoneType = ResolveHitZoneType(TargetHandle.HitComponent.Get(), TargetHandle.HitZoneType);
	const int32 AdjustedDamage = FMath::Max(1, FMath::RoundToInt(static_cast<float>(Damage) * GetHitZoneDamageMultiplier(HitZoneType)));

	FName ResolvedEventType = EventType;
	if (HitZoneType == ET66HitZoneType::Head && ResolvedEventType.IsNone())
	{
		ResolvedEventType = UT66FloatingCombatTextSubsystem::EventType_Headshot;
	}

	return TakeDamageFromHero(AdjustedDamage, DamageSourceID, ResolvedEventType);
}

bool AT66EnemyBase::TakeDamageFromEnvironment(int32 Damage, AActor* DamageCauser, FName EventType)
{
	(void)DamageCauser;
	return ApplyResolvedDamage(Damage, false, NAME_None, EventType);
}

void AT66EnemyBase::ApplyAutoAttackKnockback(const FVector& HitOrigin, float StrengthScale)
{
	if (CurrentHP <= 0 || bRisingFromGround || bEmergingFromWall)
	{
		return;
	}

	const float KnockbackSpeed = AutoAttackKnockbackSpeed * FMath::Max(0.f, StrengthScale);
	if (KnockbackSpeed <= 0.f || AutoAttackKnockbackStutterSeconds <= 0.f)
	{
		return;
	}

	FVector AwayFromHit = GetActorLocation() - HitOrigin;
	AwayFromHit.Z = 0.f;
	if (!AwayFromHit.Normalize())
	{
		AwayFromHit = -GetActorForwardVector();
		AwayFromHit.Z = 0.f;
		if (!AwayFromHit.Normalize())
		{
			return;
		}
	}

	if (UCharacterMovementComponent* Move = GetCharacterMovement())
	{
		Move->StopMovementImmediately();
		Move->Velocity = AwayFromHit * KnockbackSpeed;
	}

	AutoAttackKnockbackSecondsRemaining = FMath::Max(AutoAttackKnockbackSecondsRemaining, AutoAttackKnockbackStutterSeconds);
}

float AT66EnemyBase::GetEffectiveArmor() const
{
	return FMath::Clamp(Armor - ArmorDebuffAmount, -0.5f, 0.95f);
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

void AT66EnemyBase::ApplyMoveSlow(float SpeedMultiplier, float DurationSeconds)
{
	const float Mult = FMath::Clamp(SpeedMultiplier, 0.1f, 1.f);
	const float Dur = FMath::Clamp(DurationSeconds, 0.f, 30.f);
	if (Dur <= 0.f) return;
	MoveSlowMultiplier = FMath::Min(MoveSlowMultiplier, Mult);
	MoveSlowSecondsRemaining = FMath::Max(MoveSlowSecondsRemaining, Dur);
}

void AT66EnemyBase::ApplyForcedRunAway(float DurationSeconds)
{
	const float Dur = FMath::Clamp(DurationSeconds, 0.f, 30.f);
	if (Dur <= 0.f || CurrentHP <= 0)
	{
		return;
	}

	ForcedRunAwaySecondsRemaining = FMath::Max(ForcedRunAwaySecondsRemaining, Dur);
}

void AT66EnemyBase::ApplyStun(float DurationSeconds)
{
	const float Dur = FMath::Clamp(DurationSeconds, 0.f, 5.f);
	if (Dur <= 0.f || CurrentHP <= 0 || bRisingFromGround || bEmergingFromWall)
	{
		return;
	}

	StunSecondsRemaining = FMath::Max(StunSecondsRemaining, Dur);
	if (UCharacterMovementComponent* Move = GetCharacterMovement())
	{
		Move->StopMovementImmediately();
	}
}

void AT66EnemyBase::ApplyRoot(float DurationSeconds)
{
	const float Dur = FMath::Clamp(DurationSeconds, 0.f, 6.f);
	if (Dur <= 0.f || CurrentHP <= 0 || bRisingFromGround || bEmergingFromWall)
	{
		return;
	}

	RootSecondsRemaining = FMath::Max(RootSecondsRemaining, Dur);
	if (UCharacterMovementComponent* Move = GetCharacterMovement())
	{
		Move->StopMovementImmediately();
	}
}

void AT66EnemyBase::ApplyFreeze(float DurationSeconds)
{
	const float Dur = FMath::Clamp(DurationSeconds, 0.f, 4.f);
	if (Dur <= 0.f || CurrentHP <= 0 || bRisingFromGround || bEmergingFromWall)
	{
		return;
	}

	FreezeSecondsRemaining = FMath::Max(FreezeSecondsRemaining, Dur);
	if (UCharacterMovementComponent* Move = GetCharacterMovement())
	{
		Move->StopMovementImmediately();
	}
}

void AT66EnemyBase::ApplyPullTowards(const FVector& PullOrigin, float Distance)
{
	if (CurrentHP <= 0 || bRisingFromGround || bEmergingFromWall)
	{
		return;
	}

	T66ApplyCharacterDisplacement(this, PullOrigin, Distance, true);
}

void AT66EnemyBase::ApplyPushAwayFrom(const FVector& PushOrigin, float Distance)
{
	if (CurrentHP <= 0 || bRisingFromGround || bEmergingFromWall)
	{
		return;
	}

	T66ApplyCharacterDisplacement(this, PushOrigin, Distance, false);
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
	if (LockIndicatorWidget)
	{
		if (bLocked && !LockIndicatorWidget->GetUserWidgetObject())
		{
			LockIndicatorWidget->InitWidget();
		}
		LockIndicatorWidget->SetHiddenInGame(!bLocked, true);
		LockIndicatorWidget->SetVisibility(bLocked, true);
	}

	if (!HealthBarWidget) return;
	UT66EnemyHealthBarWidget* W = Cast<UT66EnemyHealthBarWidget>(HealthBarWidget->GetUserWidgetObject());
	if (!W) return;
	W->SetLocked(bLocked);
}

void AT66EnemyBase::OnDeath()
{
	SetLockedIndicator(false);

	UWorld* World = GetWorld();
	UGameInstance* GI = World ? World->GetGameInstance() : nullptr;
	UT66RunStateSubsystem* RunState = GI ? GI->GetSubsystem<UT66RunStateSubsystem>() : nullptr;
	UT66AchievementsSubsystem* Achievements = GI ? GI->GetSubsystem<UT66AchievementsSubsystem>() : nullptr;
	if (RunState)
	{
		const int32 AwardPoints = FMath::Max(0, ResolvedScoreAward);
		int32 AwardXP = XPValue;
		if (AT66GameMode* GameMode = World ? Cast<AT66GameMode>(World->GetAuthGameMode()) : nullptr)
		{
			if (GameMode->IsUsingTowerMainMapLayout())
			{
				AwardXP = FMath::Max(1, FMath::RoundToInt(static_cast<float>(XPValue) * 0.5f));
			}
		}
		RunState->AddEnemyKillScore(AwardPoints);
		RunState->AddHeroXP(AwardXP);
		RunState->AddStructuredEvent(ET66RunEventType::EnemyKilled, FString::Printf(TEXT("Score=%d,XP=%d"), AwardPoints, AwardXP));
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
		// Loot bag drop chance + rarity are driven by player-experience tuning and luck bias.
		UT66RngSubsystem* RngSub = GI ? GI->GetSubsystem<UT66RngSubsystem>() : nullptr;
		UT66PlayerExperienceSubSystem* PlayerExperience = GI ? GI->GetSubsystem<UT66PlayerExperienceSubSystem>() : nullptr;
		if (RngSub && RunState)
		{
			RngSub->UpdateLuckStat(RunState->GetEffectiveLuckBiasStat());
		}

		FRandomStream LocalRng(FPlatformTime::Cycles());
		FRandomStream& Stream = RngSub ? RngSub->GetRunStream() : LocalRng;
		const ET66Difficulty Difficulty = T66GI->SelectedDifficulty;
		const float BaseDropChance = PlayerExperience
			? PlayerExperience->GetDifficultyEnemyLootBagDropChanceBase(Difficulty)
			: 0.10f;
		const float DropChance = RngSub ? RngSub->BiasChance01(BaseDropChance) : FMath::Clamp(BaseDropChance, 0.f, 1.f);
		const bool bDroppedBag = RngSub ? RngSub->RollChance01(DropChance) : (Stream.GetFraction() < DropChance);
		const int32 DropDrawIndex = RngSub ? RngSub->GetLastRunDrawIndex() : INDEX_NONE;
		const int32 DropPreDrawSeed = RngSub ? RngSub->GetLastRunPreDrawSeed() : 0;
		if (RunState)
		{
			RunState->RecordLuckQuantityBool(FName(TEXT("EnemyLootBagDrop")), bDroppedBag, DropChance, DropDrawIndex, DropPreDrawSeed);
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
				UE_LOG(LogT66Enemy, Verbose, TEXT("[GOLD] EnemyPool: enemy %s returned to pool (no loot drop)"), *GetName());
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

		const FT66IntRange BagCountRange = PlayerExperience
			? PlayerExperience->GetDifficultyEnemyLootBagCountOnDrop(Difficulty)
			: FT66IntRange{ 1, 1 };
		const int32 BagCountMin = FMath::Max(1, FMath::Min(BagCountRange.Min, BagCountRange.Max));
		const int32 BagCountMax = FMath::Max(BagCountMin, FMath::Max(BagCountRange.Min, BagCountRange.Max));
		const int32 LootBagCount = RngSub
			? FMath::Max(1, RngSub->RollIntRangeBiased(BagCountRange, Stream))
			: FMath::Max(1, Stream.RandRange(BagCountMin, BagCountMax));
		const float DailyLootBagMultiplier = T66GI->GetDailyClimbFloatRuleValue(ET66DailyClimbRuleType::EnemyLootBagCountMultiplier, 1.0f);
		const int32 AdjustedLootBagCount = (DailyLootBagMultiplier > 0.0f && !FMath::IsNearlyEqual(DailyLootBagMultiplier, 1.0f))
			? FMath::Max(1, FMath::RoundToInt(static_cast<float>(LootBagCount) * DailyLootBagMultiplier))
			: LootBagCount;
		const int32 BagCountDrawIndex = RngSub ? RngSub->GetLastRunDrawIndex() : INDEX_NONE;
		const int32 BagCountPreDrawSeed = RngSub ? RngSub->GetLastRunPreDrawSeed() : 0;
		if (RunState)
		{
			RunState->RecordLuckQuantityRoll(FName(TEXT("EnemyLootBagCount")), AdjustedLootBagCount, BagCountMin, BagCountMax, BagCountDrawIndex, BagCountPreDrawSeed);
		}

		const FT66RarityWeights Weights = PlayerExperience
			? PlayerExperience->GetDifficultyEnemyLootBagRarityWeights(Difficulty)
			: FT66RarityWeights{};
		FActorSpawnParameters SpawnParams;
		SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
		for (int32 BagIndex = 0; BagIndex < AdjustedLootBagCount; ++BagIndex)
		{
			ET66Rarity BagRarity = RngSub ? RngSub->RollRarityWeighted(Weights, Stream) : FT66RarityUtil::RollDefaultRarity(Stream);
			const int32 BagRarityDrawIndex = RngSub ? RngSub->GetLastRunDrawIndex() : INDEX_NONE;
			const int32 BagRarityPreDrawSeed = RngSub ? RngSub->GetLastRunPreDrawSeed() : 0;
			if (bIsMiniBoss)
			{
				// Mini-bosses should skew toward higher tier loot bags.
				const ET66Rarity R2 = RngSub ? RngSub->RollRarityWeighted(Weights, Stream) : FT66RarityUtil::RollDefaultRarity(Stream);
				BagRarity = (static_cast<uint8>(R2) > static_cast<uint8>(BagRarity)) ? R2 : BagRarity;
			}
			if (RunState)
			{
				RunState->RecordLuckQualityRarity(
					FName(TEXT("EnemyLootBagRarity")),
					BagRarity,
					bIsMiniBoss ? INDEX_NONE : BagRarityDrawIndex,
					bIsMiniBoss ? 0 : BagRarityPreDrawSeed,
					bIsMiniBoss ? nullptr : &Weights);
			}

			const FName ItemID = T66GI->GetRandomItemIDForLootRarityFromStream(BagRarity, Stream);
			const float Angle = RngSub ? RngSub->RunFRandRange(0.f, 2.f * PI) : Stream.FRandRange(0.f, 2.f * PI);
			const float Radius = (AdjustedLootBagCount > 1)
				? (RngSub ? RngSub->RunFRandRange(0.f, 90.f) : Stream.FRandRange(0.f, 90.f))
				: 0.f;
			const FVector Offset(FMath::Cos(Angle) * Radius, FMath::Sin(Angle) * Radius, 0.f);
			AT66LootBagPickup* Loot = World->SpawnActor<AT66LootBagPickup>(
				AT66LootBagPickup::StaticClass(),
				GetActorLocation() + Offset,
				FRotator::ZeroRotator,
				SpawnParams);
			if (Loot)
			{
				Loot->SetLootRarity(BagRarity);
				Loot->SetItemID(ItemID);
			}
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
		UE_LOG(LogT66Enemy, Verbose, TEXT("[GOLD] EnemyPool: enemy %s returned to pool (after loot)"), *GetName());
	}
	else
	{
		// Fallback: deferred destruction.
		SetActorHiddenInGame(true);
		SetActorEnableCollision(false);
		SetLifeSpan(0.1f);
	}
}
