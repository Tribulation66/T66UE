// Copyright Tribulation 66. All Rights Reserved.

#include "UI/Screens/T66MiniBattleScreen.h"

#include "Core/T66AchievementsSubsystem.h"
#include "Core/T66BackendSubsystem.h"
#include "Core/T66MiniCircusSubsystem.h"
#include "Core/T66MiniDataSubsystem.h"
#include "Core/T66MiniFrontendStateSubsystem.h"
#include "Core/T66MiniLeaderboardSubsystem.h"
#include "Core/T66MiniRunStateSubsystem.h"
#include "Core/T66MiniVisualSubsystem.h"
#include "Core/T66SteamHelper.h"
#include "Data/T66MiniDataTypes.h"
#include "Engine/GameInstance.h"
#include "Engine/Texture2D.h"
#include "InputCoreTypes.h"
#include "Rendering/DrawElements.h"
#include "Save/T66MiniRunSaveGame.h"
#include "Save/T66MiniSaveSubsystem.h"
#include "Styling/CoreStyle.h"
#include "UI/T66MiniUIStyle.h"
#include "UI/T66UITypes.h"
#include "UI/Style/T66FlatStyle.h"
#include "UI/WidgetGames/T66WidgetGameResult.h"
#include "Widgets/Input/SButton.h"
#include "Widgets/Layout/SBorder.h"
#include "Widgets/Layout/SBox.h"
#include "Widgets/SBoxPanel.h"
#include "Widgets/SLeafWidget.h"
#include "Widgets/SOverlay.h"
#include "Widgets/Text/STextBlock.h"

namespace
{
	constexpr float MiniArenaHalfExtentX = 900.f;
	constexpr float MiniArenaHalfExtentY = 1600.f;
	constexpr float MiniPlayerRadius = 74.f;
	constexpr float MiniEnemyRadius = 70.f;
	constexpr float MiniPickupRadius = 70.f;
	constexpr float MiniInteractableRadius = 115.f;
	constexpr float MiniTouchDamageInterval = 0.65f;
	constexpr float MiniEnemyProjectileRadius = 82.f;
	constexpr float MiniDefaultProjectileLifetime = 3.0f;

	FVector2D T66MiniTo2D(const FVector& In)
	{
		return FVector2D(In.X, In.Y);
	}

	FVector T66MiniToVector(const FVector2D& In)
	{
		return FVector(In.X, In.Y, 0.f);
	}

	FVector2D T66MiniClampBoard(const FVector2D& In)
	{
		return FVector2D(
			FMath::Clamp(In.X, -MiniArenaHalfExtentX, MiniArenaHalfExtentX),
			FMath::Clamp(In.Y, -MiniArenaHalfExtentY, MiniArenaHalfExtentY));
	}

	FVector2D T66MiniSafeDirection(const FVector2D& From, const FVector2D& To, const FVector2D& Fallback = FVector2D(1.f, 0.f))
	{
		const FVector2D Delta = To - From;
		const float SizeSq = Delta.SizeSquared();
		return SizeSq > KINDA_SMALL_NUMBER ? Delta / FMath::Sqrt(SizeSq) : Fallback;
	}

	FString T66MiniReadableName(const FName Id)
	{
		FString Result = Id.ToString();
		Result.RemoveFromStart(TEXT("Item_"));
		Result.ReplaceInline(TEXT("_"), TEXT(" "));
		return Result;
	}

	FString T66MiniEnumLabel(const TCHAR* EnumPath, const int64 Value, const TCHAR* Fallback)
	{
		if (const UEnum* Enum = FindObject<UEnum>(nullptr, EnumPath))
		{
			return Enum->GetDisplayNameTextByValue(Value).ToString();
		}

		return Fallback;
	}

	enum class ET66MiniWidgetTransition : uint8
	{
		None,
		Shop,
		Summary
	};

	enum class ET66MiniWidgetProjectileBehavior : uint8
	{
		Basic,
		Pierce,
		Bounce,
		Aoe,
		Dot
	};

	TAttribute<FText> T66MiniBattleTextAttribute(UT66MiniBattleScreen* Screen, FText (UT66MiniBattleScreen::*Getter)() const)
	{
		return TAttribute<FText>::Create(TAttribute<FText>::FGetter::CreateUObject(Screen, Getter));
	}

	TAttribute<EVisibility> T66MiniBattleVisibilityAttribute(UT66MiniBattleScreen* Screen, EVisibility (UT66MiniBattleScreen::*Getter)() const)
	{
		return TAttribute<EVisibility>::Create(TAttribute<EVisibility>::FGetter::CreateUObject(Screen, Getter));
	}

	FPaintGeometry T66MiniPaintGeometry(const FGeometry& Geometry, const FVector2D& LocalPosition, const FVector2D& LocalSize)
	{
		return Geometry.ToPaintGeometry(
			FVector2f(static_cast<float>(LocalSize.X), static_cast<float>(LocalSize.Y)),
			FSlateLayoutTransform(FVector2f(static_cast<float>(LocalPosition.X), static_cast<float>(LocalPosition.Y))));
	}

	struct FT66MiniWidgetDot
	{
		float TickDamage = 0.f;
		float TickInterval = 0.5f;
		float Remaining = 0.f;
		float Accumulator = 0.f;
	};

	struct FT66MiniWidgetIdol
	{
		FName IdolID = NAME_None;
		FString Category;
		float BaseDamage = 0.f;
		float DamagePerLevel = 0.f;
		float BaseProperty = 0.f;
		float PropertyPerLevel = 0.f;
		float CooldownRemaining = 0.f;
		UTexture2D* EffectTexture = nullptr;
	};

	struct FT66MiniWidgetEnemy
	{
		int32 EntityID = 0;
		FName EnemyID = NAME_None;
		FString DisplayName;
		FString VisualID;
		bool bBoss = false;
		FVector2D Position = FVector2D::ZeroVector;
		FVector2D Velocity = FVector2D::ZeroVector;
		float CurrentHealth = 1.f;
		float MaxHealth = 1.f;
		float MoveSpeed = 180.f;
		float TouchDamage = 8.f;
		int32 MaterialDrop = 0;
		float ExperienceDrop = 0.f;
		ET66MiniEnemyBehaviorProfile BehaviorProfile = ET66MiniEnemyBehaviorProfile::Balanced;
		ET66MiniEnemyFamily Family = ET66MiniEnemyFamily::Melee;
		float FireInterval = 1.6f;
		float FireCooldown = 0.f;
		float ProjectileSpeed = 920.f;
		float ProjectileDamage = 8.f;
		float PreferredRange = 860.f;
		float ContactCooldown = 0.f;
		float StunRemaining = 0.f;
		float HitFlashRemaining = 0.f;
		float Lifetime = 0.f;
		TArray<FT66MiniWidgetDot> Dots;
		UTexture2D* Texture = nullptr;
	};

	struct FT66MiniWidgetProjectile
	{
		int32 EntityID = 0;
		bool bPlayerOwned = true;
		ET66MiniWidgetProjectileBehavior Behavior = ET66MiniWidgetProjectileBehavior::Basic;
		FVector2D Position = FVector2D::ZeroVector;
		FVector2D Velocity = FVector2D(1.f, 0.f);
		float Damage = 1.f;
		float Radius = 120.f;
		float Lifetime = MiniDefaultProjectileLifetime;
		int32 PierceRemaining = 0;
		int32 BounceRemaining = 0;
		int32 HomingTargetID = INDEX_NONE;
		float DotTickDamage = 0.f;
		float DotTickInterval = 0.5f;
		float DotDuration = 0.f;
		float StunDuration = 0.f;
		FName SourceIdolID = NAME_None;
		UTexture2D* Texture = nullptr;
		TSet<int32> HitEnemyIDs;
	};

	struct FT66MiniWidgetPickup
	{
		int32 EntityID = 0;
		FVector2D Position = FVector2D::ZeroVector;
		FString VisualID;
		int32 MaterialValue = 0;
		float ExperienceValue = 0.f;
		float HealValue = 0.f;
		float LifetimeRemaining = 14.f;
		FName GrantedItemID = NAME_None;
		float Age = 0.f;
		UTexture2D* Texture = nullptr;
	};

	struct FT66MiniWidgetInteractable
	{
		int32 EntityID = 0;
		FVector2D Position = FVector2D::ZeroVector;
		ET66MiniInteractableType Type = ET66MiniInteractableType::TreasureChest;
		FString VisualID;
		float LifetimeRemaining = 12.f;
		int32 MaterialReward = 0;
		int32 GoldReward = 0;
		float ExperienceReward = 0.f;
		float HealAmount = 0.f;
		bool bRequiresManualInteract = false;
		float Age = 0.f;
		UTexture2D* Texture = nullptr;
	};

	struct FT66MiniWidgetTrap
	{
		int32 EntityID = 0;
		FVector2D Position = FVector2D::ZeroVector;
		float Radius = 260.f;
		float DamagePerPulse = 8.f;
		float PulseInterval = 0.55f;
		float WarmupRemaining = 0.f;
		float ActiveRemaining = 0.f;
		float LifetimeRemaining = 4.f;
		float PulseAccumulator = 0.f;
		int32 TrapVariant = 0;
	};

	struct FT66MiniWidgetVfx
	{
		FVector2D Position = FVector2D::ZeroVector;
		float Radius = 100.f;
		float Age = 0.f;
		float Lifetime = 0.55f;
		FLinearColor Color = FLinearColor::White;
		UTexture2D* Texture = nullptr;
		FName Tag = FName(TEXT("AnimatedStyle.Mini.VFX"));
	};

	struct FT66MiniWidgetCombatText
	{
		FVector2D Position = FVector2D::ZeroVector;
		FVector2D Velocity = FVector2D(0.f, -90.f);
		FString Label;
		FLinearColor Color = FLinearColor::White;
		float Remaining = 0.9f;
		float Duration = 0.9f;
		FName Tag = FName(TEXT("AnimatedStyle.Mini.CombatText"));
	};
}

class FT66MiniBattleSimulation
{
public:
	bool Initialize(UT66MiniBattleScreen* InOwner)
	{
		Owner = InOwner;
		GameInstance = Owner.IsValid() ? Owner->GetGameInstance() : nullptr;
		DataSubsystem = GameInstance ? GameInstance->GetSubsystem<UT66MiniDataSubsystem>() : nullptr;
		VisualSubsystem = GameInstance ? GameInstance->GetSubsystem<UT66MiniVisualSubsystem>() : nullptr;
		RunState = GameInstance ? GameInstance->GetSubsystem<UT66MiniRunStateSubsystem>() : nullptr;
		SaveSubsystem = GameInstance ? GameInstance->GetSubsystem<UT66MiniSaveSubsystem>() : nullptr;
		FrontendState = GameInstance ? GameInstance->GetSubsystem<UT66MiniFrontendStateSubsystem>() : nullptr;
		ActiveRun = RunState ? RunState->GetActiveRun() : nullptr;

		if (!GameInstance || !DataSubsystem || !VisualSubsystem || !RunState || !SaveSubsystem || !FrontendState || !ActiveRun)
		{
			Status = TEXT("Mini battle could not start: missing Mini runtime state.");
			return false;
		}

		HeroID = ActiveRun->HeroID;
		CompanionID = ActiveRun->CompanionID;
		DifficultyID = ActiveRun->DifficultyID;
		WaveIndex = FMath::Max(1, ActiveRun->WaveIndex);
		HeroLevel = FMath::Max(1, ActiveRun->HeroLevel);
		Materials = FMath::Max(0, ActiveRun->Materials);
		Gold = FMath::Max(0, ActiveRun->Gold);
		Experience = FMath::Max(0.f, ActiveRun->Experience);
		UltimateCooldownRemaining = FMath::Max(0.f, ActiveRun->UltimateCooldownRemaining);
		bEnduranceCheatUsedThisWave = ActiveRun->bEnduranceCheatUsedThisWave;
		bQuickReviveReady = ActiveRun->bQuickReviveReady;
		OwnedItemIDs = ActiveRun->OwnedItemIDs;
		EquippedIdolIDs = ActiveRun->EquippedIdolIDs;
		if (EquippedIdolIDs.Num() > UT66MiniFrontendStateSubsystem::MaxIdolSlots)
		{
			EquippedIdolIDs.SetNum(UT66MiniFrontendStateSubsystem::MaxIdolSlots, EAllowShrinking::Yes);
		}

		const FT66MiniHeroDefinition* HeroDefinition = DataSubsystem->FindHero(HeroID);
		if (!HeroDefinition)
		{
			Status = FString::Printf(TEXT("Mini battle could not start: missing hero '%s'."), *HeroID.ToString());
			return false;
		}

		HeroDisplayName = HeroDefinition->DisplayName;
		UltimateType = HeroDefinition->UltimateType;
		PassiveType = HeroDefinition->PassiveType;
		BaseDamageStat = FMath::Max(GetRuntimeTuning(TEXT("HeroMinBaseDamage"), 2.f), HeroDefinition->BaseDamage);
		BaseAttackSpeedStat = FMath::Max(GetRuntimeTuning(TEXT("HeroMinBaseAttackSpeed"), 1.f), HeroDefinition->BaseAttackSpeed);
		BaseArmorStat = HeroDefinition->BaseArmor;
		BaseLuckStat = HeroDefinition->BaseLuck;
		AttackRange = GetRuntimeTuning(TEXT("HeroAttackRangeBase"), 1100.f) + (HeroDefinition->BaseSpeed * GetRuntimeTuning(TEXT("HeroSpeedRangeScalar"), 120.f));
		MoveSpeed = GetRuntimeTuning(TEXT("HeroMoveSpeedBase"), 900.f) + (HeroDefinition->BaseSpeed * GetRuntimeTuning(TEXT("HeroMoveSpeedScalar"), 160.f));
		MaxHealth = GetRuntimeTuning(TEXT("HeroMaxHealthBase"), 90.f) + (HeroDefinition->BaseArmor * GetRuntimeTuning(TEXT("HeroArmorHealthScalar"), 6.f));
		CurrentHealth = MaxHealth;
		ApplyLevelUpBonuses(HeroLevel - 1, false);
		ApplyPassiveBaseStats();

		for (const FName ItemID : OwnedItemIDs)
		{
			if (const FT66MiniItemDefinition* Item = DataSubsystem->FindItem(ItemID))
			{
				ApplyItemDefinition(*Item);
			}
		}

		if (ActiveRun->MaxHealth > 0.f)
		{
			MaxHealth = ActiveRun->MaxHealth;
		}
		CurrentHealth = FMath::Clamp(ActiveRun->CurrentHealth, 0.f, MaxHealth);
		UltimateCooldownDuration = GetRuntimeTuning(TEXT("UltimateCooldownBase"), 16.f)
			+ (WaveIndex * GetRuntimeTuning(TEXT("UltimateCooldownPerWave"), 1.1f));

		PlayerTexture = VisualSubsystem->LoadHeroTexture(HeroDisplayName);
		PlayerProjectileTexture = VisualSubsystem->LoadHeroProjectileTexture(HeroDisplayName);
		BackgroundTexture = VisualSubsystem->LoadBackgroundTexture();
		if (const FT66MiniCompanionDefinition* Companion = DataSubsystem->FindCompanion(CompanionID))
		{
			CompanionDisplayName = Companion->DisplayName;
			CompanionVisualID = Companion->VisualID;
			CompanionHealingPerSecond = SaveSubsystem->GetCompanionHealingPerSecond(CompanionID, DataSubsystem);
			CompanionFollowOffset = FVector2D(Companion->FollowOffsetX, Companion->FollowOffsetY);
			CompanionTexture = VisualSubsystem->LoadCompanionTexture(Companion->VisualID);
		}

		RefreshIdolRuntime();

		PlayerPosition = ActiveRun->bHasPlayerLocation
			? T66MiniClampBoard(T66MiniTo2D(ActiveRun->PlayerLocation))
			: FVector2D::ZeroVector;
		DesiredMovePosition = PlayerPosition;
		AimPosition = PlayerPosition + FVector2D(280.f, 0.f);

		StartWave(WaveIndex, !ActiveRun->bHasMidWaveSnapshot);
		if (ActiveRun->bHasMidWaveSnapshot)
		{
			RestoreTransientWaveState();
			RestoreEntitySnapshots();
		}

		Status = TEXT("Mini battle running in frontend Slate.");
		bInitialized = true;
		return true;
	}

	void Tick(const float DeltaSeconds)
	{
		if (!bInitialized || bPaused || PendingTransition != ET66MiniWidgetTransition::None || bRunFinalized)
		{
			return;
		}

		const float ClampedDelta = FMath::Clamp(DeltaSeconds, 0.f, 0.05f);
		if (ClampedDelta <= 0.f)
		{
			return;
		}

		TotalElapsed += ClampedDelta;
		if (ActiveRun)
		{
			ActiveRun->TotalRunSeconds += ClampedDelta;
		}

		TickPlayer(ClampedDelta);
		TickWaveDirector(ClampedDelta);
		TickEnemies(ClampedDelta);
		TickProjectiles(ClampedDelta);
		TickPickups(ClampedDelta);
		TickInteractables(ClampedDelta);
		TickTraps(ClampedDelta);
		TickEvents(ClampedDelta);
		SyncActiveRun(false);

		AutosaveAccumulator += ClampedDelta;
		if (AutosaveAccumulator >= GetRuntimeTuning(TEXT("AutosaveInterval"), 1.f))
		{
			PersistActiveRunSnapshot(true);
			AutosaveAccumulator = 0.f;
		}
	}

	void SetPointerBoardPosition(const FVector2D& BoardPosition)
	{
		bHasPointerTarget = true;
		DesiredMovePosition = T66MiniClampBoard(BoardPosition);
		AimPosition = DesiredMovePosition;
	}

	void SetKeyboardMoveInput(const FVector2D& MoveInput)
	{
		KeyboardMoveInput = MoveInput.GetClampedToMaxSize(1.f);
	}

	void RequestInteract()
	{
		if (!bInitialized || bPaused)
		{
			return;
		}

		int32 BestIndex = INDEX_NONE;
		float BestDistanceSq = FMath::Square(190.f);
		for (int32 Index = 0; Index < Interactables.Num(); ++Index)
		{
			const FT66MiniWidgetInteractable& Candidate = Interactables[Index];
			if (!Candidate.bRequiresManualInteract)
			{
				continue;
			}

			const float DistanceSq = FVector2D::DistSquared(Candidate.Position, PlayerPosition);
			if (DistanceSq <= BestDistanceSq)
			{
				BestDistanceSq = DistanceSq;
				BestIndex = Index;
			}
		}

		if (BestIndex != INDEX_NONE)
		{
			ConsumeInteractable(BestIndex);
		}
	}

	void RequestUltimateAtBoardPosition(const FVector2D& BoardPosition)
	{
		if (!bInitialized || bPaused || UltimateCooldownRemaining > 0.f || UltimateType == ET66UltimateType::None)
		{
			return;
		}

		const FVector2D Target = T66MiniClampBoard(BoardPosition);
		const float BaseDamage = GetUltimateBaseDamage();
		switch (UltimateType)
		{
		case ET66UltimateType::SpearStorm:
		case ET66UltimateType::FanTheHammer:
		case ET66UltimateType::ScopedSniper:
		{
			const int32 SideCount = UltimateType == ET66UltimateType::FanTheHammer
				? GetRuntimeTuningInt(TEXT("UltimateFanSideProjectiles"), 3)
				: GetRuntimeTuningInt(TEXT("UltimateSpearStormSideProjectiles"), 2);
			const float AngleStep = UltimateType == ET66UltimateType::FanTheHammer
				? GetRuntimeTuning(TEXT("UltimateFanAngleStep"), 5.f)
				: GetRuntimeTuning(TEXT("UltimateSpearStormAngleStep"), 7.f);
			const float DamageScalar = UltimateType == ET66UltimateType::FanTheHammer
				? GetRuntimeTuning(TEXT("UltimateFanDamageScalar"), 0.78f)
				: (UltimateType == ET66UltimateType::ScopedSniper ? GetRuntimeTuning(TEXT("UltimateSniperDamageScalar"), 3.f) : GetRuntimeTuning(TEXT("UltimateSpearStormDamageScalar"), 1.25f));
			const float Speed = UltimateType == ET66UltimateType::FanTheHammer
				? GetRuntimeTuning(TEXT("UltimateFanProjectileSpeed"), 3000.f)
				: (UltimateType == ET66UltimateType::ScopedSniper ? GetRuntimeTuning(TEXT("UltimateSniperProjectileSpeed"), 3200.f) : GetRuntimeTuning(TEXT("UltimateSpearStormProjectileSpeed"), 2600.f));
			const float Radius = UltimateType == ET66UltimateType::FanTheHammer
				? GetRuntimeTuning(TEXT("UltimateFanProjectileRadius"), 170.f)
				: (UltimateType == ET66UltimateType::ScopedSniper ? GetRuntimeTuning(TEXT("UltimateSniperProjectileRadius"), 260.f) : GetRuntimeTuning(TEXT("UltimateSpearStormProjectileRadius"), 240.f));
			const int32 Pierce = UltimateType == ET66UltimateType::FanTheHammer
				? GetRuntimeTuningInt(TEXT("UltimateFanPierceCount"), 2)
				: (UltimateType == ET66UltimateType::ScopedSniper ? GetRuntimeTuningInt(TEXT("UltimateSniperPierceCount"), 6) : GetRuntimeTuningInt(TEXT("UltimateSpearStormPierceCount"), 4));
			for (int32 Index = -SideCount; Index <= SideCount; ++Index)
			{
				FVector2D Direction = T66MiniSafeDirection(PlayerPosition, Target);
				const float Radians = FMath::DegreesToRadians(static_cast<float>(Index) * AngleStep);
				Direction = FVector2D(
					(Direction.X * FMath::Cos(Radians)) - (Direction.Y * FMath::Sin(Radians)),
					(Direction.X * FMath::Sin(Radians)) + (Direction.Y * FMath::Cos(Radians))).GetSafeNormal();
				SpawnProjectile(true, PlayerPosition + Direction * 80.f, Direction, BaseDamage * DamageScalar, Speed, Radius, ET66MiniWidgetProjectileBehavior::Pierce, Pierce, 0, 0.f, 0.5f, 0.f, UltimateType == ET66UltimateType::ScopedSniper ? GetRuntimeTuning(TEXT("UltimateSniperStun"), 0.15f) : 0.f, NAME_None, PlayerProjectileTexture);
			}
			UltimateCooldownDuration = UltimateType == ET66UltimateType::FanTheHammer
				? GetRuntimeTuning(TEXT("UltimateFanCooldown"), 16.f)
				: (UltimateType == ET66UltimateType::ScopedSniper ? GetRuntimeTuning(TEXT("UltimateSniperCooldown"), 19.f) : GetRuntimeTuning(TEXT("UltimateSpearStormCooldown"), 18.f));
			break;
		}
		case ET66UltimateType::MeteorStrike:
			for (int32 Index = 0; Index < GetRuntimeTuningInt(TEXT("UltimateMeteorCount"), 5); ++Index)
			{
				const FVector2D Scatter(FMath::FRandRange(-1.f, 1.f), FMath::FRandRange(-1.f, 1.f));
				QueueBurst(Target + Scatter.GetClampedToMaxSize(1.f) * FMath::FRandRange(0.f, GetRuntimeTuning(TEXT("UltimateMeteorScatter"), 360.f)),
					BaseDamage * GetRuntimeTuning(TEXT("UltimateMeteorDamageScalar"), 1.5f),
					GetRuntimeTuning(TEXT("UltimateMeteorRadius"), 320.f),
					GetRuntimeTuning(TEXT("UltimateMeteorDelayBase"), 0.28f) + (Index * GetRuntimeTuning(TEXT("UltimateMeteorDelayStep"), 0.12f)),
					FLinearColor(1.0f, 0.46f, 0.16f, 0.30f));
			}
			UltimateCooldownDuration = GetRuntimeTuning(TEXT("UltimateMeteorCooldown"), 20.f);
			break;
		case ET66UltimateType::ChainLightning:
			ApplyAreaDamage(PlayerPosition, GetRuntimeTuning(TEXT("UltimateChainLightningRadius"), 1250.f), BaseDamage * GetRuntimeTuning(TEXT("UltimateChainLightningDamageScalar"), 1.18f), GetRuntimeTuning(TEXT("UltimateChainLightningStun"), 0.12f), 0.f, 0.f);
			UltimateCooldownDuration = GetRuntimeTuning(TEXT("UltimateChainLightningCooldown"), 18.f);
			break;
		case ET66UltimateType::Discharge:
			ApplyAreaDamage(PlayerPosition, GetRuntimeTuning(TEXT("UltimateDischargeRadius"), 520.f), BaseDamage * GetRuntimeTuning(TEXT("UltimateDischargeDamageScalar"), 1.05f), GetRuntimeTuning(TEXT("UltimateDischargeStun"), 0.26f), 0.f, 0.f);
			UltimateCooldownDuration = GetRuntimeTuning(TEXT("UltimateDischargeCooldown"), 17.f);
			break;
		case ET66UltimateType::Shockwave:
			ApplyAreaDamage(PlayerPosition, GetRuntimeTuning(TEXT("UltimateShockwaveRadius"), 460.f), BaseDamage * GetRuntimeTuning(TEXT("UltimateShockwaveDamageScalar"), 1.35f), GetRuntimeTuning(TEXT("UltimateShockwaveStun"), 0.22f), 0.f, 0.f);
			UltimateCooldownDuration = GetRuntimeTuning(TEXT("UltimateShockwaveCooldown"), 17.f);
			break;
		case ET66UltimateType::PlagueCloud:
			AddAreaEffect(Target, GetRuntimeTuning(TEXT("UltimatePlagueCloudRadius"), 360.f), BaseDamage * GetRuntimeTuning(TEXT("UltimatePlagueCloudTickDamageScalar"), 0.40f), GetRuntimeTuning(TEXT("UltimatePlagueCloudTickInterval"), 0.40f), GetRuntimeTuning(TEXT("UltimatePlagueCloudDuration"), 5.2f), FLinearColor(0.62f, 1.0f, 0.54f, 0.22f), 0.f);
			UltimateCooldownDuration = GetRuntimeTuning(TEXT("UltimatePlagueCloudCooldown"), 17.f);
			break;
		case ET66UltimateType::MiasmaBomb:
			QueueBurst(Target, BaseDamage * GetRuntimeTuning(TEXT("UltimateMiasmaBurstDamageScalar"), 1.25f), GetRuntimeTuning(TEXT("UltimateMiasmaBurstRadius"), 300.f), GetRuntimeTuning(TEXT("UltimateMiasmaBurstDelay"), 0.25f), FLinearColor(0.62f, 1.0f, 0.54f, 0.26f), BaseDamage * GetRuntimeTuning(TEXT("UltimateMiasmaBurstDotDamageScalar"), 0.16f), GetRuntimeTuning(TEXT("UltimateMiasmaBurstDotDuration"), 3.2f), 0.f);
			AddAreaEffect(Target, GetRuntimeTuning(TEXT("UltimateMiasmaAreaRadius"), 260.f), BaseDamage * GetRuntimeTuning(TEXT("UltimateMiasmaAreaTickDamageScalar"), 0.24f), GetRuntimeTuning(TEXT("UltimateMiasmaAreaTickInterval"), 0.45f), GetRuntimeTuning(TEXT("UltimateMiasmaAreaDuration"), 4.4f), FLinearColor(0.56f, 1.0f, 0.48f, 0.20f), 0.f);
			UltimateCooldownDuration = GetRuntimeTuning(TEXT("UltimateMiasmaCooldown"), 18.f);
			break;
		case ET66UltimateType::Blizzard:
			AddAreaEffect(Target, GetRuntimeTuning(TEXT("UltimateBlizzardRadius"), 420.f), BaseDamage * GetRuntimeTuning(TEXT("UltimateBlizzardTickDamageScalar"), 0.34f), GetRuntimeTuning(TEXT("UltimateBlizzardTickInterval"), 0.42f), GetRuntimeTuning(TEXT("UltimateBlizzardDuration"), 5.5f), FLinearColor(0.72f, 0.90f, 1.0f, 0.22f), GetRuntimeTuning(TEXT("UltimateBlizzardStun"), 0.08f));
			UltimateCooldownDuration = GetRuntimeTuning(TEXT("UltimateBlizzardCooldown"), 19.f);
			break;
		case ET66UltimateType::TidalWave:
			for (int32 Index = 1; Index <= GetRuntimeTuningInt(TEXT("UltimateTidalWaveBurstCount"), 4); ++Index)
			{
				const FVector2D Direction = T66MiniSafeDirection(PlayerPosition, Target);
				QueueBurst(PlayerPosition + Direction * (Index * GetRuntimeTuning(TEXT("UltimateTidalWaveSpacing"), 220.f)),
					BaseDamage * GetRuntimeTuning(TEXT("UltimateTidalWaveDamageScalar"), 0.92f),
					GetRuntimeTuning(TEXT("UltimateTidalWaveRadius"), 280.f),
					Index * GetRuntimeTuning(TEXT("UltimateTidalWaveDelayStep"), 0.12f),
					FLinearColor(0.34f, 0.72f, 1.0f, 0.28f),
					0.f,
					0.f,
					GetRuntimeTuning(TEXT("UltimateTidalWaveStun"), 0.12f));
			}
			UltimateCooldownDuration = GetRuntimeTuning(TEXT("UltimateTidalWaveCooldown"), 18.f);
			break;
		case ET66UltimateType::Juiced:
			TemporaryDamageMultiplier = GetRuntimeTuning(TEXT("UltimateJuicedDamageMultiplier"), 1.45f);
			PassiveSecondaryBuffRemaining = GetRuntimeTuning(TEXT("UltimateJuicedDuration"), 6.f);
			UltimateCooldownDuration = GetRuntimeTuning(TEXT("UltimateJuicedCooldown"), 22.f);
			break;
		case ET66UltimateType::GoldRush:
			PassiveSecondaryBuffRemaining = GetRuntimeTuning(TEXT("UltimateGoldRushDuration"), 7.5f);
			for (int32 Index = 0; Index < GetRuntimeTuningInt(TEXT("UltimateGoldRushBurstCount"), 5); ++Index)
			{
				const float Angle = FMath::DegreesToRadians(Index * GetRuntimeTuning(TEXT("UltimateGoldRushAngleStep"), 72.f));
				const FVector2D Offset(FMath::Cos(Angle), FMath::Sin(Angle));
				QueueBurst(PlayerPosition + Offset * GetRuntimeTuning(TEXT("UltimateGoldRushOffset"), 240.f), BaseDamage, GetRuntimeTuning(TEXT("UltimateGoldRushRadius"), 220.f), Index * GetRuntimeTuning(TEXT("UltimateGoldRushDelayStep"), 0.08f), FLinearColor(1.0f, 0.84f, 0.24f, 0.28f));
			}
			UltimateCooldownDuration = GetRuntimeTuning(TEXT("UltimateGoldRushCooldown"), 21.f);
			break;
		case ET66UltimateType::RabidFrenzy:
			TemporaryDamageMultiplier = GetRuntimeTuning(TEXT("UltimateRabidDamageMultiplier"), 1.35f);
			PassiveSecondaryBuffRemaining = GetRuntimeTuning(TEXT("UltimateRabidDuration"), 6.5f);
			Heal(BaseDamage * GetRuntimeTuning(TEXT("UltimateRabidHealScalar"), 0.12f));
			AddAreaEffect(PlayerPosition, GetRuntimeTuning(TEXT("UltimateRabidAreaRadius"), 260.f), BaseDamage * GetRuntimeTuning(TEXT("UltimateRabidAreaTickDamageScalar"), 0.18f), GetRuntimeTuning(TEXT("UltimateRabidAreaTickInterval"), 0.45f), GetRuntimeTuning(TEXT("UltimateRabidAreaDuration"), 4.f), FLinearColor(1.0f, 0.40f, 0.22f, 0.18f), 0.f);
			UltimateCooldownDuration = GetRuntimeTuning(TEXT("UltimateRabidCooldown"), 19.f);
			break;
		case ET66UltimateType::DeathSpiral:
			for (int32 Index = 0; Index < GetRuntimeTuningInt(TEXT("UltimateDeathSpiralBurstCount"), 4); ++Index)
			{
				QueueBurst(PlayerPosition,
					BaseDamage * (GetRuntimeTuning(TEXT("UltimateDeathSpiralDamageScalarBase"), 0.90f) + (Index * GetRuntimeTuning(TEXT("UltimateDeathSpiralDamageScalarStep"), 0.16f))),
					GetRuntimeTuning(TEXT("UltimateDeathSpiralRadiusBase"), 260.f) + (Index * GetRuntimeTuning(TEXT("UltimateDeathSpiralRadiusStep"), 110.f)),
					GetRuntimeTuning(TEXT("UltimateDeathSpiralDelayBase"), 0.05f) + (Index * GetRuntimeTuning(TEXT("UltimateDeathSpiralDelayStep"), 0.18f)),
					FLinearColor(0.62f, 0.34f, 1.0f, 0.28f),
					BaseDamage * GetRuntimeTuning(TEXT("UltimateDeathSpiralDotDamageScalar"), 0.18f),
					GetRuntimeTuning(TEXT("UltimateDeathSpiralDotDuration"), 2.2f),
					0.f);
			}
			UltimateCooldownDuration = GetRuntimeTuning(TEXT("UltimateDeathSpiralCooldown"), 20.f);
			break;
		case ET66UltimateType::PrecisionStrike:
		case ET66UltimateType::Deadeye:
		default:
			ApplyAreaDamage(Target, 320.f, BaseDamage * GetRuntimeTuning(TEXT("UltimatePrecisionStrikeDamageScalar"), 2.6f), 0.12f, 0.f, 0.f);
			UltimateCooldownDuration = GetRuntimeTuning(TEXT("UltimatePrecisionStrikeCooldown"), 16.f);
			break;
		}

		UltimateCooldownRemaining = UltimateCooldownDuration;
		AddVfx(PlayerPosition, 240.f, FLinearColor(1.0f, 0.88f, 0.40f, 0.32f), 0.65f, nullptr, FName(TEXT("AnimatedStyle.Mini.UltimatePulse")));
	}

	void TogglePause()
	{
		bPaused = !bPaused;
		Status = bPaused ? TEXT("Mini battle paused.") : TEXT("Mini battle running in frontend Slate.");
	}

	void SaveAndExit()
	{
		PersistActiveRunSnapshot(true);
		if (Owner.IsValid())
		{
			PendingTransition = ET66MiniWidgetTransition::None;
			Owner->NavigateTo(ET66ScreenType::MiniMainMenu);
		}
	}

	bool IsPaused() const { return bPaused; }
	bool IsInitialized() const { return bInitialized; }
	ET66MiniWidgetTransition GetPendingTransition() const { return PendingTransition; }
	FString GetStatus() const { return Status; }
	void SaveWidgetGameState() { PersistActiveRunSnapshot(true); }

	FText GetWaveText() const
	{
		const int32 MaxStage = GetMaxStageIndexForCurrentDifficulty();
		return FText::FromString(FString::Printf(TEXT("WAVE %d / %d  %.0fs"), WaveIndex, MaxStage, WaveSecondsRemaining));
	}

	FText GetHealthText() const
	{
		return FText::FromString(FString::Printf(TEXT("%s  HP %.0f / %.0f  LV %d"), *HeroDisplayName, CurrentHealth, MaxHealth, HeroLevel));
	}

	FText GetResourceText() const
	{
		return FText::FromString(FString::Printf(TEXT("MAT %d  GOLD %d  XP %.0f / %.0f"), Materials, Gold, Experience, GetNextLevelThreshold()));
	}

	FText GetCombatText() const
	{
		const FString UltimateLabel = T66MiniEnumLabel(TEXT("/Script/T66.ET66UltimateType"), static_cast<int64>(UltimateType), TEXT("ULT"));
		const FString PassiveLabel = T66MiniEnumLabel(TEXT("/Script/T66.ET66PassiveType"), static_cast<int64>(PassiveType), TEXT("PASSIVE"));
		return FText::FromString(FString::Printf(TEXT("%s %.0fs  %s  ENEMIES %d  PROJECTILES %d"),
			*UltimateLabel,
			UltimateCooldownRemaining,
			*PassiveLabel,
			Enemies.Num(),
			Projectiles.Num()));
	}

	FVector2D LocalToBoard(const FGeometry& Geometry, const FVector2D& Local) const
	{
		const FVector2D Size = Geometry.GetLocalSize();
		const float Scale = GetBoardScale(Size);
		const FVector2D Center = Size * 0.5f;
		return T66MiniClampBoard(FVector2D((Local.X - Center.X) / Scale, -(Local.Y - Center.Y) / Scale));
	}

	void Draw(const FPaintArgs& Args, const FGeometry& AllottedGeometry, const FSlateRect& MyCullingRect, FSlateWindowElementList& OutDrawElements, int32& LayerId, const FWidgetStyle& InWidgetStyle, bool bParentEnabled) const
	{
		const FVector2D Size = AllottedGeometry.GetLocalSize();
		const FSlateBrush* WhiteBrush = FCoreStyle::Get().GetBrush(TEXT("WhiteBrush"));
		const bool bEnabled = bParentEnabled;
		const ESlateDrawEffect DrawEffects = bEnabled ? ESlateDrawEffect::None : ESlateDrawEffect::DisabledEffect;

		FSlateDrawElement::MakeBox(
			OutDrawElements,
			LayerId++,
			AllottedGeometry.ToPaintGeometry(),
			WhiteBrush,
			DrawEffects,
			FLinearColor(0.018f, 0.020f, 0.026f, 1.f));

		const FVector2D BoardTopLeft = BoardToLocal(Size, FVector2D(-MiniArenaHalfExtentX, MiniArenaHalfExtentY));
		const FVector2D BoardBottomRight = BoardToLocal(Size, FVector2D(MiniArenaHalfExtentX, -MiniArenaHalfExtentY));
		const FVector2D BoardSize = BoardBottomRight - BoardTopLeft;
		if (BackgroundTexture)
		{
			FSlateDrawElement::MakeBox(
				OutDrawElements,
				LayerId++,
				T66MiniPaintGeometry(AllottedGeometry, BoardTopLeft, BoardSize),
				GetBrush(BackgroundTexture, BoardSize),
				DrawEffects,
				FLinearColor(0.72f, 0.72f, 0.72f, 1.f));
		}
		else
		{
			FSlateDrawElement::MakeBox(
				OutDrawElements,
				LayerId++,
				T66MiniPaintGeometry(AllottedGeometry, BoardTopLeft, BoardSize),
				WhiteBrush,
				DrawEffects,
				FLinearColor(0.055f, 0.066f, 0.070f, 1.f));
		}

		FSlateDrawElement::MakeBox(
			OutDrawElements,
			LayerId++,
			T66MiniPaintGeometry(AllottedGeometry, BoardTopLeft, BoardSize),
			WhiteBrush,
			DrawEffects,
			FLinearColor(0.04f, 0.04f, 0.04f, 0.20f));

		DrawTraps(AllottedGeometry, Size, OutDrawElements, LayerId, DrawEffects);
		DrawVfx(AllottedGeometry, Size, OutDrawElements, LayerId, DrawEffects, true);
		DrawPickupsAndInteractables(AllottedGeometry, Size, OutDrawElements, LayerId, DrawEffects);
		DrawProjectiles(AllottedGeometry, Size, OutDrawElements, LayerId, DrawEffects);
		DrawCompanion(AllottedGeometry, Size, OutDrawElements, LayerId, DrawEffects);
		DrawEnemies(AllottedGeometry, Size, OutDrawElements, LayerId, DrawEffects);
		DrawPlayer(AllottedGeometry, Size, OutDrawElements, LayerId, DrawEffects);
		DrawVfx(AllottedGeometry, Size, OutDrawElements, LayerId, DrawEffects, false);
		DrawCombatTexts(AllottedGeometry, Size, OutDrawElements, LayerId, DrawEffects);

		if (BossTelegraphRemaining > 0.f)
		{
			DrawWorldDisc(AllottedGeometry, Size, OutDrawElements, LayerId++, DrawEffects, PendingBossSpawnLocation2D, GetRuntimeTuning(TEXT("BossTelegraphRadius"), 260.f), FLinearColor(1.0f, 0.12f, 0.08f, 0.24f));
		}
	}

private:
	float GetRuntimeTuning(const TCHAR* Key, const float Fallback) const
	{
		float Value = 0.f;
		return DataSubsystem && DataSubsystem->TryFindRuntimeTuningValue(FName(Key), Value) ? Value : Fallback;
	}

	int32 GetRuntimeTuningInt(const TCHAR* Key, const int32 Fallback) const
	{
		return FMath::RoundToInt(GetRuntimeTuning(Key, static_cast<float>(Fallback)));
	}

	void ApplyLevelUpBonuses(const int32 LevelsToApply, const bool bHeal)
	{
		for (int32 Index = 0; Index < LevelsToApply; ++Index)
		{
			MaxHealth += GetRuntimeTuning(TEXT("LevelHealthAdd"), 6.f);
			if (bHeal)
			{
				CurrentHealth = FMath::Min(MaxHealth, CurrentHealth + GetRuntimeTuning(TEXT("LevelHealAdd"), 5.f));
			}
			BaseDamageStat += GetRuntimeTuning(TEXT("LevelDamageAdd"), 0.35f);
			BaseAttackSpeedStat += GetRuntimeTuning(TEXT("LevelAttackSpeedAdd"), 0.12f);
			MoveSpeed += GetRuntimeTuning(TEXT("LevelMoveSpeedAdd"), 18.f);
		}
	}

	void ApplyPassiveBaseStats()
	{
		CritChance = GetRuntimeTuning(TEXT("BaseCritChance"), 0.05f);
		CritDamageMultiplier = GetRuntimeTuning(TEXT("BaseCritDamageMultiplier"), 1.5f);
		PassiveRegenPerSecond = 0.f;
		EvasionChance = 0.f;
		LifeStealChance = 0.f;
		DotDamageBonus = 0.f;
		AoeRadiusBonus = 0.f;
		BonusDamageMultiplier = 1.f;
		TemporaryDamageMultiplier = 1.f;
		MaterialGainMultiplier = 1.f;
		GoldGainMultiplier = 1.f;

		switch (PassiveType)
		{
		case ET66PassiveType::IronWill:
			BaseArmorStat += GetRuntimeTuning(TEXT("PassiveIronWillArmorAdd"), 1.5f);
			break;
		case ET66PassiveType::ArcaneAmplification:
			BaseDamageStat += GetRuntimeTuning(TEXT("PassiveArcaneDamageAdd"), 0.6f);
			break;
		case ET66PassiveType::MarksmanFocus:
			AttackRange += GetRuntimeTuning(TEXT("PassiveMarksmanRangeAdd"), 90.f);
			break;
		case ET66PassiveType::ToxinStacking:
			DotDamageBonus += GetRuntimeTuning(TEXT("PassiveToxinDotDamageAdd"), 0.5f);
			break;
		case ET66PassiveType::QuickDraw:
			CritChance += GetRuntimeTuning(TEXT("PassiveQuickDrawCritChanceAdd"), 0.04f);
			break;
		case ET66PassiveType::Headshot:
			CritChance += GetRuntimeTuning(TEXT("PassiveHeadshotCritChanceAdd"), 0.08f);
			AttackRange += GetRuntimeTuning(TEXT("PassiveHeadshotRangeAdd"), 150.f);
			break;
		case ET66PassiveType::StaticCharge:
		case ET66PassiveType::Overclock:
			BaseAttackSpeedStat += GetRuntimeTuning(TEXT("PassiveSpeedsterAttackSpeedAdd"), 0.18f);
			break;
		case ET66PassiveType::ChaosTheory:
			BaseLuckStat += GetRuntimeTuning(TEXT("PassiveChaosLuckAdd"), 0.8f);
			break;
		case ET66PassiveType::Endurance:
			MaxHealth += GetRuntimeTuning(TEXT("PassiveEnduranceHealthAdd"), 12.f);
			CurrentHealth += GetRuntimeTuning(TEXT("PassiveEnduranceHealthAdd"), 12.f);
			break;
		case ET66PassiveType::BrawlersFury:
			BaseDamageStat += GetRuntimeTuning(TEXT("PassiveBrawlersDamageAdd"), 0.45f);
			break;
		case ET66PassiveType::Unflinching:
			BaseArmorStat += GetRuntimeTuning(TEXT("PassiveUnflinchingArmorAdd"), 2.f);
			MaxHealth += GetRuntimeTuning(TEXT("PassiveUnflinchingHealthAdd"), 16.f);
			CurrentHealth += GetRuntimeTuning(TEXT("PassiveUnflinchingHealthAdd"), 16.f);
			break;
		case ET66PassiveType::TreasureHunter:
			MaterialGainMultiplier += GetRuntimeTuning(TEXT("PassiveTreasureMaterialGainAdd"), 0.25f);
			GoldGainMultiplier += GetRuntimeTuning(TEXT("PassiveTreasureGoldGainAdd"), 0.20f);
			BaseLuckStat += GetRuntimeTuning(TEXT("PassiveTreasureLuckAdd"), 1.f);
			break;
		case ET66PassiveType::Evasive:
			EvasionChance += GetRuntimeTuning(TEXT("PassiveEvasiveChanceAdd"), 0.10f);
			MoveSpeed += GetRuntimeTuning(TEXT("PassiveEvasiveMoveSpeedAdd"), 80.f);
			break;
		case ET66PassiveType::Frostbite:
			BonusDamageMultiplier += GetRuntimeTuning(TEXT("PassiveFrostbiteDamageMultiplierAdd"), 0.04f);
			break;
		default:
			break;
		}
	}

	void ApplyItemDefinition(const FT66MiniItemDefinition& ItemDefinition)
	{
		const FString& Stat = ItemDefinition.SecondaryStatType;
		if (Stat == TEXT("AoeDamage") || Stat == TEXT("BounceDamage") || Stat == TEXT("PierceDamage") || Stat == TEXT("DotDamage"))
		{
			BaseDamageStat += GetRuntimeTuning(TEXT("ItemDamageStatAdd"), 1.6f);
		}
		else if (Stat == TEXT("AoeSpeed") || Stat == TEXT("BounceSpeed") || Stat == TEXT("PierceSpeed") || Stat == TEXT("DotSpeed"))
		{
			BaseAttackSpeedStat += GetRuntimeTuning(TEXT("ItemAttackSpeedAdd"), 0.22f);
		}
		else if (Stat == TEXT("AoeScale"))
		{
			AoeRadiusBonus += GetRuntimeTuning(TEXT("ItemAoeRadiusAdd"), 70.f);
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
			DotDamageBonus += GetRuntimeTuning(TEXT("ItemDotDamageAdd"), 1.f);
		}
		else if (Stat == TEXT("AttackRange") || Stat == TEXT("Accuracy"))
		{
			AttackRange += GetRuntimeTuning(TEXT("ItemAttackRangeAdd"), 110.f);
		}
		else if (Stat == TEXT("CritChance"))
		{
			CritChance += GetRuntimeTuning(TEXT("ItemCritChanceAdd"), 0.05f);
		}
		else if (Stat == TEXT("CritDamage"))
		{
			CritDamageMultiplier += GetRuntimeTuning(TEXT("ItemCritDamageAdd"), 0.30f);
		}
		else if (Stat == TEXT("DamageReduction") || Stat == TEXT("Taunt"))
		{
			BaseArmorStat += GetRuntimeTuning(TEXT("ItemArmorAdd"), 1.4f);
		}
		else if (Stat == TEXT("HpRegen"))
		{
			PassiveRegenPerSecond += GetRuntimeTuning(TEXT("ItemRegenAdd"), 1.2f);
		}
		else if (Stat == TEXT("LifeSteal"))
		{
			LifeStealChance += GetRuntimeTuning(TEXT("ItemLifeStealChanceAdd"), 0.08f);
		}
		else if (Stat == TEXT("EvasionChance") || Stat == TEXT("Invisibility"))
		{
			EvasionChance += GetRuntimeTuning(TEXT("ItemEvasionChanceAdd"), 0.06f);
		}
		else if (Stat == TEXT("Cheating"))
		{
			const float HealthAdd = GetRuntimeTuning(TEXT("ItemCheatingHealthAdd"), 18.f);
			MaxHealth += HealthAdd;
			CurrentHealth += HealthAdd;
		}
		else if (Stat == TEXT("Assassinate") || Stat == TEXT("Crush") || Stat == TEXT("Stealing"))
		{
			BonusDamageMultiplier += GetRuntimeTuning(TEXT("ItemBonusDamageMultiplierAdd"), 0.06f);
		}
		else if (Stat == TEXT("CounterAttack") || Stat == TEXT("ReflectDamage"))
		{
			BaseArmorStat += GetRuntimeTuning(TEXT("ItemCounterArmorAdd"), 0.75f);
			BaseDamageStat += GetRuntimeTuning(TEXT("ItemCounterDamageAdd"), 0.4f);
		}
		else if (Stat == TEXT("TreasureChest") || Stat == TEXT("LootCrate"))
		{
			BaseLuckStat += GetRuntimeTuning(TEXT("ItemLuckAdd"), 0.8f);
		}
		else if (Stat == TEXT("Alchemy"))
		{
			BaseLuckStat += GetRuntimeTuning(TEXT("ItemAlchemyLuckAdd"), 0.6f);
			MaterialGainMultiplier += GetRuntimeTuning(TEXT("ItemAlchemyMaterialGainAdd"), 0.15f);
		}
	}

	void RefreshIdolRuntime()
	{
		EquippedIdols.Reset();
		for (const FName IdolID : EquippedIdolIDs)
		{
			const FT66MiniIdolDefinition* Definition = DataSubsystem ? DataSubsystem->FindIdol(IdolID) : nullptr;
			if (!Definition)
			{
				continue;
			}

			FT66MiniWidgetIdol Runtime;
			Runtime.IdolID = Definition->IdolID;
			Runtime.Category = Definition->Category;
			Runtime.BaseDamage = Definition->BaseDamage;
			Runtime.DamagePerLevel = Definition->DamagePerLevel;
			Runtime.BaseProperty = Definition->BaseProperty;
			Runtime.PropertyPerLevel = Definition->PropertyPerLevel;
			Runtime.EffectTexture = VisualSubsystem ? VisualSubsystem->LoadIdolEffectTexture(Definition->IdolID) : nullptr;
			EquippedIdols.Add(Runtime);
		}
	}

	void StartWave(const int32 InWaveIndex, const bool bResetTimer)
	{
		WaveIndex = FMath::Clamp(InWaveIndex, 1, GetMaxStageIndexForCurrentDifficulty());
		if (bResetTimer)
		{
			WaveSecondsRemaining = GetWaveDefinition() ? GetWaveDefinition()->DurationSeconds : 60.f;
		}
		EnemySpawnAccumulator = 0.f;
		InteractableSpawnAccumulator = 0.f;
		TrapSpawnAccumulator = 0.f;
		PostBossDelayRemaining = 0.f;
		BossTelegraphRemaining = 0.f;
		PendingBossID = NAME_None;
		PendingBossSpawnLocation2D = FVector2D::ZeroVector;
		bBossSpawnedForWave = false;
		bEnduranceCheatUsedThisWave = false;
	}

	const FT66MiniWaveDefinition* GetWaveDefinition() const
	{
		return DataSubsystem ? DataSubsystem->FindWave(DifficultyID, WaveIndex) : nullptr;
	}

	const FT66MiniDifficultyDefinition* GetDifficultyDefinition() const
	{
		return DataSubsystem ? DataSubsystem->FindDifficulty(DifficultyID) : nullptr;
	}

	const FT66MiniStageDefinition* GetCurrentStageDefinition() const
	{
		if (!DataSubsystem)
		{
			return nullptr;
		}
		if (const FT66MiniStageDefinition* Stage = DataSubsystem->FindStage(DifficultyID, WaveIndex))
		{
			return Stage;
		}
		return DataSubsystem->FindStageForWave(DifficultyID, WaveIndex);
	}

	int32 GetMaxStageIndexForCurrentDifficulty() const
	{
		const int32 DataMax = DataSubsystem ? DataSubsystem->GetMaxStageIndexForDifficulty(DifficultyID) : 0;
		return DataMax > 0 ? DataMax : GetRuntimeTuningInt(TEXT("MaxWavesPerDifficulty"), 10);
	}

	const FT66MiniEnemyDefinition* ChooseEnemyDefinition() const
	{
		if (!DataSubsystem)
		{
			return nullptr;
		}

		TArray<const FT66MiniEnemyDefinition*> Candidates;
		float TotalWeight = 0.f;
		if (const FT66MiniWaveDefinition* Wave = GetWaveDefinition())
		{
			for (const FName EnemyID : Wave->EnemyIDs)
			{
				if (const FT66MiniEnemyDefinition* Enemy = DataSubsystem->FindEnemy(EnemyID))
				{
					Candidates.Add(Enemy);
					TotalWeight += FMath::Max(0.1f, Enemy->SpawnWeight);
				}
			}
		}
		if (Candidates.Num() == 0)
		{
			for (const FT66MiniEnemyDefinition& Enemy : DataSubsystem->GetEnemies())
			{
				Candidates.Add(&Enemy);
				TotalWeight += FMath::Max(0.1f, Enemy.SpawnWeight);
			}
		}
		if (Candidates.Num() == 0)
		{
			return nullptr;
		}

		float Pick = FMath::FRandRange(0.f, TotalWeight);
		for (const FT66MiniEnemyDefinition* Candidate : Candidates)
		{
			Pick -= FMath::Max(0.1f, Candidate->SpawnWeight);
			if (Pick <= 0.f)
			{
				return Candidate;
			}
		}
		return Candidates.Last();
	}

	void RestoreTransientWaveState()
	{
		if (!ActiveRun)
		{
			return;
		}
		bBossSpawnedForWave = ActiveRun->bBossSpawnedForWave;
		BossTelegraphRemaining = FMath::Max(0.f, ActiveRun->BossTelegraphRemaining);
		PendingBossID = ActiveRun->PendingBossID;
		PendingBossSpawnLocation2D = T66MiniTo2D(ActiveRun->PendingBossSpawnLocation);
		EnemySpawnAccumulator = FMath::Max(0.f, ActiveRun->EnemySpawnAccumulator);
		InteractableSpawnAccumulator = FMath::Max(0.f, ActiveRun->InteractableSpawnAccumulator);
		TrapSpawnAccumulator = FMath::Max(0.f, ActiveRun->TrapSpawnAccumulator);
		PostBossDelayRemaining = FMath::Max(0.f, ActiveRun->PostBossDelayRemaining);
		WaveSecondsRemaining = FMath::Max(0.f, ActiveRun->WaveSecondsRemaining);
	}

	void RestoreEntitySnapshots()
	{
		if (!ActiveRun || !DataSubsystem || !VisualSubsystem)
		{
			return;
		}

		Enemies.Reset();
		Pickups.Reset();
		Interactables.Reset();
		Traps.Reset();

		for (const FT66MiniEnemySnapshot& Snapshot : ActiveRun->EnemySnapshots)
		{
			FT66MiniWidgetEnemy Enemy;
			Enemy.EntityID = AllocateEntityID();
			Enemy.EnemyID = Snapshot.EnemyID;
			Enemy.bBoss = Snapshot.bIsBoss;
			Enemy.Position = T66MiniClampBoard(T66MiniTo2D(Snapshot.Location));
			Enemy.CurrentHealth = Snapshot.CurrentHealth;
			Enemy.MaxHealth = Snapshot.MaxHealth;
			Enemy.MoveSpeed = Snapshot.MoveSpeed;
			Enemy.TouchDamage = Snapshot.TouchDamage;
			Enemy.MaterialDrop = Snapshot.MaterialDrop;
			Enemy.ExperienceDrop = Snapshot.ExperienceDrop;
			if (Snapshot.bIsBoss)
			{
				if (const FT66MiniBossDefinition* Boss = DataSubsystem->FindBoss(Snapshot.EnemyID))
				{
					Enemy.DisplayName = Boss->DisplayName;
					Enemy.VisualID = Boss->VisualID;
					Enemy.BehaviorProfile = Boss->BehaviorProfile;
					Enemy.Family = Boss->Family;
					Enemy.FireInterval = Boss->FireIntervalSeconds;
					Enemy.ProjectileSpeed = Boss->ProjectileSpeed;
					Enemy.ProjectileDamage = Boss->ProjectileDamage;
					Enemy.PreferredRange = GetRuntimeTuning(TEXT("BossPreferredRange"), 960.f);
					Enemy.Texture = VisualSubsystem->LoadBossTexture(Boss->VisualID);
				}
			}
			else if (const FT66MiniEnemyDefinition* Definition = DataSubsystem->FindEnemy(Snapshot.EnemyID))
			{
				Enemy.DisplayName = Definition->DisplayName;
				Enemy.VisualID = Definition->VisualID;
				Enemy.BehaviorProfile = Definition->BehaviorProfile;
				Enemy.Family = Definition->Family;
				Enemy.FireInterval = Definition->FireIntervalSeconds;
				Enemy.ProjectileSpeed = Definition->ProjectileSpeed;
				Enemy.ProjectileDamage = Definition->ProjectileDamage;
				Enemy.PreferredRange = Definition->PreferredRange;
				Enemy.Texture = VisualSubsystem->LoadEnemyTexture(Definition->VisualID);
			}
			Enemies.Add(Enemy);
		}

		for (const FT66MiniPickupSnapshot& Snapshot : ActiveRun->PickupSnapshots)
		{
			FT66MiniWidgetPickup Pickup;
			Pickup.EntityID = AllocateEntityID();
			Pickup.Position = T66MiniClampBoard(T66MiniTo2D(Snapshot.Location));
			Pickup.VisualID = Snapshot.VisualID;
			Pickup.MaterialValue = Snapshot.MaterialValue;
			Pickup.ExperienceValue = Snapshot.ExperienceValue;
			Pickup.HealValue = Snapshot.HealValue;
			Pickup.LifetimeRemaining = Snapshot.LifetimeRemaining;
			Pickup.GrantedItemID = Snapshot.GrantedItemID;
			Pickup.Texture = Snapshot.VisualID.IsEmpty() ? nullptr : VisualSubsystem->LoadInteractableTexture(Snapshot.VisualID);
			Pickups.Add(Pickup);
		}

		for (const FT66MiniInteractableSnapshot& Snapshot : ActiveRun->InteractableSnapshots)
		{
			FT66MiniWidgetInteractable Interactable;
			Interactable.EntityID = AllocateEntityID();
			Interactable.Position = T66MiniClampBoard(T66MiniTo2D(Snapshot.Location));
			Interactable.Type = static_cast<ET66MiniInteractableType>(Snapshot.InteractableType);
			Interactable.VisualID = Snapshot.VisualID;
			Interactable.LifetimeRemaining = Snapshot.LifetimeRemaining;
			if (const FT66MiniInteractableDefinition* Definition = DataSubsystem->FindInteractable(FName(*Snapshot.VisualID)))
			{
				Interactable.MaterialReward = Definition->MaterialReward;
				Interactable.GoldReward = Definition->GoldReward;
				Interactable.ExperienceReward = Definition->ExperienceReward;
				Interactable.HealAmount = Definition->HealAmount;
				Interactable.bRequiresManualInteract = Definition->bRequiresManualInteract;
			}
			else
			{
				Interactable.bRequiresManualInteract = Interactable.Type == ET66MiniInteractableType::QuickReviveMachine;
			}
			Interactable.Texture = Snapshot.VisualID.IsEmpty() ? nullptr : VisualSubsystem->LoadInteractableTexture(Snapshot.VisualID);
			Interactables.Add(Interactable);
		}

		for (const FT66MiniTrapSnapshot& Snapshot : ActiveRun->TrapSnapshots)
		{
			FT66MiniWidgetTrap Trap;
			Trap.EntityID = AllocateEntityID();
			Trap.Position = T66MiniClampBoard(T66MiniTo2D(Snapshot.Location));
			Trap.Radius = Snapshot.Radius;
			Trap.DamagePerPulse = Snapshot.DamagePerPulse;
			Trap.PulseInterval = Snapshot.PulseInterval;
			Trap.WarmupRemaining = Snapshot.WarmupRemaining;
			Trap.ActiveRemaining = Snapshot.ActiveRemaining;
			Trap.LifetimeRemaining = Snapshot.LifetimeRemaining;
			Trap.TrapVariant = Snapshot.TrapVariant;
			Traps.Add(Trap);
		}
	}

	void TickPlayer(const float DeltaSeconds)
	{
		UltimateCooldownRemaining = FMath::Max(0.f, UltimateCooldownRemaining - DeltaSeconds);
		PassiveBuffRemaining = FMath::Max(0.f, PassiveBuffRemaining - DeltaSeconds);
		PassiveSecondaryBuffRemaining = PassiveType == ET66PassiveType::QuickDraw
			? FMath::Min(GetRuntimeTuning(TEXT("PassiveQuickDrawChargeMax"), 2.f), PassiveSecondaryBuffRemaining + DeltaSeconds)
			: FMath::Max(0.f, PassiveSecondaryBuffRemaining - DeltaSeconds);

		if (PassiveSecondaryBuffRemaining <= 0.f && (UltimateType == ET66UltimateType::Juiced || UltimateType == ET66UltimateType::RabidFrenzy || PassiveType == ET66PassiveType::Endurance))
		{
			TemporaryDamageMultiplier = 1.f;
		}
		if (UltimateType == ET66UltimateType::GoldRush)
		{
			GoldGainMultiplier = (PassiveType == ET66PassiveType::TreasureHunter ? GetRuntimeTuning(TEXT("GoldRushTreasureHunterGoldScalar"), 1.20f) : 1.f)
				* (PassiveSecondaryBuffRemaining > 0.f ? GetRuntimeTuning(TEXT("GoldRushActiveGoldScalar"), 1.6f) : 1.f);
		}

		if (PassiveRegenPerSecond > 0.f)
		{
			Heal(PassiveRegenPerSecond * DeltaSeconds);
		}
		if (CompanionHealingPerSecond > 0.f)
		{
			Heal(CompanionHealingPerSecond * DeltaSeconds);
		}

		if (!KeyboardMoveInput.IsNearlyZero())
		{
			PlayerPosition = T66MiniClampBoard(PlayerPosition + KeyboardMoveInput.GetSafeNormal() * MoveSpeed * DeltaSeconds);
		}
		else if (bHasPointerTarget)
		{
			const FVector2D Delta = DesiredMovePosition - PlayerPosition;
			const float Distance = Delta.Size();
			const float Step = MoveSpeed * DeltaSeconds;
			PlayerPosition = Distance <= Step ? DesiredMovePosition : T66MiniClampBoard(PlayerPosition + (Delta / FMath::Max(1.f, Distance)) * Step);
		}

		CompanionPosition = FMath::Lerp(CompanionPosition, PlayerPosition + CompanionFollowOffset, FMath::Clamp(DeltaSeconds * 5.f, 0.f, 1.f));

		for (FT66MiniWidgetIdol& Idol : EquippedIdols)
		{
			Idol.CooldownRemaining = FMath::Max(0.f, Idol.CooldownRemaining - DeltaSeconds);
		}

		AutoAttackCooldownRemaining = FMath::Max(0.f, AutoAttackCooldownRemaining - DeltaSeconds);
		if (AutoAttackCooldownRemaining <= 0.f)
		{
			FireBasicAttack();
		}
	}

	void TickWaveDirector(const float DeltaSeconds)
	{
		if (PostBossDelayRemaining > 0.f)
		{
			PostBossDelayRemaining -= DeltaSeconds;
			if (PostBossDelayRemaining <= 0.f)
			{
				RecordClearedMiniStageProgression();
				if (WaveIndex >= GetMaxStageIndexForCurrentDifficulty())
				{
					FinalizeRun(true, TEXT("Difficulty cleared"));
					return;
				}
				ReturnToShopIntermission();
				return;
			}
			return;
		}

		if (BossTelegraphRemaining > 0.f)
		{
			BossTelegraphRemaining -= DeltaSeconds;
			if (BossTelegraphRemaining <= 0.f)
			{
				SpawnBossEnemy();
				bBossSpawnedForWave = true;
			}
			return;
		}

		if (!bBossSpawnedForWave)
		{
			const FT66MiniWaveDefinition* Wave = GetWaveDefinition();
			const FT66MiniDifficultyDefinition* Difficulty = GetDifficultyDefinition();
			WaveSecondsRemaining = FMath::Max(0.f, WaveSecondsRemaining - DeltaSeconds);
			EnemySpawnAccumulator += DeltaSeconds;
			InteractableSpawnAccumulator += DeltaSeconds;
			TrapSpawnAccumulator += DeltaSeconds;

			float SpawnInterval = Wave ? Wave->SpawnInterval : GetRuntimeTuning(TEXT("SpawnIntervalFallback"), 1.2f);
			if (Difficulty)
			{
				SpawnInterval /= FMath::Max(GetRuntimeTuning(TEXT("SpawnRateScalarMin"), 0.65f), Difficulty->SpawnRateScalar);
			}
			SpawnInterval = FMath::Max(GetRuntimeTuning(TEXT("SpawnIntervalMin"), 0.28f), SpawnInterval);
			while (EnemySpawnAccumulator >= SpawnInterval && WaveSecondsRemaining > 0.f)
			{
				EnemySpawnAccumulator -= SpawnInterval;
				SpawnWaveEnemy();
			}

			const float InteractableInterval = Wave ? Wave->InteractableInterval : (Difficulty ? Difficulty->InteractableInterval : GetRuntimeTuning(TEXT("InteractableIntervalFallback"), 18.f));
			if (InteractableSpawnAccumulator >= InteractableInterval)
			{
				InteractableSpawnAccumulator = 0.f;
				SpawnRandomInteractable();
			}

			const float TrapInterval = FMath::Max(
				GetRuntimeTuning(TEXT("TrapIntervalMin"), 5.5f),
				(GetRuntimeTuning(TEXT("TrapIntervalBase"), 13.5f) - (WaveIndex * GetRuntimeTuning(TEXT("TrapIntervalPerWave"), 1.25f)))
					/ FMath::Max(GetRuntimeTuning(TEXT("TrapSpawnRateScalarMin"), 0.75f), Difficulty ? Difficulty->SpawnRateScalar : 1.f));
			if (TrapSpawnAccumulator >= TrapInterval)
			{
				TrapSpawnAccumulator = 0.f;
				SpawnRandomTrap();
			}

			if (WaveSecondsRemaining <= 0.f)
			{
				BeginBossSpawnTelegraph();
			}
			return;
		}

		const bool bHasLiveBoss = Enemies.ContainsByPredicate([](const FT66MiniWidgetEnemy& Enemy)
		{
			return Enemy.bBoss && Enemy.CurrentHealth > 0.f;
		});
		if (!bHasLiveBoss && PostBossDelayRemaining <= 0.f)
		{
			PostBossDelayRemaining = GetRuntimeTuning(TEXT("PostBossDelaySeconds"), 2.f);
		}
	}

	void TickEnemies(const float DeltaSeconds)
	{
		for (FT66MiniWidgetEnemy& Enemy : Enemies)
		{
			Enemy.Lifetime += DeltaSeconds;
			Enemy.HitFlashRemaining = FMath::Max(0.f, Enemy.HitFlashRemaining - DeltaSeconds);
			Enemy.ContactCooldown = FMath::Max(0.f, Enemy.ContactCooldown - DeltaSeconds);
			Enemy.StunRemaining = FMath::Max(0.f, Enemy.StunRemaining - DeltaSeconds);
			TickEnemyDots(Enemy, DeltaSeconds);
			if (Enemy.CurrentHealth <= 0.f)
			{
				continue;
			}
			if (Enemy.StunRemaining > 0.f)
			{
				continue;
			}

			const float DistanceToPlayer = FVector2D::Distance(Enemy.Position, PlayerPosition);
			const FVector2D ToPlayer = T66MiniSafeDirection(Enemy.Position, PlayerPosition);
			bool bShouldMoveTowardPlayer = true;
			if (Enemy.Family == ET66MiniEnemyFamily::Ranged && DistanceToPlayer <= Enemy.PreferredRange)
			{
				bShouldMoveTowardPlayer = false;
			}
			if (bShouldMoveTowardPlayer)
			{
				Enemy.Position = T66MiniClampBoard(Enemy.Position + ToPlayer * Enemy.MoveSpeed * DeltaSeconds);
			}
			else
			{
				const FVector2D Strafe(-ToPlayer.Y, ToPlayer.X);
				Enemy.Position = T66MiniClampBoard(Enemy.Position + Strafe * FMath::Sin(Enemy.Lifetime * 2.4f) * Enemy.MoveSpeed * 0.22f * DeltaSeconds);
			}

			if (DistanceToPlayer <= MiniPlayerRadius + MiniEnemyRadius && Enemy.ContactCooldown <= 0.f)
			{
				ApplyDamageToPlayer(Enemy.TouchDamage);
				Enemy.ContactCooldown = MiniTouchDamageInterval;
			}

			if (Enemy.Family == ET66MiniEnemyFamily::Ranged || Enemy.Family == ET66MiniEnemyFamily::Boss || Enemy.BehaviorProfile == ET66MiniEnemyBehaviorProfile::Sharpshooter)
			{
				Enemy.FireCooldown = FMath::Max(0.f, Enemy.FireCooldown - DeltaSeconds);
				if (Enemy.FireCooldown <= 0.f && DistanceToPlayer <= FMath::Max(Enemy.PreferredRange + 260.f, 980.f))
				{
					SpawnProjectile(false, Enemy.Position, ToPlayer, Enemy.ProjectileDamage, Enemy.ProjectileSpeed, MiniEnemyProjectileRadius, ET66MiniWidgetProjectileBehavior::Basic, 0, 0, 0.f, 0.5f, 0.f, 0.f, NAME_None, nullptr);
					Enemy.FireCooldown = FMath::Max(0.35f, Enemy.FireInterval);
				}
			}
		}

		for (int32 Index = Enemies.Num() - 1; Index >= 0; --Index)
		{
			if (Enemies[Index].CurrentHealth <= 0.f)
			{
				HandleEnemyKilled(Index);
			}
		}
	}

	void TickEnemyDots(FT66MiniWidgetEnemy& Enemy, const float DeltaSeconds)
	{
		for (int32 Index = Enemy.Dots.Num() - 1; Index >= 0; --Index)
		{
			FT66MiniWidgetDot& Dot = Enemy.Dots[Index];
			Dot.Remaining -= DeltaSeconds;
			Dot.Accumulator += DeltaSeconds;
			while (Dot.Accumulator >= Dot.TickInterval && Dot.Remaining > 0.f)
			{
				Dot.Accumulator -= Dot.TickInterval;
				Enemy.CurrentHealth -= Dot.TickDamage;
				Enemy.HitFlashRemaining = 0.12f;
				AddCombatText(Enemy.Position, Dot.TickDamage, FLinearColor(0.58f, 1.0f, 0.36f, 1.f), TEXT("-"));
			}
			if (Dot.Remaining <= 0.f)
			{
				Enemy.Dots.RemoveAt(Index);
			}
		}
	}

	void TickProjectiles(const float DeltaSeconds)
	{
		for (int32 Index = Projectiles.Num() - 1; Index >= 0; --Index)
		{
			FT66MiniWidgetProjectile& Projectile = Projectiles[Index];
			Projectile.Lifetime -= DeltaSeconds;
			if (Projectile.Lifetime <= 0.f)
			{
				Projectiles.RemoveAt(Index);
				continue;
			}

			if (Projectile.bPlayerOwned && Projectile.HomingTargetID != INDEX_NONE)
			{
				if (const FT66MiniWidgetEnemy* Target = FindEnemyByID(Projectile.HomingTargetID))
				{
					const float Speed = Projectile.Velocity.Size();
					Projectile.Velocity = T66MiniSafeDirection(Projectile.Position, Target->Position) * FMath::Max(1.f, Speed);
				}
			}

			Projectile.Position = T66MiniClampBoard(Projectile.Position + Projectile.Velocity * DeltaSeconds);
			if (Projectile.bPlayerOwned)
			{
				if (HandlePlayerProjectileHit(Projectile))
				{
					Projectiles.RemoveAt(Index);
				}
			}
			else if (FVector2D::DistSquared(Projectile.Position, PlayerPosition) <= FMath::Square(Projectile.Radius + MiniPlayerRadius))
			{
				ApplyDamageToPlayer(Projectile.Damage);
				AddVfx(PlayerPosition, 160.f, FLinearColor(1.0f, 0.25f, 0.18f, 0.32f), 0.38f);
				Projectiles.RemoveAt(Index);
			}
		}
	}

	bool HandlePlayerProjectileHit(FT66MiniWidgetProjectile& Projectile)
	{
		for (int32 EnemyIndex = 0; EnemyIndex < Enemies.Num(); ++EnemyIndex)
		{
			FT66MiniWidgetEnemy& Enemy = Enemies[EnemyIndex];
			if (Enemy.CurrentHealth <= 0.f || Projectile.HitEnemyIDs.Contains(Enemy.EntityID))
			{
				continue;
			}

			if (FVector2D::DistSquared(Projectile.Position, Enemy.Position) > FMath::Square(Projectile.Radius + MiniEnemyRadius))
			{
				continue;
			}

			Projectile.HitEnemyIDs.Add(Enemy.EntityID);
			ApplyDamageToEnemy(EnemyIndex, Projectile.Damage, Projectile.StunDuration, Projectile.DotTickDamage, Projectile.DotTickInterval, Projectile.DotDuration);
			AddVfx(Enemy.Position, Projectile.Radius * 0.70f, FLinearColor(1.0f, 0.84f, 0.32f, 0.28f), 0.38f, Projectile.Texture, FName(TEXT("AnimatedStyle.Mini.ProjectileImpact")));

			if (Projectile.Behavior == ET66MiniWidgetProjectileBehavior::Aoe)
			{
				ApplyAreaDamage(Enemy.Position, Projectile.Radius, Projectile.Damage * 0.65f, Projectile.StunDuration, Projectile.DotTickDamage, Projectile.DotDuration);
				return true;
			}

			if (Projectile.Behavior == ET66MiniWidgetProjectileBehavior::Bounce && Projectile.BounceRemaining > 0)
			{
				if (const FT66MiniWidgetEnemy* NextTarget = FindClosestEnemy(Enemy.Position, Enemy.EntityID, FMath::Max(340.f, Projectile.Radius * 2.f)))
				{
					--Projectile.BounceRemaining;
					Projectile.HomingTargetID = NextTarget->EntityID;
					Projectile.Velocity = T66MiniSafeDirection(Projectile.Position, NextTarget->Position) * FMath::Max(1.f, Projectile.Velocity.Size());
					return false;
				}
			}

			if (Projectile.PierceRemaining > 0)
			{
				--Projectile.PierceRemaining;
				return false;
			}
			return true;
		}

		return false;
	}

	void TickPickups(const float DeltaSeconds)
	{
		for (int32 Index = Pickups.Num() - 1; Index >= 0; --Index)
		{
			FT66MiniWidgetPickup& Pickup = Pickups[Index];
			Pickup.Age += DeltaSeconds;
			Pickup.LifetimeRemaining -= DeltaSeconds;
			if (Pickup.LifetimeRemaining <= 0.f)
			{
				Pickups.RemoveAt(Index);
				continue;
			}

			const float MagnetRadius = GetRuntimeTuning(TEXT("PickupMagnetBaseRadius"), 420.f) + BaseLuckStat * GetRuntimeTuning(TEXT("PickupMagnetLuckRadiusScalar"), 18.f);
			if (FVector2D::DistSquared(Pickup.Position, PlayerPosition) <= FMath::Square(MagnetRadius))
			{
				const FVector2D Direction = T66MiniSafeDirection(Pickup.Position, PlayerPosition);
				const float PullSpeed = GetRuntimeTuning(TEXT("PickupMagnetBasePullSpeed"), 620.f) + BaseLuckStat * GetRuntimeTuning(TEXT("PickupMagnetLuckPullSpeedScalar"), 24.f);
				Pickup.Position += Direction * PullSpeed * DeltaSeconds;
			}

			if (FVector2D::DistSquared(Pickup.Position, PlayerPosition) <= FMath::Square(MiniPickupRadius + MiniPlayerRadius))
			{
				CollectPickup(Index);
			}
		}
	}

	void TickInteractables(const float DeltaSeconds)
	{
		for (int32 Index = Interactables.Num() - 1; Index >= 0; --Index)
		{
			FT66MiniWidgetInteractable& Interactable = Interactables[Index];
			Interactable.Age += DeltaSeconds;
			Interactable.LifetimeRemaining -= DeltaSeconds;
			if (Interactable.LifetimeRemaining <= 0.f)
			{
				Interactables.RemoveAt(Index);
				continue;
			}
			if (!Interactable.bRequiresManualInteract && FVector2D::DistSquared(Interactable.Position, PlayerPosition) <= FMath::Square(MiniInteractableRadius + MiniPlayerRadius))
			{
				ConsumeInteractable(Index);
			}
		}
	}

	void TickTraps(const float DeltaSeconds)
	{
		for (int32 Index = Traps.Num() - 1; Index >= 0; --Index)
		{
			FT66MiniWidgetTrap& Trap = Traps[Index];
			Trap.LifetimeRemaining -= DeltaSeconds;
			if (Trap.LifetimeRemaining <= 0.f)
			{
				Traps.RemoveAt(Index);
				continue;
			}
			if (Trap.WarmupRemaining > 0.f)
			{
				Trap.WarmupRemaining = FMath::Max(0.f, Trap.WarmupRemaining - DeltaSeconds);
				continue;
			}
			Trap.ActiveRemaining = FMath::Max(0.f, Trap.ActiveRemaining - DeltaSeconds);
			if (Trap.ActiveRemaining <= 0.f)
			{
				Traps.RemoveAt(Index);
				continue;
			}
			Trap.PulseAccumulator += DeltaSeconds;
			if (Trap.PulseAccumulator >= FMath::Max(0.05f, Trap.PulseInterval))
			{
				Trap.PulseAccumulator = 0.f;
				AddVfx(Trap.Position, Trap.Radius, FLinearColor(1.0f, 0.18f, 0.12f, 0.22f), 0.28f, nullptr, FName(TEXT("AnimatedStyle.Mini.TrapPulse")));
				if (FVector2D::DistSquared(Trap.Position, PlayerPosition) <= FMath::Square(Trap.Radius))
				{
					ApplyDamageToPlayer(Trap.DamagePerPulse);
				}
			}
		}
	}

	void TickEvents(const float DeltaSeconds)
	{
		for (int32 Index = VfxEvents.Num() - 1; Index >= 0; --Index)
		{
			VfxEvents[Index].Age += DeltaSeconds;
			if (VfxEvents[Index].Age >= VfxEvents[Index].Lifetime)
			{
				VfxEvents.RemoveAt(Index);
			}
		}
		for (int32 Index = CombatTexts.Num() - 1; Index >= 0; --Index)
		{
			CombatTexts[Index].Remaining -= DeltaSeconds;
			CombatTexts[Index].Position += CombatTexts[Index].Velocity * DeltaSeconds;
			if (CombatTexts[Index].Remaining <= 0.f)
			{
				CombatTexts.RemoveAt(Index);
			}
		}
		for (int32 Index = QueuedBursts.Num() - 1; Index >= 0; --Index)
		{
			QueuedBursts[Index].Delay -= DeltaSeconds;
			if (QueuedBursts[Index].Delay <= 0.f)
			{
				ApplyAreaDamage(QueuedBursts[Index].Position, QueuedBursts[Index].Radius, QueuedBursts[Index].Damage, QueuedBursts[Index].StunDuration, QueuedBursts[Index].DotTickDamage, QueuedBursts[Index].DotDuration);
				AddVfx(QueuedBursts[Index].Position, QueuedBursts[Index].Radius, QueuedBursts[Index].Tint, 0.48f, nullptr, FName(TEXT("AnimatedStyle.Mini.Burst")));
				QueuedBursts.RemoveAt(Index);
			}
		}
		for (int32 Index = AreaEffects.Num() - 1; Index >= 0; --Index)
		{
			AreaEffects[Index].Remaining -= DeltaSeconds;
			AreaEffects[Index].Accumulator += DeltaSeconds;
			while (AreaEffects[Index].Accumulator >= AreaEffects[Index].TickInterval && AreaEffects[Index].Remaining > 0.f)
			{
				AreaEffects[Index].Accumulator -= AreaEffects[Index].TickInterval;
				ApplyAreaDamage(AreaEffects[Index].Position, AreaEffects[Index].Radius, AreaEffects[Index].TickDamage, AreaEffects[Index].StunDuration, 0.f, 0.f);
			}
			if (AreaEffects[Index].Remaining <= 0.f)
			{
				AreaEffects.RemoveAt(Index);
			}
		}
	}

	void FireBasicAttack()
	{
		const float Cooldown = FMath::Max(
			GetRuntimeTuning(TEXT("AutoAttackCooldownMin"), 0.22f),
			GetRuntimeTuning(TEXT("AutoAttackCooldownNumerator"), 1.20f) / FMath::Max(1.f, BaseAttackSpeedStat + (HeroLevel * GetRuntimeTuning(TEXT("AutoAttackLevelSpeedScalar"), 0.08f))));
		AutoAttackCooldownRemaining = Cooldown;

		const FT66MiniWidgetEnemy* Target = FindClosestEnemy(PlayerPosition, INDEX_NONE, AttackRange);
		if (!Target)
		{
			return;
		}

		float Damage = ((BaseDamageStat * GetRuntimeTuning(TEXT("BasicDamageBaseScalar"), 0.95f))
			+ (HeroLevel * GetRuntimeTuning(TEXT("BasicDamageLevelScalar"), 0.45f))) * BonusDamageMultiplier * TemporaryDamageMultiplier;
		if (FMath::FRand() < CritChance)
		{
			Damage *= CritDamageMultiplier;
		}
		if (PassiveType == ET66PassiveType::QuickDraw && PassiveSecondaryBuffRemaining >= GetRuntimeTuning(TEXT("PassiveQuickDrawReadyThreshold"), 1.4f))
		{
			Damage *= GetRuntimeTuning(TEXT("PassiveQuickDrawDamageScalar"), 1.45f);
			PassiveSecondaryBuffRemaining = 0.f;
		}

		const FVector2D Direction = T66MiniSafeDirection(PlayerPosition, Target->Position);
		SpawnProjectile(true, PlayerPosition + Direction * 72.f, Direction, Damage, GetRuntimeTuning(TEXT("BasicProjectileSpeed"), 2400.f), GetRuntimeTuning(TEXT("BasicProjectileRadius"), 180.f), ET66MiniWidgetProjectileBehavior::Basic, 0, 0, 0.f, GetRuntimeTuning(TEXT("BasicProjectileDotInterval"), 0.5f), 0.f, 0.f, NAME_None, PlayerProjectileTexture, Target->EntityID);
	}

	void SpawnProjectile(
		const bool bPlayerOwned,
		const FVector2D& Position,
		const FVector2D& Direction,
		const float Damage,
		const float Speed,
		const float Radius,
		const ET66MiniWidgetProjectileBehavior Behavior,
		const int32 PierceRemaining,
		const int32 BounceRemaining,
		const float DotTickDamage,
		const float DotTickInterval,
		const float DotDuration,
		const float StunDuration,
		const FName SourceIdolID,
		UTexture2D* Texture,
		const int32 HomingTargetID = INDEX_NONE)
	{
		FT66MiniWidgetProjectile Projectile;
		Projectile.EntityID = AllocateEntityID();
		Projectile.bPlayerOwned = bPlayerOwned;
		Projectile.Position = Position;
		Projectile.Velocity = Direction.GetSafeNormal() * Speed;
		Projectile.Damage = Damage;
		Projectile.Radius = Radius;
		Projectile.Behavior = Behavior;
		Projectile.PierceRemaining = PierceRemaining;
		Projectile.BounceRemaining = BounceRemaining;
		Projectile.DotTickDamage = DotTickDamage;
		Projectile.DotTickInterval = DotTickInterval;
		Projectile.DotDuration = DotDuration;
		Projectile.StunDuration = StunDuration;
		Projectile.SourceIdolID = SourceIdolID;
		Projectile.Texture = Texture;
		Projectile.HomingTargetID = HomingTargetID;
		Projectiles.Add(Projectile);
		AddVfx(Position, Radius * 0.45f, bPlayerOwned ? FLinearColor(1.0f, 0.86f, 0.32f, 0.20f) : FLinearColor(1.0f, 0.18f, 0.12f, 0.20f), 0.22f, Texture, FName(TEXT("AnimatedStyle.Mini.Projectile")));
	}

	void ApplyDamageToEnemy(const int32 EnemyIndex, const float Damage, const float StunDuration, const float DotTickDamage, const float DotTickInterval, const float DotDuration)
	{
		if (!Enemies.IsValidIndex(EnemyIndex) || Damage <= 0.f)
		{
			return;
		}

		FT66MiniWidgetEnemy& Enemy = Enemies[EnemyIndex];
		Enemy.CurrentHealth = FMath::Max(0.f, Enemy.CurrentHealth - Damage);
		Enemy.HitFlashRemaining = 0.14f;
		if (StunDuration > 0.f)
		{
			Enemy.StunRemaining = FMath::Max(Enemy.StunRemaining, StunDuration);
		}
		if (DotTickDamage > 0.f && DotDuration > 0.f)
		{
			FT66MiniWidgetDot Dot;
			Dot.TickDamage = DotTickDamage;
			Dot.TickInterval = FMath::Max(0.05f, DotTickInterval);
			Dot.Remaining = DotDuration;
			Enemy.Dots.Add(Dot);
		}
		AddCombatText(Enemy.Position, Damage, FLinearColor(1.0f, 0.84f, 0.34f, 1.f), TEXT("-"));
		HandleSuccessfulHit(Damage);
		HandleBasicAttackImpact(EnemyIndex, Damage);
	}

	void HandleBasicAttackImpact(const int32 EnemyIndex, const float DamageDealt)
	{
		if (!Enemies.IsValidIndex(EnemyIndex) || EquippedIdols.Num() == 0)
		{
			return;
		}

		const int32 IdolCount = EquippedIdols.Num();
		for (int32 Offset = 0; Offset < IdolCount; ++Offset)
		{
			const int32 RuntimeIndex = (NextIdolProcIndex + Offset) % IdolCount;
			FT66MiniWidgetIdol& Idol = EquippedIdols[RuntimeIndex];
			if (Idol.CooldownRemaining > 0.f)
			{
				continue;
			}
			TriggerIdolFollowUp(Idol, EnemyIndex, DamageDealt);
			NextIdolProcIndex = (RuntimeIndex + 1) % IdolCount;
			break;
		}
	}

	void TriggerIdolFollowUp(FT66MiniWidgetIdol& Idol, const int32 EnemyIndex, const float DamageDealt)
	{
		if (!Enemies.IsValidIndex(EnemyIndex))
		{
			return;
		}

		FT66MiniWidgetEnemy& Enemy = Enemies[EnemyIndex];
		const float Cooldown = FMath::Max(0.35f, 1.05f / FMath::Max(1.f, BaseAttackSpeedStat + (HeroLevel * 0.05f)));
		Idol.CooldownRemaining = Cooldown;
		const float FollowUpDamage = ((BaseDamageStat * 0.30f) + (Idol.BaseDamage * 0.35f) + (HeroLevel * Idol.DamagePerLevel * 0.20f)) * BonusDamageMultiplier;
		const float Radius = FMath::Max(240.f, Idol.BaseProperty + 140.f + AoeRadiusBonus);
		const float DotTickDamage = FMath::Max(1.5f, (FollowUpDamage * 0.28f) + DotDamageBonus);
		const float StunDuration = Idol.IdolID == FName(TEXT("Idol_Electric")) ? (0.16f + (HeroLevel * 0.004f)) : 0.f;

		AddVfx(Enemy.Position, Radius * 0.45f, FLinearColor::White, 0.72f, Idol.EffectTexture, FName(TEXT("AnimatedStyle.Mini.IdolImpact")));
		if (Idol.Category.Equals(TEXT("AOE"), ESearchCase::IgnoreCase))
		{
			ApplyAreaDamage(Enemy.Position, Radius, FollowUpDamage, StunDuration, 0.f, 0.f);
		}
		else if (Idol.Category.Equals(TEXT("DOT"), ESearchCase::IgnoreCase))
		{
			ApplyDamageToEnemy(EnemyIndex, FollowUpDamage * 0.65f, 0.f, DotTickDamage, 0.42f, 2.8f + (HeroLevel * 0.05f));
		}
		else if (Idol.Category.Equals(TEXT("Bounce"), ESearchCase::IgnoreCase))
		{
			ApplyDamageToEnemy(EnemyIndex, FollowUpDamage, StunDuration, 0.f, 0.5f, 0.f);
			if (const FT66MiniWidgetEnemy* NextEnemy = FindClosestEnemy(Enemy.Position, Enemy.EntityID, FMath::Max(340.f, Radius)))
			{
				const FVector2D Direction = T66MiniSafeDirection(Enemy.Position, NextEnemy->Position);
				SpawnProjectile(true, Enemy.Position, Direction, FollowUpDamage * 0.85f, 2150.f, FMath::Max(340.f, Radius), ET66MiniWidgetProjectileBehavior::Bounce, 0, FMath::Max(0, BonusBounceCount), 0.f, 0.5f, 0.f, StunDuration * 0.85f, Idol.IdolID, Idol.EffectTexture, NextEnemy->EntityID);
			}
		}
		else
		{
			ApplyDamageToEnemy(EnemyIndex, FollowUpDamage, StunDuration, 0.f, 0.5f, 0.f);
			if (const FT66MiniWidgetEnemy* Secondary = FindClosestEnemy(Enemy.Position, Enemy.EntityID, 260.f + (BonusPierceCount * 70.f)))
			{
				const int32 SecondaryIndex = FindEnemyIndexByID(Secondary->EntityID);
				if (SecondaryIndex != INDEX_NONE)
				{
					ApplyDamageToEnemy(SecondaryIndex, FollowUpDamage * 0.70f, 0.f, 0.f, 0.5f, 0.f);
				}
			}
		}
	}

	void ApplyAreaDamage(const FVector2D& Center, const float Radius, const float Damage, const float StunDuration, const float DotTickDamage, const float DotDuration)
	{
		for (int32 Index = 0; Index < Enemies.Num(); ++Index)
		{
			if (Enemies[Index].CurrentHealth <= 0.f)
			{
				continue;
			}
			const float Distance = FVector2D::Distance(Center, Enemies[Index].Position);
			if (Distance > Radius)
			{
				continue;
			}
			const float Alpha = 1.f - FMath::Clamp(Distance / FMath::Max(1.f, Radius), 0.f, 1.f);
			ApplyDamageToEnemy(Index, Damage * (0.65f + Alpha * 0.35f), StunDuration, DotTickDamage, 0.42f, DotDuration);
		}
	}

	void ApplyDamageToPlayer(const float Amount)
	{
		if (Amount <= 0.f || CurrentHealth <= 0.f)
		{
			return;
		}
		if (EvasionChance > 0.f && FMath::FRand() < EvasionChance)
		{
			bEvasiveNextAttackAppliesDot = PassiveType == ET66PassiveType::Evasive;
			AddCombatText(PlayerPosition, 0.f, FLinearColor(0.62f, 0.86f, 1.f, 1.f), TEXT("DODGE"));
			return;
		}

		const float ArmorMitigation = BaseArmorStat / (BaseArmorStat + GetRuntimeTuning(TEXT("DamageArmorMitigationDenominator"), 20.f));
		float FinalDamage = Amount * (1.f - ArmorMitigation);
		if (PassiveType == ET66PassiveType::IronWill)
		{
			FinalDamage *= CurrentHealth <= (MaxHealth * GetRuntimeTuning(TEXT("IronWillLowHealthThreshold"), 0.35f))
				? GetRuntimeTuning(TEXT("IronWillLowHealthDamageScalar"), 0.72f)
				: GetRuntimeTuning(TEXT("IronWillDamageScalar"), 0.86f);
		}
		if (PassiveType == ET66PassiveType::Unflinching)
		{
			FinalDamage *= GetRuntimeTuning(TEXT("UnflinchingDamageScalar"), 0.84f);
		}
		FinalDamage = FMath::Max(GetRuntimeTuning(TEXT("IncomingDamageMin"), 1.f), FinalDamage);

		if (PassiveType == ET66PassiveType::Endurance && !bEnduranceCheatUsedThisWave && CurrentHealth - FinalDamage <= 0.f)
		{
			bEnduranceCheatUsedThisWave = true;
			CurrentHealth = GetRuntimeTuning(TEXT("EnduranceCheatHealth"), 1.f);
			PassiveSecondaryBuffRemaining = GetRuntimeTuning(TEXT("EnduranceBuffDuration"), 4.f);
			TemporaryDamageMultiplier = FMath::Max(TemporaryDamageMultiplier, GetRuntimeTuning(TEXT("EnduranceDamageMultiplier"), 1.18f));
			AddVfx(PlayerPosition, 220.f, FLinearColor(0.94f, 0.92f, 0.42f, 0.44f), 0.75f);
			return;
		}

		CurrentHealth = FMath::Max(0.f, CurrentHealth - FinalDamage);
		if (bQuickReviveReady && CurrentHealth <= 0.f)
		{
			bQuickReviveReady = false;
			CurrentHealth = FMath::Max(MaxHealth * GetRuntimeTuning(TEXT("QuickReviveHealthScalar"), 0.45f), GetRuntimeTuning(TEXT("QuickReviveMinHealth"), 24.f));
			AddVfx(PlayerPosition, 250.f, FLinearColor(0.40f, 1.0f, 0.64f, 0.42f), 0.90f);
			return;
		}
		AddCombatText(PlayerPosition, FinalDamage, FLinearColor(1.0f, 0.22f, 0.20f, 1.f), TEXT("-"));
		AddVfx(PlayerPosition, 150.f, FLinearColor(0.98f, 0.34f, 0.22f, 0.34f), 0.45f);

		if (PassiveType == ET66PassiveType::BrawlersFury)
		{
			PassiveStacks = FMath::Clamp(PassiveStacks + 1, 0, GetRuntimeTuningInt(TEXT("PassiveBrawlersMaxStacks"), 4));
			PassiveBuffRemaining = GetRuntimeTuning(TEXT("PassiveBrawlersBuffDuration"), 5.f);
		}
		if (PassiveType == ET66PassiveType::IronWill && CurrentHealth <= (MaxHealth * GetRuntimeTuning(TEXT("PassiveIronWillRegenHealthThreshold"), 0.30f)))
		{
			PassiveRegenPerSecond = FMath::Max(PassiveRegenPerSecond, GetRuntimeTuning(TEXT("PassiveIronWillRegenPerSecond"), 1.6f));
		}

		if (CurrentHealth <= 0.f)
		{
			FinalizeRun(false, TEXT("Defeated"));
		}
	}

	void HandleSuccessfulHit(const float DamageDealt)
	{
		if (DamageDealt > 0.f && LifeStealChance > 0.f && FMath::FRand() < LifeStealChance)
		{
			Heal(FMath::Max(GetRuntimeTuning(TEXT("LifeStealHealMin"), 1.f), DamageDealt * GetRuntimeTuning(TEXT("LifeStealDamageScalar"), 0.08f)));
		}
	}

	void Heal(const float Amount)
	{
		if (Amount > 0.f)
		{
			CurrentHealth = FMath::Min(MaxHealth, CurrentHealth + Amount);
		}
	}

	void GainMaterials(const int32 Amount)
	{
		const int32 Applied = Amount > 0 ? FMath::RoundToInt(Amount * MaterialGainMultiplier) : Amount;
		Materials = FMath::Max(0, Materials + Applied);
		if (Applied > 0)
		{
			Gold = FMath::Max(0, Gold + Applied);
		}
	}

	void GainGold(const int32 Amount)
	{
		const int32 Applied = Amount > 0 ? FMath::RoundToInt(Amount * GoldGainMultiplier) : Amount;
		Gold = FMath::Max(0, Gold + Applied);
	}

	void GainExperience(const float Amount)
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
			ApplyLevelUpBonuses(1, true);
		}
	}

	float GetNextLevelThreshold() const
	{
		return GetRuntimeTuning(TEXT("NextLevelBase"), 18.f) + (HeroLevel * GetRuntimeTuning(TEXT("NextLevelPerLevel"), 10.f));
	}

	float GetUltimateBaseDamage() const
	{
		float Damage = (BaseDamageStat * GetRuntimeTuning(TEXT("UltimateBaseDamageStatScalar"), 2.f))
			+ (HeroLevel * GetRuntimeTuning(TEXT("UltimateBaseDamageLevelScalar"), 1.4f));
		if (PassiveType == ET66PassiveType::ArcaneAmplification)
		{
			Damage *= GetRuntimeTuning(TEXT("PassiveArcaneUltimateDamageScalar"), 1.15f);
		}
		if (PassiveType == ET66PassiveType::RallyingBlow && PassiveBuffRemaining > 0.f)
		{
			Damage *= 1.f + PassiveStacks * GetRuntimeTuning(TEXT("PassiveRallyUltimateDamagePerStack"), 0.08f);
		}
		return Damage;
	}

	void HandleEnemyKilled(const int32 EnemyIndex)
	{
		if (!Enemies.IsValidIndex(EnemyIndex))
		{
			return;
		}

		const FT66MiniWidgetEnemy Enemy = Enemies[EnemyIndex];
		Enemies.RemoveAt(EnemyIndex);
		AddCombatText(Enemy.Position, 0.f, FLinearColor(1.0f, 0.80f, 0.25f, 1.f), Enemy.bBoss ? TEXT("BOSS DOWN") : TEXT("KO"));
		AddVfx(Enemy.Position, Enemy.bBoss ? 360.f : 180.f, Enemy.bBoss ? FLinearColor(1.0f, 0.42f, 0.16f, 0.40f) : FLinearColor(1.0f, 0.80f, 0.25f, 0.28f), Enemy.bBoss ? 0.90f : 0.48f, Enemy.Texture, FName(TEXT("AnimatedStyle.Mini.EnemyDeath")));
		SpawnPickup(Enemy.Position, Enemy.MaterialDrop, Enemy.ExperienceDrop, 0.f, Enemy.bBoss ? 18.f : 12.f, NAME_None, TEXT("Material"));
		if (PassiveType == ET66PassiveType::TreasureHunter)
		{
			GainGold(GetRuntimeTuningInt(Enemy.bBoss ? TEXT("PassiveTreasureBossGoldBonus") : TEXT("PassiveTreasureKillGold"), Enemy.bBoss ? 6 : 2));
		}
		if (Enemy.bBoss)
		{
			PostBossDelayRemaining = FMath::Max(PostBossDelayRemaining, GetRuntimeTuning(TEXT("PostBossDelaySeconds"), 2.f));
		}
	}

	void SpawnWaveEnemy()
	{
		const FT66MiniEnemyDefinition* Definition = ChooseEnemyDefinition();
		if (!Definition || !VisualSubsystem)
		{
			return;
		}
		const FT66MiniWaveDefinition* Wave = GetWaveDefinition();
		const FT66MiniDifficultyDefinition* Difficulty = GetDifficultyDefinition();
		const float SpawnAngle = FMath::FRandRange(0.f, UE_TWO_PI);
		const FVector2D SpawnDirection(FMath::Cos(SpawnAngle), FMath::Sin(SpawnAngle));
		const FVector2D SpawnLocation = T66MiniClampBoard(PlayerPosition + SpawnDirection * FMath::FRandRange(1450.f, 1900.f));
		const float DifficultyHealthScalar = Difficulty ? Difficulty->HealthScalar : 1.f;
		const float DifficultyDamageScalar = Difficulty ? Difficulty->DamageScalar : 1.f;
		const float DifficultySpeedScalar = Difficulty ? Difficulty->SpeedScalar : 1.f;
		const float WaveHealthScalar = Wave ? Wave->EnemyHealthScalar : 1.f;
		const float WaveDamageScalar = Wave ? Wave->EnemyDamageScalar : 1.f;
		const float WaveSpeedScalar = Wave ? Wave->EnemySpeedScalar : 1.f;
		const float ProgressScalar = GetRuntimeTuning(TEXT("EnemyProgressScalarBase"), 1.f) + ((WaveIndex - 1) * GetRuntimeTuning(TEXT("EnemyProgressScalarPerWave"), 0.16f));

		FT66MiniWidgetEnemy Enemy;
		Enemy.EntityID = AllocateEntityID();
		Enemy.EnemyID = Definition->EnemyID;
		Enemy.DisplayName = Definition->DisplayName;
		Enemy.VisualID = Definition->VisualID;
		Enemy.Position = SpawnLocation;
		Enemy.CurrentHealth = Definition->BaseHealth * DifficultyHealthScalar * WaveHealthScalar * ProgressScalar;
		Enemy.MaxHealth = Enemy.CurrentHealth;
		Enemy.MoveSpeed = Definition->BaseSpeed * DifficultySpeedScalar * WaveSpeedScalar;
		Enemy.TouchDamage = Definition->BaseTouchDamage * DifficultyDamageScalar * WaveDamageScalar;
		Enemy.MaterialDrop = FMath::RoundToInt(Definition->BaseMaterials * ProgressScalar);
		Enemy.ExperienceDrop = Definition->BaseExperience * ProgressScalar;
		Enemy.BehaviorProfile = Definition->BehaviorProfile;
		Enemy.Family = Definition->Family;
		Enemy.FireInterval = Definition->FireIntervalSeconds / FMath::Max(GetRuntimeTuning(TEXT("EnemyRangedFireSpawnRateMin"), 0.80f), Difficulty ? Difficulty->SpawnRateScalar : 1.f);
		Enemy.FireCooldown = FMath::FRandRange(0.25f, Enemy.FireInterval);
		Enemy.ProjectileSpeed = Definition->ProjectileSpeed;
		Enemy.ProjectileDamage = Definition->ProjectileDamage * DifficultyDamageScalar * WaveDamageScalar;
		Enemy.PreferredRange = Definition->PreferredRange;
		Enemy.Texture = VisualSubsystem->LoadEnemyTexture(Definition->VisualID);
		Enemies.Add(Enemy);
		AddVfx(SpawnLocation, 150.f, FLinearColor(0.96f, 0.20f, 0.18f, 0.18f), 0.55f);
	}

	void BeginBossSpawnTelegraph()
	{
		if (bBossSpawnedForWave || BossTelegraphRemaining > 0.f)
		{
			return;
		}
		if (const FT66MiniStageDefinition* Stage = GetCurrentStageDefinition(); Stage && !Stage->BossID.IsNone())
		{
			PendingBossID = Stage->BossID;
		}
		else if (const FT66MiniWaveDefinition* Wave = GetWaveDefinition())
		{
			PendingBossID = Wave->BossID;
		}
		PendingBossSpawnLocation2D = T66MiniClampBoard(PlayerPosition + FVector2D(MiniArenaHalfExtentX * GetRuntimeTuning(TEXT("BossSpawnOffsetScalar"), 0.72f), 0.f));
		const FT66MiniBossDefinition* Boss = DataSubsystem ? DataSubsystem->FindBoss(PendingBossID) : nullptr;
		if (!Boss)
		{
			PendingBossID = NAME_None;
			bBossSpawnedForWave = true;
			PostBossDelayRemaining = GetRuntimeTuning(TEXT("PostBossDelaySeconds"), 2.f);
			return;
		}
		BossTelegraphRemaining = Boss->TelegraphSeconds > 0.f ? Boss->TelegraphSeconds : GetRuntimeTuning(TEXT("BossTelegraphFallbackSeconds"), 1.2f);
		AddVfx(PendingBossSpawnLocation2D, GetRuntimeTuning(TEXT("BossTelegraphRadius"), 260.f), FLinearColor(0.98f, 0.26f, 0.20f, 0.38f), BossTelegraphRemaining, nullptr, FName(TEXT("AnimatedStyle.Mini.BossTelegraph")));
	}

	void SpawnBossEnemy()
	{
		if (!DataSubsystem || !VisualSubsystem)
		{
			return;
		}
		const FName BossID = PendingBossID.IsNone()
			? (GetCurrentStageDefinition() && !GetCurrentStageDefinition()->BossID.IsNone()
				? GetCurrentStageDefinition()->BossID
				: (GetWaveDefinition() ? GetWaveDefinition()->BossID : NAME_None))
			: PendingBossID;
		const FT66MiniBossDefinition* Boss = DataSubsystem->FindBoss(BossID);
		if (!Boss)
		{
			bBossSpawnedForWave = true;
			PostBossDelayRemaining = GetRuntimeTuning(TEXT("PostBossDelaySeconds"), 2.f);
			return;
		}
		const FT66MiniDifficultyDefinition* Difficulty = GetDifficultyDefinition();
		const float DifficultyScalar = Difficulty ? Difficulty->BossScalar : 1.f;
		const float WaveScalar = GetRuntimeTuning(TEXT("BossWaveScalarBase"), 1.f) + (WaveIndex * GetRuntimeTuning(TEXT("BossWaveScalarPerWave"), 0.14f));

		FT66MiniWidgetEnemy Enemy;
		Enemy.EntityID = AllocateEntityID();
		Enemy.EnemyID = Boss->BossID;
		Enemy.DisplayName = Boss->DisplayName;
		Enemy.VisualID = Boss->VisualID;
		Enemy.bBoss = true;
		Enemy.Position = PendingBossSpawnLocation2D.IsNearlyZero() ? T66MiniClampBoard(FVector2D(MiniArenaHalfExtentX * GetRuntimeTuning(TEXT("BossFallbackOffsetScalar"), 0.70f), 0.f)) : PendingBossSpawnLocation2D;
		Enemy.CurrentHealth = Boss->MaxHealth * DifficultyScalar * WaveScalar;
		Enemy.MaxHealth = Enemy.CurrentHealth;
		Enemy.MoveSpeed = Boss->MoveSpeed;
		Enemy.TouchDamage = Boss->TouchDamage * DifficultyScalar;
		Enemy.MaterialDrop = FMath::RoundToInt(Boss->MaterialReward * DifficultyScalar);
		Enemy.ExperienceDrop = Boss->ExperienceReward * DifficultyScalar;
		Enemy.BehaviorProfile = Boss->BehaviorProfile;
		Enemy.Family = Boss->Family;
		Enemy.FireInterval = Boss->FireIntervalSeconds / FMath::Max(GetRuntimeTuning(TEXT("BossFireIntervalScalarMin"), 0.85f), DifficultyScalar);
		Enemy.ProjectileSpeed = Boss->ProjectileSpeed;
		Enemy.ProjectileDamage = Boss->ProjectileDamage * DifficultyScalar;
		Enemy.PreferredRange = GetRuntimeTuning(TEXT("BossPreferredRange"), 960.f);
		Enemy.Texture = VisualSubsystem->LoadBossTexture(Boss->VisualID);
		Enemies.Add(Enemy);

		BossTelegraphRemaining = 0.f;
		PendingBossID = NAME_None;
		PendingBossSpawnLocation2D = FVector2D::ZeroVector;
		AddVfx(Enemy.Position, 360.f, FLinearColor(1.0f, 0.58f, 0.20f, 0.36f), 1.f, Enemy.Texture, FName(TEXT("AnimatedStyle.Mini.BossSpawn")));
	}

	void SpawnRandomInteractable()
	{
		if (!DataSubsystem || !VisualSubsystem || DataSubsystem->GetInteractables().Num() == 0)
		{
			return;
		}
		float TotalWeight = 0.f;
		for (const FT66MiniInteractableDefinition& Definition : DataSubsystem->GetInteractables())
		{
			TotalWeight += FMath::Max(0.1f, Definition.SpawnWeight);
		}
		float Pick = FMath::FRandRange(0.f, TotalWeight);
		const FT66MiniInteractableDefinition* Chosen = &DataSubsystem->GetInteractables()[0];
		for (const FT66MiniInteractableDefinition& Definition : DataSubsystem->GetInteractables())
		{
			Pick -= FMath::Max(0.1f, Definition.SpawnWeight);
			if (Pick <= 0.f)
			{
				Chosen = &Definition;
				break;
			}
		}
		FT66MiniWidgetInteractable Interactable;
		Interactable.EntityID = AllocateEntityID();
		Interactable.Position = FVector2D(
			FMath::FRandRange(-MiniArenaHalfExtentX * 0.65f, MiniArenaHalfExtentX * 0.65f),
			FMath::FRandRange(-MiniArenaHalfExtentY * 0.65f, MiniArenaHalfExtentY * 0.65f));
		Interactable.Type = Chosen->Type;
		Interactable.VisualID = Chosen->VisualID;
		Interactable.LifetimeRemaining = Chosen->LifetimeSeconds;
		Interactable.MaterialReward = Chosen->MaterialReward;
		Interactable.GoldReward = Chosen->GoldReward;
		Interactable.ExperienceReward = Chosen->ExperienceReward;
		Interactable.HealAmount = Chosen->HealAmount;
		Interactable.bRequiresManualInteract = Chosen->bRequiresManualInteract;
		Interactable.Texture = VisualSubsystem->LoadInteractableTexture(Chosen->VisualID);
		Interactables.Add(Interactable);
	}

	void SpawnRandomTrap()
	{
		const float Angle = FMath::FRandRange(0.f, UE_TWO_PI);
		const FVector2D Direction(FMath::Cos(Angle), FMath::Sin(Angle));
		FT66MiniWidgetTrap Trap;
		Trap.EntityID = AllocateEntityID();
		Trap.Position = T66MiniClampBoard(PlayerPosition + Direction * FMath::FRandRange(GetRuntimeTuning(TEXT("TrapSpawnMinDistance"), 360.f), GetRuntimeTuning(TEXT("TrapSpawnMaxDistance"), 980.f)));
		Trap.Radius = GetRuntimeTuning(TEXT("TrapRadiusBase"), 180.f)
			+ (WaveIndex * GetRuntimeTuning(TEXT("TrapRadiusPerWave"), 24.f))
			+ FMath::FRandRange(0.f, GetRuntimeTuning(TEXT("TrapRadiusRandomMax"), 80.f));
		Trap.DamagePerPulse = GetRuntimeTuning(TEXT("TrapDamageBase"), 7.f) + (WaveIndex * GetRuntimeTuning(TEXT("TrapDamagePerWave"), 1.3f));
		Trap.WarmupRemaining = GetRuntimeTuning(TEXT("TrapWarmupBase"), 0.95f) + FMath::FRandRange(0.f, GetRuntimeTuning(TEXT("TrapWarmupRandomMax"), 0.35f));
		Trap.ActiveRemaining = GetRuntimeTuning(TEXT("TrapActiveBase"), 3.8f) + (WaveIndex * GetRuntimeTuning(TEXT("TrapActivePerWave"), 0.28f));
		Trap.LifetimeRemaining = Trap.WarmupRemaining + Trap.ActiveRemaining + 0.1f;
		Trap.PulseInterval = FMath::Max(GetRuntimeTuning(TEXT("TrapPulseIntervalMin"), 0.28f), GetRuntimeTuning(TEXT("TrapPulseIntervalBase"), 0.72f) - (WaveIndex * GetRuntimeTuning(TEXT("TrapPulseIntervalPerWave"), 0.04f)));
		Trap.TrapVariant = FMath::RandRange(0, GetRuntimeTuningInt(TEXT("TrapVariantMax"), 2));
		Traps.Add(Trap);
	}

	void SpawnPickup(const FVector2D& Position, const int32 Material, const float XP, const float HealAmount, const float Lifetime, const FName GrantedItemID, const FString& VisualID)
	{
		FT66MiniWidgetPickup Pickup;
		Pickup.EntityID = AllocateEntityID();
		Pickup.Position = Position;
		Pickup.MaterialValue = Material;
		Pickup.ExperienceValue = XP;
		Pickup.HealValue = HealAmount;
		Pickup.LifetimeRemaining = Lifetime;
		Pickup.GrantedItemID = GrantedItemID;
		Pickup.VisualID = VisualID;
		Pickup.Texture = VisualSubsystem && !VisualID.IsEmpty() ? VisualSubsystem->LoadInteractableTexture(VisualID) : nullptr;
		Pickups.Add(Pickup);
	}

	void CollectPickup(const int32 PickupIndex)
	{
		if (!Pickups.IsValidIndex(PickupIndex))
		{
			return;
		}
		const FT66MiniWidgetPickup Pickup = Pickups[PickupIndex];
		Pickups.RemoveAt(PickupIndex);
		GainMaterials(Pickup.MaterialValue);
		GainExperience(Pickup.ExperienceValue);
		Heal(Pickup.HealValue);
		if (!Pickup.GrantedItemID.IsNone() && !OwnedItemIDs.Contains(Pickup.GrantedItemID))
		{
			OwnedItemIDs.Add(Pickup.GrantedItemID);
			if (const FT66MiniItemDefinition* Item = DataSubsystem ? DataSubsystem->FindItem(Pickup.GrantedItemID) : nullptr)
			{
				ApplyItemDefinition(*Item);
			}
		}
		AddVfx(Pickup.Position, 130.f, FLinearColor(0.46f, 1.0f, 0.48f, 0.28f), 0.38f, Pickup.Texture, FName(TEXT("AnimatedStyle.Mini.PickupCollect")));
	}

	void ConsumeInteractable(const int32 InteractableIndex)
	{
		if (!Interactables.IsValidIndex(InteractableIndex))
		{
			return;
		}
		const FT66MiniWidgetInteractable Interactable = Interactables[InteractableIndex];
		Interactables.RemoveAt(InteractableIndex);

		switch (Interactable.Type)
		{
		case ET66MiniInteractableType::Fountain:
			Heal(FMath::Max(Interactable.HealAmount, MaxHealth * 0.35f));
			break;
		case ET66MiniInteractableType::QuickReviveMachine:
			bQuickReviveReady = true;
			break;
		case ET66MiniInteractableType::LootCrate:
			GrantRandomItem(Interactable.Position);
			break;
		case ET66MiniInteractableType::TreasureChest:
		default:
			break;
		}
		GainMaterials(Interactable.MaterialReward);
		GainGold(Interactable.GoldReward);
		GainExperience(Interactable.ExperienceReward);
		Heal(Interactable.HealAmount);
		AddVfx(Interactable.Position, 210.f, FLinearColor(1.0f, 0.78f, 0.28f, 0.32f), 0.55f, Interactable.Texture, FName(TEXT("AnimatedStyle.Mini.Interactable")));
	}

	void GrantRandomItem(const FVector2D& SourcePosition)
	{
		if (!DataSubsystem || DataSubsystem->GetItems().Num() == 0)
		{
			return;
		}
		const FT66MiniItemDefinition& Item = DataSubsystem->GetItems()[FMath::RandRange(0, DataSubsystem->GetItems().Num() - 1)];
		if (!OwnedItemIDs.Contains(Item.ItemID))
		{
			OwnedItemIDs.Add(Item.ItemID);
			ApplyItemDefinition(Item);
			AddCombatText(SourcePosition, 0.f, FLinearColor(0.66f, 1.0f, 0.70f, 1.f), FString::Printf(TEXT("+%s"), *T66MiniReadableName(Item.ItemID)));
		}
	}

	void RecordClearedMiniStageProgression()
	{
		if (!ActiveRun || !SaveSubsystem || !DataSubsystem)
		{
			return;
		}

		const FT66MiniStageDefinition* Stage = GetCurrentStageDefinition();
		const FT66MiniDifficultyDefinition* Difficulty = GetDifficultyDefinition();
		const int32 ChadCoupons = Stage && Stage->ClearChadCoupons > 0
			? Stage->ClearChadCoupons
			: (Difficulty ? FMath::Max(0, Difficulty->StageClearChadCoupons) : 0);
		if (Stage)
		{
			ActiveRun->CurrentStageID = Stage->StageID;
			ActiveRun->StageIndex = Stage->StageIndex;
			Gold += FMath::Max(0, Stage->ClearGoldReward);
			Materials += FMath::Max(0, Stage->ClearMaterialReward);
			ActiveRun->Gold = Gold;
			ActiveRun->Materials = Materials;
		}
		else
		{
			ActiveRun->StageIndex = FMath::Max(1, WaveIndex);
		}

		SaveSubsystem->RecordClearedMiniStage(ActiveRun->CompanionID, DataSubsystem);
		if (UT66AchievementsSubsystem* Achievements = GameInstance ? GameInstance->GetSubsystem<UT66AchievementsSubsystem>() : nullptr)
		{
			Achievements->AddChadCoupons(ChadCoupons);
		}
	}

	void ReturnToShopIntermission()
	{
		if (!ActiveRun || !SaveSubsystem || !FrontendState || !DataSubsystem)
		{
			return;
		}
		SyncActiveRun(false);
		ActiveRun->bPendingShopIntermission = true;
		ActiveRun->bHasMidWaveSnapshot = false;
		ActiveRun->bHasPlayerLocation = false;
		ActiveRun->PlayerLocation = FVector::ZeroVector;
		ActiveRun->bBossSpawnedForWave = false;
		ActiveRun->BossTelegraphRemaining = 0.f;
		ActiveRun->PendingBossID = NAME_None;
		ActiveRun->PendingBossSpawnLocation = FVector::ZeroVector;
		ActiveRun->EnemySpawnAccumulator = 0.f;
		ActiveRun->InteractableSpawnAccumulator = 0.f;
		ActiveRun->TrapSpawnAccumulator = 0.f;
		ActiveRun->PostBossDelayRemaining = 0.f;
		ActiveRun->EnemySnapshots.Reset();
		ActiveRun->PickupSnapshots.Reset();
		ActiveRun->InteractableSnapshots.Reset();
		ActiveRun->TrapSnapshots.Reset();
		FrontendState->SeedFromRunSave(ActiveRun);
		FrontendState->EnterIntermissionFlow(DataSubsystem);
		FrontendState->WriteIntermissionStateToRunSave(ActiveRun);
		if (UT66MiniCircusSubsystem* Circus = GameInstance ? GameInstance->GetSubsystem<UT66MiniCircusSubsystem>() : nullptr)
		{
			Circus->WriteToRunSave(ActiveRun);
		}
		SaveSubsystem->SaveRunToSlot(RunState ? RunState->GetActiveSaveSlot() : INDEX_NONE, ActiveRun);
		PendingTransition = ET66MiniWidgetTransition::Shop;
	}

	void FinalizeRun(const bool bVictory, const FString& ResultLabel)
	{
		if (bRunFinalized || !ActiveRun || !FrontendState || !SaveSubsystem || !DataSubsystem || !RunState)
		{
			return;
		}
		bRunFinalized = true;
		SyncActiveRun(false);

		FT66MiniRunSummary Summary;
		Summary.bHasSummary = true;
		Summary.bWasVictory = bVictory;
		Summary.HeroID = ActiveRun->HeroID;
		Summary.CompanionID = ActiveRun->CompanionID;
		Summary.DifficultyID = ActiveRun->DifficultyID;
		Summary.WaveReached = FMath::Max(1, WaveIndex);
		Summary.GoldCollected = Gold;
		Summary.MaterialsCollected = Materials;
		Summary.OwnedItemCount = OwnedItemIDs.Num();
		Summary.EquippedIdolCount = EquippedIdolIDs.Num();
		Summary.RunSeconds = ActiveRun->TotalRunSeconds;
		Summary.ResultLabel = ResultLabel;
		Summary.LastUpdatedUtc = FDateTime::UtcNow().ToIso8601();
		if (const FT66MiniHeroDefinition* Hero = DataSubsystem->FindHero(Summary.HeroID))
		{
			Summary.HeroDisplayName = Hero->DisplayName;
		}
		else
		{
			Summary.HeroDisplayName = Summary.HeroID.ToString();
		}
		if (const FT66MiniCompanionDefinition* Companion = DataSubsystem->FindCompanion(Summary.CompanionID))
		{
			Summary.CompanionDisplayName = Companion->DisplayName;
		}
		else
		{
			Summary.CompanionDisplayName = Summary.CompanionID.ToString();
		}
		if (const FT66MiniDifficultyDefinition* Difficulty = DataSubsystem->FindDifficulty(Summary.DifficultyID))
		{
			Summary.DifficultyDisplayName = Difficulty->DisplayName;
		}
		else
		{
			Summary.DifficultyDisplayName = Summary.DifficultyID.ToString();
		}

		FrontendState->SetLastRunSummary(Summary);
		FrontendState->ExitIntermissionFlow();
		SaveSubsystem->RecordRunSummary(Summary, DataSubsystem);
		if (UT66MiniLeaderboardSubsystem* MiniLeaderboard = GameInstance ? GameInstance->GetSubsystem<UT66MiniLeaderboardSubsystem>() : nullptr)
		{
			MiniLeaderboard->SubmitScore(Summary.DifficultyID, Summary.MaterialsCollected);
		}
		if (UT66BackendSubsystem* Backend = GameInstance ? GameInstance->GetSubsystem<UT66BackendSubsystem>() : nullptr)
		{
			const UT66SteamHelper* SteamHelper = GameInstance ? GameInstance->GetSubsystem<UT66SteamHelper>() : nullptr;
			const bool bDailyRun = FrontendState->IsDailyRun();
			Backend->SubmitMinigameScore(
				SteamHelper ? SteamHelper->GetLocalDisplayName() : FString(TEXT("Player")),
				TEXT("mini"),
				bDailyRun ? TEXT("daily") : TEXT("alltime"),
				Summary.DifficultyID.ToString().ToLower(),
				FMath::Max(0, Summary.MaterialsCollected),
				bDailyRun ? FrontendState->GetDailyChallengeId() : FString(),
				bDailyRun ? FrontendState->GetDailySeed() : 0);
		}
		if (RunState->GetActiveSaveSlot() != INDEX_NONE)
		{
			SaveSubsystem->DeleteRunFromSlot(RunState->GetActiveSaveSlot());
		}
		if (Owner.IsValid())
		{
			Owner->ReportWidgetGameResult(bVictory, FMath::Max(0, Summary.MaterialsCollected));
		}
		RunState->ResetActiveRun();
		PendingTransition = ET66MiniWidgetTransition::Summary;
	}

	void SyncActiveRun(const bool bMarkMidWave)
	{
		if (!ActiveRun)
		{
			return;
		}
		ActiveRun->HeroID = HeroID;
		ActiveRun->CompanionID = CompanionID;
		ActiveRun->DifficultyID = DifficultyID;
		ActiveRun->WaveIndex = WaveIndex;
		ActiveRun->HeroLevel = HeroLevel;
		ActiveRun->CurrentHealth = CurrentHealth;
		ActiveRun->MaxHealth = MaxHealth;
		ActiveRun->Materials = Materials;
		ActiveRun->Gold = Gold;
		ActiveRun->Experience = Experience;
		ActiveRun->UltimateCooldownRemaining = UltimateCooldownRemaining;
		ActiveRun->bEnduranceCheatUsedThisWave = bEnduranceCheatUsedThisWave;
		ActiveRun->bQuickReviveReady = bQuickReviveReady;
		ActiveRun->OwnedItemIDs = OwnedItemIDs;
		ActiveRun->EquippedIdolIDs = EquippedIdolIDs;
		ActiveRun->PlayerLocation = T66MiniToVector(PlayerPosition);
		ActiveRun->bHasPlayerLocation = true;
		ActiveRun->bHasMidWaveSnapshot = bMarkMidWave || ActiveRun->bHasMidWaveSnapshot;
		ActiveRun->WaveSecondsRemaining = WaveSecondsRemaining;
		ActiveRun->bBossSpawnedForWave = bBossSpawnedForWave;
		ActiveRun->BossTelegraphRemaining = BossTelegraphRemaining;
		ActiveRun->PendingBossID = PendingBossID;
		ActiveRun->PendingBossSpawnLocation = T66MiniToVector(PendingBossSpawnLocation2D);
		ActiveRun->EnemySpawnAccumulator = EnemySpawnAccumulator;
		ActiveRun->InteractableSpawnAccumulator = InteractableSpawnAccumulator;
		ActiveRun->TrapSpawnAccumulator = TrapSpawnAccumulator;
		ActiveRun->PostBossDelayRemaining = PostBossDelayRemaining;
		if (const FT66MiniStageDefinition* Stage = GetCurrentStageDefinition())
		{
			ActiveRun->CurrentStageID = Stage->StageID;
			ActiveRun->StageIndex = Stage->StageIndex;
		}
		else
		{
			ActiveRun->CurrentStageID = NAME_None;
			ActiveRun->StageIndex = WaveIndex;
		}
	}

	void PersistActiveRunSnapshot(const bool bMarkMidWave)
	{
		if (!ActiveRun || !SaveSubsystem || !RunState || RunState->GetActiveSaveSlot() == INDEX_NONE)
		{
			return;
		}
		SyncActiveRun(bMarkMidWave);
		ActiveRun->EnemySnapshots.Reset();
		ActiveRun->PickupSnapshots.Reset();
		ActiveRun->InteractableSnapshots.Reset();
		ActiveRun->TrapSnapshots.Reset();
		for (const FT66MiniWidgetEnemy& Enemy : Enemies)
		{
			if (Enemy.CurrentHealth <= 0.f)
			{
				continue;
			}
			FT66MiniEnemySnapshot& Snapshot = ActiveRun->EnemySnapshots.AddDefaulted_GetRef();
			Snapshot.EnemyID = Enemy.EnemyID;
			Snapshot.bIsBoss = Enemy.bBoss;
			Snapshot.Location = T66MiniToVector(Enemy.Position);
			Snapshot.CurrentHealth = Enemy.CurrentHealth;
			Snapshot.MaxHealth = Enemy.MaxHealth;
			Snapshot.MoveSpeed = Enemy.MoveSpeed;
			Snapshot.TouchDamage = Enemy.TouchDamage;
			Snapshot.MaterialDrop = Enemy.MaterialDrop;
			Snapshot.ExperienceDrop = Enemy.ExperienceDrop;
		}
		for (const FT66MiniWidgetPickup& Pickup : Pickups)
		{
			FT66MiniPickupSnapshot& Snapshot = ActiveRun->PickupSnapshots.AddDefaulted_GetRef();
			Snapshot.Location = T66MiniToVector(Pickup.Position);
			Snapshot.VisualID = Pickup.VisualID;
			Snapshot.MaterialValue = Pickup.MaterialValue;
			Snapshot.ExperienceValue = Pickup.ExperienceValue;
			Snapshot.HealValue = Pickup.HealValue;
			Snapshot.LifetimeRemaining = Pickup.LifetimeRemaining;
			Snapshot.GrantedItemID = Pickup.GrantedItemID;
		}
		for (const FT66MiniWidgetInteractable& Interactable : Interactables)
		{
			FT66MiniInteractableSnapshot& Snapshot = ActiveRun->InteractableSnapshots.AddDefaulted_GetRef();
			Snapshot.Location = T66MiniToVector(Interactable.Position);
			Snapshot.InteractableType = static_cast<uint8>(Interactable.Type);
			Snapshot.VisualID = Interactable.VisualID;
			Snapshot.LifetimeRemaining = Interactable.LifetimeRemaining;
		}
		for (const FT66MiniWidgetTrap& Trap : Traps)
		{
			FT66MiniTrapSnapshot& Snapshot = ActiveRun->TrapSnapshots.AddDefaulted_GetRef();
			Snapshot.Location = T66MiniToVector(Trap.Position);
			Snapshot.Radius = Trap.Radius;
			Snapshot.DamagePerPulse = Trap.DamagePerPulse;
			Snapshot.PulseInterval = Trap.PulseInterval;
			Snapshot.WarmupRemaining = Trap.WarmupRemaining;
			Snapshot.ActiveRemaining = Trap.ActiveRemaining;
			Snapshot.LifetimeRemaining = Trap.LifetimeRemaining;
			Snapshot.TrapVariant = Trap.TrapVariant;
		}
		SaveSubsystem->SaveRunToSlot(RunState->GetActiveSaveSlot(), ActiveRun);
	}

	const FT66MiniWidgetEnemy* FindClosestEnemy(const FVector2D& Origin, const int32 IgnoreID, const float MaxRange) const
	{
		const FT66MiniWidgetEnemy* Best = nullptr;
		float BestDistanceSq = FMath::Square(MaxRange);
		for (const FT66MiniWidgetEnemy& Enemy : Enemies)
		{
			if (Enemy.CurrentHealth <= 0.f || Enemy.EntityID == IgnoreID)
			{
				continue;
			}
			const float DistanceSq = FVector2D::DistSquared(Origin, Enemy.Position);
			if (DistanceSq <= BestDistanceSq)
			{
				BestDistanceSq = DistanceSq;
				Best = &Enemy;
			}
		}
		return Best;
	}

	FT66MiniWidgetEnemy* FindEnemyByID(const int32 EntityID)
	{
		for (FT66MiniWidgetEnemy& Enemy : Enemies)
		{
			if (Enemy.EntityID == EntityID && Enemy.CurrentHealth > 0.f)
			{
				return &Enemy;
			}
		}
		return nullptr;
	}

	const FT66MiniWidgetEnemy* FindEnemyByID(const int32 EntityID) const
	{
		for (const FT66MiniWidgetEnemy& Enemy : Enemies)
		{
			if (Enemy.EntityID == EntityID && Enemy.CurrentHealth > 0.f)
			{
				return &Enemy;
			}
		}
		return nullptr;
	}

	int32 FindEnemyIndexByID(const int32 EntityID) const
	{
		for (int32 Index = 0; Index < Enemies.Num(); ++Index)
		{
			if (Enemies[Index].EntityID == EntityID && Enemies[Index].CurrentHealth > 0.f)
			{
				return Index;
			}
		}
		return INDEX_NONE;
	}

	void QueueBurst(const FVector2D& Position, const float Damage, const float Radius, const float Delay, const FLinearColor& Tint, const float DotTickDamage = 0.f, const float DotDuration = 0.f, const float StunDuration = 0.f)
	{
		FQueuedBurst Burst;
		Burst.Position = T66MiniClampBoard(Position);
		Burst.Damage = Damage;
		Burst.Radius = Radius;
		Burst.Delay = FMath::Max(0.f, Delay);
		Burst.Tint = Tint;
		Burst.DotTickDamage = DotTickDamage;
		Burst.DotDuration = DotDuration;
		Burst.StunDuration = StunDuration;
		QueuedBursts.Add(Burst);
		AddVfx(Burst.Position, Radius, Tint, FMath::Max(0.18f, Burst.Delay + 0.18f), nullptr, FName(TEXT("AnimatedStyle.Mini.Telegraph")));
	}

	void AddAreaEffect(const FVector2D& Position, const float Radius, const float TickDamage, const float TickInterval, const float Duration, const FLinearColor& Tint, const float StunDuration)
	{
		FAreaEffect Effect;
		Effect.Position = T66MiniClampBoard(Position);
		Effect.Radius = Radius;
		Effect.TickDamage = TickDamage;
		Effect.TickInterval = FMath::Max(0.05f, TickInterval);
		Effect.Remaining = Duration;
		Effect.StunDuration = StunDuration;
		AreaEffects.Add(Effect);
		AddVfx(Effect.Position, Radius, Tint, Duration, nullptr, FName(TEXT("AnimatedStyle.Mini.AreaEffect")));
	}

	void AddVfx(const FVector2D& Position, const float Radius, const FLinearColor& Color, const float Lifetime, UTexture2D* Texture = nullptr, const FName Tag = FName(TEXT("AnimatedStyle.Mini.VFX")))
	{
		FT66MiniWidgetVfx Vfx;
		Vfx.Position = T66MiniClampBoard(Position);
		Vfx.Radius = Radius;
		Vfx.Color = Color;
		Vfx.Lifetime = FMath::Max(0.05f, Lifetime);
		Vfx.Texture = Texture;
		Vfx.Tag = Tag;
		VfxEvents.Add(Vfx);
		if (VfxEvents.Num() > 160)
		{
			VfxEvents.RemoveAt(0, VfxEvents.Num() - 160);
		}
	}

	void AddCombatText(const FVector2D& Position, const float Value, const FLinearColor& Color, const FString& Prefix)
	{
		FT66MiniWidgetCombatText Text;
		Text.Position = Position + FVector2D(0.f, -80.f);
		Text.Color = Color;
		Text.Label = Value > 0.f ? FString::Printf(TEXT("%s%.0f"), *Prefix, Value) : Prefix;
		CombatTexts.Add(Text);
		if (CombatTexts.Num() > 80)
		{
			CombatTexts.RemoveAt(0, CombatTexts.Num() - 80);
		}
	}

	int32 AllocateEntityID()
	{
		return NextEntityID++;
	}

	float GetBoardScale(const FVector2D& Size) const
	{
		const float ScaleX = Size.X / (MiniArenaHalfExtentX * 2.f);
		const float ScaleY = Size.Y / (MiniArenaHalfExtentY * 2.f);
		return FMath::Max(0.1f, FMath::Min(ScaleX, ScaleY));
	}

	FVector2D BoardToLocal(const FVector2D& Size, const FVector2D& BoardPosition) const
	{
		const float Scale = GetBoardScale(Size);
		const FVector2D Center = Size * 0.5f;
		return Center + FVector2D(BoardPosition.X, -BoardPosition.Y) * Scale;
	}

	const FSlateBrush* GetBrush(UTexture2D* Texture, const FVector2D& Size) const
	{
		if (!Texture)
		{
			return FCoreStyle::Get().GetBrush(TEXT("WhiteBrush"));
		}
		TSharedPtr<FSlateBrush>* Existing = BrushCache.Find(Texture);
		if (!Existing)
		{
			TSharedPtr<FSlateBrush> Brush = MakeShared<FSlateBrush>();
			Brush->DrawAs = ESlateBrushDrawType::Image;
			Brush->Tiling = ESlateBrushTileType::NoTile;
			Brush->ImageSize = Size;
			Brush->SetResourceObject(Texture);
			Existing = &BrushCache.Add(Texture, Brush);
		}
		(*Existing)->ImageSize = Size;
		return Existing->Get();
	}

	void DrawWorldDisc(const FGeometry& Geometry, const FVector2D& Size, FSlateWindowElementList& OutDrawElements, const int32 Layer, const ESlateDrawEffect DrawEffects, const FVector2D& Position, const float Radius, const FLinearColor& Color) const
	{
		const float Scale = GetBoardScale(Size);
		const FVector2D Local = BoardToLocal(Size, Position);
		const FVector2D DrawSize(Radius * 2.f * Scale, Radius * 2.f * Scale);
		FSlateDrawElement::MakeBox(
			OutDrawElements,
			Layer,
			T66MiniPaintGeometry(Geometry, Local - DrawSize * 0.5f, DrawSize),
			FCoreStyle::Get().GetBrush(TEXT("WhiteBrush")),
			DrawEffects,
			Color);
	}

	void DrawTextureOrBox(const FGeometry& Geometry, const FVector2D& Size, FSlateWindowElementList& OutDrawElements, const int32 Layer, const ESlateDrawEffect DrawEffects, const FVector2D& Position, const FVector2D& WorldSize, UTexture2D* Texture, const FLinearColor& Color) const
	{
		const float Scale = GetBoardScale(Size);
		const FVector2D DrawSize(FMath::Max(12.f, WorldSize.X * Scale), FMath::Max(12.f, WorldSize.Y * Scale));
		const FVector2D Local = BoardToLocal(Size, Position);
		FSlateDrawElement::MakeBox(
			OutDrawElements,
			Layer,
			T66MiniPaintGeometry(Geometry, Local - DrawSize * 0.5f, DrawSize),
			Texture ? GetBrush(Texture, DrawSize) : FCoreStyle::Get().GetBrush(TEXT("WhiteBrush")),
			DrawEffects,
			Color);
	}

	void DrawTraps(const FGeometry& Geometry, const FVector2D& Size, FSlateWindowElementList& OutDrawElements, int32& Layer, const ESlateDrawEffect DrawEffects) const
	{
		for (const FT66MiniWidgetTrap& Trap : Traps)
		{
			const FLinearColor Color = Trap.WarmupRemaining > 0.f
				? FLinearColor(1.0f, 0.70f, 0.16f, 0.20f)
				: FLinearColor(1.0f, 0.10f, 0.08f, 0.28f);
			DrawWorldDisc(Geometry, Size, OutDrawElements, Layer++, DrawEffects, Trap.Position, Trap.Radius, Color);
		}
		for (const FAreaEffect& Effect : AreaEffects)
		{
			DrawWorldDisc(Geometry, Size, OutDrawElements, Layer++, DrawEffects, Effect.Position, Effect.Radius, FLinearColor(0.40f, 0.78f, 1.0f, 0.16f));
		}
	}

	void DrawVfx(const FGeometry& Geometry, const FVector2D& Size, FSlateWindowElementList& OutDrawElements, int32& Layer, const ESlateDrawEffect DrawEffects, const bool bUnderEntities) const
	{
		for (const FT66MiniWidgetVfx& Vfx : VfxEvents)
		{
			const float Alpha = 1.f - FMath::Clamp(Vfx.Age / FMath::Max(0.01f, Vfx.Lifetime), 0.f, 1.f);
			const float Radius = Vfx.Radius * (bUnderEntities ? 1.f : (0.55f + (1.f - Alpha) * 0.65f));
			FLinearColor Color = Vfx.Color;
			Color.A *= Alpha;
			DrawWorldDisc(Geometry, Size, OutDrawElements, Layer++, DrawEffects, Vfx.Position, Radius, Color);
			if (!bUnderEntities && Vfx.Texture)
			{
				DrawTextureOrBox(Geometry, Size, OutDrawElements, Layer++, DrawEffects, Vfx.Position, FVector2D(Radius, Radius), Vfx.Texture, FLinearColor(1.f, 1.f, 1.f, Alpha));
			}
		}
	}

	void DrawPickupsAndInteractables(const FGeometry& Geometry, const FVector2D& Size, FSlateWindowElementList& OutDrawElements, int32& Layer, const ESlateDrawEffect DrawEffects) const
	{
		for (const FT66MiniWidgetPickup& Pickup : Pickups)
		{
			const float Bob = FMath::Sin(Pickup.Age * 5.f) * 18.f;
			DrawTextureOrBox(Geometry, Size, OutDrawElements, Layer++, DrawEffects, Pickup.Position + FVector2D(0.f, Bob), FVector2D(92.f, 92.f), Pickup.Texture, Pickup.Texture ? FLinearColor::White : FLinearColor(0.45f, 1.0f, 0.42f, 0.92f));
		}
		for (const FT66MiniWidgetInteractable& Interactable : Interactables)
		{
			const float Bob = FMath::Sin(Interactable.Age * 3.f) * 12.f;
			DrawTextureOrBox(Geometry, Size, OutDrawElements, Layer++, DrawEffects, Interactable.Position + FVector2D(0.f, Bob), FVector2D(150.f, 150.f), Interactable.Texture, Interactable.Texture ? FLinearColor::White : FLinearColor(1.0f, 0.74f, 0.30f, 0.95f));
		}
	}

	void DrawProjectiles(const FGeometry& Geometry, const FVector2D& Size, FSlateWindowElementList& OutDrawElements, int32& Layer, const ESlateDrawEffect DrawEffects) const
	{
		for (const FT66MiniWidgetProjectile& Projectile : Projectiles)
		{
			DrawTextureOrBox(Geometry, Size, OutDrawElements, Layer++, DrawEffects, Projectile.Position, FVector2D(Projectile.Radius, Projectile.Radius), Projectile.Texture, Projectile.bPlayerOwned ? FLinearColor(1.0f, 0.86f, 0.28f, 0.95f) : FLinearColor(1.0f, 0.18f, 0.12f, 0.95f));
		}
	}

	void DrawCompanion(const FGeometry& Geometry, const FVector2D& Size, FSlateWindowElementList& OutDrawElements, int32& Layer, const ESlateDrawEffect DrawEffects) const
	{
		if (CompanionID.IsNone())
		{
			return;
		}
		DrawTextureOrBox(Geometry, Size, OutDrawElements, Layer++, DrawEffects, CompanionPosition, FVector2D(116.f, 116.f), CompanionTexture, CompanionTexture ? FLinearColor::White : FLinearColor(0.42f, 0.72f, 1.0f, 0.90f));
	}

	void DrawEnemies(const FGeometry& Geometry, const FVector2D& Size, FSlateWindowElementList& OutDrawElements, int32& Layer, const ESlateDrawEffect DrawEffects) const
	{
		TArray<const FT66MiniWidgetEnemy*> Sorted;
		for (const FT66MiniWidgetEnemy& Enemy : Enemies)
		{
			if (Enemy.CurrentHealth > 0.f)
			{
				Sorted.Add(&Enemy);
			}
		}
		Sorted.Sort([](const FT66MiniWidgetEnemy& A, const FT66MiniWidgetEnemy& B)
		{
			return A.Position.Y > B.Position.Y;
		});

		for (const FT66MiniWidgetEnemy* Enemy : Sorted)
		{
			const FVector2D EntitySize = Enemy->bBoss ? FVector2D(220.f, 220.f) : FVector2D(132.f, 132.f);
			DrawWorldDisc(Geometry, Size, OutDrawElements, Layer++, DrawEffects, Enemy->Position + FVector2D(0.f, -48.f), EntitySize.X * 0.36f, FLinearColor(0.f, 0.f, 0.f, 0.22f));
			DrawTextureOrBox(Geometry, Size, OutDrawElements, Layer++, DrawEffects, Enemy->Position, EntitySize, Enemy->Texture, Enemy->Texture ? FLinearColor::White : (Enemy->bBoss ? FLinearColor(0.95f, 0.22f, 0.14f, 1.f) : FLinearColor(0.80f, 0.18f, 0.12f, 1.f)));
			if (Enemy->HitFlashRemaining > 0.f)
			{
				DrawTextureOrBox(Geometry, Size, OutDrawElements, Layer++, DrawEffects, Enemy->Position, EntitySize * 1.08f, nullptr, FLinearColor(1.0f, 0.84f, 0.42f, 0.28f));
			}
			const float HealthAlpha = FMath::Clamp(Enemy->CurrentHealth / FMath::Max(1.f, Enemy->MaxHealth), 0.f, 1.f);
			const FVector2D BarPos = BoardToLocal(Size, Enemy->Position + FVector2D(0.f, Enemy->bBoss ? 150.f : 92.f));
			const FVector2D BarSize(Enemy->bBoss ? 180.f : 100.f, 9.f);
			FSlateDrawElement::MakeBox(OutDrawElements, Layer++, T66MiniPaintGeometry(Geometry, BarPos - BarSize * 0.5f, BarSize), FCoreStyle::Get().GetBrush(TEXT("WhiteBrush")), DrawEffects, FLinearColor(0.05f, 0.04f, 0.04f, 0.84f));
			FSlateDrawElement::MakeBox(OutDrawElements, Layer++, T66MiniPaintGeometry(Geometry, BarPos - BarSize * 0.5f, FVector2D(BarSize.X * HealthAlpha, BarSize.Y)), FCoreStyle::Get().GetBrush(TEXT("WhiteBrush")), DrawEffects, Enemy->bBoss ? FLinearColor(1.0f, 0.28f, 0.18f, 0.95f) : FLinearColor(0.90f, 0.22f, 0.16f, 0.92f));
		}
	}

	void DrawPlayer(const FGeometry& Geometry, const FVector2D& Size, FSlateWindowElementList& OutDrawElements, int32& Layer, const ESlateDrawEffect DrawEffects) const
	{
		DrawWorldDisc(Geometry, Size, OutDrawElements, Layer++, DrawEffects, PlayerPosition + FVector2D(0.f, -56.f), 58.f, FLinearColor(0.f, 0.f, 0.f, 0.26f));
		DrawTextureOrBox(Geometry, Size, OutDrawElements, Layer++, DrawEffects, PlayerPosition, FVector2D(150.f, 150.f), PlayerTexture, PlayerTexture ? FLinearColor::White : FLinearColor(0.24f, 0.74f, 1.0f, 1.f));
	}

	void DrawCombatTexts(const FGeometry& Geometry, const FVector2D& Size, FSlateWindowElementList& OutDrawElements, int32& Layer, const ESlateDrawEffect DrawEffects) const
	{
		const FSlateFontInfo Font = FCoreStyle::GetDefaultFontStyle(TEXT("Bold"), 18);
		for (const FT66MiniWidgetCombatText& Text : CombatTexts)
		{
			FLinearColor Color = Text.Color;
			Color.A *= FMath::Clamp(Text.Remaining / FMath::Max(0.01f, Text.Duration), 0.f, 1.f);
			const FVector2D Local = BoardToLocal(Size, Text.Position);
			FSlateDrawElement::MakeText(
				OutDrawElements,
				Layer++,
				T66MiniPaintGeometry(Geometry, Local, FVector2D(280.f, 24.f)),
				Text.Label,
				Font,
				DrawEffects,
				Color);
		}
	}

	struct FQueuedBurst
	{
		FVector2D Position = FVector2D::ZeroVector;
		float Damage = 0.f;
		float Radius = 0.f;
		float Delay = 0.f;
		FLinearColor Tint = FLinearColor::White;
		float DotTickDamage = 0.f;
		float DotDuration = 0.f;
		float StunDuration = 0.f;
	};

	struct FAreaEffect
	{
		FVector2D Position = FVector2D::ZeroVector;
		float Radius = 0.f;
		float TickDamage = 0.f;
		float TickInterval = 0.5f;
		float Remaining = 0.f;
		float Accumulator = 0.f;
		float StunDuration = 0.f;
	};

	TWeakObjectPtr<UT66MiniBattleScreen> Owner;
	UGameInstance* GameInstance = nullptr;
	UT66MiniDataSubsystem* DataSubsystem = nullptr;
	UT66MiniVisualSubsystem* VisualSubsystem = nullptr;
	UT66MiniRunStateSubsystem* RunState = nullptr;
	UT66MiniSaveSubsystem* SaveSubsystem = nullptr;
	UT66MiniFrontendStateSubsystem* FrontendState = nullptr;
	UT66MiniRunSaveGame* ActiveRun = nullptr;

	bool bInitialized = false;
	bool bPaused = false;
	bool bRunFinalized = false;
	bool bBossSpawnedForWave = false;
	bool bHasPointerTarget = false;
	bool bEnduranceCheatUsedThisWave = false;
	bool bQuickReviveReady = false;
	bool bEvasiveNextAttackAppliesDot = false;
	int32 NextEntityID = 1;
	int32 WaveIndex = 1;
	int32 HeroLevel = 1;
	int32 Materials = 0;
	int32 Gold = 0;
	int32 PassiveStacks = 0;
	int32 BonusPierceCount = 0;
	int32 BonusBounceCount = 0;
	int32 NextIdolProcIndex = 0;
	float Experience = 0.f;
	float WaveSecondsRemaining = 60.f;
	float EnemySpawnAccumulator = 0.f;
	float InteractableSpawnAccumulator = 0.f;
	float TrapSpawnAccumulator = 0.f;
	float PostBossDelayRemaining = 0.f;
	float BossTelegraphRemaining = 0.f;
	float AutosaveAccumulator = 0.f;
	float TotalElapsed = 0.f;
	float MaxHealth = 100.f;
	float CurrentHealth = 100.f;
	float BaseDamageStat = 2.f;
	float BaseAttackSpeedStat = 1.f;
	float BaseArmorStat = 0.f;
	float BaseLuckStat = 0.f;
	float AttackRange = 1100.f;
	float MoveSpeed = 900.f;
	float CritChance = 0.05f;
	float CritDamageMultiplier = 1.5f;
	float PassiveRegenPerSecond = 0.f;
	float EvasionChance = 0.f;
	float LifeStealChance = 0.f;
	float DotDamageBonus = 0.f;
	float AoeRadiusBonus = 0.f;
	float BonusDamageMultiplier = 1.f;
	float TemporaryDamageMultiplier = 1.f;
	float MaterialGainMultiplier = 1.f;
	float GoldGainMultiplier = 1.f;
	float AutoAttackCooldownRemaining = 0.f;
	float UltimateCooldownRemaining = 0.f;
	float UltimateCooldownDuration = 18.f;
	float PassiveBuffRemaining = 0.f;
	float PassiveSecondaryBuffRemaining = 0.f;
	FName HeroID = NAME_None;
	FName CompanionID = NAME_None;
	FName DifficultyID = NAME_None;
	FName PendingBossID = NAME_None;
	FString HeroDisplayName;
	FString CompanionDisplayName;
	FString CompanionVisualID;
	FString Status;
	ET66UltimateType UltimateType = ET66UltimateType::None;
	ET66PassiveType PassiveType = ET66PassiveType::None;
	ET66MiniWidgetTransition PendingTransition = ET66MiniWidgetTransition::None;
	FVector2D PlayerPosition = FVector2D::ZeroVector;
	FVector2D DesiredMovePosition = FVector2D::ZeroVector;
	FVector2D AimPosition = FVector2D(1.f, 0.f);
	FVector2D KeyboardMoveInput = FVector2D::ZeroVector;
	FVector2D CompanionPosition = FVector2D(-120.f, 80.f);
	FVector2D CompanionFollowOffset = FVector2D(-145.f, 110.f);
	FVector2D PendingBossSpawnLocation2D = FVector2D::ZeroVector;
	float CompanionHealingPerSecond = 0.f;
	TArray<FName> OwnedItemIDs;
	TArray<FName> EquippedIdolIDs;
	TArray<FT66MiniWidgetIdol> EquippedIdols;
	TArray<FT66MiniWidgetEnemy> Enemies;
	TArray<FT66MiniWidgetProjectile> Projectiles;
	TArray<FT66MiniWidgetPickup> Pickups;
	TArray<FT66MiniWidgetInteractable> Interactables;
	TArray<FT66MiniWidgetTrap> Traps;
	TArray<FT66MiniWidgetVfx> VfxEvents;
	TArray<FT66MiniWidgetCombatText> CombatTexts;
	TArray<FQueuedBurst> QueuedBursts;
	TArray<FAreaEffect> AreaEffects;
	UTexture2D* PlayerTexture = nullptr;
	UTexture2D* PlayerProjectileTexture = nullptr;
	UTexture2D* CompanionTexture = nullptr;
	UTexture2D* BackgroundTexture = nullptr;
	mutable TMap<UTexture2D*, TSharedPtr<FSlateBrush>> BrushCache;
};

class ST66MiniBattleBoardWidget : public SLeafWidget
{
public:
	SLATE_BEGIN_ARGS(ST66MiniBattleBoardWidget) {}
		SLATE_ARGUMENT(TWeakObjectPtr<UT66MiniBattleScreen>, OwnerScreen)
	SLATE_END_ARGS()

	void Construct(const FArguments& InArgs)
	{
		OwnerScreen = InArgs._OwnerScreen;
		RegisterActiveTimer(0.f, FWidgetActiveTimerDelegate::CreateSP(this, &ST66MiniBattleBoardWidget::HandleActiveTimer));
	}

	virtual FVector2D ComputeDesiredSize(float) const override
	{
		return FVector2D(1280.f, 900.f);
	}

	virtual bool SupportsKeyboardFocus() const override
	{
		return true;
	}

	virtual FReply OnMouseMove(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent) override
	{
		if (UT66MiniBattleScreen* Screen = OwnerScreen.Get())
		{
			if (const FT66MiniBattleSimulation* Simulation = Screen->GetBattleSimulationForSlate())
			{
				Screen->SetPointerBoardPosition(Simulation->LocalToBoard(MyGeometry, MyGeometry.AbsoluteToLocal(MouseEvent.GetScreenSpacePosition())));
			}
		}
		return FReply::Handled();
	}

	virtual FReply OnMouseButtonDown(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent) override
	{
		if (UT66MiniBattleScreen* Screen = OwnerScreen.Get())
		{
			const FT66MiniBattleSimulation* Simulation = Screen->GetBattleSimulationForSlate();
			const FVector2D BoardPosition = Simulation ? Simulation->LocalToBoard(MyGeometry, MyGeometry.AbsoluteToLocal(MouseEvent.GetScreenSpacePosition())) : FVector2D::ZeroVector;
			if (MouseEvent.GetEffectingButton() == EKeys::RightMouseButton)
			{
				Screen->RequestUltimateAtBoardPosition(BoardPosition);
			}
			else
			{
				Screen->SetPointerBoardPosition(BoardPosition);
				Screen->RequestInteract();
			}
		}
		return FReply::Handled().SetUserFocus(AsShared(), EFocusCause::Mouse);
	}

	virtual FReply OnKeyDown(const FGeometry& MyGeometry, const FKeyEvent& InKeyEvent) override
	{
		if (UT66MiniBattleScreen* Screen = OwnerScreen.Get())
		{
			if (InKeyEvent.GetKey() == EKeys::Escape)
			{
				Screen->ToggleBattlePause();
				return FReply::Handled();
			}
			if (InKeyEvent.GetKey() == EKeys::E)
			{
				Screen->RequestInteract();
				return FReply::Handled();
			}
			if (InKeyEvent.GetKey() == EKeys::SpaceBar)
			{
				const FT66MiniBattleSimulation* Simulation = Screen->GetBattleSimulationForSlate();
				Screen->RequestUltimateAtBoardPosition(Simulation ? Simulation->LocalToBoard(MyGeometry, MyGeometry.GetLocalSize() * 0.5f) : FVector2D::ZeroVector);
				return FReply::Handled();
			}
			UpdateMoveInput(InKeyEvent.GetKey(), true);
			Screen->SetKeyboardMoveInput(CurrentMoveInput);
		}
		return FReply::Handled();
	}

	virtual FReply OnKeyUp(const FGeometry& MyGeometry, const FKeyEvent& InKeyEvent) override
	{
		if (UT66MiniBattleScreen* Screen = OwnerScreen.Get())
		{
			UpdateMoveInput(InKeyEvent.GetKey(), false);
			Screen->SetKeyboardMoveInput(CurrentMoveInput);
		}
		return FReply::Handled();
	}

	virtual int32 OnPaint(const FPaintArgs& Args, const FGeometry& AllottedGeometry, const FSlateRect& MyCullingRect, FSlateWindowElementList& OutDrawElements, int32 LayerId, const FWidgetStyle& InWidgetStyle, bool bParentEnabled) const override
	{
		if (const UT66MiniBattleScreen* Screen = OwnerScreen.Get())
		{
			if (const FT66MiniBattleSimulation* Simulation = Screen->GetBattleSimulationForSlate())
			{
				int32 MutableLayer = LayerId;
				Simulation->Draw(Args, AllottedGeometry, MyCullingRect, OutDrawElements, MutableLayer, InWidgetStyle, bParentEnabled);
				return MutableLayer;
			}
		}
		return LayerId;
	}

private:
	EActiveTimerReturnType HandleActiveTimer(double, float DeltaTime)
	{
		if (UT66MiniBattleScreen* Screen = OwnerScreen.Get())
		{
			Screen->TickWidgetBattle(DeltaTime);
			Screen->HandleBattleTransitionFromBoard();
			Invalidate(EInvalidateWidgetReason::Paint);
			return EActiveTimerReturnType::Continue;
		}
		return EActiveTimerReturnType::Stop;
	}

	void UpdateMoveInput(const FKey& Key, const bool bPressed)
	{
		if (Key == EKeys::W || Key == EKeys::Up)
		{
			bMoveUp = bPressed;
		}
		else if (Key == EKeys::S || Key == EKeys::Down)
		{
			bMoveDown = bPressed;
		}
		else if (Key == EKeys::A || Key == EKeys::Left)
		{
			bMoveLeft = bPressed;
		}
		else if (Key == EKeys::D || Key == EKeys::Right)
		{
			bMoveRight = bPressed;
		}

		CurrentMoveInput = FVector2D((bMoveRight ? 1.f : 0.f) - (bMoveLeft ? 1.f : 0.f), (bMoveUp ? 1.f : 0.f) - (bMoveDown ? 1.f : 0.f));
	}

	TWeakObjectPtr<UT66MiniBattleScreen> OwnerScreen;
	bool bMoveUp = false;
	bool bMoveDown = false;
	bool bMoveLeft = false;
	bool bMoveRight = false;
	FVector2D CurrentMoveInput = FVector2D::ZeroVector;
};

UT66MiniBattleScreen::UT66MiniBattleScreen(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	ScreenType = ET66ScreenType::MiniBattle;
	bIsModal = false;
}

void UT66MiniBattleScreen::ActivateWidgetGame(const FT66WidgetGameHostContext& HostContext)
{
	WidgetGameHostContext = HostContext;
}

void UT66MiniBattleScreen::DeactivateWidgetGame()
{
	WidgetGameHostContext = FT66WidgetGameHostContext();
}

void UT66MiniBattleScreen::PauseWidgetGame()
{
}

void UT66MiniBattleScreen::ResumeWidgetGame()
{
}

void UT66MiniBattleScreen::RequestWidgetGameExit()
{
	if (WidgetGameHostContext.ReturnNavigationCallback)
	{
		WidgetGameHostContext.RequestExit(ET66WidgetGameExitReason::PlayerCancelled);
		return;
	}

	if (BattleSimulation)
	{
		BattleSimulation->SaveAndExit();
	}
}

void UT66MiniBattleScreen::SaveWidgetGameState()
{
	if (BattleSimulation)
	{
		BattleSimulation->SaveWidgetGameState();
	}
}

void UT66MiniBattleScreen::LoadWidgetGameState()
{
	ReleaseRetainedSlateState();
	EnsureBattleSimulation();
}

void UT66MiniBattleScreen::FlushWidgetGamePersistence()
{
	SaveWidgetGameState();
}

void UT66MiniBattleScreen::RefreshWidgetGamePersistence()
{
	LoadWidgetGameState();
}

void UT66MiniBattleScreen::BeginDestroy()
{
	ReleaseRetainedSlateState();
	Super::BeginDestroy();
}

void UT66MiniBattleScreen::OnScreenActivated_Implementation()
{
	Super::OnScreenActivated_Implementation();
	EnsureBattleSimulation();
}

void UT66MiniBattleScreen::OnScreenDeactivated_Implementation()
{
	ReleaseRetainedSlateState();
	Super::OnScreenDeactivated_Implementation();
}

void UT66MiniBattleScreen::NativeDestruct()
{
	ReleaseRetainedSlateState();
	Super::NativeDestruct();
}

bool UT66MiniBattleScreen::HandleBackAction()
{
	ToggleBattlePause();
	return true;
}

TSharedRef<SWidget> UT66MiniBattleScreen::BuildSlateUI()
{
	EnsureBattleSimulation();

	return SNew(SOverlay)
		+ SOverlay::Slot()
		[
			SAssignNew(BattleBoardWidget, ST66MiniBattleBoardWidget)
			.OwnerScreen(TWeakObjectPtr<UT66MiniBattleScreen>(this))
		]
		+ SOverlay::Slot()
		.HAlign(HAlign_Fill)
		.VAlign(VAlign_Top)
		.Padding(22.f)
		[
			FT66FlatStyle::MakeFlatSubPanel(
				ET66FlatState::Default,
				FMargin(16.f, 12.f),
				SNew(SHorizontalBox)
				+ SHorizontalBox::Slot()
				.FillWidth(1.f)
				[
					SNew(STextBlock)
					.Font(T66MiniUI::BoldFont(18))
					.ColorAndOpacity(T66MiniUI::Text())
					.Text(T66MiniBattleTextAttribute(this, &UT66MiniBattleScreen::GetWaveText))
				]
				+ SHorizontalBox::Slot()
				.FillWidth(1.5f)
				[
					SNew(STextBlock)
					.Font(T66MiniUI::BoldFont(18))
					.ColorAndOpacity(T66MiniUI::Text())
					.Justification(ETextJustify::Center)
					.Text(T66MiniBattleTextAttribute(this, &UT66MiniBattleScreen::GetHealthText))
				]
				+ SHorizontalBox::Slot()
				.FillWidth(1.4f)
				[
					SNew(STextBlock)
					.Font(T66MiniUI::BoldFont(18))
					.ColorAndOpacity(T66MiniUI::AccentGold())
					.Justification(ETextJustify::Right)
					.Text(T66MiniBattleTextAttribute(this, &UT66MiniBattleScreen::GetResourceText))
				])
		]
		+ SOverlay::Slot()
		.HAlign(HAlign_Fill)
		.VAlign(VAlign_Bottom)
		.Padding(22.f)
		[
			FT66FlatStyle::MakeFlatSubPanel(
				ET66FlatState::Default,
				FMargin(16.f, 10.f),
				SNew(SHorizontalBox)
				+ SHorizontalBox::Slot()
				.FillWidth(1.f)
				[
					SNew(STextBlock)
					.Font(T66MiniUI::BodyFont(15))
					.ColorAndOpacity(T66MiniUI::MutedText())
					.Text(T66MiniBattleTextAttribute(this, &UT66MiniBattleScreen::GetStatusText))
				]
				+ SHorizontalBox::Slot()
				.FillWidth(1.f)
				[
					SNew(STextBlock)
					.Font(T66MiniUI::BoldFont(15))
					.ColorAndOpacity(T66MiniUI::AccentBlue())
					.Justification(ETextJustify::Right)
					.Text(T66MiniBattleTextAttribute(this, &UT66MiniBattleScreen::GetCombatText))
				])
		]
		+ SOverlay::Slot()
		[
			SNew(SBorder)
			.Visibility(T66MiniBattleVisibilityAttribute(this, &UT66MiniBattleScreen::GetPauseOverlayVisibility))
			.BorderImage(T66MiniUI::WhiteBrush())
			.BorderBackgroundColor(FLinearColor(0.0f, 0.0f, 0.0f, 0.70f))
			.HAlign(HAlign_Center)
			.VAlign(VAlign_Center)
			[
				FT66FlatStyle::MakeFlatPanel(
					ET66FlatState::Selected,
					FMargin(28.f),
					SNew(SVerticalBox)
					+ SVerticalBox::Slot()
					.AutoHeight()
					.HAlign(HAlign_Center)
					.Padding(0.f, 0.f, 0.f, 18.f)
					[
						SNew(STextBlock)
						.Font(T66MiniUI::TitleFont(34))
						.ColorAndOpacity(T66MiniUI::Text())
						.Text(NSLOCTEXT("T66Mini.Battle", "Paused", "MINI PAUSED"))
					]
					+ SVerticalBox::Slot()
					.AutoHeight()
					.Padding(0.f, 0.f, 0.f, 12.f)
					[
						T66MiniUI::MakeButton(
							NSLOCTEXT("T66Mini.Battle", "Resume", "RESUME"),
							FOnClicked::CreateUObject(this, &UT66MiniBattleScreen::HandleResumeClicked),
							ET66ButtonType::Primary,
							260.f,
							48.f,
							18)
					]
					+ SVerticalBox::Slot()
					.AutoHeight()
					[
						T66MiniUI::MakeButton(
							NSLOCTEXT("T66Mini.Battle", "SaveExit", "SAVE AND EXIT"),
							FOnClicked::CreateUObject(this, &UT66MiniBattleScreen::HandleSaveAndExitClicked),
							ET66ButtonType::Neutral,
							260.f,
							48.f,
							18)
					])
			]
		];
}

void UT66MiniBattleScreen::EnsureBattleSimulation()
{
	if (!BattleSimulation)
	{
		BattleSimulation = new FT66MiniBattleSimulation();
		BattleSimulation->Initialize(this);
	}
}

void UT66MiniBattleScreen::ReleaseRetainedSlateState()
{
	BattleBoardWidget.Reset();
	if (BattleSimulation)
	{
		delete BattleSimulation;
		BattleSimulation = nullptr;
	}
}

void UT66MiniBattleScreen::ReportWidgetGameResult(const bool bSuccessful, const int32 FinalScore)
{
	FT66WidgetGameResult Result;
	Result.GameID = FName(TEXT("Frontend_Mini"));
	Result.ExitReason = ET66WidgetGameExitReason::Completed;
	Result.FinalScore = FinalScore;
	Result.bHasFinalScore = true;
	Result.bSuccessful = bSuccessful;
	Result.ResultID = Result.GameID;
	WidgetGameHostContext.ReportResult(Result);
}

void UT66MiniBattleScreen::TickWidgetBattle(const float DeltaSeconds)
{
	if (BattleSimulation)
	{
		BattleSimulation->Tick(DeltaSeconds);
	}
}

void UT66MiniBattleScreen::HandleBattleTransitionFromBoard()
{
	if (!BattleSimulation)
	{
		return;
	}
	switch (BattleSimulation->GetPendingTransition())
	{
	case ET66MiniWidgetTransition::Shop:
		NavigateTo(ET66ScreenType::MiniShop);
		break;
	case ET66MiniWidgetTransition::Summary:
		NavigateTo(ET66ScreenType::MiniRunSummary);
		break;
	default:
		break;
	}
}

void UT66MiniBattleScreen::ToggleBattlePause()
{
	if (BattleSimulation)
	{
		BattleSimulation->TogglePause();
	}
}

void UT66MiniBattleScreen::RequestInteract()
{
	if (BattleSimulation)
	{
		BattleSimulation->RequestInteract();
	}
}

void UT66MiniBattleScreen::RequestUltimateAtBoardPosition(const FVector2D& BoardPosition)
{
	if (BattleSimulation)
	{
		BattleSimulation->RequestUltimateAtBoardPosition(BoardPosition);
	}
}

void UT66MiniBattleScreen::SetPointerBoardPosition(const FVector2D& BoardPosition)
{
	if (BattleSimulation)
	{
		BattleSimulation->SetPointerBoardPosition(BoardPosition);
	}
}

void UT66MiniBattleScreen::SetKeyboardMoveInput(const FVector2D& MoveInput)
{
	if (BattleSimulation)
	{
		BattleSimulation->SetKeyboardMoveInput(MoveInput);
	}
}

FReply UT66MiniBattleScreen::HandleResumeClicked()
{
	if (BattleSimulation && BattleSimulation->IsPaused())
	{
		BattleSimulation->TogglePause();
	}
	return FReply::Handled();
}

FReply UT66MiniBattleScreen::HandleSaveAndExitClicked()
{
	if (BattleSimulation)
	{
		BattleSimulation->SaveAndExit();
	}
	return FReply::Handled();
}

FText UT66MiniBattleScreen::GetStatusText() const
{
	return FText::FromString(BattleSimulation ? BattleSimulation->GetStatus() : FString(TEXT("Mini battle initializing.")));
}

FText UT66MiniBattleScreen::GetWaveText() const
{
	return BattleSimulation ? BattleSimulation->GetWaveText() : NSLOCTEXT("T66Mini.Battle", "WaveFallback", "WAVE");
}

FText UT66MiniBattleScreen::GetHealthText() const
{
	return BattleSimulation ? BattleSimulation->GetHealthText() : NSLOCTEXT("T66Mini.Battle", "HealthFallback", "HP");
}

FText UT66MiniBattleScreen::GetResourceText() const
{
	return BattleSimulation ? BattleSimulation->GetResourceText() : NSLOCTEXT("T66Mini.Battle", "ResourceFallback", "MAT 0  GOLD 0");
}

FText UT66MiniBattleScreen::GetCombatText() const
{
	return BattleSimulation ? BattleSimulation->GetCombatText() : NSLOCTEXT("T66Mini.Battle", "CombatFallback", "ULT");
}

EVisibility UT66MiniBattleScreen::GetPauseOverlayVisibility() const
{
	return BattleSimulation && BattleSimulation->IsPaused() ? EVisibility::Visible : EVisibility::Collapsed;
}
