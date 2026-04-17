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
#include "Gameplay/T66SessionPlayerState.h"
#include "Gameplay/Components/T66MiniDirectionResolverComponent.h"
#include "Gameplay/Components/T66MiniHitFlashComponent.h"
#include "Gameplay/Components/T66MiniPickupMagnetComponent.h"
#include "Gameplay/Components/T66MiniShadowComponent.h"
#include "Gameplay/Components/T66MiniSpritePresentationComponent.h"
#include "Gameplay/T66MiniEnemyBase.h"
#include "Gameplay/T66MiniGameMode.h"
#include "Gameplay/T66MiniGameState.h"
#include "Gameplay/T66MiniProjectile.h"
#include "GameFramework/PlayerController.h"
#include "GameFramework/SpringArmComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Net/UnrealNetwork.h"
#include "Save/T66MiniSaveSubsystem.h"
#include "Save/T66MiniRunSaveGame.h"
#include "Sound/SoundBase.h"

namespace
{
	const TArray<TObjectPtr<AT66MiniEnemyBase>>* T66MiniResolveLiveEnemies(const UWorld* World)
	{
		const AT66MiniGameMode* MiniGameMode = World ? World->GetAuthGameMode<AT66MiniGameMode>() : nullptr;
		return MiniGameMode ? &MiniGameMode->GetLiveEnemies() : nullptr;
	}
}

AT66MiniPlayerPawn::AT66MiniPlayerPawn()
{
	PrimaryActorTick.bCanEverTick = true;
	bReplicates = true;
	bAlwaysRelevant = true;
	SetReplicateMovement(true);

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

void AT66MiniPlayerPawn::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(AT66MiniPlayerPawn, HeroID);
	DOREPLIFETIME(AT66MiniPlayerPawn, SelectedCompanionID);
	DOREPLIFETIME(AT66MiniPlayerPawn, BaseDamageStat);
	DOREPLIFETIME(AT66MiniPlayerPawn, BaseAttackSpeedStat);
	DOREPLIFETIME(AT66MiniPlayerPawn, BaseArmorStat);
	DOREPLIFETIME(AT66MiniPlayerPawn, AttackRange);
	DOREPLIFETIME(AT66MiniPlayerPawn, MaxHealth);
	DOREPLIFETIME(AT66MiniPlayerPawn, CurrentHealth);
	DOREPLIFETIME(AT66MiniPlayerPawn, CritChance);
	DOREPLIFETIME(AT66MiniPlayerPawn, PassiveRegenPerSecond);
	DOREPLIFETIME(AT66MiniPlayerPawn, EvasionChance);
	DOREPLIFETIME(AT66MiniPlayerPawn, LifeStealChance);
	DOREPLIFETIME(AT66MiniPlayerPawn, Materials);
	DOREPLIFETIME(AT66MiniPlayerPawn, Gold);
	DOREPLIFETIME(AT66MiniPlayerPawn, Experience);
	DOREPLIFETIME(AT66MiniPlayerPawn, HeroLevel);
	DOREPLIFETIME(AT66MiniPlayerPawn, UltimateCooldownRemaining);
	DOREPLIFETIME(AT66MiniPlayerPawn, UltimateCooldownDuration);
	DOREPLIFETIME(AT66MiniPlayerPawn, CurrentWaveIndex);
	DOREPLIFETIME(AT66MiniPlayerPawn, bEnduranceCheatUsedThisWave);
	DOREPLIFETIME(AT66MiniPlayerPawn, bQuickReviveReady);
	DOREPLIFETIME(AT66MiniPlayerPawn, UltimateType);
	DOREPLIFETIME(AT66MiniPlayerPawn, PassiveType);
	DOREPLIFETIME(AT66MiniPlayerPawn, EquippedIdolIDs);
	DOREPLIFETIME(AT66MiniPlayerPawn, OwnedItemIDs);
}

void AT66MiniPlayerPawn::BeginPlay()
{
	Super::BeginPlay();
	DesiredMoveLocation = GetActorLocation();
	if (HasAuthority() || IsLocallyControlled())
	{
		InitializeFromMiniRun();
	}

	if (IsLocallyControlled())
	{
		UpdateCameraAnchor();
	}
}

void AT66MiniPlayerPawn::Tick(const float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	if (!bInitializedFromRun && (HasAuthority() || IsLocallyControlled()))
	{
		InitializeFromMiniRun();
	}

	if (IsLocallyControlled())
	{
		UpdateCameraAnchor();
	}

	if (!HasAuthority() || !IsHeroAlive())
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

	UpdateCombat(DeltaSeconds);
}

void AT66MiniPlayerPawn::SetDesiredMoveLocation(const FVector& InDesiredLocation)
{
	DesiredMoveLocation = InDesiredLocation;
	if (!HasAuthority())
	{
		ServerSetDesiredMoveLocation(InDesiredLocation);
	}
}

void AT66MiniPlayerPawn::SetAimLocation(const FVector& InAimLocation)
{
	if (!HasAuthority())
	{
		ServerSetAimLocation(InAimLocation);
	}

	if (DirectionResolverComponent)
	{
		DirectionResolverComponent->SetFacingWorldTarget(InAimLocation);
	}
}

void AT66MiniPlayerPawn::ServerSetDesiredMoveLocation_Implementation(const FVector& InDesiredLocation)
{
	DesiredMoveLocation = InDesiredLocation;
}

void AT66MiniPlayerPawn::ServerSetAimLocation_Implementation(const FVector& InAimLocation)
{
	if (DirectionResolverComponent)
	{
		DirectionResolverComponent->SetFacingWorldTarget(InAimLocation);
	}
}

void AT66MiniPlayerPawn::ServerTryActivateUltimate_Implementation(const FVector& TargetLocation)
{
	TryActivateUltimate(TargetLocation);
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

	const int32 AppliedAmount = Amount > 0
		? FMath::RoundToInt(static_cast<float>(Amount) * GoldGainMultiplier)
		: Amount;
	Gold = FMath::Max(0, Gold + AppliedAmount);
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
		if (PassiveType == ET66PassiveType::Evasive)
		{
			bEvasiveNextAttackAppliesDot = true;
		}
		return;
	}

	const float ArmorMitigation = BaseArmorStat / (BaseArmorStat + 20.f);
	float FinalDamage = Amount * (1.f - ArmorMitigation);
	if (PassiveType == ET66PassiveType::IronWill)
	{
		FinalDamage *= CurrentHealth <= (MaxHealth * 0.35f) ? 0.72f : 0.86f;
	}
	if (PassiveType == ET66PassiveType::Unflinching)
	{
		FinalDamage *= 0.84f;
	}
	FinalDamage = FMath::Max(1.f, FinalDamage);
	if (PassiveType == ET66PassiveType::Endurance && !bEnduranceCheatUsedThisWave && CurrentHealth - FinalDamage <= 0.f)
	{
		bEnduranceCheatUsedThisWave = true;
		CurrentHealth = 1.f;
		PassiveSecondaryBuffRemaining = 4.0f;
		TemporaryDamageMultiplier = FMath::Max(TemporaryDamageMultiplier, 1.18f);
		if (UT66MiniVFXSubsystem* VfxSubsystem = GetGameInstance() ? GetGameInstance()->GetSubsystem<UT66MiniVFXSubsystem>() : nullptr)
		{
			VfxSubsystem->SpawnPulse(GetWorld(), GetActorLocation() + FVector(0.f, 0.f, 12.f), FVector(0.36f, 0.36f, 1.f), 0.20f, FLinearColor(0.94f, 0.92f, 0.42f, 0.44f), 0.75f);
		}
		return;
	}

	CurrentHealth = FMath::Max(0.f, CurrentHealth - FinalDamage);
	if (bQuickReviveReady && CurrentHealth <= 0.f)
	{
		bQuickReviveReady = false;
		CurrentHealth = FMath::Max(MaxHealth * 0.45f, 24.f);
		if (UT66MiniVFXSubsystem* VfxSubsystem = GetGameInstance() ? GetGameInstance()->GetSubsystem<UT66MiniVFXSubsystem>() : nullptr)
		{
			VfxSubsystem->SpawnPulse(GetWorld(), GetActorLocation() + FVector(0.f, 0.f, 14.f), FVector(0.42f, 0.42f, 1.f), 0.22f, FLinearColor(0.40f, 1.0f, 0.64f, 0.42f), 0.90f);
		}
		return;
	}
	if (AT66MiniGameMode* MiniGameMode = GetWorld() ? Cast<AT66MiniGameMode>(GetWorld()->GetAuthGameMode()) : nullptr)
	{
		MiniGameMode->AddCombatText(GetActorLocation() + FVector(0.f, 0.f, 92.f), FinalDamage, FLinearColor(1.0f, 0.22f, 0.20f, 1.0f), 0.95f, TEXT("-"));
	}
	if (HitFlashComponent)
	{
		HitFlashComponent->TriggerHitFlash(FLinearColor(0.96f, 0.42f, 0.24f, 0.46f), 0.14f);
	}
	if (UT66MiniVFXSubsystem* VfxSubsystem = GetGameInstance() ? GetGameInstance()->GetSubsystem<UT66MiniVFXSubsystem>() : nullptr)
	{
		VfxSubsystem->SpawnPulse(GetWorld(), GetActorLocation() + FVector(0.f, 0.f, 8.f), FVector(0.26f, 0.26f, 1.f), 0.12f, FLinearColor(0.98f, 0.34f, 0.22f, 0.34f), 0.45f);
		VfxSubsystem->PlayHitSfx(this, 0.08f, 0.92f);
	}
	HandlePostDamageTaken(FinalDamage);
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
		if (!ItemDefinition)
		{
			return false;
		}
	}

	OwnedItemIDs.Add(ItemID);
	if (ActiveRun)
	{
		ActiveRun->OwnedItemIDs = OwnedItemIDs;
	}
	ApplyItemDefinition(*ItemDefinition);
	CurrentHealth = FMath::Min(CurrentHealth, MaxHealth);
	RefreshPickupMagnetProfile();
	if (HasAuthority())
	{
		ForceNetUpdate();
	}
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
	UT66MiniSaveSubsystem* SaveSubsystem = GameInstance ? GameInstance->GetSubsystem<UT66MiniSaveSubsystem>() : nullptr;
	UT66MiniVisualSubsystem* VisualSubsystem = GameInstance ? GameInstance->GetSubsystem<UT66MiniVisualSubsystem>() : nullptr;
	UT66MiniRunSaveGame* ActiveRun = RunState ? RunState->GetActiveRun() : nullptr;
	const AT66MiniGameState* MiniGameState = GetWorld() ? GetWorld()->GetGameState<AT66MiniGameState>() : nullptr;
	const AT66SessionPlayerState* SessionPlayerState = GetPlayerState<AT66SessionPlayerState>();
	const FT66LobbyPlayerInfo* LobbyInfo = SessionPlayerState ? &SessionPlayerState->GetLobbyInfo() : nullptr;
	const bool bHasLobbySeed = LobbyInfo && LobbyInfo->bMiniFlowActive && !LobbyInfo->MiniSelectedHeroID.IsNone();
	const bool bHasReplicatedSeed = !HeroID.IsNone();
	const bool bOnlinePartyMode = MiniGameState && MiniGameState->bOnlinePartyMode;
	const FT66MiniPartyPlayerSnapshot* PartySnapshot = nullptr;
	if (bOnlinePartyMode && ActiveRun && SaveSubsystem)
	{
		int32 SnapshotIndex = INDEX_NONE;
		const FString SnapshotPlayerId = SessionPlayerState ? SessionPlayerState->GetSteamId() : FString();
		if ((!SnapshotPlayerId.IsEmpty() && SaveSubsystem->ResolvePartyPlayerSnapshotIndex(ActiveRun, SnapshotPlayerId, SnapshotIndex))
			|| SaveSubsystem->ResolveLocalPartyPlayerSnapshotIndex(ActiveRun, SnapshotIndex))
		{
			PartySnapshot = ActiveRun->PartyPlayerSnapshots.IsValidIndex(SnapshotIndex)
				? &ActiveRun->PartyPlayerSnapshots[SnapshotIndex]
				: nullptr;
		}
	}

	const bool bHasPartySnapshot = PartySnapshot != nullptr;
	const UT66MiniRunSaveGame* SeedRun = (bOnlinePartyMode && bHasPartySnapshot) ? ActiveRun : (bOnlinePartyMode ? nullptr : ActiveRun);
	if (!DataSubsystem || (!SeedRun && !bHasPartySnapshot && !bHasLobbySeed && !bHasReplicatedSeed))
	{
		return;
	}

	HeroID = SeedRun
		? SeedRun->HeroID
		: (bHasPartySnapshot ? PartySnapshot->HeroID : (bHasLobbySeed ? LobbyInfo->MiniSelectedHeroID : HeroID));
	SelectedCompanionID = SeedRun
		? SeedRun->CompanionID
		: (bHasPartySnapshot ? PartySnapshot->CompanionID : (bHasLobbySeed ? LobbyInfo->MiniSelectedCompanionID : SelectedCompanionID));
	EquippedIdolIDs = SeedRun
		? SeedRun->EquippedIdolIDs
		: (bHasPartySnapshot ? PartySnapshot->EquippedIdolIDs : (bHasLobbySeed ? LobbyInfo->MiniSelectedIdolIDs : EquippedIdolIDs));
	OwnedItemIDs = SeedRun ? SeedRun->OwnedItemIDs : (bHasPartySnapshot ? PartySnapshot->OwnedItemIDs : OwnedItemIDs);
	HeroLevel = SeedRun ? FMath::Max(1, SeedRun->HeroLevel) : (bHasPartySnapshot ? FMath::Max(1, PartySnapshot->HeroLevel) : FMath::Max(1, HeroLevel));
	Materials = SeedRun ? SeedRun->Materials : (bHasPartySnapshot ? PartySnapshot->Materials : Materials);
	Gold = SeedRun ? SeedRun->Gold : (bHasPartySnapshot ? PartySnapshot->Gold : Gold);
	Experience = SeedRun ? SeedRun->Experience : (bHasPartySnapshot ? PartySnapshot->Experience : Experience);
	const float SavedMaxHealth = SeedRun ? SeedRun->MaxHealth : (bHasPartySnapshot ? PartySnapshot->MaxHealth : MaxHealth);
	const float SavedCurrentHealth = SeedRun ? SeedRun->CurrentHealth : (bHasPartySnapshot ? PartySnapshot->CurrentHealth : CurrentHealth);
	CritChance = 0.05f;
	CritDamageMultiplier = 1.5f;
	PassiveRegenPerSecond = 0.f;
	EvasionChance = 0.f;
	LifeStealChance = 0.f;
	DotDamageBonus = 0.f;
	AoeRadiusBonus = 0.f;
	BonusDamageMultiplier = 1.f;
	TemporaryDamageMultiplier = 1.f;
	BonusPierceCount = 0;
	BonusBounceCount = 0;
	MaterialGainMultiplier = 1.0f;
	GoldGainMultiplier = 1.0f;
	UltimateCooldownRemaining = SeedRun ? SeedRun->UltimateCooldownRemaining : (bHasPartySnapshot ? PartySnapshot->UltimateCooldownRemaining : UltimateCooldownRemaining);
	bEnduranceCheatUsedThisWave = SeedRun ? SeedRun->bEnduranceCheatUsedThisWave : (bHasPartySnapshot ? PartySnapshot->bEnduranceCheatUsedThisWave : bEnduranceCheatUsedThisWave);
	bQuickReviveReady = SeedRun ? SeedRun->bQuickReviveReady : (bHasPartySnapshot ? PartySnapshot->bQuickReviveReady : bQuickReviveReady);
	PassiveBuffRemaining = 0.f;
	PassiveSecondaryBuffRemaining = 0.f;
	PassiveStacks = 0;
	PassiveShotCounter = 0;
	CurrentWaveIndex = SeedRun ? FMath::Max(1, SeedRun->WaveIndex) : FMath::Max(1, CurrentWaveIndex);
	UltimateType = ET66UltimateType::None;
	PassiveType = ET66PassiveType::None;
	QueuedBursts.Reset();
	ActiveAreaEffects.Reset();
	FocusTarget.Reset();
	bChaosNextAttackBounces = false;
	bEvasiveNextAttackAppliesDot = false;

	if (const FT66MiniHeroDefinition* HeroDefinition = DataSubsystem->FindHero(HeroID))
	{
		HeroDisplayName = HeroDefinition->DisplayName;
		UltimateType = HeroDefinition->UltimateType;
		PassiveType = HeroDefinition->PassiveType;
		BaseDamageStat = FMath::Max(2.f, HeroDefinition->BaseDamage);
		BaseAttackSpeedStat = FMath::Max(1.f, HeroDefinition->BaseAttackSpeed);
		BaseArmorStat = HeroDefinition->BaseArmor;
		BaseLuckStat = HeroDefinition->BaseLuck;
		AttackRange = 1100.f + (HeroDefinition->BaseSpeed * 120.f);
		MoveSpeed = 700.f + (HeroDefinition->BaseSpeed * 120.f);
		MaxHealth = 90.f + (HeroDefinition->BaseArmor * 6.f);
		CurrentHealth = MaxHealth;
		ApplyLevelUpBonuses(HeroLevel - 1);
	}

	RefreshHeroPresentation();

	switch (PassiveType)
	{
	case ET66PassiveType::IronWill:
		BaseArmorStat += 1.5f;
		break;

	case ET66PassiveType::ArcaneAmplification:
		BaseDamageStat += 0.6f;
		break;

	case ET66PassiveType::MarksmanFocus:
		AttackRange += 90.f;
		break;

	case ET66PassiveType::ToxinStacking:
		DotDamageBonus += 0.5f;
		break;

	case ET66PassiveType::QuickDraw:
		CritChance += 0.04f;
		break;

	case ET66PassiveType::Headshot:
		CritChance += 0.08f;
		AttackRange += 150.f;
		break;

	case ET66PassiveType::StaticCharge:
	case ET66PassiveType::Overclock:
		BaseAttackSpeedStat += 0.18f;
		break;

	case ET66PassiveType::ChaosTheory:
		BaseLuckStat += 0.8f;
		break;

	case ET66PassiveType::Endurance:
		MaxHealth += 12.f;
		CurrentHealth += 12.f;
		break;

	case ET66PassiveType::BrawlersFury:
		BaseDamageStat += 0.45f;
		break;

	case ET66PassiveType::Unflinching:
		BaseArmorStat += 2.0f;
		MaxHealth += 16.f;
		CurrentHealth += 16.f;
		break;

	case ET66PassiveType::TreasureHunter:
		MaterialGainMultiplier += 0.25f;
		GoldGainMultiplier += 0.20f;
		BaseLuckStat += 1.0f;
		break;

	case ET66PassiveType::Evasive:
		EvasionChance += 0.10f;
		MoveSpeed += 80.f;
		break;

	case ET66PassiveType::Frostbite:
		BonusDamageMultiplier += 0.04f;
		break;

	default:
		break;
	}

	for (const FName OwnedItemID : OwnedItemIDs)
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
	UltimateCooldownDuration = 16.0f + (CurrentWaveIndex * 1.1f);
	if (SpritePresentationComponent)
	{
		SpritePresentationComponent->SetBaseScale(SpriteComponent->GetRelativeScale3D());
	}
	RefreshPickupMagnetProfile();

	RefreshEquippedIdolRuntime();

	AutoAttackCooldownRemaining = 0.f;
	bInitializedFromRun = true;
}

void AT66MiniPlayerPawn::RefreshHeroPresentation()
{
	UGameInstance* GameInstance = GetGameInstance();
	UT66MiniDataSubsystem* DataSubsystem = GameInstance ? GameInstance->GetSubsystem<UT66MiniDataSubsystem>() : nullptr;
	UT66MiniVisualSubsystem* VisualSubsystem = GameInstance ? GameInstance->GetSubsystem<UT66MiniVisualSubsystem>() : nullptr;
	if (!DataSubsystem)
	{
		return;
	}

	HeroDisplayName.Reset();
	HeroProjectileTexture = nullptr;

	const FT66MiniHeroDefinition* HeroDefinition = DataSubsystem->FindHero(HeroID);
	if (!HeroDefinition)
	{
		return;
	}

	HeroDisplayName = HeroDefinition->DisplayName;
	if (!VisualSubsystem)
	{
		return;
	}

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
	if (!HeroTexture)
	{
		return;
	}

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

		SpritePresentationComponent->SetBaseScale(SpriteComponent->GetRelativeScale3D());
	}

	HeroProjectileTexture = VisualSubsystem->LoadHeroProjectileTexture(HeroDisplayName);
}

void AT66MiniPlayerPawn::RefreshEquippedIdolRuntime()
{
	UGameInstance* GameInstance = GetGameInstance();
	UT66MiniDataSubsystem* DataSubsystem = GameInstance ? GameInstance->GetSubsystem<UT66MiniDataSubsystem>() : nullptr;
	UT66MiniVisualSubsystem* VisualSubsystem = GameInstance ? GameInstance->GetSubsystem<UT66MiniVisualSubsystem>() : nullptr;

	EquippedIdols.Reset();
	NextIdolProcIndex = 0;
	if (!DataSubsystem)
	{
		return;
	}

	for (const FName IdolID : EquippedIdolIDs)
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
}

void AT66MiniPlayerPawn::OnRep_HeroConfiguration()
{
	RefreshHeroPresentation();
}

void AT66MiniPlayerPawn::OnRep_RuntimeInventory()
{
	RefreshEquippedIdolRuntime();
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
	UltimateCooldownRemaining = FMath::Max(0.f, UltimateCooldownRemaining - DeltaSeconds);
	PassiveBuffRemaining = FMath::Max(0.f, PassiveBuffRemaining - DeltaSeconds);
	if (PassiveType == ET66PassiveType::QuickDraw)
	{
		PassiveSecondaryBuffRemaining = FMath::Min(2.0f, PassiveSecondaryBuffRemaining + DeltaSeconds);
	}
	else
	{
		PassiveSecondaryBuffRemaining = FMath::Max(0.f, PassiveSecondaryBuffRemaining - DeltaSeconds);
	}

	if (UltimateType == ET66UltimateType::GoldRush)
	{
		GoldGainMultiplier = (PassiveType == ET66PassiveType::TreasureHunter ? 1.20f : 1.0f) * (PassiveSecondaryBuffRemaining > 0.f ? 1.6f : 1.0f);
	}

	if (PassiveBuffRemaining <= 0.f && (PassiveType == ET66PassiveType::RallyingBlow || PassiveType == ET66PassiveType::BrawlersFury || PassiveType == ET66PassiveType::Overclock))
	{
		PassiveStacks = 0;
	}
	if (PassiveSecondaryBuffRemaining <= 0.f && (UltimateType == ET66UltimateType::Juiced || UltimateType == ET66UltimateType::RabidFrenzy || PassiveType == ET66PassiveType::Endurance))
	{
		TemporaryDamageMultiplier = 1.f;
	}

	UpdateQueuedBursts(DeltaSeconds);
	UpdateAreaEffects(DeltaSeconds);

	for (FEquippedIdolRuntime& IdolRuntime : EquippedIdols)
	{
		IdolRuntime.CooldownRemaining = FMath::Max(0.f, IdolRuntime.CooldownRemaining - DeltaSeconds);
	}

	float AttackSpeedScalar = 1.f;
	if (PassiveType == ET66PassiveType::RallyingBlow && PassiveBuffRemaining > 0.f)
	{
		AttackSpeedScalar += PassiveStacks * 0.14f;
	}
	if (PassiveType == ET66PassiveType::BrawlersFury && PassiveBuffRemaining > 0.f)
	{
		AttackSpeedScalar += PassiveStacks * 0.08f;
	}
	if (PassiveType == ET66PassiveType::Overclock && PassiveBuffRemaining > 0.f)
	{
		AttackSpeedScalar += PassiveStacks * 0.10f;
	}

	AutoAttackCooldownRemaining = FMath::Max(0.f, AutoAttackCooldownRemaining - (DeltaSeconds * AttackSpeedScalar));
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
	float PrimaryDamage = ((BaseDamageStat * 0.95f) + (HeroLevel * 0.45f)) * BonusDamageMultiplier * TemporaryDamageMultiplier;
	if (PassiveType == ET66PassiveType::BrawlersFury && PassiveBuffRemaining > 0.f)
	{
		PrimaryDamage *= 1.f + (PassiveStacks * 0.06f);
	}
	if (PassiveType == ET66PassiveType::MarksmanFocus && FocusTarget.Get() == TargetEnemy)
	{
		PrimaryDamage *= 1.f + (PassiveStacks * 0.08f);
	}
	if (PassiveType == ET66PassiveType::ToxinStacking && TargetEnemy->HasActiveDot())
	{
		PrimaryDamage *= 1.16f;
	}
	if (PassiveType == ET66PassiveType::QuickDraw && PassiveSecondaryBuffRemaining >= 1.4f)
	{
		PrimaryDamage *= 1.45f;
		PassiveSecondaryBuffRemaining = 0.f;
	}
	float EffectiveCritChance = CritChance;
	if (PassiveType == ET66PassiveType::Headshot)
	{
		EffectiveCritChance += 0.10f;
	}
	const bool bGuaranteedCrit = PassiveType == ET66PassiveType::Headshot && ((PassiveShotCounter + 1) % 6 == 0);
	const bool bDidCrit = bGuaranteedCrit || (FMath::FRand() < EffectiveCritChance);
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

	if (PassiveType == ET66PassiveType::Overclock)
	{
		PassiveStacks = FMath::Clamp(PassiveStacks + 1, 0, 5);
		PassiveBuffRemaining = 2.2f;
	}
	if (PassiveType == ET66PassiveType::ChaosTheory)
	{
		++PassiveShotCounter;
		if (PassiveShotCounter % 5 == 0)
		{
			bChaosNextAttackBounces = true;
		}
	}
	else
	{
		++PassiveShotCounter;
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

bool AT66MiniPlayerPawn::TryActivateUltimate(const FVector& TargetLocation)
{
	if (!HasAuthority())
	{
		ServerTryActivateUltimate(TargetLocation);
		return true;
	}

	if (!IsHeroAlive() || UltimateCooldownRemaining > 0.f || UltimateType == ET66UltimateType::None)
	{
		return false;
	}

	const float BaseDamage = GetUltimateBaseDamage();
	const FVector HeroLocation = GetActorLocation();

	switch (UltimateType)
	{
	case ET66UltimateType::SpearStorm:
		for (int32 Index = -2; Index <= 2; ++Index)
		{
			const FVector Direction = (TargetLocation - HeroLocation).GetSafeNormal2D().RotateAngleAxis(static_cast<float>(Index) * 7.f, FVector::UpVector);
			FireProjectileTowardLocation(HeroLocation + (Direction * 180.f), BaseDamage * 1.25f, ET66MiniProjectileBehavior::Pierce, 2600.f, 240.f, 4, 0, 0.f, 0.5f, 0.f, 0.f);
		}
		UltimateCooldownDuration = 18.f;
		break;

	case ET66UltimateType::MeteorStrike:
		for (int32 Index = 0; Index < 5; ++Index)
		{
			const FVector Offset(FMath::FRandRange(-360.f, 360.f), FMath::FRandRange(-360.f, 360.f), 0.f);
			QueueBurst(TargetLocation + Offset, BaseDamage * 1.5f, 320.f, 0.28f + (Index * 0.12f), FLinearColor(1.0f, 0.52f, 0.18f, 0.32f), 0.f, 0.f, 0.0f, true);
		}
		UltimateCooldownDuration = 20.f;
		break;

	case ET66UltimateType::ChainLightning:
		ApplyAreaDamage(HeroLocation, 1250.f, BaseDamage * 1.18f, 0.12f, 0.f, 0.f);
		UltimateCooldownDuration = 18.f;
		break;

	case ET66UltimateType::PlagueCloud:
		AddAreaEffect(TargetLocation, 360.f, BaseDamage * 0.40f, 0.40f, 5.2f, FLinearColor(0.54f, 1.0f, 0.44f, 0.30f), 0.f);
		UltimateCooldownDuration = 17.f;
		break;

	case ET66UltimateType::PrecisionStrike:
	case ET66UltimateType::ScopedSniper:
	case ET66UltimateType::Deadeye:
		FireProjectileTowardLocation(TargetLocation, BaseDamage * (UltimateType == ET66UltimateType::PrecisionStrike ? 2.6f : 3.0f), ET66MiniProjectileBehavior::Pierce, 3200.f, 260.f, 6, 0, 0.f, 0.5f, 0.f, 0.15f);
		UltimateCooldownDuration = UltimateType == ET66UltimateType::PrecisionStrike ? 16.f : 19.f;
		break;

	case ET66UltimateType::FanTheHammer:
		for (int32 Index = -3; Index <= 3; ++Index)
		{
			const FVector Direction = (TargetLocation - HeroLocation).GetSafeNormal2D().RotateAngleAxis(static_cast<float>(Index) * 5.f, FVector::UpVector);
			FireProjectileTowardLocation(HeroLocation + (Direction * 160.f), BaseDamage * 0.78f, ET66MiniProjectileBehavior::Pierce, 3000.f, 170.f, 2, 0, 0.f, 0.5f, 0.f, 0.f);
		}
		UltimateCooldownDuration = 16.f;
		break;

	case ET66UltimateType::Discharge:
		ApplyAreaDamage(HeroLocation, 520.f, BaseDamage * 1.05f, 0.26f, 0.f, 0.f);
		UltimateCooldownDuration = 17.f;
		break;

	case ET66UltimateType::Juiced:
		TemporaryDamageMultiplier = 1.45f;
		PassiveSecondaryBuffRemaining = 6.0f;
		UltimateCooldownDuration = 22.f;
		break;

	case ET66UltimateType::DeathSpiral:
		for (int32 Index = 0; Index < 4; ++Index)
		{
			QueueBurst(HeroLocation, BaseDamage * (0.90f + (Index * 0.16f)), 260.f + (Index * 110.f), 0.05f + (Index * 0.18f), FLinearColor(0.62f, 0.34f, 1.0f, 0.28f), BaseDamage * 0.18f, 2.2f, 0.f, false);
		}
		UltimateCooldownDuration = 20.f;
		break;

	case ET66UltimateType::Shockwave:
		ApplyAreaDamage(HeroLocation, 460.f, BaseDamage * 1.35f, 0.22f, 0.f, 0.f);
		UltimateCooldownDuration = 17.f;
		break;

	case ET66UltimateType::TidalWave:
		for (int32 Index = 1; Index <= 4; ++Index)
		{
			const FVector BurstLocation = HeroLocation + ((TargetLocation - HeroLocation).GetSafeNormal2D() * (Index * 220.f));
			QueueBurst(BurstLocation, BaseDamage * 0.92f, 280.f, Index * 0.12f, FLinearColor(0.34f, 0.72f, 1.0f, 0.28f), 0.f, 0.f, 0.12f, false);
		}
		UltimateCooldownDuration = 18.f;
		break;

	case ET66UltimateType::GoldRush:
		PassiveSecondaryBuffRemaining = 7.5f;
		for (int32 Index = 0; Index < 5; ++Index)
		{
			const float AngleDeg = static_cast<float>(Index) * 72.f;
			const FVector Offset = FVector(1.f, 0.f, 0.f).RotateAngleAxis(AngleDeg, FVector::UpVector) * 240.f;
			QueueBurst(HeroLocation + Offset, BaseDamage, 220.f, 0.08f * Index, FLinearColor(1.0f, 0.84f, 0.24f, 0.28f), 0.f, 0.f, 0.f, false);
		}
		UltimateCooldownDuration = 21.f;
		break;

	case ET66UltimateType::MiasmaBomb:
		QueueBurst(TargetLocation, BaseDamage * 1.25f, 300.f, 0.25f, FLinearColor(0.62f, 1.0f, 0.54f, 0.26f), BaseDamage * 0.16f, 3.2f, 0.f, true);
		AddAreaEffect(TargetLocation, 260.f, BaseDamage * 0.24f, 0.45f, 4.4f, FLinearColor(0.56f, 1.0f, 0.48f, 0.20f), 0.f);
		UltimateCooldownDuration = 18.f;
		break;

	case ET66UltimateType::RabidFrenzy:
		TemporaryDamageMultiplier = 1.35f;
		PassiveSecondaryBuffRemaining = 6.5f;
		Heal(BaseDamage * 0.12f);
		AddAreaEffect(HeroLocation, 260.f, BaseDamage * 0.18f, 0.45f, 4.0f, FLinearColor(1.0f, 0.40f, 0.22f, 0.18f), 0.f);
		UltimateCooldownDuration = 19.f;
		break;

	case ET66UltimateType::Blizzard:
		AddAreaEffect(TargetLocation, 420.f, BaseDamage * 0.34f, 0.42f, 5.5f, FLinearColor(0.72f, 0.90f, 1.0f, 0.22f), 0.08f);
		UltimateCooldownDuration = 19.f;
		break;

	default:
		return false;
	}

	UltimateCooldownRemaining = UltimateCooldownDuration;
	if (UT66MiniVFXSubsystem* VfxSubsystem = GetGameInstance() ? GetGameInstance()->GetSubsystem<UT66MiniVFXSubsystem>() : nullptr)
	{
		VfxSubsystem->SpawnPulse(GetWorld(), HeroLocation + FVector(0.f, 0.f, 20.f), FVector(0.32f, 0.32f, 1.f), 0.16f, FLinearColor(1.0f, 0.88f, 0.40f, 0.30f), 0.65f);
		VfxSubsystem->PlayShotSfx(this, 0.24f, 0.78f);
	}

	return true;
}

FString AT66MiniPlayerPawn::GetUltimateLabel() const
{
	if (const UEnum* Enum = StaticEnum<ET66UltimateType>())
	{
		return Enum->GetDisplayNameTextByValue(static_cast<int64>(UltimateType)).ToString();
	}

	return TEXT("ULT");
}

FString AT66MiniPlayerPawn::GetPassiveLabel() const
{
	if (const UEnum* Enum = StaticEnum<ET66PassiveType>())
	{
		return Enum->GetDisplayNameTextByValue(static_cast<int64>(PassiveType)).ToString();
	}

	return TEXT("PASSIVE");
}

void AT66MiniPlayerPawn::HandleEnemyKilled(const AT66MiniEnemyBase* Enemy)
{
	if (PassiveType == ET66PassiveType::RallyingBlow)
	{
		PassiveStacks = FMath::Clamp(PassiveStacks + 1, 0, 3);
		PassiveBuffRemaining = 3.5f;
	}

	if (PassiveType == ET66PassiveType::TreasureHunter)
	{
		GainGold(2 + (Enemy && Enemy->IsBossEnemy() ? 6 : 0));
	}
}

void AT66MiniPlayerPawn::HandleWaveStarted(const int32 WaveIndex)
{
	CurrentWaveIndex = FMath::Max(1, WaveIndex);
	bEnduranceCheatUsedThisWave = false;
	FocusTarget.Reset();
	PassiveStacks = 0;
	PassiveBuffRemaining = 0.f;
}

void AT66MiniPlayerPawn::GrantQuickRevive()
{
	bQuickReviveReady = true;
}

void AT66MiniPlayerPawn::UpdateQueuedBursts(const float DeltaSeconds)
{
	for (int32 Index = QueuedBursts.Num() - 1; Index >= 0; --Index)
	{
		FQueuedBurst& Burst = QueuedBursts[Index];
		Burst.DelayRemaining -= DeltaSeconds;
		if (Burst.DelayRemaining > 0.f)
		{
			continue;
		}

		ApplyAreaDamage(Burst.Location, Burst.Radius, Burst.Damage, Burst.StunDuration, Burst.DotTickDamage, Burst.DotDuration);
		if (UT66MiniVFXSubsystem* VfxSubsystem = GetGameInstance() ? GetGameInstance()->GetSubsystem<UT66MiniVFXSubsystem>() : nullptr)
		{
			VfxSubsystem->SpawnPulse(GetWorld(), Burst.Location + FVector(0.f, 0.f, 8.f), FVector(FMath::Clamp(Burst.Radius / 600.f, 0.22f, 0.90f), FMath::Clamp(Burst.Radius / 600.f, 0.22f, 0.90f), 1.f), 0.16f, Burst.Tint, 0.75f);
		}

		QueuedBursts.RemoveAtSwap(Index);
	}
}

void AT66MiniPlayerPawn::UpdateAreaEffects(const float DeltaSeconds)
{
	for (int32 Index = ActiveAreaEffects.Num() - 1; Index >= 0; --Index)
	{
		FActiveAreaEffect& Effect = ActiveAreaEffects[Index];
		Effect.RemainingDuration -= DeltaSeconds;
		Effect.TickAccumulator += DeltaSeconds;

		while (Effect.TickAccumulator >= Effect.TickInterval && Effect.RemainingDuration > 0.f)
		{
			Effect.TickAccumulator -= Effect.TickInterval;
			ApplyAreaDamage(Effect.Location, Effect.Radius, Effect.TickDamage, Effect.StunDuration);
		}

		if (Effect.RemainingDuration <= 0.f)
		{
			ActiveAreaEffects.RemoveAtSwap(Index);
		}
	}
}

void AT66MiniPlayerPawn::FireProjectileTowardLocation(const FVector& TargetLocation, const float Damage, const ET66MiniProjectileBehavior Behavior, const float Speed, const float Radius, const int32 RemainingHits, const int32 RemainingBounces, const float DotTickDamage, const float DotTickInterval, const float DotDuration, const float StunDuration)
{
	UWorld* World = GetWorld();
	if (!World)
	{
		return;
	}

	const FVector SpawnLocation = GetActorLocation() + FVector(0.f, 0.f, 56.f);
	const FVector FireDirection = (TargetLocation - SpawnLocation).GetSafeNormal();
	FActorSpawnParameters SpawnParams;
	SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
	if (AT66MiniProjectile* Projectile = World->SpawnActor<AT66MiniProjectile>(AT66MiniProjectile::StaticClass(), SpawnLocation, FireDirection.Rotation(), SpawnParams))
	{
		Projectile->InitializeProjectile(
			this,
			SpawnLocation,
			FireDirection,
			Behavior,
			Damage,
			Damage * 0.82f,
			Speed,
			Radius,
			RemainingHits,
			RemainingBounces,
			DotTickDamage,
			DotTickInterval,
			DotDuration,
			HeroProjectileTexture,
			nullptr,
			nullptr,
			NAME_None,
			StunDuration);
	}
}

void AT66MiniPlayerPawn::ApplyAreaDamage(const FVector& Center, const float Radius, const float Damage, const float StunDuration, const float DotTickDamage, const float DotDuration)
{
	UWorld* World = GetWorld();
	if (!World)
	{
		return;
	}

	const TArray<TObjectPtr<AT66MiniEnemyBase>>* LiveEnemies = T66MiniResolveLiveEnemies(World);
	if (!LiveEnemies)
	{
		for (TActorIterator<AT66MiniEnemyBase> It(World); It; ++It)
		{
			AT66MiniEnemyBase* Candidate = *It;
			if (!Candidate || Candidate->IsEnemyDead())
			{
				continue;
			}

			const float DistanceSq = FVector::DistSquared2D(Center, Candidate->GetActorLocation());
			if (DistanceSq > FMath::Square(Radius))
			{
				continue;
			}

			float AppliedDamage = Damage * TemporaryDamageMultiplier;
			if (PassiveType == ET66PassiveType::ArcaneAmplification)
			{
				AppliedDamage *= 1.18f;
			}
			if (PassiveType == ET66PassiveType::ToxinStacking && Candidate->HasActiveDot())
			{
				AppliedDamage *= 1.14f;
			}

			Candidate->ApplyDamage(AppliedDamage);
			HandleSuccessfulHit(AppliedDamage);
			if (!Candidate->IsEnemyDead() && StunDuration > 0.f)
			{
				Candidate->ApplyStun(StunDuration);
			}
			if (!Candidate->IsEnemyDead() && DotTickDamage > 0.f && DotDuration > 0.f)
			{
				Candidate->ApplyDot(DotTickDamage, 0.40f, DotDuration);
			}
		}
		return;
	}

	for (AT66MiniEnemyBase* Candidate : *LiveEnemies)
	{
		if (!Candidate || Candidate->IsEnemyDead())
		{
			continue;
		}

		const float DistanceSq = FVector::DistSquared2D(Center, Candidate->GetActorLocation());
		if (DistanceSq > FMath::Square(Radius))
		{
			continue;
		}

		float AppliedDamage = Damage * TemporaryDamageMultiplier;
		if (PassiveType == ET66PassiveType::ArcaneAmplification)
		{
			AppliedDamage *= 1.18f;
		}
		if (PassiveType == ET66PassiveType::ToxinStacking && Candidate->HasActiveDot())
		{
			AppliedDamage *= 1.14f;
		}

		Candidate->ApplyDamage(AppliedDamage);
		HandleSuccessfulHit(AppliedDamage);
		if (!Candidate->IsEnemyDead() && StunDuration > 0.f)
		{
			Candidate->ApplyStun(StunDuration);
		}
		if (!Candidate->IsEnemyDead() && DotTickDamage > 0.f && DotDuration > 0.f)
		{
			Candidate->ApplyDot(DotTickDamage, 0.40f, DotDuration);
		}
	}
}

void AT66MiniPlayerPawn::QueueBurst(const FVector& Location, const float Damage, const float Radius, const float Delay, const FLinearColor& Tint, const float DotTickDamage, const float DotDuration, const float StunDuration, const bool bSpawnTelegraph)
{
	FQueuedBurst& Burst = QueuedBursts.AddDefaulted_GetRef();
	Burst.Location = Location;
	Burst.Damage = Damage;
	Burst.Radius = Radius;
	Burst.DelayRemaining = Delay;
	Burst.DotTickDamage = DotTickDamage;
	Burst.DotDuration = DotDuration;
	Burst.StunDuration = StunDuration;
	Burst.Tint = Tint;
	Burst.bSpawnTelegraph = bSpawnTelegraph;

	if (bSpawnTelegraph)
	{
		if (UT66MiniVFXSubsystem* VfxSubsystem = GetGameInstance() ? GetGameInstance()->GetSubsystem<UT66MiniVFXSubsystem>() : nullptr)
		{
			VfxSubsystem->SpawnGroundTelegraph(GetWorld(), Location, Radius, Delay, Tint);
		}
	}
}

void AT66MiniPlayerPawn::AddAreaEffect(const FVector& Location, const float Radius, const float TickDamage, const float TickInterval, const float Duration, const FLinearColor& Tint, const float StunDuration)
{
	FActiveAreaEffect& Effect = ActiveAreaEffects.AddDefaulted_GetRef();
	Effect.Location = Location;
	Effect.Radius = Radius;
	Effect.TickDamage = TickDamage;
	Effect.TickInterval = TickInterval;
	Effect.RemainingDuration = Duration;
	Effect.TickAccumulator = 0.f;
	Effect.StunDuration = StunDuration;
	Effect.Tint = Tint;

	if (UT66MiniVFXSubsystem* VfxSubsystem = GetGameInstance() ? GetGameInstance()->GetSubsystem<UT66MiniVFXSubsystem>() : nullptr)
	{
		VfxSubsystem->SpawnGroundTelegraph(GetWorld(), Location, Radius, Duration, Tint);
	}
}

float AT66MiniPlayerPawn::GetPassiveDamageMultiplierAgainst(const AT66MiniEnemyBase* Enemy) const
{
	float DamageScale = TemporaryDamageMultiplier;
	if (PassiveType == ET66PassiveType::MarksmanFocus && FocusTarget.Get() == Enemy)
	{
		DamageScale *= 1.f + (PassiveStacks * 0.08f);
	}
	if (PassiveType == ET66PassiveType::ToxinStacking && Enemy && Enemy->HasActiveDot())
	{
		DamageScale *= 1.16f;
	}
	if (PassiveType == ET66PassiveType::BrawlersFury && PassiveBuffRemaining > 0.f)
	{
		DamageScale *= 1.f + (PassiveStacks * 0.06f);
	}

	return DamageScale;
}

float AT66MiniPlayerPawn::ConsumeOutgoingDamageScalar(const AT66MiniEnemyBase* Enemy)
{
	float DamageScale = GetPassiveDamageMultiplierAgainst(Enemy);
	if (PassiveType == ET66PassiveType::QuickDraw && PassiveSecondaryBuffRemaining >= 1.4f)
	{
		DamageScale *= 1.45f;
		PassiveSecondaryBuffRemaining = 0.f;
	}

	return DamageScale;
}

void AT66MiniPlayerPawn::HandlePostDamageTaken(const float DamageTaken)
{
	if (PassiveType == ET66PassiveType::BrawlersFury)
	{
		PassiveStacks = FMath::Clamp(PassiveStacks + 1, 0, 4);
		PassiveBuffRemaining = 5.0f;
	}

	if (PassiveType == ET66PassiveType::IronWill && CurrentHealth <= (MaxHealth * 0.30f))
	{
		PassiveRegenPerSecond = FMath::Max(PassiveRegenPerSecond, 1.6f);
	}
}

void AT66MiniPlayerPawn::HandlePassiveOnBasicHit(AT66MiniEnemyBase* ImpactEnemy, const FVector& ImpactLocation, const float DamageDealt)
{
	if (!ImpactEnemy || ImpactEnemy->IsEnemyDead())
	{
		return;
	}

	if (PassiveType == ET66PassiveType::MarksmanFocus)
	{
		if (FocusTarget.Get() == ImpactEnemy)
		{
			PassiveStacks = FMath::Clamp(PassiveStacks + 1, 0, 5);
		}
		else
		{
			FocusTarget = ImpactEnemy;
			PassiveStacks = 1;
		}
	}

	if (PassiveType == ET66PassiveType::StaticCharge && (PassiveShotCounter % 4) == 0)
	{
		if (AT66MiniEnemyBase* SecondaryEnemy = FindClosestEnemyFromLocation(ImpactEnemy->GetActorLocation(), ImpactEnemy, 380.f))
		{
			SecondaryEnemy->ApplyDamage(DamageDealt * 0.60f);
			HandleSuccessfulHit(DamageDealt * 0.60f);
		}
	}

	if (PassiveType == ET66PassiveType::Frostbite && (PassiveShotCounter % 4) == 0)
	{
		ImpactEnemy->ApplyStun(0.22f);
	}

	if (PassiveType == ET66PassiveType::ChaosTheory && bChaosNextAttackBounces)
	{
		bChaosNextAttackBounces = false;
		if (AT66MiniEnemyBase* SecondaryEnemy = FindClosestEnemyFromLocation(ImpactEnemy->GetActorLocation(), ImpactEnemy, 340.f))
		{
			SecondaryEnemy->ApplyDamage(DamageDealt * 0.72f);
			HandleSuccessfulHit(DamageDealt * 0.72f);
		}
	}

	if (bEvasiveNextAttackAppliesDot)
	{
		bEvasiveNextAttackAppliesDot = false;
		ImpactEnemy->ApplyDot(FMath::Max(1.0f, DamageDealt * 0.18f), 0.45f, 2.6f);
	}
}

float AT66MiniPlayerPawn::GetUltimateBaseDamage() const
{
	float BaseDamage = ((BaseDamageStat * 2.0f) + (HeroLevel * 1.4f)) * BonusDamageMultiplier * TemporaryDamageMultiplier;
	if (PassiveType == ET66PassiveType::ArcaneAmplification)
	{
		BaseDamage *= 1.15f;
	}
	if (PassiveType == ET66PassiveType::RallyingBlow && PassiveBuffRemaining > 0.f)
	{
		BaseDamage *= 1.f + (PassiveStacks * 0.08f);
	}

	return BaseDamage;
}

void AT66MiniPlayerPawn::HandleBasicAttackImpact(AT66MiniEnemyBase* ImpactEnemy, const FVector& ImpactLocation)
{
	if (!ImpactEnemy || ImpactEnemy->IsEnemyDead())
	{
		return;
	}

	const float EstimatedDamage = ((BaseDamageStat * 0.95f) + (HeroLevel * 0.45f)) * BonusDamageMultiplier * TemporaryDamageMultiplier;
	HandlePassiveOnBasicHit(ImpactEnemy, ImpactLocation, EstimatedDamage);

	if (EquippedIdols.Num() == 0)
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

	float BaseScale = 0.44f;
	if (IdolRuntime.Category.Equals(TEXT("AOE"), ESearchCase::IgnoreCase))
	{
		BaseScale = 0.62f;
	}
	else if (IdolRuntime.Category.Equals(TEXT("Bounce"), ESearchCase::IgnoreCase))
	{
		BaseScale = 0.52f;
	}
	else if (IdolRuntime.Category.Equals(TEXT("DOT"), ESearchCase::IgnoreCase))
	{
		BaseScale = 0.48f;
	}

	const FVector Scale = FVector(BaseScale, BaseScale, 1.f) * ScaleMultiplier;
	if (IdolRuntime.EffectTexture)
	{
		VfxSubsystem->SpawnSpritePulse(GetWorld(), ImpactLocation, Scale, 0.22f, FLinearColor::White, IdolRuntime.EffectTexture, 0.72f);
	}
	else
	{
		VfxSubsystem->SpawnPulse(GetWorld(), ImpactLocation, Scale, 0.20f, FLinearColor(1.0f, 0.84f, 0.40f, 0.34f), 0.82f);
	}
}

void AT66MiniPlayerPawn::ApplyAoeIdolBurst(const FVector& ImpactLocation, const float Damage, const float Radius)
{
	UWorld* World = GetWorld();
	if (!World)
	{
		return;
	}

	const TArray<TObjectPtr<AT66MiniEnemyBase>>* LiveEnemies = T66MiniResolveLiveEnemies(World);
	if (!LiveEnemies)
	{
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
		return;
	}

	for (AT66MiniEnemyBase* Candidate : *LiveEnemies)
	{
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
	const TArray<TObjectPtr<AT66MiniEnemyBase>>* LiveEnemies = T66MiniResolveLiveEnemies(World);
	if (!LiveEnemies)
	{
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

	for (AT66MiniEnemyBase* Candidate : *LiveEnemies)
	{
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
	const TArray<TObjectPtr<AT66MiniEnemyBase>>* LiveEnemies = T66MiniResolveLiveEnemies(World);
	if (!LiveEnemies)
	{
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

	for (AT66MiniEnemyBase* Candidate : *LiveEnemies)
	{
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
