// Copyright Tribulation 66. All Rights Reserved.

#include "UI/T66GameplayHUDWidget.h"
#include "UI/HUD/T66GameplayHUDWidget_Private.h"

DEFINE_LOG_CATEGORY(LogT66HUD);

TSharedRef<SWidget> UT66GameplayHUDWidget::RebuildWidget()
{
	return FT66Style::MakeResponsiveRoot(BuildSlateUI());
}


UT66RunStateSubsystem* UT66GameplayHUDWidget::GetRunState() const
{
	UGameInstance* GI = GetGameInstance();
	return GI ? GI->GetSubsystem<UT66RunStateSubsystem>() : nullptr;
}


UT66DamageLogSubsystem* UT66GameplayHUDWidget::GetDamageLog() const
{
	UGameInstance* GI = GetGameInstance();
	return GI ? GI->GetSubsystem<UT66DamageLogSubsystem>() : nullptr;
}


void UT66GameplayHUDWidget::MarkHUDDirty()
{
	bHUDDirty = true;
}


FT66HUDPresentationController& UT66GameplayHUDWidget::GetPresentationController()
{
	if (!PresentationController)
	{
		PresentationController = MakeUnique<FT66HUDPresentationController>(*this);
	}
	return *PresentationController;
}


const FT66HUDPresentationController& UT66GameplayHUDWidget::GetPresentationController() const
{
	check(PresentationController);
	return *PresentationController;
}


void UT66GameplayHUDWidget::NativeTick(const FGeometry& MyGeometry, float InDeltaTime)
{
	Super::NativeTick(MyGeometry, InDeltaTime);

	const bool bPausePresentationActive = IsPausePresentationActive();
	if (bPausePresentationActive != bLastPausePresentationActive)
	{
		RefreshPausePresentation();
	}

	if (bHUDDirty)
	{
		RefreshHUD();
	}

	RefreshSpeedRunTimers();
	DPSRefreshAccumSeconds += InDeltaTime;
	if (DPSRefreshAccumSeconds >= DPSRefreshIntervalSeconds)
	{
		DPSRefreshAccumSeconds = 0.f;
		RefreshDPS();
	}

	const APawn* RevealPawn = GetOwningPlayerPawn();
	if (!RevealPawn)
	{
		if (const APlayerController* PC = GetOwningPlayer())
		{
			RevealPawn = PC->GetPawn();
		}
	}
	if (RevealPawn)
	{
		TowerRevealAccumSeconds += InDeltaTime;
		if (TowerRevealAccumSeconds >= TowerRevealIntervalSeconds)
		{
			TowerRevealAccumSeconds = 0.f;
			UpdateTowerMapReveal(RevealPawn->GetActorLocation());
		}
	}
	else
	{
		TowerRevealAccumSeconds = TowerRevealIntervalSeconds;
	}

	GetPresentationController().Tick(InDeltaTime);

	if (AT66PlayerController* T66PC = Cast<AT66PlayerController>(GetOwningPlayer()))
	{
		const bool bScoped = T66PC->IsHeroOneScopeViewEnabled();
		const bool bCrosshairLocked = T66PC->HasAttackLockedEnemy();
		if (CenterCrosshairWidget.IsValid() && bCrosshairLocked != bLastCrosshairLocked)
		{
			CenterCrosshairWidget->SetLocked(bCrosshairLocked);
			bLastCrosshairLocked = bCrosshairLocked;
		}
		if (CenterCrosshairBox.IsValid())
		{
			CenterCrosshairBox->SetRenderTransformPivot(FVector2D(0.5f, 0.5f));
			FVector2D CrosshairScreenPosition = FVector2D::ZeroVector;
			if (T66PC->GetAttackLockScreenPosition(CrosshairScreenPosition))
			{
				int32 ViewportSizeX = 0;
				int32 ViewportSizeY = 0;
				T66PC->GetViewportSize(ViewportSizeX, ViewportSizeY);
				const FVector2D ViewportCenter(
					static_cast<float>(ViewportSizeX) * 0.5f,
					static_cast<float>(ViewportSizeY) * 0.5f);
				CenterCrosshairBox->SetRenderTransform(FSlateRenderTransform(CrosshairScreenPosition - ViewportCenter));
			}
			else
			{
				CenterCrosshairBox->SetRenderTransform(FSlateRenderTransform(FVector2D::ZeroVector));
			}
			if (bScoped != bLastScopedHudVisible)
			{
				CenterCrosshairBox->SetVisibility(bScoped ? EVisibility::Collapsed : EVisibility::HitTestInvisible);
			}
		}
		if (ScopedSniperOverlayBorder.IsValid())
		{
			if (bScoped != bLastScopedHudVisible)
			{
				ScopedSniperOverlayBorder->SetVisibility(bScoped ? EVisibility::HitTestInvisible : EVisibility::Collapsed);
			}
		}
		if (bScoped)
		{
			if (ScopedUltTimerText.IsValid())
			{
				const int32 UltDisplayTenths = FMath::RoundToInt(FMath::Max(0.f, T66PC->GetHeroOneScopedUltRemainingSeconds()) * 10.f);
				if (!bLastScopedHudVisible || UltDisplayTenths != LastScopedUltDisplayTenths)
				{
					LastScopedUltDisplayTenths = UltDisplayTenths;
					ScopedUltTimerText->SetText(FText::FromString(FString::Printf(TEXT("ULT %.1fs"), static_cast<float>(UltDisplayTenths) / 10.f)));
				}
			}
			if (ScopedShotCooldownText.IsValid())
			{
				const float ShotCooldown = T66PC->GetHeroOneScopedShotCooldownRemainingSeconds();
				const bool bShotReady = ShotCooldown <= 0.f;
				const int32 ShotDisplayCentis = bShotReady ? 0 : FMath::RoundToInt(FMath::Max(0.f, ShotCooldown) * 100.f);
				if (!bLastScopedHudVisible
					|| bShotReady != bLastScopedShotReady
					|| (!bShotReady && ShotDisplayCentis != LastScopedShotDisplayCentis))
				{
					bLastScopedShotReady = bShotReady;
					LastScopedShotDisplayCentis = ShotDisplayCentis;
					ScopedShotCooldownText->SetText(
						bShotReady
							? NSLOCTEXT("T66.HUD", "ScopedShotReady", "SHOT READY")
							: FText::FromString(FString::Printf(TEXT("SHOT %.2fs"), static_cast<float>(ShotDisplayCentis) / 100.f)));
				}
			}
		}
		else if (bLastScopedHudVisible)
		{
			LastScopedUltDisplayTenths = INDEX_NONE;
			LastScopedShotDisplayCentis = INDEX_NONE;
			bLastScopedShotReady = false;
		}

		bLastScopedHudVisible = bScoped;
	}
	else
	{
		if (CenterCrosshairWidget.IsValid() && bLastCrosshairLocked)
		{
			CenterCrosshairWidget->SetLocked(false);
		}
		if (CenterCrosshairBox.IsValid())
		{
			CenterCrosshairBox->SetRenderTransform(FSlateRenderTransform(FVector2D::ZeroVector));
			CenterCrosshairBox->SetVisibility(EVisibility::HitTestInvisible);
		}
		if (ScopedSniperOverlayBorder.IsValid())
		{
			ScopedSniperOverlayBorder->SetVisibility(EVisibility::Collapsed);
		}
		bLastCrosshairLocked = false;
		bLastScopedHudVisible = false;
		LastScopedUltDisplayTenths = INDEX_NONE;
		LastScopedShotDisplayCentis = INDEX_NONE;
		bLastScopedShotReady = false;
	}
}


void UT66GameplayHUDWidget::NativeConstruct()
{
	Super::NativeConstruct();
	UT66RunStateSubsystem* RunState = GetRunState();
	if (!RunState) return;

	RunState->HeartsChanged.AddDynamic(this, &UT66GameplayHUDWidget::RefreshHearts);
	RunState->GoldChanged.AddDynamic(this, &UT66GameplayHUDWidget::RefreshEconomy);
	RunState->DebtChanged.AddDynamic(this, &UT66GameplayHUDWidget::RefreshEconomy);
	RunState->InventoryChanged.AddDynamic(this, &UT66GameplayHUDWidget::RefreshEconomy);
	RunState->InventoryChanged.AddDynamic(this, &UT66GameplayHUDWidget::MarkHUDDirty);
	RunState->PanelVisibilityChanged.AddDynamic(this, &UT66GameplayHUDWidget::MarkHUDDirty);
	RunState->ScoreChanged.AddDynamic(this, &UT66GameplayHUDWidget::RefreshEconomy);
	RunState->StageChanged.AddDynamic(this, &UT66GameplayHUDWidget::MarkHUDDirty);
	RunState->StageTimerChanged.AddDynamic(this, &UT66GameplayHUDWidget::RefreshStageAndTimer);
	RunState->SpeedRunTimerChanged.AddDynamic(this, &UT66GameplayHUDWidget::RefreshSpeedRunTimers);
	RunState->BossChanged.AddDynamic(this, &UT66GameplayHUDWidget::RefreshBossBar);
	RunState->DifficultyChanged.AddDynamic(this, &UT66GameplayHUDWidget::MarkHUDDirty);
	RunState->CowardiceGatesTakenChanged.AddDynamic(this, &UT66GameplayHUDWidget::MarkHUDDirty);
	if (UGameInstance* GI = GetGameInstance())
	{
		if (UT66IdolManagerSubsystem* IdolManager = GI->GetSubsystem<UT66IdolManagerSubsystem>())
		{
			IdolManager->IdolStateChanged.AddDynamic(this, &UT66GameplayHUDWidget::MarkHUDDirty);
		}
	}
	RunState->HeroProgressChanged.AddDynamic(this, &UT66GameplayHUDWidget::MarkHUDDirty);
	RunState->UltimateChanged.AddDynamic(this, &UT66GameplayHUDWidget::MarkHUDDirty);
	RunState->SurvivalChanged.AddDynamic(this, &UT66GameplayHUDWidget::MarkHUDDirty);
	RunState->QuickReviveChanged.AddDynamic(this, &UT66GameplayHUDWidget::RefreshQuickReviveState);
	RunState->StatusEffectsChanged.AddDynamic(this, &UT66GameplayHUDWidget::RefreshStatusEffects);
	RunState->TutorialHintChanged.AddDynamic(this, &UT66GameplayHUDWidget::RefreshTutorialHint);
	RunState->TutorialSubtitleChanged.AddDynamic(this, &UT66GameplayHUDWidget::RefreshTutorialSubtitle);
	RunState->DevCheatsChanged.AddDynamic(this, &UT66GameplayHUDWidget::MarkHUDDirty);

	if (UGameInstance* GI = GetGameInstance())
	{
		if (UT66PlayerSettingsSubsystem* PS = GI->GetSubsystem<UT66PlayerSettingsSubsystem>())
		{
			PS->OnSettingsChanged.AddDynamic(this, &UT66GameplayHUDWidget::MarkHUDDirty);
		}
		if (UT66MediaViewerSubsystem* MV = GI->GetSubsystem<UT66MediaViewerSubsystem>())
		{
			MV->OnMediaViewerOpenChanged.AddDynamic(this, &UT66GameplayHUDWidget::HandleMediaViewerOpenChanged);
		}
		if (UT66AchievementsSubsystem* Ach = GI->GetSubsystem<UT66AchievementsSubsystem>())
		{
			Ach->AchievementsUnlocked.AddDynamic(this, &UT66GameplayHUDWidget::HandleAchievementsUnlocked);
			Ach->AchievementsStateChanged.AddDynamic(this, &UT66GameplayHUDWidget::MarkHUDDirty);
		}
		if (UT66BackendSubsystem* Backend = GI->GetSubsystem<UT66BackendSubsystem>())
		{
			Backend->OnLeaderboardDataReady.AddUObject(this, &UT66GameplayHUDWidget::HandleBackendLeaderboardDataReady);
			Backend->OnRunSummaryReady.AddUObject(this, &UT66GameplayHUDWidget::HandleBackendRunSummaryReady);
		}
	}

	// Loot prompt should update immediately on overlap changes (no stage-timer polling).
	if (AT66PlayerController* PC = Cast<AT66PlayerController>(GetOwningPlayer()))
	{
		PC->NearbyLootBagChanged.AddDynamic(this, &UT66GameplayHUDWidget::RefreshLootPrompt);
	}

	// Map/minimap refresh (lightweight, throttled timer; no per-frame UI thinking).
	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().SetTimer(MapRefreshTimerHandle, this, &UT66GameplayHUDWidget::RefreshMapData, 0.5f, true);
		if (FPSText.IsValid() || ElevationText.IsValid())
		{
			World->GetTimerManager().SetTimer(FPSTimerHandle, this, &UT66GameplayHUDWidget::RefreshFPS, 0.25f, true);
		}
	}

	// Bottom-left HUD scale (anchor bottom-left)
	if (BottomLeftHUDBox.IsValid())
	{
		BottomLeftHUDBox->SetRenderTransformPivot(FVector2D(0.f, 1.f));
		BottomLeftHUDBox->SetRenderTransform(FSlateRenderTransform(FTransform2D(GT66BottomLeftHudScale)));
	}

	ApplyInventoryInspectMode();
	TowerRevealPointsByFloor.Reset();

	// Passive and Ultimate ability tooltips (hero-specific)
	UT66LocalizationSubsystem* Loc = GetGameInstance() ? GetGameInstance()->GetSubsystem<UT66LocalizationSubsystem>() : nullptr;
	UT66GameInstance* T66GI = Cast<UT66GameInstance>(GetGameInstance());
	FHeroData HD;
	if (T66GI && T66GI->GetSelectedHeroData(HD) && Loc)
	{
		if (PassiveBorder.IsValid())
		{
			PassiveBorder->SetToolTip(CreateRichTooltip(Loc->GetText_PassiveName(HD.PassiveType), Loc->GetText_PassiveDescription(HD.PassiveType)));
		}
		if (UltimateBorder.IsValid())
		{
			UltimateBorder->SetToolTip(CreateRichTooltip(Loc->GetText_UltimateName(HD.UltimateType), Loc->GetText_UltimateDescription(HD.UltimateType)));
		}
	}

	MarkHUDDirty();
	RefreshHUD();
	RefreshTutorialHint();
	RefreshTutorialSubtitle();
	RefreshSpeedRunTimers();
	RefreshDPS();
	RefreshLootPrompt();
	RefreshHearts();
	RefreshQuickReviveState();
	RefreshStatusEffects();
}


void UT66GameplayHUDWidget::NativeDestruct()
{
	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().ClearTimer(MapRefreshTimerHandle);
		World->GetTimerManager().ClearTimer(FPSTimerHandle);
		World->GetTimerManager().ClearTimer(TikTokOverlaySyncHandle);
	}

	GetPresentationController().Reset();
	PresentationController.Reset();

	UT66RunStateSubsystem* RunState = GetRunState();
	if (RunState)
	{
		RunState->HeartsChanged.RemoveDynamic(this, &UT66GameplayHUDWidget::RefreshHearts);
		RunState->GoldChanged.RemoveDynamic(this, &UT66GameplayHUDWidget::RefreshEconomy);
		RunState->DebtChanged.RemoveDynamic(this, &UT66GameplayHUDWidget::RefreshEconomy);
		RunState->InventoryChanged.RemoveDynamic(this, &UT66GameplayHUDWidget::RefreshEconomy);
		RunState->InventoryChanged.RemoveDynamic(this, &UT66GameplayHUDWidget::MarkHUDDirty);
		RunState->PanelVisibilityChanged.RemoveDynamic(this, &UT66GameplayHUDWidget::MarkHUDDirty);
		RunState->ScoreChanged.RemoveDynamic(this, &UT66GameplayHUDWidget::RefreshEconomy);
		RunState->StageChanged.RemoveDynamic(this, &UT66GameplayHUDWidget::MarkHUDDirty);
		RunState->StageTimerChanged.RemoveDynamic(this, &UT66GameplayHUDWidget::RefreshStageAndTimer);
		RunState->SpeedRunTimerChanged.RemoveDynamic(this, &UT66GameplayHUDWidget::RefreshSpeedRunTimers);
		RunState->BossChanged.RemoveDynamic(this, &UT66GameplayHUDWidget::RefreshBossBar);
		RunState->DifficultyChanged.RemoveDynamic(this, &UT66GameplayHUDWidget::MarkHUDDirty);
		RunState->CowardiceGatesTakenChanged.RemoveDynamic(this, &UT66GameplayHUDWidget::MarkHUDDirty);
		if (UGameInstance* GI = GetGameInstance())
		{
			if (UT66IdolManagerSubsystem* IdolManager = GI->GetSubsystem<UT66IdolManagerSubsystem>())
			{
				IdolManager->IdolStateChanged.RemoveDynamic(this, &UT66GameplayHUDWidget::MarkHUDDirty);
			}
		}
		RunState->HeroProgressChanged.RemoveDynamic(this, &UT66GameplayHUDWidget::MarkHUDDirty);
		RunState->UltimateChanged.RemoveDynamic(this, &UT66GameplayHUDWidget::MarkHUDDirty);
		RunState->SurvivalChanged.RemoveDynamic(this, &UT66GameplayHUDWidget::MarkHUDDirty);
		RunState->QuickReviveChanged.RemoveDynamic(this, &UT66GameplayHUDWidget::RefreshQuickReviveState);
		RunState->TutorialHintChanged.RemoveDynamic(this, &UT66GameplayHUDWidget::RefreshTutorialHint);
		RunState->TutorialSubtitleChanged.RemoveDynamic(this, &UT66GameplayHUDWidget::RefreshTutorialSubtitle);
		RunState->DevCheatsChanged.RemoveDynamic(this, &UT66GameplayHUDWidget::MarkHUDDirty);
		RunState->StatusEffectsChanged.RemoveDynamic(this, &UT66GameplayHUDWidget::RefreshStatusEffects);
	}
	if (UGameInstance* GI = GetGameInstance())
	{
		if (UT66PlayerSettingsSubsystem* PS = GI->GetSubsystem<UT66PlayerSettingsSubsystem>())
		{
			PS->OnSettingsChanged.RemoveDynamic(this, &UT66GameplayHUDWidget::MarkHUDDirty);
		}
		if (UT66MediaViewerSubsystem* MV = GI->GetSubsystem<UT66MediaViewerSubsystem>())
		{
			MV->OnMediaViewerOpenChanged.RemoveDynamic(this, &UT66GameplayHUDWidget::HandleMediaViewerOpenChanged);
		}
		if (UT66AchievementsSubsystem* Ach = GI->GetSubsystem<UT66AchievementsSubsystem>())
		{
			Ach->AchievementsUnlocked.RemoveDynamic(this, &UT66GameplayHUDWidget::HandleAchievementsUnlocked);
			Ach->AchievementsStateChanged.RemoveDynamic(this, &UT66GameplayHUDWidget::MarkHUDDirty);
		}
		if (UT66BackendSubsystem* Backend = GI->GetSubsystem<UT66BackendSubsystem>())
		{
			Backend->OnLeaderboardDataReady.RemoveAll(this);
			Backend->OnRunSummaryReady.RemoveAll(this);
		}
	}

	if (AT66PlayerController* PC = Cast<AT66PlayerController>(GetOwningPlayer()))
	{
		PC->NearbyLootBagChanged.RemoveDynamic(this, &UT66GameplayHUDWidget::RefreshLootPrompt);
	}
	TowerRevealPointsByFloor.Reset();
	bHasAppliedMediaViewerOpenState = false;
	Super::NativeDestruct();
}

