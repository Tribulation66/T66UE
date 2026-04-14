// Copyright Tribulation 66. All Rights Reserved.

#include "Gameplay/T66MiniPlayerPawn.h"

#include "Camera/CameraComponent.h"
#include "Components/BillboardComponent.h"
#include "Components/SphereComponent.h"
#include "Core/T66GameInstance.h"
#include "Core/T66MiniDataSubsystem.h"
#include "Core/T66MiniFrontendStateSubsystem.h"
#include "Core/T66MiniRunStateSubsystem.h"
#include "Core/T66MiniVFXSubsystem.h"
#include "Core/T66MiniVisualSubsystem.h"
#include "Data/T66MiniDataTypes.h"
#include "Engine/Texture2D.h"
#include "EngineUtils.h"
#include "Gameplay/Components/T66MiniDirectionResolverComponent.h"
#include "Gameplay/Components/T66MiniHitFlashComponent.h"
#include "Gameplay/Components/T66MiniPickupMagnetComponent.h"
#include "Gameplay/Components/T66MiniShadowComponent.h"
#include "Gameplay/Components/T66MiniSpritePresentationComponent.h"
#include "Gameplay/T66MiniEnemyBase.h"
#include "Gameplay/T66MiniGameMode.h"
#include "Gameplay/T66MiniProjectile.h"
#include "GameFramework/PlayerController.h"
#include "GameFramework/SpringArmComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Save/T66MiniRunSaveGame.h"
#include "Sound/SoundBase.h"

AT66MiniPlayerPawn::AT66MiniPlayerPawn()
{
	PrimaryActorTick.bCanEverTick = true;

	SceneRoot = CreateDefaultSubobject<USceneComponent>(TEXT("SceneRoot"));
	SetRootComponent(SceneRoot);

	CollisionComponent = CreateDefaultSubobject<USphereComponent>(TEXT("Collision"));
	CollisionComponent->SetupAttachment(SceneRoot);
	CollisionComponent->InitSphereRadius(34.f);
	CollisionComponent->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	CollisionComponent->SetCollisionResponseToAllChannels(ECR_Ignore);
	CollisionComponent->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);

	VisualPivotComponent = CreateDefaultSubobject<USceneComponent>(TEXT("VisualPivot"));
	VisualPivotComponent->SetupAttachment(SceneRoot);

	LegsSpriteComponent = CreateDefaultSubobject<UBillboardComponent>(TEXT("LegsSprite"));
	LegsSpriteComponent->SetupAttachment(VisualPivotComponent);
	LegsSpriteComponent->SetRelativeLocation(FVector(0.f, 0.f, 55.5f));
	LegsSpriteComponent->SetRelativeScale3D(FVector(2.75f, 2.75f, 2.75f));
	LegsSpriteComponent->SetHiddenInGame(false, true);
	LegsSpriteComponent->SetVisibility(false, true);

	SpriteComponent = CreateDefaultSubobject<UBillboardComponent>(TEXT("Sprite"));
	SpriteComponent->SetupAttachment(VisualPivotComponent);
	SpriteComponent->SetRelativeLocation(FVector(0.f, 0.f, 56.f));
	SpriteComponent->SetRelativeScale3D(FVector(2.75f, 2.75f, 2.75f));
	SpriteComponent->SetHiddenInGame(false, true);
	SpriteComponent->SetVisibility(true, true);

	SpritePresentationComponent = CreateDefaultSubobject<UT66MiniSpritePresentationComponent>(TEXT("SpritePresentation"));
	SpritePresentationComponent->BindSprite(SpriteComponent);
	SpritePresentationComponent->BindLowerBodySprite(LegsSpriteComponent);
	SpritePresentationComponent->BindVisualRoot(VisualPivotComponent);

	DirectionResolverComponent = CreateDefaultSubobject<UT66MiniDirectionResolverComponent>(TEXT("DirectionResolver"));
	DirectionResolverComponent->BindPresentation(SpritePresentationComponent);

	ShadowComponent = CreateDefaultSubobject<UT66MiniShadowComponent>(TEXT("Shadow"));
	ShadowComponent->InitializeShadow(SceneRoot, FVector(0.f, 0.f, 1.5f), FVector(0.34f, 0.34f, 1.f), FLinearColor(0.f, 0.f, 0.f, 0.18f));

	HitFlashComponent = CreateDefaultSubobject<UT66MiniHitFlashComponent>(TEXT("HitFlash"));
	HitFlashComponent->InitializeFlash(SceneRoot, FVector(0.f, 0.f, 5.f), FVector(0.38f, 0.38f, 1.f), FLinearColor(1.0f, 0.82f, 0.48f, 0.44f));

	PickupMagnetComponent = CreateDefaultSubobject<UT66MiniPickupMagnetComponent>(TEXT("PickupMagnet"));

	CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));
	CameraBoom->SetupAttachment(SceneRoot);
	CameraBoom->TargetArmLength = 1800.f;
	CameraBoom->SetUsingAbsoluteLocation(true);
	CameraBoom->SetUsingAbsoluteRotation(true);
	CameraBoom->SetWorldRotation(FRotator(-90.f, 0.f, 0.f));
	CameraBoom->bDoCollisionTest = false;
	CameraBoom->bInheritPitch = false;
	CameraBoom->bInheritRoll = false;
	CameraBoom->bInheritYaw = false;

	CameraComponent = CreateDefaultSubobject<UCameraComponent>(TEXT("Camera"));
	CameraComponent->SetupAttachment(CameraBoom, USpringArmComponent::SocketName);
	CameraComponent->ProjectionMode = ECameraProjectionMode::Orthographic;
	CameraComponent->OrthoWidth = 2800.f;

	AutoPossessPlayer = EAutoReceiveInput::Disabled;
}

void AT66MiniPlayerPawn::BeginPlay()
{
	Super::BeginPlay();
	DesiredMoveLocation = GetActorLocation();
	InitializeFromMiniRun();
	UpdateCameraAnchor();
}

void AT66MiniPlayerPawn::Tick(const float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	if (!bInitializedFromRun)
	{
		InitializeFromMiniRun();
	}

	if (!IsHeroAlive())
	{
		return;
	}

	if (PassiveRegenPerSecond > 0.f)
	{
		CurrentHealth = FMath::Min(MaxHealth, CurrentHealth + (PassiveRegenPerSecond * DeltaSeconds));
	}

	const FVector CurrentLocation = GetActorLocation();
	const FVector FlatDesired = FVector(DesiredMoveLocation.X, DesiredMoveLocation.Y, CurrentLocation.Z);
	const FVector Delta = FlatDesired - CurrentLocation;
	const float Distance = Delta.Size2D();
	if (Distance > KINDA_SMALL_NUMBER)
	{
		const float StepDistance = MoveSpeed * DeltaSeconds;
		const FVector Step = Delta.GetSafeNormal2D() * FMath::Min(Distance, StepDistance);
		FVector NextLocation = CurrentLocation + FVector(Step.X, Step.Y, 0.f);
		if (AT66MiniGameMode* MiniGameMode = GetWorld() ? Cast<AT66MiniGameMode>(GetWorld()->GetAuthGameMode()) : nullptr)
		{
			NextLocation = MiniGameMode->ClampPointToArena(NextLocation);
		}
		SetActorLocation(NextLocation);
	}

	UpdateCameraAnchor();
	UpdateCombat(DeltaSeconds);
}

void AT66MiniPlayerPawn::SetDesiredMoveLocation(const FVector& InDesiredLocation)
{
	DesiredMoveLocation = InDesiredLocation;
}

void AT66MiniPlayerPawn::SetAimLocation(const FVector& InAimLocation)
{
	if (DirectionResolverComponent)
	{
		DirectionResolverComponent->SetFacingWorldTarget(InAimLocation);
	}
}

void AT66MiniPlayerPawn::GainMaterials(const int32 Amount)
{
	int32 AppliedAmount = Amount;
	if (Amount > 0)
	{
		AppliedAmount = FMath::RoundToInt(static_cast<float>(Amount) * MaterialGainMultiplier);
	}

	Materials = FMath::Max(0, Materials + AppliedAmount);
	if (AppliedAmount > 0)
	{
		Gold = FMath::Max(0, Gold + AppliedAmount);
	}
}

void AT66MiniPlayerPawn::GainGold(const int32 Amount)
{
	if (Amount == 0)
	{
		return;
	}

	Gold = FMath::Max(0, Gold + Amount);
}

bool AT66MiniPlayerPawn::SpendGold(const int32 Amount)
{
	if (Amount <= 0)
	{
		return true;
	}

	if (Gold < Amount)
	{
		return false;
	}

	Gold -= Amount;
	return true;
}

void AT66MiniPlayerPawn::GainExperience(const float Amount)
{
	if (Amount <= 0.f)
	{
		return;
	}

	Experience += Amount;
	while (Experience >= GetNextLevelThreshold())
	{
		Experience -= GetNextLevelThreshold();
		++HeroLevel;
		MaxHealth += 6.f;
		CurrentHealth = FMath::Min(MaxHealth, CurrentHealth + 5.f);
		BaseDamageStat += 0.35f;
		BaseAttackSpeedStat += 0.12f;
		MoveSpeed += 18.f;
	}
}

void AT66MiniPlayerPawn::Heal(const float Amount)
{
	if (Amount <= 0.f)
	{
		return;
	}

	CurrentHealth = FMath::Min(MaxHealth, CurrentHealth + Amount);
}

void AT66MiniPlayerPawn::GrantHeart(const int32 HeartCount, const bool bHealToFull)
{
	if (HeartCount <= 0)
	{
		return;
	}

	MaxHealth += HeartHealthUnit * static_cast<float>(HeartCount);
	if (bHealToFull)
	{
		CurrentHealth = MaxHealth;
	}
	else
	{
		CurrentHealth = FMath::Min(MaxHealth, CurrentHealth + (HeartHealthUnit * static_cast<float>(HeartCount)));
	}
}

void AT66MiniPlayerPawn::ApplyDamage(const float Amount)
{
	if (Amount <= 0.f || !IsHeroAlive())
	{
		return;
	}

	if (EvasionChance > 0.f && FMath::FRand() < EvasionChance)
	{
		return;
	}

	const float ArmorMitigation = BaseArmorStat / (BaseArmorStat + 20.f);
	const float FinalDamage = Amount * (1.f - ArmorMitigation);
	CurrentHealth = FMath::Max(0.f, CurrentHealth - FinalDamage);
	if (HitFlashComponent)
	{
		HitFlashComponent->TriggerHitFlash(FLinearColor(0.96f, 0.42f, 0.24f, 0.46f), 0.14f);
	}
	if (UT66MiniVFXSubsystem* VfxSubsystem = GetGameInstance() ? GetGameInstance()->GetSubsystem<UT66MiniVFXSubsystem>() : nullptr)
	{
		VfxSubsystem->SpawnPulse(GetWorld(), GetActorLocation() + FVector(0.f, 0.f, 8.f), FVector(0.26f, 0.26f, 1.f), 0.12f, FLinearColor(0.98f, 0.34f, 0.22f, 0.34f), 0.45f);
		VfxSubsystem->PlayHitSfx(this, 0.08f, 0.92f);
	}
	if (CurrentHealth > 0.f)
	{
		return;
	}

	if (AT66MiniGameMode* MiniGameMode = GetWorld() ? Cast<AT66MiniGameMode>(GetWorld()->GetAuthGameMode()) : nullptr)
	{
		MiniGameMode->HandlePlayerDefeated();
	}
}

void AT66MiniPlayerPawn::HandleSuccessfulHit(const float DamageDealt)
{
	if (DamageDealt <= 0.f || LifeStealChance <= 0.f)
	{
		return;
	}

	if (FMath::FRand() < LifeStealChance)
	{
		Heal(FMath::Max(1.f, DamageDealt * 0.08f));
	}
}

bool AT66MiniPlayerPawn::AcquireItem(const FName ItemID)
{
	if (ItemID.IsNone())
	{
		return false;
	}

	UGameInstance* GameInstance = GetGameInstance();
	UT66MiniDataSubsystem* DataSubsystem = GameInstance ? GameInstance->GetSubsystem<UT66MiniDataSubsystem>() : nullptr;
	UT66MiniRunStateSubsystem* RunState = GameInstance ? GameInstance->GetSubsystem<UT66MiniRunStateSubsystem>() : nullptr;
	UT66MiniRunSaveGame* ActiveRun = RunState ? RunState->GetActiveRun() : nullptr;
	const FT66MiniItemDefinition* ItemDefinition = DataSubsystem ? DataSubsystem->FindItem(ItemID) : nullptr;
	if (!ActiveRun || !ItemDefinition)
	{
		return false;
	}

	ActiveRun->OwnedItemIDs.Add(ItemID);
	ApplyItemDefinition(*ItemDefinition);
	CurrentHealth = FMath::Min(CurrentHealth, MaxHealth);
	RefreshPickupMagnetProfile();
	return true;
}

void AT66MiniPlayerPawn::ApplyItemDefinition(const FT66MiniItemDefinition& ItemDefinition)
{
	const FString& Stat = ItemDefinition.SecondaryStatType;
	if (Stat == TEXT("AoeDamage") || Stat == TEXT("BounceDamage") || Stat == TEXT("PierceDamage") || Stat == TEXT("DotDamage"))
	{
		BaseDamageStat += 1.6f;
	}
	else if (Stat == TEXT("AoeSpeed") || Stat == TEXT("BounceSpeed") || Stat == TEXT("PierceSpeed") || Stat == TEXT("DotSpeed"))
	{
		BaseAttackSpeedStat += 0.22f;
	}
	else if (Stat == TEXT("AoeScale"))
	{
		AoeRadiusBonus += 70.f;
	}
	else if (Stat == TEXT("BounceScale"))
	{
		++BonusBounceCount;
	}
	else if (Stat == TEXT("PierceScale"))
	{
		++BonusPierceCount;
	}
	else if (Stat == TEXT("DotScale"))
	{
		DotDamageBonus += 1.0f;
	}
	else if (Stat == TEXT("AttackRange") || Stat == TEXT("Accuracy"))
	{
		AttackRange += 110.f;
	}
	else if (Stat == TEXT("CritChance"))
	{
		CritChance += 0.05f;
	}
	else if (Stat == TEXT("CritDamage"))
	{
		CritDamageMultiplier += 0.30f;
	}
	else if (Stat == TEXT("DamageReduction") || Stat == TEXT("Taunt"))
	{
		BaseArmorStat += 1.4f;
	}
	else if (Stat == TEXT("HpRegen"))
	{
		PassiveRegenPerSecond += 1.2f;
	}
	else if (Stat == TEXT("LifeSteal"))
	{
		LifeStealChance += 0.08f;
	}
	else if (Stat == TEXT("EvasionChance") || Stat == TEXT("Invisibility"))
	{
		EvasionChance += 0.06f;
	}
	else if (Stat == TEXT("Cheating"))
	{
		MaxHealth += 18.f;
		CurrentHealth += 18.f;
	}
	else if (Stat == TEXT("Assassinate") || Stat == TEXT("Crush") || Stat == TEXT("Stealing"))
	{
		BonusDamageMultiplier += 0.06f;
	}
	else if (Stat == TEXT("CounterAttack") || Stat == TEXT("ReflectDamage"))
	{
		BaseArmorStat += 0.75f;
		BaseDamageStat += 0.4f;
	}
	else if (Stat == TEXT("TreasureChest") || Stat == TEXT("LootCrate"))
	{
		BaseLuckStat += 0.8f;
	}
	else if (Stat == TEXT("Alchemy"))
	{
		BaseLuckStat += 0.6f;
		MaterialGainMultiplier += 0.15f;
	}
}

void AT66MiniPlayerPawn::ApplyLevelUpBonuses(const int32 LevelsToApply)
{
	if (LevelsToApply <= 0)
	{
		return;
	}

	MaxHealth += LevelsToApply * 6.f;
	BaseDamageStat += LevelsToApply * 0.35f;
	BaseAttackSpeedStat += LevelsToApply * 0.12f;
	MoveSpeed += LevelsToApply * 18.f;
}

void AT66MiniPlayerPawn::InitializeFromMiniRun()
{
	UGameInstance* GameInstance = GetGameInstance();
	UT66MiniRunStateSubsystem* RunState = GameInstance ? GameInstance->GetSubsystem<UT66MiniRunStateSubsystem>() : nullptr;
	UT66MiniDataSubsystem* DataSubsystem = GameInstance ? GameInstance->GetSubsystem<UT66MiniDataSubsystem>() : nullptr;
	UT66MiniVisualSubsystem* VisualSubsystem = GameInstance ? GameInstance->GetSubsystem<UT66MiniVisualSubsystem>() : nullptr;
	UT66MiniRunSaveGame* ActiveRun = RunState ? RunState->GetActiveRun() : nullptr;
	if (!RunState || !DataSubsystem || !ActiveRun)
	{
		return;
	}

	HeroID = ActiveRun->HeroID;
	HeroLevel = FMath::Max(1, ActiveRun->HeroLevel);
	Materials = ActiveRun->Materials;
	Gold = ActiveRun->Gold;
	Experience = ActiveRun->Experience;
	const float SavedMaxHealth = ActiveRun->MaxHealth;
	const float SavedCurrentHealth = ActiveRun->CurrentHealth;
	CritChance = 0.05f;
	CritDamageMultiplier = 1.5f;
	PassiveRegenPerSecond = 0.f;
	EvasionChance = 0.f;
	LifeStealChance = 0.f;
	DotDamageBonus = 0.f;
	AoeRadiusBonus = 0.f;
	BonusDamageMultiplier = 1.f;
	BonusPierceCount = 0;
	BonusBounceCount = 0;
	MaterialGainMultiplier = 1.0f;

	if (const FT66MiniHeroDefinition* HeroDefinition = DataSubsystem->FindHero(HeroID))
	{
		HeroDisplayName = HeroDefinition->DisplayName;
		BaseDamageStat = FMath::Max(2.f, HeroDefinition->BaseDamage);
		BaseAttackSpeedStat = FMath::Max(1.f, HeroDefinition->BaseAttackSpeed);
		BaseArmorStat = HeroDefinition->BaseArmor;
		BaseLuckStat = HeroDefinition->BaseLuck;
		AttackRange = 1100.f + (HeroDefinition->BaseSpeed * 120.f);
		MoveSpeed = 700.f + (HeroDefinition->BaseSpeed * 120.f);
		MaxHealth = 90.f + (HeroDefinition->BaseArmor * 6.f);
		CurrentHealth = MaxHealth;
		ApplyLevelUpBonuses(HeroLevel - 1);

		if (VisualSubsystem)
		{
			UTexture2D* IdleTextureRight = VisualSubsystem->LoadHeroAnimationTexture(HeroDisplayName, TEXT("Idle_R"));
			UTexture2D* WalkATextureRight = VisualSubsystem->LoadHeroAnimationTexture(HeroDisplayName, TEXT("WalkA_R"));
			UTexture2D* WalkBTextureRight = VisualSubsystem->LoadHeroAnimationTexture(HeroDisplayName, TEXT("WalkB_R"));
			UTexture2D* WalkCTextureRight = VisualSubsystem->LoadHeroAnimationTexture(HeroDisplayName, TEXT("WalkC_R"));
			UTexture2D* AttackTextureRight = VisualSubsystem->LoadHeroAnimationTexture(HeroDisplayName, TEXT("Attack_R"));
			UTexture2D* IdleTextureLeft = VisualSubsystem->LoadHeroAnimationTexture(HeroDisplayName, TEXT("Idle_L"));
			UTexture2D* WalkATextureLeft = VisualSubsystem->LoadHeroAnimationTexture(HeroDisplayName, TEXT("WalkA_L"));
			UTexture2D* WalkBTextureLeft = VisualSubsystem->LoadHeroAnimationTexture(HeroDisplayName, TEXT("WalkB_L"));
			UTexture2D* WalkCTextureLeft = VisualSubsystem->LoadHeroAnimationTexture(HeroDisplayName, TEXT("WalkC_L"));
			UTexture2D* AttackTextureLeft = VisualSubsystem->LoadHeroAnimationTexture(HeroDisplayName, TEXT("Attack_L"));
			UTexture2D* UpperTextureRight = VisualSubsystem->LoadHeroAnimationTexture(HeroDisplayName, TEXT("Upper_R"));
			UTexture2D* UpperTextureLeft = VisualSubsystem->LoadHeroAnimationTexture(HeroDisplayName, TEXT("Upper_L"));
			UTexture2D* LowerIdleTextureRight = VisualSubsystem->LoadHeroAnimationTexture(HeroDisplayName, TEXT("LegsIdle_R"));
			UTexture2D* LowerWalkATextureRight = VisualSubsystem->LoadHeroAnimationTexture(HeroDisplayName, TEXT("LegsWalkA_R"));
			UTexture2D* LowerWalkBTextureRight = VisualSubsystem->LoadHeroAnimationTexture(HeroDisplayName, TEXT("LegsWalkB_R"));
			UTexture2D* LowerWalkCTextureRight = VisualSubsystem->LoadHeroAnimationTexture(HeroDisplayName, TEXT("LegsWalkC_R"));
			UTexture2D* LowerIdleTextureLeft = VisualSubsystem->LoadHeroAnimationTexture(HeroDisplayName, TEXT("LegsIdle_L"));
			UTexture2D* LowerWalkATextureLeft = VisualSubsystem->LoadHeroAnimationTexture(HeroDisplayName, TEXT("LegsWalkA_L"));
			UTexture2D* LowerWalkBTextureLeft = VisualSubsystem->LoadHeroAnimationTexture(HeroDisplayName, TEXT("LegsWalkB_L"));
			UTexture2D* LowerWalkCTextureLeft = VisualSubsystem->LoadHeroAnimationTexture(HeroDisplayName, TEXT("LegsWalkC_L"));
			const bool bHasFullBodyWalkCycle = WalkATextureRight || WalkATextureLeft || WalkBTextureRight || WalkBTextureLeft || WalkCTextureRight || WalkCTextureLeft;
			UTexture2D* HeroTexture = IdleTextureRight ? IdleTextureRight : (UpperTextureRight ? UpperTextureRight : VisualSubsystem->LoadHeroTexture(HeroDisplayName));
			if (HeroTexture)
			{
				SpriteComponent->SetSprite(HeroTexture);
				const bool bHasLegLayers = !bHasFullBodyWalkCycle && (LowerIdleTextureRight || LowerIdleTextureLeft);
				if (LegsSpriteComponent)
				{
					LegsSpriteComponent->SetVisibility(bHasLegLayers, true);
					if (bHasLegLayers)
					{
						LegsSpriteComponent->SetSprite(LowerIdleTextureRight ? LowerIdleTextureRight : LowerIdleTextureLeft);
					}
				}
				if (SpritePresentationComponent)
				{
					if (bHasFullBodyWalkCycle)
					{
						SpritePresentationComponent->SetAnimationSet(
							IdleTextureRight ? IdleTextureRight : HeroTexture,
							WalkATextureRight,
							WalkBTextureRight,
							WalkCTextureRight,
							AttackTextureRight,
							IdleTextureLeft,
							WalkATextureLeft,
							WalkBTextureLeft,
							WalkCTextureLeft,
							AttackTextureLeft);
						SpritePresentationComponent->SetLowerBodyAnimationSet(nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr);
					}
					else
					{
						SpritePresentationComponent->SetAnimationSet(
							HeroTexture,
							nullptr,
							nullptr,
							nullptr,
							AttackTextureRight ? AttackTextureRight : HeroTexture,
							UpperTextureLeft,
							nullptr,
							nullptr,
							nullptr,
							AttackTextureLeft ? AttackTextureLeft : UpperTextureLeft);
						SpritePresentationComponent->SetLowerBodyAnimationSet(
							LowerIdleTextureRight,
							LowerWalkATextureRight,
							LowerWalkBTextureRight,
							LowerWalkCTextureRight,
							LowerIdleTextureLeft,
							LowerWalkATextureLeft,
							LowerWalkBTextureLeft,
							LowerWalkCTextureLeft);
					}
				}
			}

			HeroProjectileTexture = VisualSubsystem->LoadHeroProjectileTexture(HeroDisplayName);
		}
	}

	for (const FName OwnedItemID : ActiveRun->OwnedItemIDs)
	{
		const FT66MiniItemDefinition* ItemDefinition = DataSubsystem->FindItem(OwnedItemID);
		if (!ItemDefinition)
		{
			continue;
		}

		ApplyItemDefinition(*ItemDefinition);
	}

	if (SavedMaxHealth > 0.f)
	{
		MaxHealth = SavedMaxHealth;
	}

	CurrentHealth = FMath::Clamp(SavedCurrentHealth, 0.f, MaxHealth);
	if (SpritePresentationComponent)
	{
		SpritePresentationComponent->SetBaseScale(SpriteComponent->GetRelativeScale3D());
	}
	RefreshPickupMagnetProfile();

	EquippedIdols.Reset();
	NextIdolProcIndex = 0;
	for (const FName IdolID : ActiveRun->EquippedIdolIDs)
	{
		if (const FT66MiniIdolDefinition* IdolDefinition = DataSubsystem->FindIdol(IdolID))
		{
			FEquippedIdolRuntime& IdolRuntime = EquippedIdols.AddDefaulted_GetRef();
			IdolRuntime.IdolID = IdolDefinition->IdolID;
			IdolRuntime.Category = IdolDefinition->Category;
			IdolRuntime.IconPath = IdolDefinition->IconPath;
			IdolRuntime.BaseDamage = IdolDefinition->BaseDamage;
			IdolRuntime.BaseProperty = IdolDefinition->BaseProperty;
			IdolRuntime.DamagePerLevel = IdolDefinition->DamagePerLevel;
			IdolRuntime.CooldownRemaining = 0.08f * static_cast<float>(EquippedIdols.Num() - 1);
			IdolRuntime.EffectTexture = VisualSubsystem ? VisualSubsystem->LoadIdolEffectTexture(IdolDefinition->IdolID) : nullptr;
		}
	}

	AutoAttackCooldownRemaining = 0.f;
	bInitializedFromRun = true;
}

void AT66MiniPlayerPawn::RefreshPickupMagnetProfile()
{
	if (PickupMagnetComponent)
	{
		PickupMagnetComponent->SetMagnetProfile(420.f + (BaseLuckStat * 18.f), 620.f + (BaseLuckStat * 24.f));
	}
}

void AT66MiniPlayerPawn::UpdateCameraAnchor()
{
	if (!CameraBoom || !CameraComponent)
	{
		return;
	}

	FVector CameraAnchor = GetActorLocation();
	if (AT66MiniGameMode* MiniGameMode = GetWorld() ? Cast<AT66MiniGameMode>(GetWorld()->GetAuthGameMode()) : nullptr)
	{
		int32 ViewportX = 1920;
		int32 ViewportY = 1080;
		if (APlayerController* PlayerController = Cast<APlayerController>(GetController()))
		{
			PlayerController->GetViewportSize(ViewportX, ViewportY);
		}

		const float AspectRatio = ViewportY > 0 ? (static_cast<float>(ViewportX) / static_cast<float>(ViewportY)) : (16.f / 9.f);
		const float HalfViewWidth = CameraComponent->OrthoWidth * 0.5f;
		const float HalfViewHeight = HalfViewWidth / FMath::Max(0.1f, AspectRatio);
		const FVector ArenaCenter = MiniGameMode->GetArenaOrigin();
		const float CameraClampX = FMath::Max(0.f, MiniGameMode->GetArenaHalfExtent() - HalfViewWidth);
		const float CameraClampY = FMath::Max(0.f, MiniGameMode->GetArenaHalfExtent() - HalfViewHeight);

		CameraAnchor.X = FMath::Clamp(GetActorLocation().X, ArenaCenter.X - CameraClampX, ArenaCenter.X + CameraClampX);
		CameraAnchor.Y = FMath::Clamp(GetActorLocation().Y, ArenaCenter.Y - CameraClampY, ArenaCenter.Y + CameraClampY);
		CameraAnchor.Z = ArenaCenter.Z + 20.f;
	}

	CameraBoom->SetWorldLocation(CameraAnchor);
	CameraBoom->SetWorldRotation(FRotator(-90.f, 0.f, 0.f));
}

void AT66MiniPlayerPawn::UpdateCombat(const float DeltaSeconds)
{
	for (FEquippedIdolRuntime& IdolRuntime : EquippedIdols)
	{
		IdolRuntime.CooldownRemaining = FMath::Max(0.f, IdolRuntime.CooldownRemaining - DeltaSeconds);
	}

	AutoAttackCooldownRemaining = FMath::Max(0.f, AutoAttackCooldownRemaining - DeltaSeconds);
	if (AutoAttackCooldownRemaining <= 0.f)
	{
		FireBasicAttack();
	}
}

void AT66MiniPlayerPawn::FireBasicAttack()
{
	AT66MiniEnemyBase* TargetEnemy = FindBestTarget(AttackRange);
	const float CooldownScale = FMath::Max(0.22f, 1.20f / FMath::Max(1.f, BaseAttackSpeedStat + (HeroLevel * 0.08f)));
	AutoAttackCooldownRemaining = CooldownScale;
	if (!TargetEnemy)
	{
		return;
	}

	UWorld* World = GetWorld();
	if (!World)
	{
		return;
	}

	const FVector SpawnLocation = GetActorLocation() + FVector(0.f, 0.f, 56.f);
	const FVector FireDirection = (TargetEnemy->GetActorLocation() - SpawnLocation).GetSafeNormal();
	float PrimaryDamage = ((BaseDamageStat * 0.95f) + (HeroLevel * 0.45f)) * BonusDamageMultiplier;
	const bool bDidCrit = FMath::FRand() < CritChance;
	if (bDidCrit)
	{
		PrimaryDamage *= CritDamageMultiplier;
	}

	FActorSpawnParameters SpawnParams;
	SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
	if (AT66MiniProjectile* Projectile = World->SpawnActor<AT66MiniProjectile>(AT66MiniProjectile::StaticClass(), SpawnLocation, FireDirection.Rotation(), SpawnParams))
	{
		Projectile->InitializeProjectile(
			this,
			SpawnLocation,
			FireDirection,
			ET66MiniProjectileBehavior::BasicAttack,
			PrimaryDamage,
			0.f,
			2400.f,
			180.f,
			1,
			0,
			0.f,
			0.5f,
			0.f,
			HeroProjectileTexture,
			nullptr,
			TargetEnemy,
			NAME_None,
			0.f);
	}

	if (SpritePresentationComponent)
	{
		SpritePresentationComponent->TriggerAttackFrame(0.10f);
	}

	if (GetWorld()->GetTimeSeconds() >= NextShotSoundTime)
	{
		if (UT66MiniVFXSubsystem* VfxSubsystem = GetGameInstance() ? GetGameInstance()->GetSubsystem<UT66MiniVFXSubsystem>() : nullptr)
		{
			VfxSubsystem->SpawnPulse(GetWorld(), GetActorLocation() + FVector(0.f, 0.f, 18.f), FVector(0.18f, 0.18f, 1.f), 0.09f, FLinearColor(1.0f, 0.86f, 0.40f, 0.28f), 0.55f);
			VfxSubsystem->PlayShotSfx(this, 0.18f, 1.0f + FMath::FRandRange(-0.06f, 0.06f));
		}

		NextShotSoundTime = GetWorld()->GetTimeSeconds() + 0.08f;
	}
}

void AT66MiniPlayerPawn::HandleBasicAttackImpact(AT66MiniEnemyBase* ImpactEnemy, const FVector& ImpactLocation)
{
	if (!ImpactEnemy || ImpactEnemy->IsEnemyDead() || EquippedIdols.Num() == 0)
	{
		return;
	}

	const int32 IdolCount = EquippedIdols.Num();
	for (int32 OffsetIndex = 0; OffsetIndex < IdolCount; ++OffsetIndex)
	{
		const int32 RuntimeIndex = (NextIdolProcIndex + OffsetIndex) % IdolCount;
		FEquippedIdolRuntime& IdolRuntime = EquippedIdols[RuntimeIndex];
		if (IdolRuntime.CooldownRemaining > 0.f)
		{
			continue;
		}

		TriggerIdolFollowUp(IdolRuntime, ImpactEnemy, ImpactLocation);
		NextIdolProcIndex = (RuntimeIndex + 1) % IdolCount;
		break;
	}
}

void AT66MiniPlayerPawn::TriggerIdolFollowUp(FEquippedIdolRuntime& IdolRuntime, AT66MiniEnemyBase* ImpactEnemy, const FVector& ImpactLocation)
{
	if (!ImpactEnemy || ImpactEnemy->IsEnemyDead())
	{
		return;
	}

	const float CooldownScale = FMath::Max(0.35f, 1.05f / FMath::Max(1.f, BaseAttackSpeedStat + (HeroLevel * 0.05f)));
	IdolRuntime.CooldownRemaining = CooldownScale;

	float FollowUpDamage = ((BaseDamageStat * 0.30f) + (IdolRuntime.BaseDamage * 0.35f) + (HeroLevel * IdolRuntime.DamagePerLevel * 0.20f)) * BonusDamageMultiplier;
	const float Radius = FMath::Max(240.f, IdolRuntime.BaseProperty + 140.f + AoeRadiusBonus);
	const float DotTickDamage = FMath::Max(1.5f, (FollowUpDamage * 0.28f) + DotDamageBonus);
	const float StunDuration = IdolRuntime.IdolID == FName(TEXT("Idol_Electric")) ? (0.16f + (HeroLevel * 0.004f)) : 0.f;
	const FVector VfxLocation = ImpactLocation + FVector(0.f, 0.f, 10.f);

	if (IdolRuntime.Category.Equals(TEXT("Bounce"), ESearchCase::IgnoreCase))
	{
		SpawnIdolImpactVfx(IdolRuntime, VfxLocation, IdolRuntime.IdolID == FName(TEXT("Idol_Electric")) ? 1.25f : 1.05f);
		ImpactEnemy->ApplyDamage(FollowUpDamage);
		HandleSuccessfulHit(FollowUpDamage);
		if (!ImpactEnemy->IsEnemyDead() && StunDuration > 0.f)
		{
			ImpactEnemy->ApplyStun(StunDuration);
		}

		if (AT66MiniEnemyBase* NextEnemy = FindClosestEnemyFromLocation(ImpactEnemy->GetActorLocation(), ImpactEnemy, FMath::Max(340.f, Radius)))
		{
			UWorld* World = GetWorld();
			if (World)
			{
				const FVector SpawnLocation = ImpactEnemy->GetActorLocation() + FVector(0.f, 0.f, 34.f);
				const FVector ChainDirection = (NextEnemy->GetActorLocation() - SpawnLocation).GetSafeNormal();
				FActorSpawnParameters SpawnParams;
				SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
				if (AT66MiniProjectile* Projectile = World->SpawnActor<AT66MiniProjectile>(AT66MiniProjectile::StaticClass(), SpawnLocation, ChainDirection.Rotation(), SpawnParams))
				{
					Projectile->InitializeProjectile(
						this,
						SpawnLocation,
						ChainDirection,
						ET66MiniProjectileBehavior::Bounce,
						0.f,
						FollowUpDamage * 0.85f,
						2150.f,
						FMath::Max(340.f, Radius),
						0,
						FMath::Max(0, BonusBounceCount),
						0.f,
						0.5f,
						0.f,
						nullptr,
						IdolRuntime.EffectTexture,
						NextEnemy,
						IdolRuntime.IdolID,
						StunDuration * 0.85f);
				}
			}
		}

		return;
	}

	if (IdolRuntime.Category.Equals(TEXT("AOE"), ESearchCase::IgnoreCase))
	{
		SpawnIdolImpactVfx(IdolRuntime, VfxLocation, 1.45f);
		ApplyAoeIdolBurst(ImpactEnemy->GetActorLocation(), FollowUpDamage, Radius);
		return;
	}

	if (IdolRuntime.Category.Equals(TEXT("DOT"), ESearchCase::IgnoreCase))
	{
		SpawnIdolImpactVfx(IdolRuntime, VfxLocation, 1.10f);
		ImpactEnemy->ApplyDamage(FollowUpDamage * 0.65f);
		ImpactEnemy->ApplyDot(DotTickDamage, 0.42f, 2.8f + (HeroLevel * 0.05f));
		HandleSuccessfulHit(FollowUpDamage * 0.65f);
		return;
	}

	SpawnIdolImpactVfx(IdolRuntime, VfxLocation, 1.0f);
	ImpactEnemy->ApplyDamage(FollowUpDamage);
	HandleSuccessfulHit(FollowUpDamage);
	if (AT66MiniEnemyBase* SecondaryEnemy = FindClosestEnemyFromLocation(ImpactEnemy->GetActorLocation(), ImpactEnemy, 260.f + (BonusPierceCount * 70.f)))
	{
		SpawnIdolImpactVfx(IdolRuntime, SecondaryEnemy->GetActorLocation() + FVector(0.f, 0.f, 10.f), 0.90f);
		SecondaryEnemy->ApplyDamage(FollowUpDamage * 0.70f);
		HandleSuccessfulHit(FollowUpDamage * 0.70f);
	}
}

void AT66MiniPlayerPawn::SpawnIdolImpactVfx(const FEquippedIdolRuntime& IdolRuntime, const FVector& ImpactLocation, const float ScaleMultiplier) const
{
	UT66MiniVFXSubsystem* VfxSubsystem = GetGameInstance() ? GetGameInstance()->GetSubsystem<UT66MiniVFXSubsystem>() : nullptr;
	if (!VfxSubsystem)
	{
		return;
	}

	const FVector Scale = FVector(0.22f, 0.22f, 1.f) * ScaleMultiplier;
	if (IdolRuntime.EffectTexture)
	{
		VfxSubsystem->SpawnSpritePulse(GetWorld(), ImpactLocation, Scale, 0.18f, FLinearColor::White, IdolRuntime.EffectTexture, 0.35f);
	}
	else
	{
		VfxSubsystem->SpawnPulse(GetWorld(), ImpactLocation, Scale, 0.16f, FLinearColor(1.0f, 0.84f, 0.40f, 0.30f), 0.45f);
	}
}

void AT66MiniPlayerPawn::ApplyAoeIdolBurst(const FVector& ImpactLocation, const float Damage, const float Radius)
{
	UWorld* World = GetWorld();
	if (!World)
	{
		return;
	}

	for (TActorIterator<AT66MiniEnemyBase> It(World); It; ++It)
	{
		AT66MiniEnemyBase* Candidate = *It;
		if (!Candidate || Candidate->IsEnemyDead())
		{
			continue;
		}

		const float DistanceSq = FVector::DistSquared2D(ImpactLocation, Candidate->GetActorLocation());
		if (DistanceSq > FMath::Square(Radius))
		{
			continue;
		}

		const float DistanceAlpha = 1.f - FMath::Clamp(FMath::Sqrt(DistanceSq) / FMath::Max(1.f, Radius), 0.f, 1.f);
		const float AppliedDamage = Damage * (0.65f + (DistanceAlpha * 0.35f));
		Candidate->ApplyDamage(AppliedDamage);
		HandleSuccessfulHit(AppliedDamage);
	}
}

AT66MiniEnemyBase* AT66MiniPlayerPawn::FindClosestEnemyFromLocation(const FVector& Origin, const AActor* IgnoreActor, const float MaxRange) const
{
	UWorld* World = GetWorld();
	if (!World)
	{
		return nullptr;
	}

	AT66MiniEnemyBase* BestEnemy = nullptr;
	float BestDistanceSq = FMath::Square(MaxRange);
	for (TActorIterator<AT66MiniEnemyBase> It(World); It; ++It)
	{
		AT66MiniEnemyBase* Candidate = *It;
		if (!Candidate || Candidate == IgnoreActor || Candidate->IsEnemyDead())
		{
			continue;
		}

		const float DistanceSq = FVector::DistSquared2D(Origin, Candidate->GetActorLocation());
		if (DistanceSq < BestDistanceSq)
		{
			BestDistanceSq = DistanceSq;
			BestEnemy = Candidate;
		}
	}

	return BestEnemy;
}

AT66MiniEnemyBase* AT66MiniPlayerPawn::FindBestTarget(const float MaxRange) const
{
	UWorld* World = GetWorld();
	if (!World)
	{
		return nullptr;
	}

	AT66MiniEnemyBase* BestEnemy = nullptr;
	float BestDistanceSq = FMath::Square(MaxRange);
	for (TActorIterator<AT66MiniEnemyBase> It(World); It; ++It)
	{
		AT66MiniEnemyBase* Candidate = *It;
		if (!Candidate || Candidate->IsEnemyDead())
		{
			continue;
		}

		const float DistanceSq = FVector::DistSquared2D(GetActorLocation(), Candidate->GetActorLocation());
		if (DistanceSq < BestDistanceSq)
		{
			BestDistanceSq = DistanceSq;
			BestEnemy = Candidate;
		}
	}

	return BestEnemy;
}

float AT66MiniPlayerPawn::GetNextLevelThreshold() const
{
	return 18.f + (HeroLevel * 10.f);
}

float AT66MiniPlayerPawn::GetPickupMagnetRadius() const
{
	return PickupMagnetComponent ? PickupMagnetComponent->GetMagnetRadius() : 420.f;
}

int32 AT66MiniPlayerPawn::GetMaxHearts() const
{
	return FMath::Max(1, FMath::CeilToInt(MaxHealth / HeartHealthUnit));
}

float AT66MiniPlayerPawn::GetHeartFill(const int32 HeartIndex) const
{
	if (HeartIndex < 0)
	{
		return 0.f;
	}

	const float HeartStart = static_cast<float>(HeartIndex) * HeartHealthUnit;
	return FMath::Clamp((CurrentHealth - HeartStart) / HeartHealthUnit, 0.f, 1.f);
}

float AT66MiniPlayerPawn::GetPickupMagnetPullSpeed() const
{
	return PickupMagnetComponent ? PickupMagnetComponent->GetPullSpeed() : 620.f;
}
