// Copyright Tribulation 66. All Rights Reserved.

#include "Gameplay/T66MobBase.h"

#include "Core/T66ActorRegistrySubsystem.h"
#include "Core/T66CharacterVisualSubsystem.h"
#include "Core/T66DamageLogSubsystem.h"
#include "Core/T66FloatingCombatTextSubsystem.h"
#include "Core/T66GameInstance.h"
#include "Core/T66PlayerExperienceSubSystem.h"
#include "Core/T66RngSubsystem.h"
#include "Core/T66RunStateSubsystem.h"
#include "Core/T66ShelvedFeatureGate.h"
#include "CollisionQueryParams.h"
#include "Components/CapsuleComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Components/WidgetComponent.h"
#include "Engine/GameInstance.h"
#include "Engine/World.h"
#include "GameFramework/PlayerController.h"
#include "Gameplay/Enemies/Projectiles/T66EnemyProjectileBase.h"
#include "Gameplay/Enemies/Projectiles/T66SpitProjectile.h"
#include "Gameplay/T66BoostInteractable.h"
#include "Gameplay/T66CombatHitZoneComponent.h"
#include "Gameplay/T66EnemyDirector.h"
#include "Gameplay/T66HeroBase.h"
#include "Gameplay/T66LootBagPickup.h"
#include "Gameplay/T66MobManagerSubsystem.h"
#include "Gameplay/T66MobLootSubsystem.h"
#include "Gameplay/T66ProjectileManagerSubsystem.h"
#include "Gameplay/T66VisualUtil.h"
#include "TimerManager.h"
#include "UI/T66EnemyLockWidget.h"

DEFINE_LOG_CATEGORY_STATIC(LogT66MobBase, Log, All);

namespace
{
const FName T66MobVATClip_Idle(TEXT("Idle"));
const FName T66MobVATClip_Move(TEXT("Move"));
const FName T66MobVATClip_AttackCue(TEXT("AttackCue"));
const FName T66MobVATClip_HitReact(TEXT("HitReact"));
const FName T66MobVATClip_Death(TEXT("Death"));
constexpr float T66NormalMobStatBoostDropChance = 0.02f;
constexpr float T66MiniBossStatBoostDropChance = 0.08f;
#if !UE_BUILD_SHIPPING
const FName T66ForceMobLootBagDropTag(TEXT("T66_ForceMobLootBagDrop"));
#endif

struct FT66MobKillBoostDropTarget
{
	bool bUsesStat = false;
	ET66HeroStatType BaseStatType = ET66HeroStatType::Damage;
	ET66StatType StatType = ET66StatType::None;
};

const TArray<FT66MobKillBoostDropTarget>& T66GetMobKillBoostDropTargets()
{
	static const TArray<FT66MobKillBoostDropTarget> Targets = {
		{ false, ET66HeroStatType::Damage, ET66StatType::None },
		{ false, ET66HeroStatType::AttackSpeed, ET66StatType::None },
		{ false, ET66HeroStatType::AttackScale, ET66StatType::None },
		{ false, ET66HeroStatType::Accuracy, ET66StatType::None },
		{ false, ET66HeroStatType::Armor, ET66StatType::None },
		{ false, ET66HeroStatType::Evasion, ET66StatType::None },
		{ false, ET66HeroStatType::Luck, ET66StatType::None },
		{ false, ET66HeroStatType::Speed, ET66StatType::None },
		{ true, ET66HeroStatType::Special, ET66StatType::FirePower },
		{ true, ET66HeroStatType::Special, ET66StatType::IcePower },
		{ true, ET66HeroStatType::Special, ET66StatType::ElectricityPower },
		{ true, ET66HeroStatType::Special, ET66StatType::NaturePower },
		{ true, ET66HeroStatType::Special, ET66StatType::WindPower },
	};
	return Targets;
}

void T66TrySpawnMobKillStatBoost(AActor* SourceActor, const bool bIsMiniBoss)
{
	if (!SourceActor)
	{
		return;
	}
	if (!FT66ShelvedFeatureGate::IsMobLootEnabled())
	{
		return;
	}

	UWorld* World = SourceActor->GetWorld();
	const TArray<FT66MobKillBoostDropTarget>& Targets = T66GetMobKillBoostDropTargets();
	const float DropChance = bIsMiniBoss ? T66MiniBossStatBoostDropChance : T66NormalMobStatBoostDropChance;
	if (!World || Targets.Num() <= 0 || FMath::FRand() >= DropChance)
	{
		return;
	}

	const FT66MobKillBoostDropTarget& Target = Targets[FMath::RandRange(0, Targets.Num() - 1)];
	FActorSpawnParameters SpawnParams;
	SpawnParams.Owner = SourceActor;
	SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
	if (AT66BoostInteractable* Boost = World->SpawnActor<AT66BoostInteractable>(
		AT66BoostInteractable::StaticClass(),
		SourceActor->GetActorLocation() + FVector(0.f, 0.f, 80.f),
		FRotator::ZeroRotator,
		SpawnParams))
	{
		if (Target.bUsesStat)
		{
			Boost->ConfigureStatBoost(Target.StatType, 8, 10.f);
		}
		else
		{
			Boost->ConfigureBoost(Target.BaseStatType, 8, 10.f);
		}
		UE_LOG(LogT66MobBase, Display, TEXT("[T66Proof][StatBoostDrop] Source=LightweightMob MiniBoss=%d Chance=%.3f Primary=%d Secondary=%d UsesSecondary=%d"),
			bIsMiniBoss ? 1 : 0,
			DropChance,
			static_cast<int32>(Target.BaseStatType),
			static_cast<int32>(Target.StatType),
			Target.bUsesStat ? 1 : 0);
	}
}

void T66TrySpawnMobKillLootBags(AActor* SourceActor, const bool bIsMiniBoss)
{
	if (!SourceActor)
	{
		return;
	}
	if (!FT66ShelvedFeatureGate::IsMobLootEnabled())
	{
		return;
	}

	UWorld* World = SourceActor->GetWorld();
	UGameInstance* GI = World ? World->GetGameInstance() : nullptr;
	UT66GameInstance* T66GI = Cast<UT66GameInstance>(GI);
	UT66RunStateSubsystem* RunState = GI ? GI->GetSubsystem<UT66RunStateSubsystem>() : nullptr;
	if (!World || !T66GI)
	{
		return;
	}

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
	const bool bRolledDrop = RngSub ? RngSub->RollChance01(DropChance) : (Stream.GetFraction() < DropChance);
#if !UE_BUILD_SHIPPING
	const bool bDroppedBag = bRolledDrop || SourceActor->ActorHasTag(T66ForceMobLootBagDropTag);
#else
	const bool bDroppedBag = bRolledDrop;
#endif
	const int32 DropDrawIndex = RngSub ? RngSub->GetLastRunDrawIndex() : INDEX_NONE;
	const int32 DropPreDrawSeed = RngSub ? RngSub->GetLastRunPreDrawSeed() : 0;
	if (RunState)
	{
		RunState->RecordLuckQuantityBool(FName(TEXT("EnemyLootBagDrop")), bDroppedBag, DropChance, DropDrawIndex, DropPreDrawSeed);
	}
	if (!bDroppedBag)
	{
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
	const float LootBagMultiplier = RunState ? RunState->GetRunModifierEnemyLootBagCountMultiplier() : 1.0f;
	const int32 AdjustedLootBagCount = (LootBagMultiplier > 0.0f && !FMath::IsNearlyEqual(LootBagMultiplier, 1.0f))
		? FMath::Max(1, FMath::RoundToInt(static_cast<float>(LootBagCount) * LootBagMultiplier))
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
	SpawnParams.Owner = SourceActor;
	for (int32 BagIndex = 0; BagIndex < AdjustedLootBagCount; ++BagIndex)
	{
		ET66Rarity BagRarity = RngSub ? RngSub->RollRarityWeighted(Weights, Stream) : FT66RarityUtil::RollDefaultRarity(Stream);
		const int32 BagRarityDrawIndex = RngSub ? RngSub->GetLastRunDrawIndex() : INDEX_NONE;
		const int32 BagRarityPreDrawSeed = RngSub ? RngSub->GetLastRunPreDrawSeed() : 0;
		if (bIsMiniBoss)
		{
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
		const FVector Offset(FMath::Cos(Angle) * Radius, FMath::Sin(Angle) * Radius, 90.f);
		if (AT66LootBagPickup* Loot = World->SpawnActor<AT66LootBagPickup>(
			AT66LootBagPickup::StaticClass(),
			SourceActor->GetActorLocation() + Offset,
			FRotator::ZeroRotator,
			SpawnParams))
		{
			Loot->SetLootRarity(BagRarity);
			Loot->SetItemID(ItemID);
		}
	}
}

ET66EnemyFamily T66ResolveEnemyFamilyFromName(const FName FamilyID, const ET66EnemyFamily FallbackFamily)
{
	if (FamilyID == FName(TEXT("Flying")))
	{
		return ET66EnemyFamily::Flying;
	}
	if (FamilyID == FName(TEXT("Ranged")))
	{
		return ET66EnemyFamily::Ranged;
	}
	if (FamilyID == FName(TEXT("Rush")))
	{
		return ET66EnemyFamily::Rush;
	}
	if (FamilyID == FName(TEXT("Melee")))
	{
		return ET66EnemyFamily::Melee;
	}
	return FallbackFamily;
}

float T66ResolveFamilyChaseSpeed(const ET66EnemyFamily Family)
{
	switch (Family)
	{
	case ET66EnemyFamily::Flying:
		return 215.f;
	case ET66EnemyFamily::Ranged:
		return 160.f;
	case ET66EnemyFamily::Rush:
		return 165.f;
	case ET66EnemyFamily::Melee:
	default:
		return 175.f;
	}
}
}

AT66MobBase::AT66MobBase()
{
	PrimaryActorTick.bCanEverTick = false;
	PrimaryActorTick.bStartWithTickEnabled = false;

	CapsuleComponent = CreateDefaultSubobject<UCapsuleComponent>(TEXT("MobCapsule"));
	SetRootComponent(CapsuleComponent);
	CapsuleComponent->PrimaryComponentTick.bCanEverTick = false;
	CapsuleComponent->SetComponentTickEnabled(false);
	CapsuleComponent->InitCapsuleSize(42.f, 88.f);
	CapsuleComponent->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	CapsuleComponent->SetCollisionObjectType(ECC_Pawn);
	CapsuleComponent->SetCollisionResponseToAllChannels(ECR_Ignore);
	CapsuleComponent->SetCollisionResponseToChannel(ECC_WorldStatic, ECR_Block);
	CapsuleComponent->SetCollisionResponseToChannel(ECC_WorldDynamic, ECR_Block);
	CapsuleComponent->SetCollisionResponseToChannel(ECC_Pawn, ECR_Block);
	CapsuleComponent->SetCollisionResponseToChannel(ECC_Visibility, ECR_Block);
	CapsuleComponent->SetGenerateOverlapEvents(true);
	CapsuleComponent->SetCanEverAffectNavigation(false);

	VisualMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("VisualMesh"));
	VisualMesh->SetupAttachment(RootComponent);
	VisualMesh->PrimaryComponentTick.bCanEverTick = false;
	VisualMesh->SetComponentTickEnabled(false);
	VisualMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	VisualMesh->SetGenerateOverlapEvents(false);
	VisualMesh->SetCanEverAffectNavigation(false);
	VisualMesh->SetRelativeLocation(FVector(0.f, 0.f, -38.f));
	if (UStaticMesh* Sphere = FT66VisualUtil::GetBasicShapeSphere())
	{
		VisualMesh->SetStaticMesh(Sphere);
		VisualMesh->SetRelativeScale3D(FVector(0.85f, 0.85f, 0.85f));
		FT66VisualUtil::ApplyT66Color(VisualMesh, this, FLinearColor(0.90f, 0.25f, 0.20f, 1.f));
	}

	LockIndicatorWidget = CreateDefaultSubobject<UWidgetComponent>(TEXT("LockIndicatorWidget"));
	LockIndicatorWidget->SetupAttachment(RootComponent);
	LockIndicatorWidget->PrimaryComponentTick.bCanEverTick = false;
	LockIndicatorWidget->SetComponentTickEnabled(false);
	LockIndicatorWidget->SetRelativeLocation(FVector(0.f, 0.f, 225.f));
	LockIndicatorWidget->SetWidgetSpace(EWidgetSpace::Screen);
	LockIndicatorWidget->SetDrawAtDesiredSize(true);
	LockIndicatorWidget->SetDrawSize(FVector2D(52.f, 52.f));
	LockIndicatorWidget->SetWidgetClass(UT66EnemyLockWidget::StaticClass());
	LockIndicatorWidget->SetHiddenInGame(true, true);
	LockIndicatorWidget->SetVisibility(false, true);

	BodyHitZone = CreateDefaultSubobject<UT66CombatHitZoneComponent>(TEXT("BodyHitZone"));
	BodyHitZone->SetupAttachment(RootComponent);
	BodyHitZone->PrimaryComponentTick.bCanEverTick = false;
	BodyHitZone->SetComponentTickEnabled(false);
	BodyHitZone->HitZoneType = ET66HitZoneType::Body;
	BodyHitZone->DamageMultiplier = BodyDamageMultiplier;
	BodyHitZone->SetRelativeLocation(FVector(0.f, 0.f, 64.f));
	BodyHitZone->InitSphereRadius(42.f);

	HeadHitZone = CreateDefaultSubobject<UT66CombatHitZoneComponent>(TEXT("HeadHitZone"));
	HeadHitZone->SetupAttachment(RootComponent);
	HeadHitZone->PrimaryComponentTick.bCanEverTick = false;
	HeadHitZone->SetComponentTickEnabled(false);
	HeadHitZone->HitZoneType = ET66HitZoneType::Head;
	HeadHitZone->DamageMultiplier = HeadDamageMultiplier;
	HeadHitZone->SetRelativeLocation(FVector(0.f, 0.f, 124.f));
	HeadHitZone->InitSphereRadius(24.f);
}

void AT66MobBase::BeginPlay()
{
	Super::BeginPlay();

	if (UWorld* World = GetWorld())
	{
		if (UT66MobManagerSubsystem* Manager = World->GetSubsystem<UT66MobManagerSubsystem>())
		{
			Manager->RegisterMob(this);
		}
		else
		{
			UE_LOG(LogT66MobBase, Warning, TEXT("BeginPlay could not resolve UT66MobManagerSubsystem for mob=%s"), *GetName());
		}

		if (UT66ActorRegistrySubsystem* Registry = World->GetSubsystem<UT66ActorRegistrySubsystem>())
		{
			Registry->RegisterMob(this);
		}
		else
		{
			UE_LOG(LogT66MobBase, Warning, TEXT("BeginPlay could not resolve UT66ActorRegistrySubsystem for mob=%s"), *GetName());
		}
		return;
	}

	UE_LOG(LogT66MobBase, Warning, TEXT("BeginPlay could not resolve world for mob=%s"), *GetName());
}

void AT66MobBase::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	if (OwningDirector && LifecycleState == ET66MobLifecycleState::Active && EndPlayReason == EEndPlayReason::Destroyed)
	{
		NotifyOwningDirectorOfDeath();
	}

	if (UWorld* World = GetWorld())
	{
		if (UT66MobManagerSubsystem* Manager = World->GetSubsystem<UT66MobManagerSubsystem>())
		{
			Manager->UnregisterMob(this);
		}
		else
		{
			UE_LOG(LogT66MobBase, Warning, TEXT("EndPlay could not resolve UT66MobManagerSubsystem for mob=%s"), *GetName());
		}

		if (UT66ActorRegistrySubsystem* Registry = World->GetSubsystem<UT66ActorRegistrySubsystem>())
		{
			Registry->UnregisterMob(this);
		}
		else
		{
			UE_LOG(LogT66MobBase, Warning, TEXT("EndPlay could not resolve UT66ActorRegistrySubsystem for mob=%s"), *GetName());
		}
	}
	else
	{
		UE_LOG(LogT66MobBase, Warning, TEXT("EndPlay could not resolve world for mob=%s"), *GetName());
	}

	Super::EndPlay(EndPlayReason);
}

void AT66MobBase::NotifyOwningDirectorOfDeath()
{
	if (!OwningDirector)
	{
		return;
	}

	OwningDirector->NotifyMobDied(this);
	OwningDirector = nullptr;
}

float AT66MobBase::GetHitZoneDamageMultiplier(const ET66HitZoneType HitZoneType) const
{
	return HitZoneType == ET66HitZoneType::Head
		? FMath::Max(0.1f, HeadDamageMultiplier)
		: FMath::Max(0.1f, BodyDamageMultiplier);
}

FT66CombatTargetHandle AT66MobBase::ResolveCombatTargetHandle(const UPrimitiveComponent* HitComponent, const ET66HitZoneType PreferredZone) const
{
	FT66CombatTargetHandle Handle;
	Handle.Actor = const_cast<AT66MobBase*>(this);
	Handle.HitZoneType = ET66HitZoneType::Body;

	if (HitComponent && HeadHitZone && HitComponent == HeadHitZone)
	{
		Handle.HitZoneType = ET66HitZoneType::Head;
	}
	else if (HitComponent && BodyHitZone && HitComponent == BodyHitZone)
	{
		Handle.HitZoneType = ET66HitZoneType::Body;
	}
	else if (PreferredZone == ET66HitZoneType::Head && HeadHitZone && HeadHitZone->bTargetable)
	{
		Handle.HitZoneType = ET66HitZoneType::Head;
	}

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

void AT66MobBase::ConfigureAsMob(
	FName InMobID,
	const ET66EnemyFamily InFamily,
	FName InArchetype,
	const int32 StageNum,
	const float DifficultyScalar,
	const float EnemyProgressionScalar,
	const float FinaleScalar,
	const bool bInIsMiniBoss)
{
	MobID = InMobID.IsNone() ? FName(TEXT("Slime")) : InMobID;
	CharacterVisualID = MobID;
	EnemyFamily = InFamily;
	Archetype = InArchetype;
	bIsMiniBoss = false;
	XPValue = 20;

	if (bInIsMiniBoss)
	{
		ensureMsgf(false, TEXT("AT66MobBase::ConfigureAsMob received mini-boss routing before B.6 guard logic exists."));
		UE_LOG(LogT66MobBase, Warning, TEXT("ConfigureAsMob mini-boss request ignored for MobID=%s; lightweight mini-boss routing is out of scope for B.5."),
			*MobID.ToString());
	}

	FT66EnemyData EnemyData;
	if (const UWorld* World = GetWorld())
	{
		if (UT66GameInstance* T66GI = Cast<UT66GameInstance>(World->GetGameInstance()))
		{
			if (T66GI->GetEnemyData(MobID, EnemyData))
			{
				EnemyFamily = T66ResolveEnemyFamilyFromName(EnemyData.FamilyID, EnemyFamily);
				Archetype = EnemyData.Archetype.IsNone() ? Archetype : EnemyData.Archetype;
				XPValue = FMath::Max(0, EnemyData.XPValue);
			}
			else
			{
				UE_LOG(LogT66MobBase, Warning, TEXT("ConfigureAsMob could not find Enemies row for MobID=%s; using supplied family/defaults."),
					*MobID.ToString());
			}
		}
	}

	ChaseSpeed = T66ResolveFamilyChaseSpeed(EnemyFamily);
	bIsRushing = false;
	RushDirection = FVector::ZeroVector;
	RushSecondsRemaining = 0.f;
	RushCooldownRemaining = EnemyFamily == ET66EnemyFamily::Rush ? InitialRushCooldownSeconds : 0.f;
	HoverHeight = 180.f;
	HoverBobFrequency = 2.2f;
	HoverBobAmplitude = 35.f;
	HoverAnchorZ = EnemyFamily == ET66EnemyFamily::Flying ? GetActorLocation().Z + HoverHeight : 0.f;
	HoverBobTime = EnemyFamily == ET66EnemyFamily::Flying && HoverBobFrequency > KINDA_SMALL_NUMBER
		? FMath::FRandRange(0.f, (2.f * PI) / HoverBobFrequency)
		: 0.f;
	DesiredMinRange = 800.f;
	DesiredMaxRange = 1400.f;
	FireRangeGrace = 150.f;
	ProjectileSpawnHeight = 80.f;
	FireCooldownDuration = 1.6f;
	FireCooldownRemaining = EnemyFamily == ET66EnemyFamily::Ranged ? 0.6f : 0.f;
	ProjectileClass = EnemyFamily == ET66EnemyFamily::Ranged ? AT66SpitProjectile::StaticClass() : nullptr;
	TouchDamageHearts = 1;

	const int32 ClampedStage = FMath::Clamp(StageNum, 1, 999);
	const float StageMultiplier = FMath::Pow(1.1f, static_cast<float>(ClampedStage - 1));
	int32 ResolvedMaxHP = FMath::Max(1, FMath::RoundToInt(50.f * StageMultiplier));
	ResolvedMaxHP = FMath::Max(1, FMath::RoundToInt(static_cast<float>(ResolvedMaxHP) * FMath::Clamp(DifficultyScalar, 1.f, 99.f)));
	ResolvedMaxHP = FMath::Max(1, FMath::RoundToInt(static_cast<float>(ResolvedMaxHP) * FMath::Clamp(EnemyProgressionScalar, 1.f, 99.f)));
	ResolvedMaxHP = FMath::Max(1, FMath::RoundToInt(static_cast<float>(ResolvedMaxHP) * FMath::Clamp(FinaleScalar, 1.f, 99.f)));

	if (const UWorld* World = GetWorld())
	{
		if (const UGameInstance* GameInstance = World->GetGameInstance())
		{
			const UT66RunStateSubsystem* RunState = GameInstance->GetSubsystem<UT66RunStateSubsystem>();
			const float EnemyHealthMultiplier = RunState ? RunState->GetRunModifierEnemyHealthMultiplier() : 1.0f;
			if (EnemyHealthMultiplier > 0.f && !FMath::IsNearlyEqual(EnemyHealthMultiplier, 1.f))
			{
				ResolvedMaxHP = FMath::Max(1, FMath::RoundToInt(static_cast<float>(ResolvedMaxHP) * EnemyHealthMultiplier));
			}
		}
	}

	int32 ResolvedTouchDamage = FMath::Max(1, FMath::RoundToInt(static_cast<float>(TouchDamageHearts) * FMath::Clamp(DifficultyScalar, 1.f, 99.f)));
	ResolvedTouchDamage = FMath::Max(1, FMath::RoundToInt(static_cast<float>(ResolvedTouchDamage) * FMath::Clamp(EnemyProgressionScalar, 1.f, 99.f)));
	ResolvedTouchDamage = FMath::Max(1, FMath::RoundToInt(static_cast<float>(ResolvedTouchDamage) * FMath::Clamp(FinaleScalar, 1.f, 99.f)));

	MaxHP = static_cast<float>(ResolvedMaxHP);
	CurrentHP = MaxHP;
	TouchDamageHearts = ResolvedTouchDamage;
	TouchDamageCooldownSeconds = 0.f;
	bIsTouchingHero = false;
	LifecycleState = ET66MobLifecycleState::Active;

	if (EnemyFamily != ET66EnemyFamily::Melee
		&& EnemyFamily != ET66EnemyFamily::Rush
		&& EnemyFamily != ET66EnemyFamily::Flying
		&& EnemyFamily != ET66EnemyFamily::Ranged)
	{
		UE_LOG(LogT66MobBase, Warning, TEXT("ConfigureAsMob MobID=%s family=%d archetype=%s uses lightweight fallback chase; full family behavior lands in a later migration pass."),
			*MobID.ToString(),
			static_cast<int32>(EnemyFamily),
			Archetype.IsNone() ? TEXT("None") : *Archetype.ToString());
	}

	ApplyConfiguredVisual();

	UE_LOG(LogT66MobBase, VeryVerbose, TEXT("ConfigureAsMob mob=%s MobID=%s family=%d archetype=%s stage=%d hp=%.1f/%.1f touchHearts=%d chaseSpeed=%.1f visual=%s"),
		*GetName(),
		*MobID.ToString(),
		static_cast<int32>(EnemyFamily),
		Archetype.IsNone() ? TEXT("None") : *Archetype.ToString(),
		ClampedStage,
		CurrentHP,
		MaxHP,
		TouchDamageHearts,
		ChaseSpeed,
		IsUsingMobVertexAnimation() ? TEXT("VAT") : TEXT("StaticOrPlaceholder"));
}

void AT66MobBase::ApplyConfiguredVisual()
{
	if (!VisualMesh)
	{
		return;
	}

	if (TryApplyMobVertexAnimationVisual())
	{
		return;
	}

	if (UWorld* World = GetWorld())
	{
		if (UGameInstance* GameInstance = World->GetGameInstance())
		{
			if (UT66CharacterVisualSubsystem* Visuals = GameInstance->GetSubsystem<UT66CharacterVisualSubsystem>())
			{
				if (Visuals->HasCharacterVisual(CharacterVisualID)
					&& Visuals->ApplyCharacterVisual(CharacterVisualID, nullptr, nullptr, true, false, false, VisualMesh))
				{
					if (UT66MobManagerSubsystem* Manager = World->GetSubsystem<UT66MobManagerSubsystem>())
					{
						Manager->ClearMobVertexAnimationState(this);
					}
					return;
				}
			}
		}
	}

	// Keep the constructor placeholder visible when no data-driven visual can be applied.
	if (UWorld* World = GetWorld())
	{
		if (UT66MobManagerSubsystem* Manager = World->GetSubsystem<UT66MobManagerSubsystem>())
		{
			Manager->ClearMobVertexAnimationState(this);
		}
	}
	VisualMesh->SetHiddenInGame(false, true);
	VisualMesh->SetVisibility(true, true);
}

bool AT66MobBase::TryApplyMobVertexAnimationVisual()
{
	if (CharacterVisualID.IsNone() || !VisualMesh)
	{
		return false;
	}

	if (UWorld* World = GetWorld())
	{
		if (UGameInstance* GameInstance = World->GetGameInstance())
		{
			if (UT66CharacterVisualSubsystem* Visuals = GameInstance->GetSubsystem<UT66CharacterVisualSubsystem>())
			{
				UMaterialInstanceDynamic* DynamicMaterial = nullptr;
				FT66MobVertexAnimationRow Row;
				if (Visuals->ApplyMobVertexAnimationVisual(CharacterVisualID, VisualMesh, DynamicMaterial, Row) && DynamicMaterial)
				{
					if (UT66MobManagerSubsystem* Manager = World->GetSubsystem<UT66MobManagerSubsystem>())
					{
						Manager->ConfigureMobVertexAnimationState(this, Row, DynamicMaterial);
						return true;
					}

					UE_LOG(LogT66MobBase, Warning, TEXT("TryApplyMobVertexAnimationVisual could not resolve UT66MobManagerSubsystem for mob=%s visual=%s"),
						*GetName(),
						CharacterVisualID.IsNone() ? TEXT("None") : *CharacterVisualID.ToString());
					return false;
				}
			}
		}
	}

	return false;
}

void AT66MobBase::SetMobVertexAnimationClip(FName ClipName, const float OverrideSeconds)
{
	if (UWorld* World = GetWorld())
	{
		if (UT66MobManagerSubsystem* Manager = World->GetSubsystem<UT66MobManagerSubsystem>())
		{
			Manager->SetMobVertexAnimationClip(this, ClipName, OverrideSeconds);
		}
	}
}

bool AT66MobBase::IsUsingMobVertexAnimation() const
{
	if (const UWorld* World = GetWorld())
	{
		if (const UT66MobManagerSubsystem* Manager = World->GetSubsystem<UT66MobManagerSubsystem>())
		{
			return Manager->IsMobUsingVertexAnimation(this);
		}
	}

	return false;
}

#if !UE_BUILD_SHIPPING
void AT66MobBase::ForceMobVertexAnimationClipForAutomation(FName ClipName, float OverrideSeconds)
{
	SetMobVertexAnimationClip(ClipName, OverrideSeconds);
}
#endif

bool AT66MobBase::TakeDamageFromHeroHitZone(int32 Damage, const FT66CombatTargetHandle& TargetHandle, FName DamageSourceID, FName EventType)
{
	if (Damage <= 0 || CurrentHP <= 0.f || LifecycleState == ET66MobLifecycleState::Dying)
	{
		return false;
	}

	const FT66CombatTargetHandle ResolvedHandle = TargetHandle.IsValid()
		? TargetHandle
		: ResolveCombatTargetHandle(nullptr, ET66HitZoneType::Body);
	const ET66HitZoneType HitZoneType = ResolvedHandle.HitZoneType == ET66HitZoneType::Head
		? ET66HitZoneType::Head
		: ET66HitZoneType::Body;
	const float Multiplier = GetHitZoneDamageMultiplier(HitZoneType);
	const int32 ResolvedDamage = FMath::Max(1, FMath::RoundToInt(static_cast<float>(Damage) * Multiplier));
	const FName ResolvedSource = DamageSourceID.IsNone()
		? UT66DamageLogSubsystem::SourceID_AutoAttack
		: DamageSourceID;
	if (HitZoneType == ET66HitZoneType::Head && EventType.IsNone())
	{
		EventType = UT66FloatingCombatTextSubsystem::EventType_Headshot;
	}

	CurrentHP = FMath::Max(0.f, CurrentHP - static_cast<float>(ResolvedDamage));

	if (UWorld* World = GetWorld())
	{
		if (UGameInstance* GameInstance = World->GetGameInstance())
		{
			if (UT66DamageLogSubsystem* DamageLog = GameInstance->GetSubsystem<UT66DamageLogSubsystem>())
			{
				DamageLog->RecordDamageDealt(ResolvedSource, ResolvedDamage);
			}

			if (UT66FloatingCombatTextSubsystem* FloatingText = GameInstance->GetSubsystem<UT66FloatingCombatTextSubsystem>())
			{
				FloatingText->ShowDamageNumber(this, ResolvedDamage, EventType);
			}
		}
	}

	UE_LOG(LogT66MobBase, VeryVerbose, TEXT("MobHitDamage mob=%s MobID=%s damage=%d hp=%.1f/%.1f hitZone=%d source=%s event=%s"),
		*GetName(),
		MobID.IsNone() ? TEXT("unset") : *MobID.ToString(),
		ResolvedDamage,
		CurrentHP,
		MaxHP,
		static_cast<int32>(HitZoneType),
		*ResolvedSource.ToString(),
		*EventType.ToString());

	const bool bDied = CurrentHP <= 0.f;
	if (bDied)
	{
		LifecycleState = ET66MobLifecycleState::Dying;
		StoredVelocity = FVector::ZeroVector;
		KnockbackVelocity = FVector::ZeroVector;
		bPhysicalLaunchActive = false;
		PhysicalLaunchVelocity = FVector::ZeroVector;
		PhysicalLaunchSecondsRemaining = 0.f;
		SetMobVertexAnimationClip(T66MobVATClip_Death, 0.45f);
		if (UWorld* World = GetWorld())
		{
			if (UGameInstance* GameInstance = World->GetGameInstance())
			{
				if (UT66RunStateSubsystem* RunState = GameInstance->GetSubsystem<UT66RunStateSubsystem>())
				{
					RunState->AddHeroXP(XPValue);
				}
			}

			if (UT66MobLootSubsystem* MobLoot = World->GetSubsystem<UT66MobLootSubsystem>())
			{
				FT66MobLootHandle MobLootHandle;
				MobLoot->SpawnMobLootFromNonBossDeath(this, MobID, bIsMiniBoss, MobLootHandle);
			}
			T66TrySpawnMobKillLootBags(this, bIsMiniBoss);
			T66TrySpawnMobKillStatBoost(this, bIsMiniBoss);
		}
		NotifyOwningDirectorOfDeath();

		if (UWorld* World = GetWorld())
		{
			if (UT66MobManagerSubsystem* Manager = World->GetSubsystem<UT66MobManagerSubsystem>())
			{
				Manager->NotifyMobDying(this);
			}
		}
	}
	else
	{
		SetMobVertexAnimationClip(T66MobVATClip_HitReact, 0.16f);
	}

	return bDied;
}

void AT66MobBase::ApplyStun(float DurationSeconds)
{
	UE_LOG(LogT66MobBase, VeryVerbose, TEXT("ApplyStun mob=%s duration=%.3f"), *GetName(), DurationSeconds);
	StunDurationSeconds = DurationSeconds;
	StunSecondsRemaining = DurationSeconds;
}

void AT66MobBase::ApplyRoot(float DurationSeconds)
{
	UE_LOG(LogT66MobBase, VeryVerbose, TEXT("ApplyRoot mob=%s duration=%.3f"), *GetName(), DurationSeconds);
	RootDurationSeconds = DurationSeconds;
	RootSecondsRemaining = DurationSeconds;
}

void AT66MobBase::ApplyFreeze(float DurationSeconds)
{
	UE_LOG(LogT66MobBase, VeryVerbose, TEXT("ApplyFreeze mob=%s duration=%.3f"), *GetName(), DurationSeconds);
	FreezeDurationSeconds = DurationSeconds;
	FreezeSecondsRemaining = DurationSeconds;
}

void AT66MobBase::ApplySlow(float SpeedMultiplier, float DurationSeconds)
{
	const float ClampedStrength = FMath::Clamp(SpeedMultiplier, 0.f, 1.f);
	UE_LOG(LogT66MobBase, VeryVerbose, TEXT("ApplySlow mob=%s strength=%.3f duration=%.3f"), *GetName(), ClampedStrength, DurationSeconds);
	SlowStrength = ClampedStrength;
	SlowMultiplier = 1.f - ClampedStrength;
	SlowDurationSeconds = DurationSeconds;
	SlowSecondsRemaining = DurationSeconds;
}

void AT66MobBase::ApplyMoveSlow(float SpeedMultiplier, float DurationSeconds)
{
	const float ClampedMultiplier = FMath::Clamp(SpeedMultiplier, 0.f, 1.f);
	ApplySlow(1.f - ClampedMultiplier, DurationSeconds);
}

void AT66MobBase::ApplyAutoAttackKnockback(const FVector& HitOrigin, float StrengthScale)
{
	UE_LOG(LogT66MobBase, VeryVerbose, TEXT("ApplyAutoAttackKnockback mob=%s origin=%s strength=%.3f"), *GetName(), *HitOrigin.ToCompactString(), StrengthScale);
	constexpr float KnockbackSpeed = 260.f;
	constexpr float KnockbackDuration = 0.12f;
	const float ResolvedSpeed = KnockbackSpeed * FMath::Max(0.f, StrengthScale);
	if (ResolvedSpeed <= 0.f || KnockbackDuration <= 0.f)
	{
		return;
	}

	FVector Direction = GetActorLocation() - HitOrigin;
	Direction.Z = 0.f;
	if (!Direction.Normalize())
	{
		Direction = GetActorForwardVector();
		Direction.Z = 0.f;
		if (!Direction.Normalize())
		{
			Direction = FVector::ForwardVector;
		}
	}

	KnockbackVelocity = Direction * ResolvedSpeed;
	KnockbackDurationSeconds = KnockbackDuration;
	KnockbackSecondsRemaining = FMath::Max(KnockbackSecondsRemaining, KnockbackDuration);
}

void AT66MobBase::ApplyPhysicalKnockback(const FVector& LaunchVelocity)
{
	if (LifecycleState != ET66MobLifecycleState::Active || CurrentHP <= 0.f)
	{
		return;
	}
	if (LaunchVelocity.IsNearlyZero())
	{
		return;
	}

	FVector Clamped = LaunchVelocity;
	const float MaxSpeed = FMath::Max(0.f, PhysicalKnockbackMaxLaunchSpeed);
	if (MaxSpeed > 0.f && Clamped.SizeSquared() > FMath::Square(MaxSpeed))
	{
		Clamped = Clamped.GetSafeNormal() * MaxSpeed;
	}

	PhysicalLaunchVelocity = Clamped;
	PhysicalLaunchRestZ = GetActorLocation().Z;
	PhysicalLaunchSecondsRemaining = FMath::Max(0.05f, PhysicalKnockbackMaxAirborneSeconds);
	bPhysicalLaunchActive = true;

	// Clear the legacy planar knockback so the two paths never fight; the manager checks
	// bPhysicalLaunchActive first and uses ballistic integration while the launch is live.
	KnockbackVelocity = FVector::ZeroVector;
	KnockbackSecondsRemaining = 0.f;
	KnockbackDurationSeconds = 0.f;

	UE_LOG(LogT66MobBase, VeryVerbose, TEXT("ApplyPhysicalKnockback mob=%s launch=%s restZ=%.1f"),
		*GetName(), *Clamped.ToCompactString(), PhysicalLaunchRestZ);
}

void AT66MobBase::ApplyPullTowards(const FVector& PullOrigin, float Distance)
{
	UE_LOG(LogT66MobBase, VeryVerbose, TEXT("ApplyPullTowards mob=%s origin=%s distance=%.3f"), *GetName(), *PullOrigin.ToCompactString(), Distance);
}

void AT66MobBase::ApplyPushAwayFrom(const FVector& PushOrigin, float Distance)
{
	UE_LOG(LogT66MobBase, VeryVerbose, TEXT("ApplyPushAwayFrom mob=%s origin=%s distance=%.3f"), *GetName(), *PushOrigin.ToCompactString(), Distance);
}

void AT66MobBase::ShowLockIndicator()
{
	UE_LOG(LogT66MobBase, VeryVerbose, TEXT("ShowLockIndicator mob=%s"), *GetName());
	bIsLockedOn = true;
	if (LockIndicatorWidget)
	{
		LockIndicatorWidget->SetHiddenInGame(false, true);
		LockIndicatorWidget->SetVisibility(true, true);
	}
}

void AT66MobBase::HideLockIndicator()
{
	UE_LOG(LogT66MobBase, VeryVerbose, TEXT("HideLockIndicator mob=%s"), *GetName());
	bIsLockedOn = false;
	if (LockIndicatorWidget)
	{
		LockIndicatorWidget->SetHiddenInGame(true, true);
		LockIndicatorWidget->SetVisibility(false, true);
	}
}

void AT66MobBase::FireProjectile()
{
	const AT66HeroBase* Hero = nullptr;
	if (UWorld* World = GetWorld())
	{
		if (const APlayerController* PlayerController = World->GetFirstPlayerController())
		{
			Hero = Cast<AT66HeroBase>(PlayerController->GetPawn());
		}
	}

	(void)TryFireProjectileAtHero(Hero);
}

bool AT66MobBase::TryFireProjectileAtHero(const AT66HeroBase* Hero)
{
	if (!Hero)
	{
		return false;
	}

	UWorld* World = GetWorld();
	if (!World)
	{
		return false;
	}
	UT66MobManagerSubsystem* MobManager = World->GetSubsystem<UT66MobManagerSubsystem>();

	const FVector Start = GetActorLocation() + FVector(0.f, 0.f, ProjectileSpawnHeight);
	const FVector Target = Hero->GetActorLocation() + FVector(0.f, 0.f, 60.f);
	const float Dist2D = FVector::Dist2D(GetActorLocation(), Hero->GetActorLocation());
	FString LOSBlockerName;
	const AActor* LOSBlockerActor = nullptr;
	const UPrimitiveComponent* LOSBlockerComponent = nullptr;
	if (!HasProjectileLineOfSightToHero(Hero, Start, Target, LOSBlockerName, LOSBlockerActor, LOSBlockerComponent))
	{
		if (MobManager)
		{
			MobManager->RecordRangedLosBlocked(true, MobID, Dist2D, LOSBlockerActor, LOSBlockerComponent);
		}
		UE_LOG(
			LogT66MobBase,
			VeryVerbose,
			TEXT("[MobRange] BlockedShot mob=%s MobID=%s hero=%s dist2D=%.1f start=%s target=%s blocker=%s"),
			*GetName(),
			*MobID.ToString(),
			*Hero->GetName(),
			Dist2D,
			*Start.ToCompactString(),
			*Target.ToCompactString(),
			*LOSBlockerName);
		return false;
	}
	if (MobManager)
	{
		MobManager->RecordRangedLosPassed(true, MobID, Dist2D);
	}

	FVector ShotDirection = Target - Start;
	if (!ShotDirection.Normalize())
	{
		if (MobManager)
		{
			MobManager->RecordRangedZeroDirectionShot(true, MobID);
		}
		return false;
	}
	if (MobManager)
	{
		MobManager->RecordRangedDispatchReached(true, MobID, Dist2D);
	}
	UT66ProjectileManagerSubsystem* ProjectileManager = World->GetSubsystem<UT66ProjectileManagerSubsystem>();
	if (ProjectileManager && ProjectileManager->FireProjectile(this, MobID, Start, ShotDirection, 2400.f, 20.f, 18.f, 4.f, UT66ProjectileManagerSubsystem::EnemySpitProjectileTypeIndex))
	{
		if (MobManager)
		{
			MobManager->RecordRangedProjectileSpawned(true, MobID);
		}
		UE_LOG(
			LogT66MobBase,
			VeryVerbose,
			TEXT("[MobRange] FiredShot mob=%s MobID=%s projectile=ManagedEnemySpit hero=%s dist2D=%.1f start=%s target=%s"),
			*GetName(),
			*MobID.ToString(),
			*Hero->GetName(),
			Dist2D,
			*Start.ToCompactString(),
			*Target.ToCompactString());
		return true;
	}

	if (MobManager)
	{
		MobManager->RecordRangedProjectileSpawnFailed(true, MobID);
	}
	return false;
}

bool AT66MobBase::HasProjectileLineOfSightToHero(const AT66HeroBase* Hero, const FVector& Start, const FVector& End, FString& OutBlockerName, const AActor*& OutBlockerActor, const UPrimitiveComponent*& OutBlockerComponent) const
{
	OutBlockerName = TEXT("None");
	OutBlockerActor = nullptr;
	OutBlockerComponent = nullptr;
	UWorld* World = GetWorld();
	if (!World || !Hero)
	{
		OutBlockerName = TEXT("NoWorldOrHero");
		return false;
	}

	FCollisionQueryParams Params(SCENE_QUERY_STAT(T66MobRangedProjectileLOS), false, this);
	FHitResult Hit;
	if (!World->LineTraceSingleByChannel(Hit, Start, End, ECC_Visibility, Params))
	{
		return true;
	}

	const AActor* HitActor = Hit.GetActor();
	if (HitActor == Hero)
	{
		return true;
	}

	OutBlockerActor = HitActor;
	OutBlockerComponent = Hit.GetComponent();
	OutBlockerName = HitActor
		? FString::Printf(TEXT("%s/%s"), *HitActor->GetName(), HitActor->GetClass() ? *HitActor->GetClass()->GetName() : TEXT("None"))
		: FString(TEXT("WorldStatic"));
	return false;
}

void AT66MobBase::ResetForReuse()
{
	UE_LOG(LogT66MobBase, VeryVerbose, TEXT("ResetForReuse mob=%s"), *GetName());
	CurrentHP = MaxHP;
	StoredVelocity = FVector::ZeroVector;
	bIsRushing = false;
	RushDirection = FVector::ZeroVector;
	RushSecondsRemaining = 0.f;
	RushCooldownRemaining = EnemyFamily == ET66EnemyFamily::Rush ? InitialRushCooldownSeconds : 0.f;
	HoverAnchorZ = 0.f;
	HoverBobTime = HoverBobFrequency > KINDA_SMALL_NUMBER
		? FMath::FRandRange(0.f, (2.f * PI) / HoverBobFrequency)
		: 0.f;
	DesiredMinRange = 800.f;
	DesiredMaxRange = 1400.f;
	FireRangeGrace = 150.f;
	ProjectileSpawnHeight = 80.f;
	FireCooldownDuration = 1.6f;
	FireCooldownRemaining = EnemyFamily == ET66EnemyFamily::Ranged ? 0.6f : 0.f;
	ProjectileClass = EnemyFamily == ET66EnemyFamily::Ranged ? AT66SpitProjectile::StaticClass() : nullptr;
	StunSecondsRemaining = 0.f;
	RootSecondsRemaining = 0.f;
	FreezeSecondsRemaining = 0.f;
	SlowSecondsRemaining = 0.f;
	SlowMultiplier = 1.f;
	SlowStrength = 0.f;
	KnockbackSecondsRemaining = 0.f;
	KnockbackDurationSeconds = 0.f;
	KnockbackVelocity = FVector::ZeroVector;
	bPhysicalLaunchActive = false;
	PhysicalLaunchVelocity = FVector::ZeroVector;
	PhysicalLaunchSecondsRemaining = 0.f;
	PhysicalLaunchRestZ = 0.f;
	TouchDamageCooldownSeconds = 0.f;
	bIsTouchingHero = false;
	HideLockIndicator();
	LifecycleState = ET66MobLifecycleState::Active;
	SetActorHiddenInGame(false);
	SetActorEnableCollision(true);
	SetActorTickEnabled(false);
	if (CapsuleComponent)
	{
		CapsuleComponent->SetComponentTickEnabled(false);
		CapsuleComponent->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
		CapsuleComponent->SetGenerateOverlapEvents(true);
	}
	if (BodyHitZone)
	{
		BodyHitZone->SetComponentTickEnabled(false);
		BodyHitZone->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
		BodyHitZone->SetGenerateOverlapEvents(true);
		BodyHitZone->bTargetable = true;
	}
	if (HeadHitZone)
	{
		HeadHitZone->SetComponentTickEnabled(false);
		HeadHitZone->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
		HeadHitZone->SetGenerateOverlapEvents(true);
		HeadHitZone->bTargetable = true;
	}
	if (VisualMesh)
	{
		VisualMesh->SetComponentTickEnabled(false);
		VisualMesh->SetHiddenInGame(false, true);
		VisualMesh->SetVisibility(true, true);
	}
	if (LockIndicatorWidget)
	{
		LockIndicatorWidget->SetComponentTickEnabled(false);
	}
}
