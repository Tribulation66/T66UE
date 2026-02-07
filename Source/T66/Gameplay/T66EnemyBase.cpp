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
#include "EngineUtils.h"

AT66EnemyBase::AT66EnemyBase()
{
	PrimaryActorTick.bCanEverTick = true;
	AIControllerClass = AT66EnemyAIController::StaticClass();
	AutoPossessAI = EAutoPossessAI::PlacedInWorldOrSpawned;
	CurrentHP = MaxHP;

	if (UCharacterMovementComponent* Move = GetCharacterMovement())
	{
		Move->MaxWalkSpeed = 350.f;
	}

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

void AT66EnemyBase::ApplyDifficultyScalar(float Scalar)
{
	if (!bBaseTuningInitialized)
	{
		BaseMaxHP = MaxHP;
		BaseTouchDamageHearts = TouchDamageHearts;
		BasePointValue = PointValue;
		bBaseTuningInitialized = true;
	}

	const float ClampedScalar = FMath::Clamp(Scalar, 1.0f, 99.0f);

	// Preserve current HP percentage when scaling mid-fight.
	const float PrevMax = static_cast<float>(FMath::Max(1, MaxHP));
	const float PrevCur = static_cast<float>(FMath::Clamp(CurrentHP, 0, MaxHP));
	const float Pct = FMath::Clamp(PrevCur / PrevMax, 0.f, 1.f);

	MaxHP = FMath::Max(1, FMath::RoundToInt(static_cast<float>(BaseMaxHP) * ClampedScalar));
	TouchDamageHearts = FMath::Max(1, FMath::RoundToInt(static_cast<float>(BaseTouchDamageHearts) * ClampedScalar));

	// Re-apply mini-boss multipliers if this enemy was promoted.
	if (bIsMiniBoss)
	{
		MaxHP = FMath::Max(1, FMath::RoundToInt(static_cast<float>(MaxHP) * FMath::Max(0.1f, MiniBossHPScalarApplied)));
		TouchDamageHearts = FMath::Max(1, FMath::RoundToInt(static_cast<float>(TouchDamageHearts) * FMath::Max(0.1f, MiniBossDamageScalarApplied)));
		SetActorScale3D(FVector(MiniBossScaleScalarApplied, MiniBossScaleScalarApplied, MiniBossScaleScalarApplied));
	}

	CurrentHP = FMath::Clamp(FMath::RoundToInt(static_cast<float>(MaxHP) * Pct), 1, MaxHP);
	UpdateHealthBar();
}

void AT66EnemyBase::ApplyDifficultyTier(int32 Tier)
{
	// Tier 0 = 1.0x, Tier 1 = 1.1x, Tier 2 = 1.2x, ...
	const int32 ClampedTier = FMath::Max(0, Tier);
	const float Scalar = 1.0f + (0.1f * static_cast<float>(ClampedTier));
	ApplyDifficultyScalar(Scalar);
}

void AT66EnemyBase::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);
	if (CurrentHP <= 0) return;

	APawn* PlayerPawn = UGameplayStatics::GetPlayerPawn(this, 0);
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

		// Cache safe-zone NPCs per-world so we don't do a full actor iterator per enemy.
		struct FSafeNPCache
		{
			TWeakObjectPtr<UWorld> World;
			double LastRefreshSeconds = -1.0;
			TArray<TWeakObjectPtr<AT66HouseNPCBase>> NPCs;
		};
		static FSafeNPCache Cache;

		const double NowSeconds = FPlatformTime::Seconds();
		const bool bWorldChanged = (Cache.World.Get() != GetWorld());
		const bool bNeedsRefresh = bWorldChanged || (Cache.LastRefreshSeconds < 0.0) || ((NowSeconds - Cache.LastRefreshSeconds) > 2.0);
		if (bNeedsRefresh)
		{
			Cache.World = GetWorld();
			Cache.LastRefreshSeconds = NowSeconds;
			Cache.NPCs.Reset();
			for (TActorIterator<AT66HouseNPCBase> It(GetWorld()); It; ++It)
			{
				if (AT66HouseNPCBase* NPC = *It)
				{
					Cache.NPCs.Add(NPC);
				}
			}
		}

		for (const TWeakObjectPtr<AT66HouseNPCBase>& WeakNPC : Cache.NPCs)
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
	RunState->ApplyDamage(TouchDamageHearts);
}

bool AT66EnemyBase::TakeDamageFromHero(int32 Damage)
{
	if (Damage <= 0 || CurrentHP <= 0) return false;
	CurrentHP = FMath::Max(0, CurrentHP - Damage);
	UpdateHealthBar();
	if (CurrentHP <= 0)
	{
		OnDeath();
		return true;
	}
	return false;
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
			Destroy();
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

	Destroy();
}
