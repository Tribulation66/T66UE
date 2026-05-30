// Copyright Tribulation 66. All Rights Reserved.

#include "Core/T66RunStateSubsystem.h"
#include "Core/RunState/T66RunStateSubsystem_Private.h"

using namespace T66RunStatePrivate;

void UT66RunStateSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);

	ResetHeartSlotTiers();
	SyncMaxHPToHeartTiers();
	CurrentHP = FMath::Clamp(CurrentHP, 0.f, MaxHP);

	Collection.InitializeDependency<UT66IdolManagerSubsystem>();
	Collection.InitializeDependency<UT66WeaponManagerSubsystem>();
	if (UT66IdolManagerSubsystem* IdolManager = GetIdolManager())
	{
		IdolManager->IdolStateChanged.RemoveDynamic(this, &UT66RunStateSubsystem::HandleIdolStateChanged);
		IdolManager->IdolStateChanged.AddDynamic(this, &UT66RunStateSubsystem::HandleIdolStateChanged);
	}
}


void UT66RunStateSubsystem::Deinitialize()
{
	if (UT66IdolManagerSubsystem* IdolManager = GetIdolManager())
	{
		IdolManager->IdolStateChanged.RemoveDynamic(this, &UT66RunStateSubsystem::HandleIdolStateChanged);
	}

	Super::Deinitialize();
}


void UT66RunStateSubsystem::HandleIdolStateChanged()
{
	IdolsChanged.Broadcast();
}


UT66IdolManagerSubsystem* UT66RunStateSubsystem::GetIdolManager() const
{
	UGameInstance* GI = GetGameInstance();
	return GI ? GI->GetSubsystem<UT66IdolManagerSubsystem>() : nullptr;
}

UT66WeaponManagerSubsystem* UT66RunStateSubsystem::GetWeaponManager() const
{
	UGameInstance* GI = GetGameInstance();
	return GI ? GI->GetSubsystem<UT66WeaponManagerSubsystem>() : nullptr;
}


void UT66RunStateSubsystem::TrimLogsIfNeeded()
{
	if (EventLog.Num() > MaxEventLogEntries)
	{
		const int32 RemoveCount = EventLog.Num() - MaxEventLogEntries;
		EventLog.RemoveAt(0, RemoveCount, EAllowShrinking::No);
	}
	if (StructuredEventLog.Num() > MaxStructuredEventLogEntries)
	{
		const int32 RemoveCount = StructuredEventLog.Num() - MaxStructuredEventLogEntries;
		StructuredEventLog.RemoveAt(0, RemoveCount, EAllowShrinking::No);
	}
}


void UT66RunStateSubsystem::AddPowerCrystalsEarnedThisRun(int32 Amount)
{
	if (Amount <= 0) return;
	PowerCrystalsEarnedThisRun = FMath::Clamp(PowerCrystalsEarnedThisRun + Amount, 0, 2000000000);
}


void UT66RunStateSubsystem::MarkPendingPowerCrystalsGrantedToWallet()
{
	PowerCrystalsGrantedToWalletThisRun = FMath::Clamp(PowerCrystalsEarnedThisRun, 0, 2000000000);
}


void UT66RunStateSubsystem::MarkPendingPowerCrystalsSuppressedForWallet()
{
	PowerCrystalsGrantedToWalletThisRun = FMath::Clamp(PowerCrystalsEarnedThisRun, 0, 2000000000);
}


bool UT66RunStateSubsystem::ShouldSuppressPendingPowerCrystalsForWallet()
{
	UGameInstance* GI = GetGameInstance();
	UT66RunIntegritySubsystem* Integrity = GI ? GI->GetSubsystem<UT66RunIntegritySubsystem>() : nullptr;
	if (!Integrity)
	{
		return false;
	}

	Integrity->FinalizeCurrentRun();
	FT66RunIntegrityContext IntegrityContext;
	Integrity->CopyCurrentContextTo(IntegrityContext);
	return !IntegrityContext.ShouldAllowRankedSubmission();
}


void UT66RunStateSubsystem::ActivatePendingSingleUseBuffsForRunStart()
{
	SingleUseSecondaryMultipliers.Reset();

	if (UGameInstance* GI = GetGameInstance())
	{
		if (UT66BuffSubsystem* Buffs = GI->GetSubsystem<UT66BuffSubsystem>())
		{
			SingleUseSecondaryMultipliers = Buffs->ConsumePendingSingleUseBuffMultipliers();
		}

		if (UT66RngSubsystem* Rng = GI->GetSubsystem<UT66RngSubsystem>())
		{
			Rng->UpdateLuckStat(GetEffectiveLuckBiasStat());
		}
	}
}

#if !UE_BUILD_SHIPPING
void UT66RunStateSubsystem::DebugActivatePendingSingleUseBuffsForRunStartWithoutConsuming()
{
	SingleUseSecondaryMultipliers.Reset();

	if (UGameInstance* GI = GetGameInstance())
	{
		if (const UT66BuffSubsystem* Buffs = GI->GetSubsystem<UT66BuffSubsystem>())
		{
			SingleUseSecondaryMultipliers = Buffs->GetPendingSingleUseBuffMultipliers();
		}

		if (UT66RngSubsystem* Rng = GI->GetSubsystem<UT66RngSubsystem>())
		{
			Rng->UpdateLuckStat(GetEffectiveLuckBiasStat());
		}
	}
}
#endif


void UT66RunStateSubsystem::SetSaintBlessingActive(const bool bActive)
{
	if (bSaintBlessingActive == bActive)
	{
		return;
	}

	bSaintBlessingActive = bActive;
	HeartsChanged.Broadcast();
	HeroProgressChanged.Broadcast();
}


void UT66RunStateSubsystem::BeginSaintBlessingEmpowerment()
{
	if (bSaintBlessingLoadoutSnapshotValid)
	{
		return;
	}

	bSaintBlessingLoadoutSnapshotValid = true;
	SaintBlessingInventorySnapshot = InventorySlots;

	if (UGameInstance* GI = GetGameInstance())
	{
		if (UT66IdolManagerSubsystem* IdolManager = GI->GetSubsystem<UT66IdolManagerSubsystem>())
		{
			SaintBlessingEquippedIdolsSnapshot = IdolManager->GetEquippedIdols();
			SaintBlessingEquippedIdolTiersSnapshot = IdolManager->GetEquippedIdolTierValues();

			TArray<uint8> BlessedTiers = SaintBlessingEquippedIdolTiersSnapshot;
			const int32 MaxCount = FMath::Min(BlessedTiers.Num(), SaintBlessingEquippedIdolsSnapshot.Num());
			for (int32 Index = 0; Index < MaxCount; ++Index)
			{
				if (!SaintBlessingEquippedIdolsSnapshot[Index].IsNone())
				{
					BlessedTiers[Index] = static_cast<uint8>(UT66IdolManagerSubsystem::MaxIdolLevel);
				}
			}

			IdolManager->RestoreState(
				SaintBlessingEquippedIdolsSnapshot,
				BlessedTiers,
				IdolManager->GetCurrentDifficulty());
		}
	}

	bool bInventoryChanged = false;
	for (FT66InventorySlot& Slot : InventorySlots)
	{
		if (!Slot.IsValid() || T66_IsVendorTokenItem(Slot.ItemTemplateID))
		{
			continue;
		}

		Slot.Line1RolledValue = T66_MapBlessingRollToWhiteRange(Slot);
		Slot.SecondaryStatBonusOverride = FItemData::GetFlatSecondaryStatBonus(ET66ItemRarity::White);
		Slot.Line2MultiplierOverride = FMath::Max(Slot.GetLine2Multiplier(), FItemData::GetLine2RarityMultiplier(ET66ItemRarity::White));
		Slot.Rarity = ET66ItemRarity::White;
		bInventoryChanged = true;
	}

	if (bInventoryChanged)
	{
		RecomputeItemDerivedStats();
		InventoryChanged.Broadcast();
	}
}


void UT66RunStateSubsystem::EndSaintBlessingEmpowerment()
{
	if (!bSaintBlessingLoadoutSnapshotValid)
	{
		return;
	}

	TArray<FT66InventorySlot> RestoredInventory = SaintBlessingInventorySnapshot;
	for (int32 Index = SaintBlessingInventorySnapshot.Num(); Index < InventorySlots.Num(); ++Index)
	{
		RestoredInventory.Add(InventorySlots[Index]);
	}

	InventorySlots = MoveTemp(RestoredInventory);
	RecomputeItemDerivedStats();
	InventoryChanged.Broadcast();

	if (UGameInstance* GI = GetGameInstance())
	{
		if (UT66IdolManagerSubsystem* IdolManager = GI->GetSubsystem<UT66IdolManagerSubsystem>())
		{
			IdolManager->RestoreState(
				SaintBlessingEquippedIdolsSnapshot,
				SaintBlessingEquippedIdolTiersSnapshot,
				IdolManager->GetCurrentDifficulty());
		}
	}

	SaintBlessingInventorySnapshot.Reset();
	SaintBlessingEquippedIdolsSnapshot.Reset();
	SaintBlessingEquippedIdolTiersSnapshot.Reset();
	bSaintBlessingLoadoutSnapshotValid = false;
}


void UT66RunStateSubsystem::SetFinalSurvivalEnemyScalar(const float Scalar)
{
	const float ClampedScalar = FMath::Clamp(Scalar, 1.f, 99.f);
	if (FMath::IsNearlyEqual(FinalSurvivalEnemyScalar, ClampedScalar, 0.05f))
	{
		return;
	}

	FinalSurvivalEnemyScalar = ClampedScalar;
	DifficultyChanged.Broadcast();
}


void UT66RunStateSubsystem::SetTutorialHint(const FText& InLine1, const FText& InLine2)
{
	bTutorialHintVisible = true;
	TutorialHintLine1 = InLine1;
	TutorialHintLine2 = InLine2;
	TutorialHintChanged.Broadcast();
}


void UT66RunStateSubsystem::ClearTutorialHint()
{
	if (!bTutorialHintVisible && TutorialHintLine1.IsEmpty() && TutorialHintLine2.IsEmpty())
	{
		return;
	}
	bTutorialHintVisible = false;
	TutorialHintLine1 = FText::GetEmpty();
	TutorialHintLine2 = FText::GetEmpty();
	TutorialHintChanged.Broadcast();
}


void UT66RunStateSubsystem::SetTutorialSubtitle(const FText& InSpeaker, const FText& InText)
{
	bTutorialSubtitleVisible = true;
	TutorialSubtitleSpeaker = InSpeaker;
	TutorialSubtitleText = InText;
	TutorialSubtitleChanged.Broadcast();
}


void UT66RunStateSubsystem::ClearTutorialSubtitle()
{
	if (!bTutorialSubtitleVisible && TutorialSubtitleSpeaker.IsEmpty() && TutorialSubtitleText.IsEmpty())
	{
		return;
	}

	bTutorialSubtitleVisible = false;
	TutorialSubtitleSpeaker = FText::GetEmpty();
	TutorialSubtitleText = FText::GetEmpty();
	TutorialSubtitleChanged.Broadcast();
}


void UT66RunStateSubsystem::NotifyTutorialMoveInput()
{
	if (bTutorialMoveInputSeen) return;
	bTutorialMoveInputSeen = true;
	TutorialInputChanged.Broadcast();
}


void UT66RunStateSubsystem::NotifyTutorialJumpInput()
{
	if (bTutorialJumpInputSeen) return;
	bTutorialJumpInputSeen = true;
	TutorialInputChanged.Broadcast();
}


void UT66RunStateSubsystem::NotifyTutorialLookInput()
{
	if (bTutorialLookInputSeen) return;
	bTutorialLookInputSeen = true;
	TutorialInputChanged.Broadcast();
}


void UT66RunStateSubsystem::NotifyTutorialAttackLockInput()
{
	if (bTutorialAttackLockInputSeen) return;
	bTutorialAttackLockInputSeen = true;
	TutorialInputChanged.Broadcast();
}


void UT66RunStateSubsystem::NotifyTutorialUltimateUsed()
{
	if (bTutorialUltimateUsedSeen) return;
	bTutorialUltimateUsedSeen = true;
	TutorialInputChanged.Broadcast();
}


void UT66RunStateSubsystem::ResetTutorialInputFlags()
{
	const bool bWasMove = bTutorialMoveInputSeen;
	const bool bWasJump = bTutorialJumpInputSeen;
	const bool bWasLook = bTutorialLookInputSeen;
	const bool bWasLock = bTutorialAttackLockInputSeen;
	const bool bWasUltimate = bTutorialUltimateUsedSeen;
	bTutorialMoveInputSeen = false;
	bTutorialJumpInputSeen = false;
	bTutorialLookInputSeen = false;
	bTutorialAttackLockInputSeen = false;
	bTutorialUltimateUsedSeen = false;
	if (bWasMove || bWasJump || bWasLook || bWasLock || bWasUltimate)
	{
		TutorialInputChanged.Broadcast();
	}
}


void UT66RunStateSubsystem::ToggleDevImmortality()
{
	bDevImmortality = !bDevImmortality;
	DevCheatsChanged.Broadcast();
}


void UT66RunStateSubsystem::ToggleDevPower()
{
	bDevPower = !bDevPower;
	DevCheatsChanged.Broadcast();
}


void UT66RunStateSubsystem::RefreshActiveRunModifiersFromGameInstance()
{
	ActiveRunModifiers = FT66RunModifierSnapshot{};
	if (UT66GameInstance* T66GI = Cast<UT66GameInstance>(GetGameInstance()))
	{
		if (T66GI->IsDailyClimbRun() && T66GI->ActiveDailyClimbChallenge.IsValid())
		{
			ActiveRunModifiers.Merge(FT66RunModifierCatalog::FromDailyChallenge(T66GI->ActiveDailyClimbChallenge));
		}

		FT66CommunityContentEntry ActiveCommunityEntry;
		const UT66CommunityContentSubsystem* Community = T66GI->GetSubsystem<UT66CommunityContentSubsystem>();
		if (Community && Community->GetActiveEntry(ActiveCommunityEntry))
		{
			ActiveRunModifiers.Merge(FT66RunModifierCatalog::FromCommunityRules(ActiveCommunityEntry.Rules));
		}
	}
}


void UT66RunStateSubsystem::ResetForNewRun()
{
	RefreshActiveRunModifiersFromGameInstance();

	ResetHeartSlotTiers();
	SyncMaxHPToHeartTiers();
	CurrentHP = MaxHP;
	DeferredRunStartItemId = NAME_None;
	CurrentGold = 0;
	CurrentDebt = 0;
	bLoanSharkPending = false;
	DifficultyTier = 0;
	DifficultySkulls = 0;
	TotemsActivatedCount = 0;
	ResetShopForStage();
	OwedBossIDs.Empty();
	CowardiceGatesTakenCount = 0;
	InventorySlots.Empty();
	ActiveVendorTokenLevel = 0;
	BuybackPool.Empty();
	BuybackDisplaySlots.Empty();
	BuybackDisplayPage = 0;
	RecomputeItemDerivedStats();
	ActiveDOTs.Empty();
	RallyStacks = 0;
	RallyTimerEndWorldTime = 0.0;
	if (UWorld* W = GetGameInstance() ? GetGameInstance()->GetWorld() : nullptr)
	{
		if (DOTTimerHandle.IsValid())
		{
			W->GetTimerManager().ClearTimer(DOTTimerHandle);
			DOTTimerHandle.Invalidate();
		}
	}
	EventLog.Empty();
	StructuredEventLog.Empty();
	AntiCheatLuckEvents.Empty();
	AntiCheatHitCheckEvents.Empty();
	AntiCheatGamblerSummaries.Empty();
	AntiCheatGamblerEvents.Empty();
	bAntiCheatLuckEventsTruncated = false;
	bAntiCheatHitCheckEventsTruncated = false;
	bAntiCheatGamblerEventsTruncated = false;
	StagePacingPoints.Empty();
	ResetLuckRatingTracking();
	AntiCheatIncomingHitChecks = 0;
	AntiCheatDamageTakenHitCount = 0;
	AntiCheatDodgeCount = 0;
	AntiCheatCurrentConsecutiveDodges = 0;
	AntiCheatMaxConsecutiveDodges = 0;
	AntiCheatTotalEvasionChance = 0.f;
	InitializeAntiCheatEvasionBuckets();
	ResetAntiCheatPressureTracking();
	CurrentStage = 1;
	bStageTimerActive = false;
	StageTimerSecondsRemaining = StageTimerDurationSeconds;
	LastBroadcastStageTimerSecond = static_cast<int32>(StageTimerDurationSeconds);
	ResetSpeedRunTimer();
	bThisRunSetNewPersonalBestSpeedRunTime = false;
	CompletedStageActiveSeconds = 0.f;
	FinalRunElapsedSeconds = 0.f;
	bRunEnded = false;
	bRunEndedAsVictory = false;
	bSaintBlessingActive = false;
	SaintBlessingInventorySnapshot.Reset();
	SaintBlessingEquippedIdolsSnapshot.Reset();
	SaintBlessingEquippedIdolTiersSnapshot.Reset();
	bSaintBlessingLoadoutSnapshotValid = false;
	FinalSurvivalEnemyScalar = 1.f;
	CurrentScore = 0;
	ResetScoreBudgetContext();
	LastDamageTime = -9999.f;
	PowerCrystalsEarnedThisRun = 0;
	PowerCrystalsGrantedToWalletThisRun = 0;
	SeedLuck0To100 = -1;
	CompanionHealingDoneThisRun = 0.f;
	SingleUseSecondaryMultipliers.Reset();
	if (UGameInstance* GI = GetGameInstance())
	{
		if (UT66AchievementsSubsystem* Achievements = GI->GetSubsystem<UT66AchievementsSubsystem>())
		{
			Achievements->ResetCurrentRunAchievementUnlockSummary();
		}
	}

	// Skill Rating: reset per brand new run.
	if (UGameInstance* GI3 = GetGameInstance())
	{
		if (UT66SkillRatingSubsystem* Skill = GI3->GetSubsystem<UT66SkillRatingSubsystem>())
		{
			Skill->ResetForNewRun();
		}
	}

	bHUDPanelsVisible = true;
	ClearTutorialHint();
	ClearTutorialSubtitle();
	ResetTutorialInputFlags();

	// Clear transient stage/status effects at run start.
	StageMoveSpeedMultiplier = 1.f;
	StageMoveSpeedSecondsRemaining = 0.f;
	TemporaryPrimaryStatAmplifiers.Reset();
	StatusBurnSecondsRemaining = 0.f;
	StatusBurnDamagePerSecond = 0.f;
	StatusBurnAccumDamage = 0.f;
	StatusChillSecondsRemaining = 0.f;
	StatusChillMoveSpeedMultiplier = 1.f;
	StatusCurseSecondsRemaining = 0.f;
	HeroLevel = DefaultHeroLevel;
	HeroPreciseStats = FT66HeroPreciseStatBlock{};
	ItemPrimaryStatBonusesPrecise = FT66HeroPreciseStatBlock{};
	ClearPersistentSecondaryStatBonuses();
	PermanentSecondaryStatBonusTenths.Reset();
	ItemSecondaryStatBonusTenths.Reset();
	if (UT66GameInstance* T66GI = Cast<UT66GameInstance>(GetGameInstance()))
	{
		if (UT66IdolManagerSubsystem* IdolManager = GetIdolManager())
		{
			IdolManager->ResetForNewRun(T66GI->SelectedDifficulty);
		}
		if (UT66WeaponManagerSubsystem* WeaponManager = GetWeaponManager())
		{
			WeaponManager->ResetForNewRun(T66GI->SelectedHeroID);
		}
	}
	else if (UT66IdolManagerSubsystem* IdolManager = GetIdolManager())
	{
		IdolManager->ResetForNewRun(ET66Difficulty::Easy);
		if (UT66WeaponManagerSubsystem* WeaponManager = GetWeaponManager())
		{
			WeaponManager->RestoreState(NAME_None);
		}
	}
	InitializeHeroStatsForNewRun();

	if (UT66GameInstance* T66GI = Cast<UT66GameInstance>(GetGameInstance()))
	{
		FT66CommunityContentEntry ActiveCommunityEntry;
		const UT66CommunityContentSubsystem* Community = T66GI->GetSubsystem<UT66CommunityContentSubsystem>();
		const bool bHasCommunityEntry = Community && Community->GetActiveEntry(ActiveCommunityEntry);
		if (bHasCommunityEntry)
		{
			const FT66CommunityRuleSet& Rules = ActiveCommunityEntry.Rules;
			UE_LOG(LogTemp, Log, TEXT("[Community] ResetForNewRun applying '%s' (kind=%d startItem=%s passive=%d ultimate=%d). Deprecated stat/level overrides ignored."),
				*ActiveCommunityEntry.Title,
				static_cast<int32>(ActiveCommunityEntry.Kind),
				*Rules.StartingItemId.ToString(),
				static_cast<int32>(Rules.PassiveOverride),
				static_cast<int32>(Rules.UltimateOverride));

			if (Rules.PassiveOverride != ET66PassiveType::None)
			{
				PassiveType = Rules.PassiveOverride;
			}

			SyncLegacyHeroStatsFromPrecise();

			if (!Rules.StartingItemId.IsNone())
			{
				DeferredRunStartItemId = Rules.StartingItemId;
				UE_LOG(LogTemp, Log, TEXT("[Community] Deferred run-start item grant for '%s': %s"),
					*ActiveCommunityEntry.Title,
					*DeferredRunStartItemId.ToString());
			}
		}
	}

	// Central RNG: seed a new run stream and set the initial effective Luck stat for biasing.
	if (UGameInstance* GI = GetGameInstance())
	{
		if (UT66RngSubsystem* Rng = GI->GetSubsystem<UT66RngSubsystem>())
		{
			Rng->BeginRun(1);
			SeedLuck0To100 = Rng->RollSeedLuck0To100();
			Rng->UpdateLuckStat(GetEffectiveLuckBiasStat());
		}
	}

	HeroXP = 0;
	XPToNextLevel = GetDataDrivenLevelUpXPThreshold();
	UltimateCooldownRemainingSeconds = 0.f;
	LastBroadcastUltimateSecond = 0;
	ResetBossState();
	HeartsChanged.Broadcast();
	GoldChanged.Broadcast();
	DebtChanged.Broadcast();
	DifficultyChanged.Broadcast();
	ShopChanged.Broadcast();
	InventoryChanged.Broadcast();
	IdolsChanged.Broadcast();
	PanelVisibilityChanged.Broadcast();
	ScoreChanged.Broadcast();
	StageTimerChanged.Broadcast();
	SpeedRunTimerChanged.Broadcast();
	BossChanged.Broadcast();
	HeroProgressChanged.Broadcast();
	UltimateChanged.Broadcast();
	QuickReviveChanged.Broadcast();
	StatusEffectsChanged.Broadcast();
}


FName UT66RunStateSubsystem::ConsumeDeferredRunStartItemId()
{
	const FName ItemId = DeferredRunStartItemId;
	DeferredRunStartItemId = NAME_None;
	return ItemId;
}


void UT66RunStateSubsystem::ToggleHUDPanels()
{
	bHUDPanelsVisible = !bHUDPanelsVisible;
	PanelVisibilityChanged.Broadcast();
}


void UT66RunStateSubsystem::AddLogEntry(const FString& Entry)
{
	EventLog.Add(Entry);
	TrimLogsIfNeeded();
}
