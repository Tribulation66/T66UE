// Copyright Tribulation 66. All Rights Reserved.

#include "UI/HUD/T66HUDPresentationController.h"
#include "UI/HUD/T66GameplayHUDWidget_Private.h"
#include "UI/T66CrateOverlayWidget.h"
#include "UI/T66LootWheelOverlayWidget.h"

FT66HUDPresentationController::FT66HUDPresentationController(UT66GameplayHUDWidget& InOwner)
	: Owner(InOwner)
{
}


void FT66HUDPresentationController::Tick(float InDeltaTime)
{
	TickChestRewardPresentation(InDeltaTime);
	TickLootBagRevealPresentation(InDeltaTime);
	TryShowQueuedPresentation();
}


void FT66HUDPresentationController::Reset()
{
	bResettingPresentations = true;
	if (UWorld* World = Owner.GetWorld())
	{
		World->GetTimerManager().ClearTimer(AchievementNotificationTimerHandle);
	}

	HidePickupCard();
	HideLootBagReveal();
	HideChestReward();
	DrainQueuedPresentationsForTeardown();
	QueuedPresentations.Reset();
	AchievementNotificationQueue.Reset();

	if (UT66CrateOverlayWidget* Overlay = ActiveCrateOverlay.Get())
	{
		Overlay->RemoveFromParent();
	}
	ActiveCrateOverlay.Reset();
	if (UT66LootWheelOverlayWidget* Overlay = ActiveLootWheelOverlay.Get())
	{
		Overlay->RemoveFromParent();
	}
	ActiveLootWheelOverlay.Reset();

	if (Owner.AchievementNotificationBox.IsValid())
	{
		Owner.AchievementNotificationBox->SetVisibility(EVisibility::Collapsed);
	}
	if (Owner.AchievementNotificationTitleText.IsValid())
	{
		Owner.AchievementNotificationTitleText->SetText(FText::GetEmpty());
	}
	bResettingPresentations = false;
}


void FT66HUDPresentationController::HandleAchievementsUnlocked(const TArray<FName>& NewlyUnlockedIDs)
{
	if (T66_IsHudReviewStateCommandLine())
	{
		return;
	}

	AchievementNotificationQueue.Append(NewlyUnlockedIDs);
	ShowNextAchievementNotification();
}


void FT66HUDPresentationController::ShowNextAchievementNotification()
{
	if (AchievementNotificationQueue.Num() == 0)
	{
		if (Owner.AchievementNotificationBox.IsValid())
		{
			Owner.AchievementNotificationBox->SetVisibility(EVisibility::Collapsed);
		}
		return;
	}

	UGameInstance* GI = Owner.GetGameInstance();
	UT66AchievementsSubsystem* Ach = GI ? GI->GetSubsystem<UT66AchievementsSubsystem>() : nullptr;
	if (!Ach || !Owner.AchievementNotificationBorder.IsValid() || !Owner.AchievementNotificationTitleText.IsValid() || !Owner.AchievementNotificationBox.IsValid())
	{
		return;
	}

	const FName AchievementID = AchievementNotificationQueue[0];
	const TArray<FAchievementData> All = Ach->GetAllAchievements();
	const FAchievementData* Data = All.FindByPredicate([&AchievementID](const FAchievementData& AchievementData)
	{
		return AchievementData.AchievementID == AchievementID;
	});
	if (!Data)
	{
		AchievementNotificationQueue.RemoveAt(0);
		ShowNextAchievementNotification();
		return;
	}

	auto GetTierBorderColor = [](ET66AchievementTier Tier) -> FLinearColor
	{
		switch (Tier)
		{
		case ET66AchievementTier::Black: return FLinearColor(0.15f, 0.15f, 0.15f, 1.0f);
		case ET66AchievementTier::Red: return FLinearColor(0.6f, 0.15f, 0.15f, 1.0f);
		case ET66AchievementTier::Yellow: return FLinearColor(0.6f, 0.5f, 0.1f, 1.0f);
		case ET66AchievementTier::White: return FLinearColor(0.8f, 0.8f, 0.8f, 1.0f);
		default: return FLinearColor(0.15f, 0.15f, 0.15f, 1.0f);
		}
	};

	Owner.AchievementNotificationBorder->SetBorderBackgroundColor(GetTierBorderColor(Data->Tier));
	Owner.AchievementNotificationTitleText->SetText(Data->DisplayName);
	Owner.AchievementNotificationBox->SetVisibility(EVisibility::Visible);

	if (UWorld* World = Owner.GetWorld())
	{
		World->GetTimerManager().SetTimer(
			AchievementNotificationTimerHandle,
			FTimerDelegate::CreateRaw(this, &FT66HUDPresentationController::HideAchievementNotificationAndShowNext),
			UT66GameplayHUDWidget::AchievementNotificationDisplaySeconds,
			false);
	}
}


void FT66HUDPresentationController::HideAchievementNotificationAndShowNext()
{
	if (AchievementNotificationQueue.Num() > 0)
	{
		AchievementNotificationQueue.RemoveAt(0);
	}

	ShowNextAchievementNotification();
}


bool FT66HUDPresentationController::IsRewardPresentationBusy() const
{
	const UT66CrateOverlayWidget* CrateOverlay = ActiveCrateOverlay.Get();
	const UT66LootWheelOverlayWidget* LootWheelOverlay = ActiveLootWheelOverlay.Get();
	return (CrateOverlay && CrateOverlay->IsInViewport())
		|| (LootWheelOverlay && LootWheelOverlay->IsInViewport())
		|| bChestRewardVisible
		|| bLootBagRevealVisible
		|| bPickupCardVisible;
}

void FT66HUDPresentationController::DispatchChestRewardCommitIfNeeded()
{
	if (bChestRewardCommitDispatched)
	{
		return;
	}

	bChestRewardCommitDispatched = true;
	ChestRewardDisplayedGold = ChestRewardTargetGold;
	ChestRewardMinimumDisplayedGold = ChestRewardTargetGold;
	if (ActiveChestRewardCommitCallback)
	{
		ActiveChestRewardCommitCallback();
		ActiveChestRewardCommitCallback = nullptr;
	}
}

void FT66HUDPresentationController::DispatchChestRewardFinishedIfNeeded()
{
	if (ActiveChestRewardFinishedCallback)
	{
		ActiveChestRewardFinishedCallback();
		ActiveChestRewardFinishedCallback = nullptr;
	}
}

void FT66HUDPresentationController::DrainQueuedPresentationsForTeardown()
{
	for (FQueuedPresentation& QueuedPresentation : QueuedPresentations)
	{
		if (QueuedPresentation.Type == EQueuedPresentationType::CrateOpen)
		{
			UT66CrateOverlayWidget::CommitImmediateCrateReward(Owner.GetWorld(), QueuedPresentation.Rarity);
			continue;
		}

		if (QueuedPresentation.Type == EQueuedPresentationType::ChestReward)
		{
			if (QueuedPresentation.OnCommit)
			{
				QueuedPresentation.OnCommit();
				QueuedPresentation.OnCommit = nullptr;
			}
			if (QueuedPresentation.OnFinished)
			{
				QueuedPresentation.OnFinished();
				QueuedPresentation.OnFinished = nullptr;
			}
			continue;
		}

		if (QueuedPresentation.Type == EQueuedPresentationType::LootWheelSpin)
		{
			if (QueuedPresentation.LootWheelParams.OnLandingCommit)
			{
				QueuedPresentation.LootWheelParams.OnLandingCommit();
				QueuedPresentation.LootWheelParams.OnLandingCommit = nullptr;
			}
			if (QueuedPresentation.LootWheelParams.OnFinished)
			{
				QueuedPresentation.LootWheelParams.OnFinished();
				QueuedPresentation.LootWheelParams.OnFinished = nullptr;
			}
		}
	}
}


void FT66HUDPresentationController::StartCrateOpen(const ET66Rarity SourceCrateRarity)
{
	if (bResettingPresentations)
	{
		return;
	}

	APlayerController* PC = Owner.GetOwningPlayer();
	if (!PC)
	{
		return;
	}

	if (IsRewardPresentationBusy())
	{
		FQueuedPresentation& QueuedPresentation = QueuedPresentations.AddDefaulted_GetRef();
		QueuedPresentation.Type = EQueuedPresentationType::CrateOpen;
		QueuedPresentation.Rarity = SourceCrateRarity;
		UE_LOG(LogT66HUD, Display, TEXT("[HUDPresentationQueue] queued type=CrateOpen pending=%d"), QueuedPresentations.Num());
		return;
	}

	HidePickupCard();

	UT66CrateOverlayWidget* Overlay = CreateWidget<UT66CrateOverlayWidget>(PC, UT66CrateOverlayWidget::StaticClass());
	if (Overlay)
	{
		Overlay->SetPresentationHost(&Owner);
		Overlay->SetSourceCrateRarity(SourceCrateRarity);
		Overlay->SetVisibility(ESlateVisibility::HitTestInvisible);
		Overlay->AddToViewport(100);
		ActiveCrateOverlay = Overlay;
	}
}

bool FT66HUDPresentationController::StartLootWheelSpin(FT66LootWheelPresentationParams Params)
{
	if (bResettingPresentations)
	{
		if (Params.OnLandingCommit)
		{
			Params.OnLandingCommit();
		}
		if (Params.OnFinished)
		{
			Params.OnFinished();
		}
		return false;
	}

	APlayerController* PC = Owner.GetOwningPlayer();
	if (!PC)
	{
		if (Params.OnLandingCommit)
		{
			Params.OnLandingCommit();
		}
		if (Params.OnFinished)
		{
			Params.OnFinished();
		}
		return false;
	}

	if (IsRewardPresentationBusy())
	{
		FQueuedPresentation& QueuedPresentation = QueuedPresentations.AddDefaulted_GetRef();
		QueuedPresentation.Type = EQueuedPresentationType::LootWheelSpin;
		QueuedPresentation.LootWheelParams = MoveTemp(Params);
		UE_LOG(LogT66HUD, Display, TEXT("[HUDPresentationQueue] queued type=LootWheelSpin pending=%d"), QueuedPresentations.Num());
		return true;
	}

	HidePickupCard();

	UT66LootWheelOverlayWidget* Overlay = CreateWidget<UT66LootWheelOverlayWidget>(PC, UT66LootWheelOverlayWidget::StaticClass());
	if (!Overlay)
	{
		if (Params.OnLandingCommit)
		{
			Params.OnLandingCommit();
		}
		if (Params.OnFinished)
		{
			Params.OnFinished();
		}
		return false;
	}

	Overlay->SetPresentationHost(&Owner);
	Overlay->Configure(MoveTemp(Params));
	Overlay->SetVisibility(ESlateVisibility::HitTestInvisible);
	Overlay->AddToViewport(100);
	ActiveLootWheelOverlay = Overlay;
	return true;
}


bool FT66HUDPresentationController::StartChestReward(
	const ET66Rarity ChestRarity,
	const int32 GoldAmount,
	TFunction<void()> OnCommit,
	TFunction<void()> OnFinished)
{
	if (bResettingPresentations)
	{
		if (OnCommit)
		{
			OnCommit();
		}
		if (OnFinished)
		{
			OnFinished();
		}
		return false;
	}

	if (!Owner.ChestRewardBox.IsValid())
	{
		if (OnCommit)
		{
			OnCommit();
		}
		if (OnFinished)
		{
			OnFinished();
		}
		return false;
	}

	if (IsRewardPresentationBusy())
	{
		FQueuedPresentation& QueuedPresentation = QueuedPresentations.AddDefaulted_GetRef();
		QueuedPresentation.Type = EQueuedPresentationType::ChestReward;
		QueuedPresentation.Rarity = ChestRarity;
		QueuedPresentation.GoldAmount = GoldAmount;
		QueuedPresentation.OnCommit = MoveTemp(OnCommit);
		QueuedPresentation.OnFinished = MoveTemp(OnFinished);
		UE_LOG(LogT66HUD, Display, TEXT("[HUDPresentationQueue] queued type=ChestReward pending=%d"), QueuedPresentations.Num());
		return true;
	}

	HidePickupCard();

	if (Owner.GoldCurrencyBrush.IsValid())
	{
		BindRuntimeHudBrush(Owner.GoldCurrencyBrush, GetGoldCurrencyRelativePath(), FVector2D(32.f, 32.f));
	}
	if (Owner.ChestRewardCoinBrush.IsValid())
	{
		BindRuntimeHudBrush(Owner.ChestRewardCoinBrush, GetChestRewardCoinRelativePath(), FVector2D(40.f, 40.f));
		if (!Owner.ChestRewardCoinBrush->GetResourceObject())
		{
			BindRuntimeHudBrush(Owner.ChestRewardCoinBrush, GetGoldCurrencyRelativePath(), FVector2D(40.f, 40.f));
		}
	}

	const FVector2D ChestClosedImageSize(312.f, 240.f);
	const FVector2D ChestOpenImageSize(360.f, 308.f);
	static constexpr ET66Rarity ChestPresentationArtRarity = ET66Rarity::Yellow;
	if (Owner.ChestRewardClosedBrush.IsValid())
	{
		BindRuntimeHudBrush(Owner.ChestRewardClosedBrush, GetChestRewardClosedRelativePath(ChestPresentationArtRarity), ChestClosedImageSize);
		if (!Owner.ChestRewardClosedBrush->GetResourceObject())
		{
			BindHudAssetBrush(Owner.ChestRewardClosedBrush, GetChestRewardFallbackAssetPath(ChestPresentationArtRarity), ChestClosedImageSize);
		}
		if (!Owner.ChestRewardClosedBrush->GetResourceObject())
		{
			BindRuntimeHudBrush(Owner.ChestRewardClosedBrush, GetChestRewardFallbackRelativePath(ChestPresentationArtRarity), ChestClosedImageSize);
		}
	}
	if (Owner.ChestRewardOpenBrush.IsValid())
	{
		BindRuntimeHudBrush(Owner.ChestRewardOpenBrush, GetChestRewardOpenRelativePath(ChestPresentationArtRarity), ChestOpenImageSize);
		if (!Owner.ChestRewardOpenBrush->GetResourceObject())
		{
			BindHudAssetBrush(Owner.ChestRewardOpenBrush, GetChestRewardFallbackAssetPath(ChestPresentationArtRarity), ChestOpenImageSize);
		}
		if (!Owner.ChestRewardOpenBrush->GetResourceObject())
		{
			BindRuntimeHudBrush(Owner.ChestRewardOpenBrush, GetChestRewardFallbackRelativePath(ChestPresentationArtRarity), ChestOpenImageSize);
		}
	}

	if (Owner.ChestRewardClosedImage.IsValid())
	{
		Owner.ChestRewardClosedImage->SetImage(Owner.ChestRewardClosedBrush.Get());
	}
	if (Owner.ChestRewardOpenImage.IsValid())
	{
		Owner.ChestRewardOpenImage->SetImage(Owner.ChestRewardOpenBrush.Get());
	}
	if (Owner.ChestRewardClosedBox.IsValid())
	{
		Owner.ChestRewardClosedBox->SetRenderOpacity(1.f);
		Owner.ChestRewardClosedBox->SetWidthOverride(312.f);
		Owner.ChestRewardClosedBox->SetHeightOverride(240.f);
		Owner.ChestRewardClosedBox->SetRenderTransform(FSlateRenderTransform(FVector2D::ZeroVector));
	}
	if (Owner.ChestRewardOpenBox.IsValid())
	{
		Owner.ChestRewardOpenBox->SetRenderOpacity(0.f);
		Owner.ChestRewardOpenBox->SetWidthOverride(360.f);
		Owner.ChestRewardOpenBox->SetHeightOverride(308.f);
		Owner.ChestRewardOpenBox->SetRenderTransform(FSlateRenderTransform(FVector2D::ZeroVector));
	}

	ActiveChestRewardRarity = ChestRarity;
	ChestRewardTargetGold = FMath::Max(0, GoldAmount);
	ChestRewardDisplayedGold = 0;
	ChestRewardMinimumDisplayedGold = 0;
	bChestRewardCommitDispatched = false;
	bChestRewardCountTimelineStarted = false;
	ActiveChestRewardCommitCallback = MoveTemp(OnCommit);
	ActiveChestRewardFinishedCallback = MoveTemp(OnFinished);
	ChestRewardElapsedSeconds = 0.f;
	ChestRewardRemainingSeconds = UT66GameplayHUDWidget::ChestRewardDisplaySeconds;
	bChestRewardVisible = true;
	ChestRewardMarkerDispatcher.ClearHandlers();
	ChestRewardMarkerDispatcher.RegisterHandler(
		FName(TEXT("Chest.InventoryCommit")),
		[this](const FT66AnimationMarkerEvent&)
		{
			DispatchChestRewardCommitIfNeeded();
			RefreshChestRewardVisualState();
		});

	ChestRewardCountTimeline = FT66AnimationTimeline(FName(TEXT("Chest.RewardCount")));
	ChestRewardCountTimeline.SetDuration(0.80f);
	ChestRewardCountTimeline.SetCurve(FT66AnimationCurveSpec(ET66AnimationCurve::EaseOutCubic));
	ChestRewardCountTimeline.SetProgressCallback([this](const float CurveValue)
	{
		ChestRewardDisplayedGold = FMath::RoundToInt(static_cast<float>(ChestRewardTargetGold) * FMath::Clamp(CurveValue, 0.f, 1.f));
		ChestRewardDisplayedGold = FMath::Max(ChestRewardDisplayedGold, ChestRewardMinimumDisplayedGold);
	});
	ChestRewardCountTimeline.AddMarker({
		FName(TEXT("Chest.InventoryCommit")),
		ET66AnimationMarkerType::ProgressBased,
		1.f,
		0.f,
		ET66AnimationMarkerFirePolicy::Once,
		FName(TEXT("Chest.InventoryCommit"))
	});
	ChestRewardCountTimeline.Cancel();

	if (Owner.ChestRewardCounterText.IsValid())
	{
		Owner.ChestRewardCounterText->SetText(FText::Format(
			NSLOCTEXT("T66.ChestReward", "GoldCounterFormat", "+{0}"),
			FText::AsNumber(0)));
	}
	if (Owner.ChestRewardSkipText.IsValid())
	{
		Owner.ChestRewardSkipText->SetText(BuildSkipCountdownText(UT66GameplayHUDWidget::ChestRewardDisplaySeconds, FName(TEXT("Interact"))));
	}

	Owner.ChestRewardBox->SetVisibility(EVisibility::Visible);
	Owner.ChestRewardBox->SetRenderOpacity(1.f);
	RefreshChestRewardVisualState();

	for (int32 CoinIndex = 0; CoinIndex < Owner.ChestRewardCoinBoxes.Num(); ++CoinIndex)
	{
		if (Owner.ChestRewardCoinBoxes[CoinIndex].IsValid())
		{
			Owner.ChestRewardCoinBoxes[CoinIndex]->SetVisibility(EVisibility::Visible);
			Owner.ChestRewardCoinBoxes[CoinIndex]->SetRenderOpacity(0.f);
		}
		if (Owner.ChestRewardCoinImages.IsValidIndex(CoinIndex) && Owner.ChestRewardCoinImages[CoinIndex].IsValid())
		{
			Owner.ChestRewardCoinImages[CoinIndex]->SetImage(Owner.ChestRewardCoinBrush.IsValid() ? Owner.ChestRewardCoinBrush.Get() : Owner.GoldCurrencyBrush.Get());
		}
	}
	for (const TSharedPtr<SBox>& BeamBox : Owner.ChestRewardBeamBoxes)
	{
		if (BeamBox.IsValid())
		{
			BeamBox->SetVisibility(EVisibility::Visible);
			BeamBox->SetRenderOpacity(0.f);
			BeamBox->SetRenderTransform(FSlateRenderTransform(FVector2D::ZeroVector));
		}
	}
	for (const TSharedPtr<SBox>& SparkleBox : Owner.ChestRewardSparkleBoxes)
	{
		if (SparkleBox.IsValid())
		{
			SparkleBox->SetVisibility(EVisibility::Visible);
			SparkleBox->SetRenderOpacity(0.f);
			SparkleBox->SetRenderTransform(FSlateRenderTransform(FVector2D::ZeroVector));
		}
	}

	return true;
}


bool FT66HUDPresentationController::TrySkipActivePresentation()
{
	if (UT66CrateOverlayWidget* Overlay = ActiveCrateOverlay.Get())
	{
		if (Overlay->IsInViewport())
		{
			Overlay->RequestSkip();
			return true;
		}
		ActiveCrateOverlay.Reset();
	}

	if (UT66LootWheelOverlayWidget* Overlay = ActiveLootWheelOverlay.Get())
	{
		if (Overlay->IsInViewport())
		{
			Overlay->RequestSkip();
			return true;
		}
		ActiveLootWheelOverlay.Reset();
	}

	if (bChestRewardVisible)
	{
		const ET66Rarity CurrentDisplayRarity = ResolveChestRewardDisplayedRarity(FMath::Max(ChestRewardDisplayedGold, ChestRewardMinimumDisplayedGold));
		if (CurrentDisplayRarity == ActiveChestRewardRarity)
		{
			ChestRewardDisplayedGold = ChestRewardTargetGold;
			ChestRewardMinimumDisplayedGold = ChestRewardTargetGold;
			DispatchChestRewardCommitIfNeeded();
			RefreshChestRewardVisualState();
			HideChestReward();
		}
		else
		{
			const int32 CurrentStageIndex = T66RarityToStageIndex(CurrentDisplayRarity);
			const int32 FinalStageIndex = T66RarityToStageIndex(ActiveChestRewardRarity);
			const ET66Rarity NextDisplayRarity = T66StageIndexToRarity(FMath::Clamp(CurrentStageIndex + 1, 0, FinalStageIndex));
			ChestRewardMinimumDisplayedGold = FMath::Max(ChestRewardMinimumDisplayedGold, GetChestRewardRevealThresholdGold(NextDisplayRarity));
			ChestRewardDisplayedGold = FMath::Max(ChestRewardDisplayedGold, ChestRewardMinimumDisplayedGold);
			RefreshChestRewardVisualState();
		}
		return true;
	}

	if (bLootBagRevealVisible)
	{
		CompleteLootBagRevealToPickupCard();
		return true;
	}

	if (bPickupCardVisible)
	{
		HidePickupCard();
		return true;
	}

	return false;
}


void FT66HUDPresentationController::ClearActiveCratePresentation(UT66CrateOverlayWidget* Overlay)
{
	if (!Overlay || ActiveCrateOverlay.Get() == Overlay)
	{
		ActiveCrateOverlay.Reset();
	}
}

void FT66HUDPresentationController::ClearActiveLootWheelPresentation(UT66LootWheelOverlayWidget* Overlay)
{
	if (!Overlay || ActiveLootWheelOverlay.Get() == Overlay)
	{
		ActiveLootWheelOverlay.Reset();
	}
}


int32 FT66HUDPresentationController::GetChestRewardRevealThresholdGold(const ET66Rarity Rarity) const
{
	if (Rarity == ET66Rarity::Black)
	{
		return 0;
	}

	UT66GameInstance* T66GI = Cast<UT66GameInstance>(Owner.GetGameInstance());
	UT66PlayerExperienceSubSystem* PlayerExperience = T66GI ? T66GI->GetSubsystem<UT66PlayerExperienceSubSystem>() : nullptr;
	const ET66Difficulty Difficulty = T66GI ? T66GI->SelectedDifficulty : ET66Difficulty::Easy;
	const ET66Rarity PriorRarity = [Rarity]() -> ET66Rarity
	{
		switch (Rarity)
		{
		case ET66Rarity::Red: return ET66Rarity::Black;
		case ET66Rarity::Yellow: return ET66Rarity::Red;
		case ET66Rarity::White: return ET66Rarity::Yellow;
		case ET66Rarity::Black:
		default: return ET66Rarity::Black;
		}
	}();
	if (PlayerExperience)
	{
		const FT66IntRange GoldRange = PlayerExperience->GetDifficultyChestGoldRange(Difficulty, PriorRarity);
		return FMath::Max(0, FMath::Max(GoldRange.Min, GoldRange.Max) + 1);
	}

	switch (Rarity)
	{
	case ET66Rarity::Red: return 76;
	case ET66Rarity::Yellow: return 181;
	case ET66Rarity::White: return 381;
	case ET66Rarity::Black:
	default: return 0;
	}
}


ET66Rarity FT66HUDPresentationController::ResolveChestRewardDisplayedRarity(const int32 DisplayedGold) const
{
	ET66Rarity ResolvedRarity = ET66Rarity::Black;
	const int32 FinalStageIndex = T66RarityToStageIndex(ActiveChestRewardRarity);
	for (int32 StageIndex = 1; StageIndex <= FinalStageIndex; ++StageIndex)
	{
		const ET66Rarity StageRarity = T66StageIndexToRarity(StageIndex);
		if (DisplayedGold >= GetChestRewardRevealThresholdGold(StageRarity))
		{
			ResolvedRarity = StageRarity;
			continue;
		}

		break;
	}

	return ResolvedRarity;
}


void FT66HUDPresentationController::RefreshChestRewardVisualState()
{
	const ET66Rarity DisplayRarity = ResolveChestRewardDisplayedRarity(FMath::Max(ChestRewardDisplayedGold, ChestRewardMinimumDisplayedGold));
	if (Owner.ChestRewardTileBorder.IsValid())
	{
		const FLinearColor AccentColor = FT66RarityUtil::GetRarityColor(DisplayRarity) * 0.58f + FLinearColor(0.06f, 0.05f, 0.04f, 0.52f);
		Owner.ChestRewardTileBorder->SetBorderBackgroundColor(AccentColor);
	}

	if (Owner.ChestRewardCounterText.IsValid())
	{
		Owner.ChestRewardCounterText->SetText(FText::Format(
			NSLOCTEXT("T66.ChestReward", "GoldCounterFormat", "+{0}"),
			FText::AsNumber(FMath::Max(0, ChestRewardDisplayedGold))));
	}
}


void FT66HUDPresentationController::TickChestRewardPresentation(const float InDeltaTime)
{
	if (!bChestRewardVisible || !Owner.ChestRewardBox.IsValid())
	{
		return;
	}

	ChestRewardElapsedSeconds += InDeltaTime;
	ChestRewardRemainingSeconds = FMath::Max(0.f, ChestRewardRemainingSeconds - InDeltaTime);

	TArray<FT66AnimationMarkerEvent> MarkerEvents;
	static constexpr float CountStartSeconds = 0.52f;
	if (!bChestRewardCountTimelineStarted && ChestRewardElapsedSeconds >= CountStartSeconds)
	{
		bChestRewardCountTimelineStarted = true;
		ChestRewardCountTimeline.Play();
	}
	ChestRewardCountTimeline.Tick(InDeltaTime, MarkerEvents);
	ChestRewardMarkerDispatcher.Dispatch(MarkerEvents);
	if (ChestRewardRemainingSeconds <= 0.f)
	{
		DispatchChestRewardCommitIfNeeded();
	}

	RefreshChestRewardVisualState();
	if (Owner.ChestRewardSkipText.IsValid())
	{
		Owner.ChestRewardSkipText->SetText(BuildSkipCountdownText(ChestRewardRemainingSeconds, FName(TEXT("Interact"))));
	}

	const float FadeAlpha = (ChestRewardRemainingSeconds > UT66GameplayHUDWidget::ChestRewardFadeOutSeconds)
		? 1.f
		: FMath::Clamp(ChestRewardRemainingSeconds / UT66GameplayHUDWidget::ChestRewardFadeOutSeconds, 0.f, 1.f);
	Owner.ChestRewardBox->SetRenderOpacity(FadeAlpha);

	const float PreOpenPulse = FMath::Sin(FMath::Clamp(ChestRewardElapsedSeconds / 0.28f, 0.f, 1.f) * PI);
	const float OpenAlpha = FMath::Clamp((ChestRewardElapsedSeconds - 0.26f) / 0.22f, 0.f, 1.f);
	const float OpenPopAlpha = FMath::Clamp((ChestRewardElapsedSeconds - 0.22f) / 0.36f, 0.f, 1.f);
	const float OpenOvershoot = FMath::Sin(OpenPopAlpha * PI) * 18.f;
	if (Owner.ChestRewardClosedBox.IsValid())
	{
		Owner.ChestRewardClosedBox->SetRenderOpacity(1.f - OpenAlpha);
		Owner.ChestRewardClosedBox->SetWidthOverride(312.f + PreOpenPulse * 18.f);
		Owner.ChestRewardClosedBox->SetHeightOverride(240.f + PreOpenPulse * 10.f);
		Owner.ChestRewardClosedBox->SetRenderTransform(FSlateRenderTransform(FVector2D(0.f, -PreOpenPulse * 7.f)));
	}
	if (Owner.ChestRewardOpenBox.IsValid())
	{
		Owner.ChestRewardOpenBox->SetRenderOpacity(OpenAlpha);
		Owner.ChestRewardOpenBox->SetWidthOverride(360.f + OpenOvershoot);
		Owner.ChestRewardOpenBox->SetHeightOverride(308.f + OpenOvershoot * 0.85f);
		Owner.ChestRewardOpenBox->SetRenderTransform(FSlateRenderTransform(FVector2D(0.f, (1.f - OpenPopAlpha) * 18.f - OpenOvershoot * 0.18f)));
	}

	const float BeamAlpha = FMath::Clamp((ChestRewardElapsedSeconds - 0.30f) / 0.22f, 0.f, 1.f);
	for (int32 BeamIndex = 0; BeamIndex < Owner.ChestRewardBeamBoxes.Num(); ++BeamIndex)
	{
		TSharedPtr<SBox> BeamBox = Owner.ChestRewardBeamBoxes[BeamIndex];
		if (!BeamBox.IsValid())
		{
			continue;
		}

		const float BeamSpread = static_cast<float>(BeamIndex) - (static_cast<float>(Owner.ChestRewardBeamBoxes.Num() - 1) * 0.5f);
		const float SweepDegrees = BeamSpread * 13.f + FMath::Sin(ChestRewardElapsedSeconds * 2.2f + BeamIndex * 0.7f) * 2.5f;
		const float BeamPulse = 0.62f + 0.38f * FMath::Sin(ChestRewardElapsedSeconds * 5.4f + BeamIndex);
		const float BeamWidth = 34.f + FMath::Abs(BeamSpread) * 9.f + BeamPulse * 18.f;
		BeamBox->SetWidthOverride(BeamWidth);
		BeamBox->SetHeightOverride(330.f + BeamPulse * 54.f);
		BeamBox->SetRenderOpacity(BeamAlpha * FadeAlpha * (0.20f + BeamPulse * 0.22f));
		BeamBox->SetRenderTransform(FSlateRenderTransform(FQuat2D(FMath::DegreesToRadians(SweepDegrees))));
		if (Owner.ChestRewardBeamBorders.IsValidIndex(BeamIndex) && Owner.ChestRewardBeamBorders[BeamIndex].IsValid())
		{
			Owner.ChestRewardBeamBorders[BeamIndex]->SetBorderBackgroundColor(FLinearColor(1.f, 0.74f + BeamPulse * 0.18f, 0.18f, 0.20f + BeamPulse * 0.18f));
		}
	}

	const float CoinStartTime = 0.42f;
	const float CoinLifeSeconds = 1.58f;
	for (int32 CoinIndex = 0; CoinIndex < Owner.ChestRewardCoinBoxes.Num(); ++CoinIndex)
	{
		TSharedPtr<SBox> CoinBox = Owner.ChestRewardCoinBoxes[CoinIndex];
		if (!CoinBox.IsValid())
		{
			continue;
		}

		const float PerCoinDelay = CoinStartTime + (CoinIndex * 0.06f);
		const float LocalTime = ChestRewardElapsedSeconds - PerCoinDelay;
		if (LocalTime <= 0.f || LocalTime >= CoinLifeSeconds || ChestRewardRemainingSeconds <= 0.08f)
		{
			CoinBox->SetRenderOpacity(0.f);
			continue;
		}

		const float Progress = FMath::Clamp(LocalTime / CoinLifeSeconds, 0.f, 1.f);
		const float EaseOut = 1.f - FMath::Pow(1.f - Progress, 2.6f);
		const float ArcAlpha = FMath::Sin(Progress * PI);
		const float FanIndex = static_cast<float>((CoinIndex % 14) - 6);
		const float WaveIndex = static_cast<float>(CoinIndex / 14);
		const float Direction = (FanIndex / 6.f) * 1.05f;
		const float Spread = 110.f + (CoinIndex % 5) * 22.f + WaveIndex * 36.f;
		const float LateralDrift = Direction * Spread * EaseOut + FMath::Sin(Progress * PI * 2.f + CoinIndex) * 18.f * ArcAlpha;
		const float VerticalDrift = -42.f - (165.f + (CoinIndex % 7) * 22.f) * EaseOut + FMath::Sin(Progress * PI) * -64.f;
		const float CoinSize = 22.f + ArcAlpha * 18.f + (CoinIndex % 3) * 2.f;

		CoinBox->SetWidthOverride(CoinSize);
		CoinBox->SetHeightOverride(FMath::Max(8.f, CoinSize * (0.62f + 0.38f * FMath::Abs(FMath::Sin(Progress * PI * 4.f + CoinIndex)))));
		CoinBox->SetRenderTransform(FSlateRenderTransform(FVector2D(LateralDrift, VerticalDrift)));
		CoinBox->SetRenderOpacity(ArcAlpha * OpenAlpha * FadeAlpha);
	}

	const float SparkleStartTime = 0.34f;
	const float SparkleLifeSeconds = 1.45f;
	for (int32 SparkleIndex = 0; SparkleIndex < Owner.ChestRewardSparkleBoxes.Num(); ++SparkleIndex)
	{
		TSharedPtr<SBox> SparkleBox = Owner.ChestRewardSparkleBoxes[SparkleIndex];
		if (!SparkleBox.IsValid())
		{
			continue;
		}

		const float PerSparkleDelay = SparkleStartTime + SparkleIndex * 0.045f;
		const float LocalTime = FMath::Fmod(FMath::Max(0.f, ChestRewardElapsedSeconds - PerSparkleDelay), SparkleLifeSeconds);
		if (ChestRewardElapsedSeconds < PerSparkleDelay || ChestRewardRemainingSeconds <= 0.08f)
		{
			SparkleBox->SetRenderOpacity(0.f);
			continue;
		}

		const float Progress = FMath::Clamp(LocalTime / SparkleLifeSeconds, 0.f, 1.f);
		const float GlowAlpha = FMath::Sin(Progress * PI);
		const float Angle = static_cast<float>(SparkleIndex) * 2.399963f;
		const float Radius = (70.f + SparkleIndex * 9.f) * (0.35f + Progress * 0.88f);
		const float X = FMath::Cos(Angle) * Radius;
		const float Y = -72.f - FMath::Abs(FMath::Sin(Angle)) * 130.f - Progress * 96.f;
		const float SparkleSize = 4.f + GlowAlpha * (8.f + (SparkleIndex % 4) * 2.f);
		SparkleBox->SetWidthOverride(SparkleSize);
		SparkleBox->SetHeightOverride(SparkleSize);
		SparkleBox->SetRenderTransform(FSlateRenderTransform(FVector2D(X, Y)));
		SparkleBox->SetRenderOpacity(GlowAlpha * OpenAlpha * FadeAlpha);
		if (Owner.ChestRewardSparkleBorders.IsValidIndex(SparkleIndex) && Owner.ChestRewardSparkleBorders[SparkleIndex].IsValid())
		{
			const float Warm = 0.72f + 0.28f * GlowAlpha;
			Owner.ChestRewardSparkleBorders[SparkleIndex]->SetBorderBackgroundColor(FLinearColor(1.f, Warm, 0.38f, 0.55f + GlowAlpha * 0.38f));
		}
	}

	if (ChestRewardRemainingSeconds <= 0.f)
	{
		HideChestReward();
	}
}


void FT66HUDPresentationController::HideChestReward()
{
	if (bChestRewardVisible)
	{
		DispatchChestRewardCommitIfNeeded();
	}

	bChestRewardVisible = false;
	ChestRewardRemainingSeconds = 0.f;
	ChestRewardElapsedSeconds = 0.f;
	ChestRewardTargetGold = 0;
	ChestRewardDisplayedGold = 0;
	ChestRewardMinimumDisplayedGold = 0;
	bChestRewardCommitDispatched = false;
	bChestRewardCountTimelineStarted = false;
	ChestRewardCountTimeline.Cancel();
	ChestRewardMarkerDispatcher.ClearHandlers();
	if (Owner.ChestRewardBox.IsValid())
	{
		Owner.ChestRewardBox->SetVisibility(EVisibility::Collapsed);
		Owner.ChestRewardBox->SetRenderOpacity(1.f);
	}
	if (Owner.ChestRewardClosedBox.IsValid())
	{
		Owner.ChestRewardClosedBox->SetRenderOpacity(1.f);
		Owner.ChestRewardClosedBox->SetWidthOverride(312.f);
		Owner.ChestRewardClosedBox->SetHeightOverride(240.f);
		Owner.ChestRewardClosedBox->SetRenderTransform(FSlateRenderTransform(FVector2D::ZeroVector));
	}
	if (Owner.ChestRewardOpenBox.IsValid())
	{
		Owner.ChestRewardOpenBox->SetRenderOpacity(0.f);
		Owner.ChestRewardOpenBox->SetWidthOverride(360.f);
		Owner.ChestRewardOpenBox->SetHeightOverride(308.f);
		Owner.ChestRewardOpenBox->SetRenderTransform(FSlateRenderTransform(FVector2D::ZeroVector));
	}
	if (Owner.ChestRewardCounterText.IsValid())
	{
		Owner.ChestRewardCounterText->SetText(FText::GetEmpty());
	}
	if (Owner.ChestRewardSkipText.IsValid())
	{
		Owner.ChestRewardSkipText->SetText(FText::GetEmpty());
	}
	for (const TSharedPtr<SBox>& CoinBox : Owner.ChestRewardCoinBoxes)
	{
		if (CoinBox.IsValid())
		{
			CoinBox->SetVisibility(EVisibility::Collapsed);
			CoinBox->SetRenderOpacity(0.f);
			CoinBox->SetRenderTransform(FSlateRenderTransform(FVector2D::ZeroVector));
		}
	}
	for (const TSharedPtr<SBox>& BeamBox : Owner.ChestRewardBeamBoxes)
	{
		if (BeamBox.IsValid())
		{
			BeamBox->SetVisibility(EVisibility::Collapsed);
			BeamBox->SetRenderOpacity(0.f);
			BeamBox->SetRenderTransform(FSlateRenderTransform(FVector2D::ZeroVector));
		}
	}
	for (const TSharedPtr<SBox>& SparkleBox : Owner.ChestRewardSparkleBoxes)
	{
		if (SparkleBox.IsValid())
		{
			SparkleBox->SetVisibility(EVisibility::Collapsed);
			SparkleBox->SetRenderOpacity(0.f);
			SparkleBox->SetRenderTransform(FSlateRenderTransform(FVector2D::ZeroVector));
		}
	}

	DispatchChestRewardFinishedIfNeeded();
}


void FT66HUDPresentationController::TryShowQueuedPresentation()
{
	if (bResettingPresentations)
	{
		return;
	}

	if (IsRewardPresentationBusy())
	{
		return;
	}

	if (QueuedPresentations.Num() <= 0)
	{
		return;
	}

	FQueuedPresentation NextPresentation = MoveTemp(QueuedPresentations[0]);
	QueuedPresentations.RemoveAt(0);
	UE_LOG(LogT66HUD, Display, TEXT("[HUDPresentationQueue] dequeued type=%d remaining=%d"), static_cast<int32>(NextPresentation.Type), QueuedPresentations.Num());
	switch (NextPresentation.Type)
	{
	case EQueuedPresentationType::CrateOpen:
		StartCrateOpen(NextPresentation.Rarity);
		return;
	case EQueuedPresentationType::LootWheelSpin:
		StartLootWheelSpin(MoveTemp(NextPresentation.LootWheelParams));
		return;
	case EQueuedPresentationType::ChestReward:
		StartChestReward(
			NextPresentation.Rarity,
			NextPresentation.GoldAmount,
			MoveTemp(NextPresentation.OnCommit),
			MoveTemp(NextPresentation.OnFinished));
		return;
	case EQueuedPresentationType::LootBagReveal:
		ShowLootBagItemReveal(NextPresentation.ItemID, NextPresentation.ItemRarity);
		return;
	case EQueuedPresentationType::PickupCard:
	default:
		ShowPickupItemCard(NextPresentation.ItemID, NextPresentation.ItemRarity);
		return;
	}
}


void FT66HUDPresentationController::PopulatePickupCardContent(const FName ItemID, const ET66ItemRarity ItemRarity, const bool bPopulateLootBagProxy)
{
	UT66GameInstance* GI = Cast<UT66GameInstance>(Owner.GetGameInstance());
	UT66LocalizationSubsystem* Loc = Owner.GetGameInstance() ? Owner.GetGameInstance()->GetSubsystem<UT66LocalizationSubsystem>() : nullptr;
	UT66RunStateSubsystem* RunState = Owner.GetRunState();

	FItemData ItemData;
	const bool bHasData = GI && GI->GetItemData(ItemID, ItemData);
	const FText ItemNameText = Loc ? Loc->GetText_ItemDisplayNameForRarity(ItemID, ItemRarity) : FText::FromName(ItemID);

	if (Owner.PickupCardNameText.IsValid())
	{
		Owner.PickupCardNameText->SetText(ItemNameText);
	}
	if (Owner.LootBagRevealCardNameText.IsValid() && bPopulateLootBagProxy)
	{
		Owner.LootBagRevealCardNameText->SetText(ItemNameText);
	}

	if (Owner.PickupCardDescText.IsValid())
	{
		if (!bHasData)
		{
			Owner.PickupCardDescText->SetText(FText::GetEmpty());
		}
		else
		{
			int32 MainValue = 0;
			const float ScaleMult = RunState ? RunState->GetHeroScaleMultiplier() : 1.f;
			float Line2Multiplier = 0.f;
			if (RunState)
			{
				const TArray<FT66InventorySlot>& Slots = RunState->GetInventorySlots();
				if (Slots.Num() > 0 && Slots.Last().ItemTemplateID == ItemID)
				{
					MainValue = Slots.Last().Line1RolledValue;
					Line2Multiplier = Slots.Last().GetLine2Multiplier();
				}
				if (ItemID == FName(TEXT("Item_VendorToken")))
				{
					MainValue = RunState->GetActiveVendorTokenStacks();
				}
			}
			Owner.PickupCardDescText->SetText(T66ItemCardTextUtils::BuildItemCardDescription(Loc, ItemData, ItemRarity, MainValue, ScaleMult, Line2Multiplier));
		}
	}

	if (!Owner.PickupCardIconBrush.IsValid())
	{
		Owner.PickupCardIconBrush = MakeShared<FSlateBrush>();
		Owner.PickupCardIconBrush->DrawAs = ESlateBrushDrawType::Image;
		Owner.PickupCardIconBrush->ImageSize = FVector2D(UT66GameplayHUDWidget::PickupCardWidth, UT66GameplayHUDWidget::PickupCardWidth);
	}
	if (!Owner.LootBagRevealCardIconBrush.IsValid())
	{
		Owner.LootBagRevealCardIconBrush = MakeShared<FSlateBrush>();
		Owner.LootBagRevealCardIconBrush->DrawAs = ESlateBrushDrawType::Image;
		Owner.LootBagRevealCardIconBrush->ImageSize = FVector2D(84.f, 84.f);
	}

	const TSoftObjectPtr<UTexture2D> PickupIconSoft = bHasData ? ItemData.GetIconForRarity(ItemRarity) : TSoftObjectPtr<UTexture2D>();
	if (!PickupIconSoft.IsNull())
	{
		UT66UITexturePoolSubsystem* TexPool = Owner.GetGameInstance() ? Owner.GetGameInstance()->GetSubsystem<UT66UITexturePoolSubsystem>() : nullptr;
		if (TexPool)
		{
			T66SlateTexture::BindSharedBrushAsync(TexPool, PickupIconSoft, &Owner, Owner.PickupCardIconBrush, FName(TEXT("HUDPickupCard")), true);
			if (bPopulateLootBagProxy)
			{
				T66SlateTexture::BindSharedBrushAsync(TexPool, PickupIconSoft, &Owner, Owner.LootBagRevealCardIconBrush, FName(TEXT("HUDLootBagRevealCard")), true);
			}
		}
	}
	if (Owner.PickupCardIconImage.IsValid())
	{
		Owner.PickupCardIconImage->SetImage(Owner.PickupCardIconBrush.Get());
		Owner.PickupCardIconImage->SetVisibility(!PickupIconSoft.IsNull() ? EVisibility::Visible : EVisibility::Collapsed);
	}
	if (Owner.LootBagRevealCardIconImage.IsValid() && bPopulateLootBagProxy)
	{
		Owner.LootBagRevealCardIconImage->SetImage(Owner.LootBagRevealCardIconBrush.Get());
		Owner.LootBagRevealCardIconImage->SetVisibility(!PickupIconSoft.IsNull() ? EVisibility::Visible : EVisibility::Collapsed);
	}

	const FLinearColor AccentColor = bHasData
		? (FItemData::GetItemRarityColor(ItemRarity) * 0.52f + FLinearColor(0.05f, 0.05f, 0.06f, 0.48f))
		: FT66FlatStyle::Tokens::Panel;
	if (Owner.PickupCardTileBorder.IsValid())
	{
		Owner.PickupCardTileBorder->SetBorderBackgroundColor(AccentColor);
	}
	if (Owner.LootBagRevealCardTileBorder.IsValid() && bPopulateLootBagProxy)
	{
		Owner.LootBagRevealCardTileBorder->SetBorderBackgroundColor(AccentColor);
	}
	if (Owner.PickupCardIconBorder.IsValid())
	{
		Owner.PickupCardIconBorder->SetBorderBackgroundColor(FLinearColor(0.04f, 0.04f, 0.05f, 1.f));
	}
	if (Owner.LootBagRevealCardIconBorder.IsValid() && bPopulateLootBagProxy)
	{
		Owner.LootBagRevealCardIconBorder->SetBorderBackgroundColor(FLinearColor(0.04f, 0.04f, 0.05f, 1.f));
	}
	if (Owner.PickupCardSkipText.IsValid())
	{
		const FText KeyText = GetActionKeyText(FName(TEXT("Interact")));
		Owner.PickupCardSkipText->SetText(
			KeyText.IsEmpty()
				? NSLOCTEXT("T66.Presentation", "PickupCardCloseInteract", "Interact to close")
				: FText::Format(NSLOCTEXT("T66.Presentation", "PickupCardCloseKey", "Close: {0}"), KeyText));
	}
}


void FT66HUDPresentationController::ShowPickupItemCard(const FName ItemID, const ET66ItemRarity ItemRarity)
{
	if (bResettingPresentations)
	{
		return;
	}

	if (ItemID.IsNone() || !Owner.PickupCardBox.IsValid())
	{
		return;
	}
	if (IsRewardPresentationBusy())
	{
		FQueuedPresentation& QueuedPresentation = QueuedPresentations.AddDefaulted_GetRef();
		QueuedPresentation.Type = EQueuedPresentationType::PickupCard;
		QueuedPresentation.ItemID = ItemID;
		QueuedPresentation.ItemRarity = ItemRarity;
		UE_LOG(LogT66HUD, Display, TEXT("[HUDPresentationQueue] queued type=PickupCard item=%s pending=%d"), *ItemID.ToString(), QueuedPresentations.Num());
		return;
	}

	UWorld* World = Owner.GetWorld();
	if (!World)
	{
		return;
	}

	PopulatePickupCardContent(ItemID, ItemRarity, false);

	ActivePickupCardItemID = ItemID;
	ActivePickupCardRarity = ItemRarity;
	bPickupCardVisible = true;
	Owner.PickupCardBox->SetVisibility(EVisibility::Visible);
	Owner.PickupCardBox->SetRenderOpacity(1.f);
	Owner.PickupCardBox->SetRenderTransform(FSlateRenderTransform(FVector2D::ZeroVector));
}


void FT66HUDPresentationController::ShowLootBagItemReveal(const FName ItemID, const ET66ItemRarity ItemRarity)
{
	if (bResettingPresentations)
	{
		return;
	}

	if (ItemID.IsNone() || !Owner.LootBagRevealBox.IsValid() || !Owner.PickupCardBox.IsValid())
	{
		ShowPickupItemCard(ItemID, ItemRarity);
		return;
	}
	if (IsRewardPresentationBusy())
	{
		FQueuedPresentation& QueuedPresentation = QueuedPresentations.AddDefaulted_GetRef();
		QueuedPresentation.Type = EQueuedPresentationType::LootBagReveal;
		QueuedPresentation.ItemID = ItemID;
		QueuedPresentation.ItemRarity = ItemRarity;
		UE_LOG(LogT66HUD, Display, TEXT("[HUDPresentationQueue] queued type=LootBagReveal item=%s pending=%d"), *ItemID.ToString(), QueuedPresentations.Num());
		return;
	}

	UWorld* World = Owner.GetWorld();
	if (!World)
	{
		ShowPickupItemCard(ItemID, ItemRarity);
		return;
	}

	HidePickupCard();
	ResetLootBagRevealWidgets();
	PopulatePickupCardContent(ItemID, ItemRarity, true);

	const FVector2D ClosedSize(260.f, 246.f);
	const FVector2D OpenSize(286.f, 270.f);
	if (Owner.LootBagRevealClosedBrush.IsValid())
	{
		BindRuntimeHudBrush(Owner.LootBagRevealClosedBrush, GetLootBagRevealClosedRelativePath(), ClosedSize);
	}
	if (Owner.LootBagRevealOpenBrush.IsValid())
	{
		BindRuntimeHudBrush(Owner.LootBagRevealOpenBrush, GetLootBagRevealOpenRelativePath(), OpenSize);
	}

	const bool bHasClosedBag = Owner.LootBagRevealClosedBrush.IsValid() && Owner.LootBagRevealClosedBrush->GetResourceObject();
	const bool bHasOpenBag = Owner.LootBagRevealOpenBrush.IsValid() && Owner.LootBagRevealOpenBrush->GetResourceObject();
	if (!bHasClosedBag || !bHasOpenBag)
	{
		UE_LOG(LogT66HUD, Warning, TEXT("[LootBagReveal] Missing generated bag art (closed=%d open=%d); falling back to direct item card. This does not satisfy the LootBag animation gate."), bHasClosedBag ? 1 : 0, bHasOpenBag ? 1 : 0);
		ShowPickupItemCard(ItemID, ItemRarity);
		return;
	}

	if (Owner.LootBagRevealClosedImage.IsValid())
	{
		Owner.LootBagRevealClosedImage->SetImage(Owner.LootBagRevealClosedBrush.Get());
	}
	if (Owner.LootBagRevealOpenImage.IsValid())
	{
		Owner.LootBagRevealOpenImage->SetImage(Owner.LootBagRevealOpenBrush.Get());
	}

	ActiveLootBagRevealItemID = ItemID;
	ActiveLootBagRevealRarity = ItemRarity;
	ActivePickupCardItemID = ItemID;
	ActivePickupCardRarity = ItemRarity;
	LootBagRevealElapsedSeconds = 0.f;
	LootBagRevealRemainingSeconds = UT66GameplayHUDWidget::LootBagRevealDisplaySeconds;
	bLootBagRevealVisible = true;
	bLootBagRevealHandoffComplete = false;

	if (Owner.PickupCardBox.IsValid())
	{
		Owner.PickupCardBox->SetVisibility(EVisibility::Collapsed);
		Owner.PickupCardBox->SetRenderOpacity(1.f);
		Owner.PickupCardBox->SetRenderTransform(FSlateRenderTransform(FVector2D::ZeroVector));
	}
	if (Owner.LootBagRevealBox.IsValid())
	{
		Owner.LootBagRevealBox->SetVisibility(EVisibility::Visible);
		Owner.LootBagRevealBox->SetRenderOpacity(1.f);
	}
	if (Owner.LootBagRevealCardBox.IsValid())
	{
		Owner.LootBagRevealCardBox->SetVisibility(EVisibility::Visible);
	}
	if (Owner.LootBagRevealClosedBox.IsValid())
	{
		Owner.LootBagRevealClosedBox->SetVisibility(EVisibility::Visible);
	}
	if (Owner.LootBagRevealOpenBox.IsValid())
	{
		Owner.LootBagRevealOpenBox->SetVisibility(EVisibility::Visible);
	}
	for (const TSharedPtr<SBox>& SparkleBox : Owner.LootBagRevealSparkleBoxes)
	{
		if (SparkleBox.IsValid())
		{
			SparkleBox->SetVisibility(EVisibility::Visible);
		}
	}

	UE_LOG(LogT66HUD, Display, TEXT("[LootBagReveal] started item=%s rarity=%d via ShowLootBagItemReveal."), *ItemID.ToString(), static_cast<int32>(ItemRarity));
}


void FT66HUDPresentationController::TickLootBagRevealPresentation(const float InDeltaTime)
{
	if (!bLootBagRevealVisible || !Owner.LootBagRevealBox.IsValid())
	{
		return;
	}

	LootBagRevealElapsedSeconds += InDeltaTime;
	LootBagRevealRemainingSeconds = FMath::Max(0.f, LootBagRevealRemainingSeconds - InDeltaTime);

	static constexpr float OpenStartSeconds = 0.24f;
	static constexpr float OpenDurationSeconds = 0.24f;
	static constexpr float CardStartSeconds = 0.38f;
	static constexpr float CardTravelSeconds = 0.74f;
	static constexpr float HandoffSeconds = 1.28f;

	const float FadeAlpha = (LootBagRevealRemainingSeconds > UT66GameplayHUDWidget::LootBagRevealFadeOutSeconds)
		? 1.f
		: FMath::Clamp(LootBagRevealRemainingSeconds / UT66GameplayHUDWidget::LootBagRevealFadeOutSeconds, 0.f, 1.f);
	Owner.LootBagRevealBox->SetRenderOpacity(FadeAlpha);

	const float PreOpenPulse = FMath::Sin(FMath::Clamp(LootBagRevealElapsedSeconds / OpenStartSeconds, 0.f, 1.f) * PI);
	const float OpenAlpha = FMath::Clamp((LootBagRevealElapsedSeconds - OpenStartSeconds) / OpenDurationSeconds, 0.f, 1.f);
	const float OpenPopAlpha = 1.f - FMath::Pow(1.f - OpenAlpha, 3.f);
	if (Owner.LootBagRevealClosedBox.IsValid())
	{
		Owner.LootBagRevealClosedBox->SetRenderOpacity((1.f - OpenAlpha) * FadeAlpha);
		Owner.LootBagRevealClosedBox->SetWidthOverride(260.f + PreOpenPulse * 14.f);
		Owner.LootBagRevealClosedBox->SetHeightOverride(246.f + PreOpenPulse * 10.f);
		Owner.LootBagRevealClosedBox->SetRenderTransform(FSlateRenderTransform(FVector2D(0.f, -PreOpenPulse * 8.f)));
	}
	if (Owner.LootBagRevealOpenBox.IsValid())
	{
		const float OpenOvershoot = FMath::Sin(OpenAlpha * PI) * 18.f;
		Owner.LootBagRevealOpenBox->SetRenderOpacity(OpenAlpha * FadeAlpha);
		Owner.LootBagRevealOpenBox->SetWidthOverride(286.f + OpenOvershoot);
		Owner.LootBagRevealOpenBox->SetHeightOverride(270.f + OpenOvershoot * 0.85f);
		Owner.LootBagRevealOpenBox->SetRenderTransform(FSlateRenderTransform(FVector2D(0.f, (1.f - OpenPopAlpha) * 18.f - OpenOvershoot * 0.12f)));
	}

	const float CardProgress = FMath::Clamp((LootBagRevealElapsedSeconds - CardStartSeconds) / CardTravelSeconds, 0.f, 1.f);
	const float CardEase = 1.f - FMath::Pow(1.f - CardProgress, 3.f);
	if (Owner.LootBagRevealCardBox.IsValid())
	{
		const float CardY = (1.f - CardEase) * 126.f;
		const float CardOpacity = FMath::Clamp((CardProgress - 0.04f) / 0.22f, 0.f, 1.f);
		Owner.LootBagRevealCardBox->SetRenderOpacity(CardOpacity * FadeAlpha);
		Owner.LootBagRevealCardBox->SetRenderTransform(FSlateRenderTransform(FVector2D(0.f, CardY)));
		Owner.LootBagRevealCardBox->SetWidthOverride(UT66GameplayHUDWidget::PickupCardWidth);
		Owner.LootBagRevealCardBox->SetHeightOverride(UT66GameplayHUDWidget::PickupCardHeight);
	}

	for (int32 SparkleIndex = 0; SparkleIndex < Owner.LootBagRevealSparkleBoxes.Num(); ++SparkleIndex)
	{
		TSharedPtr<SBox> SparkleBox = Owner.LootBagRevealSparkleBoxes[SparkleIndex];
		if (!SparkleBox.IsValid())
		{
			continue;
		}

		const float Delay = OpenStartSeconds + SparkleIndex * 0.045f;
		const float LocalTime = LootBagRevealElapsedSeconds - Delay;
		if (LocalTime <= 0.f)
		{
			SparkleBox->SetRenderOpacity(0.f);
			continue;
		}

		const float Progress = FMath::Clamp(LocalTime / 0.92f, 0.f, 1.f);
		const float GlowAlpha = FMath::Sin(Progress * PI);
		const float Angle = static_cast<float>(SparkleIndex) * 2.399963f;
		const float Radius = 30.f + Progress * (58.f + SparkleIndex * 4.f);
		const float X = FMath::Cos(Angle) * Radius;
		const float Y = -74.f - FMath::Abs(FMath::Sin(Angle)) * 72.f - Progress * 82.f;
		const float Size = 5.f + GlowAlpha * (9.f + static_cast<float>(SparkleIndex % 3) * 2.f);
		SparkleBox->SetWidthOverride(Size);
		SparkleBox->SetHeightOverride(Size);
		SparkleBox->SetRenderTransform(FSlateRenderTransform(FVector2D(X, Y)));
		SparkleBox->SetRenderOpacity(GlowAlpha * OpenAlpha * FadeAlpha);
		if (Owner.LootBagRevealSparkleBorders.IsValidIndex(SparkleIndex) && Owner.LootBagRevealSparkleBorders[SparkleIndex].IsValid())
		{
			Owner.LootBagRevealSparkleBorders[SparkleIndex]->SetBorderBackgroundColor(FLinearColor(1.f, 0.78f + GlowAlpha * 0.18f, 0.26f, 0.58f + GlowAlpha * 0.28f));
		}
	}

	if (LootBagRevealElapsedSeconds >= HandoffSeconds || LootBagRevealRemainingSeconds <= 0.f)
	{
		CompleteLootBagRevealToPickupCard();
	}
}


void FT66HUDPresentationController::CompleteLootBagRevealToPickupCard()
{
	if (!bLootBagRevealVisible || bLootBagRevealHandoffComplete)
	{
		return;
	}

	bLootBagRevealHandoffComplete = true;
	bLootBagRevealVisible = false;

	ResetLootBagRevealWidgets();
	if (Owner.PickupCardBox.IsValid())
	{
		Owner.PickupCardBox->SetVisibility(EVisibility::Visible);
		Owner.PickupCardBox->SetRenderOpacity(1.f);
		Owner.PickupCardBox->SetRenderTransform(FSlateRenderTransform(FVector2D::ZeroVector));
	}

	bPickupCardVisible = true;
	ActivePickupCardItemID = ActiveLootBagRevealItemID;
	ActivePickupCardRarity = ActiveLootBagRevealRarity;
	ActiveLootBagRevealItemID = NAME_None;
	ActiveLootBagRevealRarity = ET66ItemRarity::Black;
	LootBagRevealElapsedSeconds = 0.f;
	LootBagRevealRemainingSeconds = 0.f;

	UE_LOG(LogT66HUD, Display, TEXT("[LootBagReveal] handoff complete to pickup card."));
}


void FT66HUDPresentationController::HideLootBagReveal()
{
	bLootBagRevealVisible = false;
	bLootBagRevealHandoffComplete = false;
	ActiveLootBagRevealItemID = NAME_None;
	ActiveLootBagRevealRarity = ET66ItemRarity::Black;
	LootBagRevealElapsedSeconds = 0.f;
	LootBagRevealRemainingSeconds = 0.f;
	ResetLootBagRevealWidgets();
}


void FT66HUDPresentationController::ResetLootBagRevealWidgets()
{
	if (Owner.LootBagRevealBox.IsValid())
	{
		Owner.LootBagRevealBox->SetVisibility(EVisibility::Collapsed);
		Owner.LootBagRevealBox->SetRenderOpacity(1.f);
	}
	if (Owner.LootBagRevealClosedBox.IsValid())
	{
		Owner.LootBagRevealClosedBox->SetVisibility(EVisibility::Collapsed);
		Owner.LootBagRevealClosedBox->SetRenderOpacity(1.f);
		Owner.LootBagRevealClosedBox->SetWidthOverride(260.f);
		Owner.LootBagRevealClosedBox->SetHeightOverride(246.f);
		Owner.LootBagRevealClosedBox->SetRenderTransform(FSlateRenderTransform(FVector2D::ZeroVector));
	}
	if (Owner.LootBagRevealOpenBox.IsValid())
	{
		Owner.LootBagRevealOpenBox->SetVisibility(EVisibility::Collapsed);
		Owner.LootBagRevealOpenBox->SetRenderOpacity(0.f);
		Owner.LootBagRevealOpenBox->SetWidthOverride(286.f);
		Owner.LootBagRevealOpenBox->SetHeightOverride(270.f);
		Owner.LootBagRevealOpenBox->SetRenderTransform(FSlateRenderTransform(FVector2D::ZeroVector));
	}
	if (Owner.LootBagRevealCardBox.IsValid())
	{
		Owner.LootBagRevealCardBox->SetVisibility(EVisibility::Collapsed);
		Owner.LootBagRevealCardBox->SetRenderOpacity(0.f);
		Owner.LootBagRevealCardBox->SetWidthOverride(UT66GameplayHUDWidget::PickupCardWidth);
		Owner.LootBagRevealCardBox->SetHeightOverride(UT66GameplayHUDWidget::PickupCardHeight);
		Owner.LootBagRevealCardBox->SetRenderTransform(FSlateRenderTransform(FVector2D::ZeroVector));
	}
	for (const TSharedPtr<SBox>& SparkleBox : Owner.LootBagRevealSparkleBoxes)
	{
		if (SparkleBox.IsValid())
		{
			SparkleBox->SetVisibility(EVisibility::Collapsed);
			SparkleBox->SetRenderOpacity(0.f);
			SparkleBox->SetRenderTransform(FSlateRenderTransform(FVector2D::ZeroVector));
		}
	}
	if (Owner.LootBagRevealCardNameText.IsValid())
	{
		Owner.LootBagRevealCardNameText->SetText(FText::GetEmpty());
	}
}


void FT66HUDPresentationController::HidePickupCard()
{
	bPickupCardVisible = false;
	ActivePickupCardItemID = NAME_None;
	ActivePickupCardRarity = ET66ItemRarity::Black;
	if (Owner.PickupCardBox.IsValid())
	{
		Owner.PickupCardBox->SetVisibility(EVisibility::Collapsed);
		Owner.PickupCardBox->SetRenderOpacity(1.f);
		Owner.PickupCardBox->SetRenderTransform(FSlateRenderTransform(FVector2D::ZeroVector));
	}
	if (Owner.PickupCardSkipText.IsValid())
	{
		Owner.PickupCardSkipText->SetText(FText::GetEmpty());
	}
}
