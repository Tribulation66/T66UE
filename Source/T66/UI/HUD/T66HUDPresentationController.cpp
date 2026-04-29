// Copyright Tribulation 66. All Rights Reserved.

#include "UI/HUD/T66HUDPresentationController.h"
#include "UI/HUD/T66GameplayHUDWidget_Private.h"

FT66HUDPresentationController::FT66HUDPresentationController(UT66GameplayHUDWidget& InOwner)
	: Owner(InOwner)
{
}


void FT66HUDPresentationController::Tick(float InDeltaTime)
{
	TickChestRewardPresentation(InDeltaTime);
	TryShowQueuedPresentation();
}


void FT66HUDPresentationController::Reset()
{
	if (UWorld* World = Owner.GetWorld())
	{
		World->GetTimerManager().ClearTimer(WheelSpinTickHandle);
		World->GetTimerManager().ClearTimer(WheelResolveHandle);
		World->GetTimerManager().ClearTimer(WheelCloseHandle);
		World->GetTimerManager().ClearTimer(AchievementNotificationTimerHandle);
	}

	HidePickupCard();
	HideChestReward();
	QueuedChestRewards.Reset();
	QueuedPickupCards.Reset();
	AchievementNotificationQueue.Reset();

	if (UT66CrateOverlayWidget* Overlay = ActiveCrateOverlay.Get())
	{
		Overlay->RemoveFromParent();
	}
	ActiveCrateOverlay.Reset();

	if (Owner.AchievementNotificationBox.IsValid())
	{
		Owner.AchievementNotificationBox->SetVisibility(EVisibility::Collapsed);
	}
	if (Owner.AchievementNotificationTitleText.IsValid())
	{
		Owner.AchievementNotificationTitleText->SetText(FText::GetEmpty());
	}

	CloseWheelSpin();
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


void FT66HUDPresentationController::QueueActivePickupCardToFront()
{
	if (bPickupCardVisible && !ActivePickupCardItemID.IsNone())
	{
		QueuedPickupCards.InsertDefaulted(0, 1);
		FQueuedPickupCard& QueuedPickup = QueuedPickupCards[0];
		QueuedPickup.ItemID = ActivePickupCardItemID;
		QueuedPickup.ItemRarity = ActivePickupCardRarity;
	}
}


void FT66HUDPresentationController::StartWheelSpin(const ET66Rarity WheelRarity)
{
	UWorld* World = Owner.GetWorld();
	if (!World)
	{
		return;
	}

	if (!Owner.WheelSpinBox.IsValid() || !Owner.WheelSpinDisk.IsValid() || !Owner.WheelSpinText.IsValid() || !Owner.WheelSpinSkipText.IsValid())
	{
		return;
	}

	if (ActiveCrateOverlay.IsValid() || bChestRewardVisible || bWheelPanelOpen)
	{
		return;
	}

	QueueActivePickupCardToFront();
	HidePickupCard();

	World->GetTimerManager().ClearTimer(WheelSpinTickHandle);
	World->GetTimerManager().ClearTimer(WheelResolveHandle);
	World->GetTimerManager().ClearTimer(WheelCloseHandle);

	ActiveWheelRarity = WheelRarity;
	bWheelPanelOpen = true;
	bWheelSpinning = true;
	WheelSpinElapsed = 0.f;
	WheelSpinDuration = UT66GameplayHUDWidget::ChestRewardDisplaySeconds;
	WheelStartAngleDeg = 0.f;
	WheelTotalAngleDeg = 0.f;
	WheelLastTickTimeSeconds = static_cast<float>(World->GetTimeSeconds());

	FRandomStream SpinRng(static_cast<int32>(FPlatformTime::Cycles()));

	int32 PendingGold = 50;
	int32 MinGold = 0;
	int32 MaxGold = 0;
	if (UGameInstance* GI = Owner.GetGameInstance())
	{
		if (UT66RngSubsystem* RngSub = GI->GetSubsystem<UT66RngSubsystem>())
		{
			UT66RunStateSubsystem* RunState = Owner.GetRunState();
			if (RunState)
			{
				RngSub->UpdateLuckStat(RunState->GetEffectiveLuckBiasStat());
			}

			if (UT66GameInstance* T66GI = Cast<UT66GameInstance>(GI))
			{
				if (UT66PlayerExperienceSubSystem* PlayerExperience = GI->GetSubsystem<UT66PlayerExperienceSubSystem>())
				{
					const FT66FloatRange Range = PlayerExperience->GetDifficultyWheelGoldRange(T66GI->SelectedDifficulty, WheelRarity);
					MinGold = FMath::FloorToInt(FMath::Min(Range.Min, Range.Max));
					MaxGold = FMath::CeilToInt(FMath::Max(Range.Min, Range.Max));

					FRandomStream& Stream = RngSub->GetRunStream();
					PendingGold = FMath::Max(0, FMath::RoundToInt(RngSub->RollFloatRangeBiased(Range, Stream)));
					const int32 DrawIndex = RngSub->GetLastRunDrawIndex();
					const int32 PreDrawSeed = RngSub->GetLastRunPreDrawSeed();
					if (RunState)
					{
						RunState->RecordLuckQuantityFloatRollRounded(
							FName(TEXT("WheelGold")),
							PendingGold,
							MinGold,
							MaxGold,
							Range.Min,
							Range.Max,
							DrawIndex,
							PreDrawSeed);
					}
				}
			}
		}
	}
	WheelPendingGold = PendingGold;

	if (Owner.GoldCurrencyBrush.IsValid())
	{
		BindRuntimeHudBrush(Owner.GoldCurrencyBrush, GetGoldCurrencyRelativePath(), FVector2D(32.f, 32.f));
	}

	Owner.WheelSpinDisk->SetColorAndOpacity(FT66RarityUtil::GetRarityColor(WheelRarity));
	Owner.WheelSpinDisk->SetRenderTransformPivot(FVector2D(0.5f, 0.5f));
	Owner.WheelSpinDisk->SetRenderTransform(FSlateRenderTransform(FVector2D::ZeroVector));
	Owner.WheelSpinText->SetText(FText::Format(
		NSLOCTEXT("T66.Wheel", "GoldCounterFormat", "+{0}"),
		FText::AsNumber(0)));
	Owner.WheelSpinSkipText->SetText(BuildSkipCountdownText(WheelSpinDuration, FName(TEXT("Interact"))));
	Owner.WheelSpinBox->SetVisibility(EVisibility::Visible);
	Owner.WheelSpinBox->SetRenderOpacity(1.f);

	WheelTotalAngleDeg = static_cast<float>(SpinRng.RandRange(5, 9)) * 360.f + static_cast<float>(SpinRng.RandRange(0, 359));

	World->GetTimerManager().SetTimer(
		WheelSpinTickHandle,
		FTimerDelegate::CreateRaw(this, &FT66HUDPresentationController::TickWheelSpin),
		0.033f,
		true);
}


void FT66HUDPresentationController::StartCrateOpen()
{
	APlayerController* PC = Owner.GetOwningPlayer();
	if (!PC)
	{
		return;
	}

	HidePickupCard();
	if (ActiveCrateOverlay.IsValid() || bChestRewardVisible || bWheelPanelOpen)
	{
		return;
	}

	UT66CrateOverlayWidget* Overlay = CreateWidget<UT66CrateOverlayWidget>(PC, UT66CrateOverlayWidget::StaticClass());
	if (Overlay)
	{
		Overlay->SetPresentationHost(&Owner);
		Overlay->SetVisibility(ESlateVisibility::HitTestInvisible);
		Overlay->AddToViewport(100);
		ActiveCrateOverlay = Overlay;
	}
}


void FT66HUDPresentationController::StartChestReward(const ET66Rarity ChestRarity, const int32 GoldAmount)
{
	if (!Owner.ChestRewardBox.IsValid())
	{
		return;
	}

	if (ActiveCrateOverlay.IsValid() || bChestRewardVisible || bWheelPanelOpen)
	{
		FQueuedChestReward& QueuedReward = QueuedChestRewards.AddDefaulted_GetRef();
		QueuedReward.Rarity = ChestRarity;
		QueuedReward.GoldAmount = GoldAmount;
		return;
	}

	QueueActivePickupCardToFront();
	HidePickupCard();

	if (Owner.GoldCurrencyBrush.IsValid())
	{
		BindRuntimeHudBrush(Owner.GoldCurrencyBrush, GetGoldCurrencyRelativePath(), FVector2D(32.f, 32.f));
	}

	const FVector2D ChestImageSize(108.f, 108.f);
	static constexpr ET66Rarity ChestPresentationArtRarity = ET66Rarity::Yellow;
	if (Owner.ChestRewardClosedBrush.IsValid())
	{
		BindRuntimeHudBrush(Owner.ChestRewardClosedBrush, GetChestRewardClosedRelativePath(ChestPresentationArtRarity), ChestImageSize);
		if (!Owner.ChestRewardClosedBrush->GetResourceObject())
		{
			BindHudAssetBrush(Owner.ChestRewardClosedBrush, GetChestRewardFallbackAssetPath(ChestPresentationArtRarity), ChestImageSize);
		}
		if (!Owner.ChestRewardClosedBrush->GetResourceObject())
		{
			BindRuntimeHudBrush(Owner.ChestRewardClosedBrush, GetChestRewardFallbackRelativePath(ChestPresentationArtRarity), ChestImageSize);
		}
	}
	if (Owner.ChestRewardOpenBrush.IsValid())
	{
		BindRuntimeHudBrush(Owner.ChestRewardOpenBrush, GetChestRewardOpenRelativePath(ChestPresentationArtRarity), ChestImageSize);
		if (!Owner.ChestRewardOpenBrush->GetResourceObject())
		{
			BindHudAssetBrush(Owner.ChestRewardOpenBrush, GetChestRewardFallbackAssetPath(ChestPresentationArtRarity), ChestImageSize);
		}
		if (!Owner.ChestRewardOpenBrush->GetResourceObject())
		{
			BindRuntimeHudBrush(Owner.ChestRewardOpenBrush, GetChestRewardFallbackRelativePath(ChestPresentationArtRarity), ChestImageSize);
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
	}
	if (Owner.ChestRewardOpenBox.IsValid())
	{
		Owner.ChestRewardOpenBox->SetRenderOpacity(0.f);
	}

	ActiveChestRewardRarity = ChestRarity;
	ChestRewardTargetGold = FMath::Max(0, GoldAmount);
	ChestRewardDisplayedGold = 0;
	ChestRewardMinimumDisplayedGold = 0;
	ChestRewardElapsedSeconds = 0.f;
	ChestRewardRemainingSeconds = UT66GameplayHUDWidget::ChestRewardDisplaySeconds;
	bChestRewardVisible = true;

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
			Owner.ChestRewardCoinImages[CoinIndex]->SetImage(Owner.GoldCurrencyBrush.Get());
		}
	}
}


bool FT66HUDPresentationController::TrySkipActivePresentation()
{
	if (UT66CrateOverlayWidget* Overlay = ActiveCrateOverlay.Get())
	{
		Overlay->RequestSkip();
		return true;
	}

	if (bWheelPanelOpen)
	{
		if (bWheelSpinning)
		{
			WheelSpinElapsed = WheelSpinDuration;
			if (Owner.WheelSpinText.IsValid())
			{
				Owner.WheelSpinText->SetText(FText::Format(
					NSLOCTEXT("T66.Wheel", "GoldCounterFormat", "+{0}"),
					FText::AsNumber(WheelPendingGold)));
			}
			ResolveWheelSpin();
		}
		else
		{
			CloseWheelSpin();
		}
		return true;
	}

	if (bChestRewardVisible)
	{
		const ET66Rarity CurrentDisplayRarity = ResolveChestRewardDisplayedRarity(FMath::Max(ChestRewardDisplayedGold, ChestRewardMinimumDisplayedGold));
		if (CurrentDisplayRarity == ActiveChestRewardRarity)
		{
			ChestRewardDisplayedGold = ChestRewardTargetGold;
			ChestRewardMinimumDisplayedGold = ChestRewardTargetGold;
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


void FT66HUDPresentationController::TickWheelSpin()
{
	if (!bWheelSpinning || !Owner.WheelSpinDisk.IsValid())
	{
		return;
	}

	UWorld* World = Owner.GetWorld();
	if (!World)
	{
		return;
	}

	const float Now = static_cast<float>(World->GetTimeSeconds());
	float Delta = Now - WheelLastTickTimeSeconds;
	WheelLastTickTimeSeconds = Now;
	Delta = FMath::Clamp(Delta, 0.f, 0.05f);

	WheelSpinElapsed += Delta;
	const float Alpha = FMath::Clamp(WheelSpinElapsed / FMath::Max(0.01f, WheelSpinDuration), 0.f, 1.f);
	const float Ease = FMath::InterpEaseOut(0.f, 1.f, Alpha, 4.2f);
	const float Angle = WheelStartAngleDeg + (WheelTotalAngleDeg * Ease);

	Owner.WheelSpinDisk->SetRenderTransformPivot(FVector2D(0.5f, 0.5f));
	Owner.WheelSpinDisk->SetRenderTransform(FSlateRenderTransform(FTransform2D(FQuat2D(FMath::DegreesToRadians(Angle)))));
	if (Owner.WheelSpinText.IsValid())
	{
		const float CounterEase = FMath::InterpEaseOut(0.f, 1.f, Alpha, 2.5f);
		const int32 DisplayedGold = FMath::RoundToInt(static_cast<float>(WheelPendingGold) * CounterEase);
		Owner.WheelSpinText->SetText(FText::Format(
			NSLOCTEXT("T66.Wheel", "GoldCounterFormat", "+{0}"),
			FText::AsNumber(DisplayedGold)));
	}
	if (Owner.WheelSpinSkipText.IsValid())
	{
		Owner.WheelSpinSkipText->SetText(BuildSkipCountdownText(FMath::Max(0.f, WheelSpinDuration - WheelSpinElapsed), FName(TEXT("Interact"))));
	}

	if (Alpha >= 1.f)
	{
		ResolveWheelSpin();
	}
}


void FT66HUDPresentationController::ResolveWheelSpin()
{
	if (!bWheelSpinning)
	{
		return;
	}

	bWheelSpinning = false;

	if (UWorld* World = Owner.GetWorld())
	{
		World->GetTimerManager().ClearTimer(WheelSpinTickHandle);
	}

	if (UT66RunStateSubsystem* RunState = Owner.GetRunState())
	{
		if (WheelPendingGold > 0)
		{
			RunState->AddGold(WheelPendingGold);
		}
	}

	CloseWheelSpin();
}


void FT66HUDPresentationController::CloseWheelSpin()
{
	bWheelPanelOpen = false;
	bWheelSpinning = false;
	WheelSpinElapsed = 0.f;
	WheelPendingGold = 0;
	WheelLastTickTimeSeconds = 0.f;
	WheelTotalAngleDeg = 0.f;
	if (UWorld* World = Owner.GetWorld())
	{
		World->GetTimerManager().ClearTimer(WheelSpinTickHandle);
		World->GetTimerManager().ClearTimer(WheelResolveHandle);
		World->GetTimerManager().ClearTimer(WheelCloseHandle);
	}
	if (Owner.WheelSpinBox.IsValid())
	{
		Owner.WheelSpinBox->SetVisibility(EVisibility::Collapsed);
		Owner.WheelSpinBox->SetRenderOpacity(1.f);
	}
	if (Owner.WheelSpinDisk.IsValid())
	{
		Owner.WheelSpinDisk->SetRenderTransform(FSlateRenderTransform(FVector2D::ZeroVector));
	}
	if (Owner.WheelSpinText.IsValid())
	{
		Owner.WheelSpinText->SetText(FText::GetEmpty());
	}
	if (Owner.WheelSpinSkipText.IsValid())
	{
		Owner.WheelSpinSkipText->SetText(FText::GetEmpty());
	}

	TryShowQueuedPresentation();
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

	const float CounterProgress = FMath::Clamp(ChestRewardElapsedSeconds / 1.05f, 0.f, 1.f);
	const float CounterEase = FMath::InterpEaseOut(0.f, 1.f, CounterProgress, 2.5f);
	ChestRewardDisplayedGold = FMath::RoundToInt(static_cast<float>(ChestRewardTargetGold) * CounterEase);
	ChestRewardDisplayedGold = FMath::Max(ChestRewardDisplayedGold, ChestRewardMinimumDisplayedGold);
	if (ChestRewardRemainingSeconds <= 0.f)
	{
		ChestRewardDisplayedGold = ChestRewardTargetGold;
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

	const float OpenAlpha = FMath::Clamp((ChestRewardElapsedSeconds - 0.08f) / 0.16f, 0.f, 1.f);
	if (Owner.ChestRewardClosedBox.IsValid())
	{
		Owner.ChestRewardClosedBox->SetRenderOpacity(1.f - OpenAlpha);
		Owner.ChestRewardClosedBox->SetWidthOverride(92.f + (1.f - OpenAlpha) * 12.f);
		Owner.ChestRewardClosedBox->SetHeightOverride(92.f + (1.f - OpenAlpha) * 12.f);
		Owner.ChestRewardClosedBox->SetRenderTransform(FSlateRenderTransform(FVector2D(0.f, (1.f - OpenAlpha) * 4.f)));
	}
	if (Owner.ChestRewardOpenBox.IsValid())
	{
		const float OpenScaleAlpha = FMath::Clamp((ChestRewardElapsedSeconds - 0.02f) / 0.22f, 0.f, 1.f);
		Owner.ChestRewardOpenBox->SetRenderOpacity(OpenAlpha);
		Owner.ChestRewardOpenBox->SetWidthOverride(96.f + OpenScaleAlpha * 16.f);
		Owner.ChestRewardOpenBox->SetHeightOverride(96.f + OpenScaleAlpha * 16.f);
		Owner.ChestRewardOpenBox->SetRenderTransform(FSlateRenderTransform(FVector2D(0.f, (1.f - OpenScaleAlpha) * 8.f)));
	}

	const float CoinStartTime = 0.12f;
	const float CoinLoopSeconds = 1.1f;
	for (int32 CoinIndex = 0; CoinIndex < Owner.ChestRewardCoinBoxes.Num(); ++CoinIndex)
	{
		TSharedPtr<SBox> CoinBox = Owner.ChestRewardCoinBoxes[CoinIndex];
		if (!CoinBox.IsValid())
		{
			continue;
		}

		const float PerCoinDelay = CoinStartTime + (CoinIndex * 0.06f);
		const float LocalTime = ChestRewardElapsedSeconds - PerCoinDelay;
		if (LocalTime <= 0.f || ChestRewardRemainingSeconds <= 0.08f)
		{
			CoinBox->SetRenderOpacity(0.f);
			continue;
		}

		const float LoopProgress = FMath::Fmod(LocalTime, CoinLoopSeconds) / CoinLoopSeconds;
		const float ArcAlpha = FMath::Sin(LoopProgress * PI);
		const float Spread = 10.f + CoinIndex * 4.f;
		const float LateralDrift = FMath::Sin((CoinIndex * 0.85f) + (LoopProgress * PI * 2.1f)) * Spread;
		const float VerticalDrift = -58.f * LoopProgress - (CoinIndex % 2 == 0 ? 8.f : 0.f);
		const float CoinSize = 10.f + (ArcAlpha * 12.f);

		CoinBox->SetWidthOverride(CoinSize);
		CoinBox->SetHeightOverride(CoinSize);
		CoinBox->SetRenderTransform(FSlateRenderTransform(FVector2D(LateralDrift, VerticalDrift)));
		CoinBox->SetRenderOpacity(ArcAlpha * OpenAlpha * FadeAlpha);
	}

	if (ChestRewardRemainingSeconds <= 0.f)
	{
		HideChestReward();
	}
}


void FT66HUDPresentationController::HideChestReward()
{
	bChestRewardVisible = false;
	ChestRewardRemainingSeconds = 0.f;
	ChestRewardElapsedSeconds = 0.f;
	ChestRewardTargetGold = 0;
	ChestRewardDisplayedGold = 0;
	ChestRewardMinimumDisplayedGold = 0;
	if (Owner.ChestRewardBox.IsValid())
	{
		Owner.ChestRewardBox->SetVisibility(EVisibility::Collapsed);
		Owner.ChestRewardBox->SetRenderOpacity(1.f);
	}
	if (Owner.ChestRewardClosedBox.IsValid())
	{
		Owner.ChestRewardClosedBox->SetRenderOpacity(1.f);
		Owner.ChestRewardClosedBox->SetRenderTransform(FSlateRenderTransform(FVector2D::ZeroVector));
	}
	if (Owner.ChestRewardOpenBox.IsValid())
	{
		Owner.ChestRewardOpenBox->SetRenderOpacity(0.f);
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
}


void FT66HUDPresentationController::TryShowQueuedPresentation()
{
	if (ActiveCrateOverlay.IsValid() || bChestRewardVisible || bPickupCardVisible || bWheelPanelOpen)
	{
		return;
	}

	if (QueuedChestRewards.Num() > 0)
	{
		const FQueuedChestReward NextChestReward = QueuedChestRewards[0];
		QueuedChestRewards.RemoveAt(0);
		StartChestReward(NextChestReward.Rarity, NextChestReward.GoldAmount);
		return;
	}

	if (QueuedPickupCards.Num() > 0)
	{
		const FQueuedPickupCard NextPickup = QueuedPickupCards[0];
		QueuedPickupCards.RemoveAt(0);
		ShowPickupItemCard(NextPickup.ItemID, NextPickup.ItemRarity);
	}
}


void FT66HUDPresentationController::ShowPickupItemCard(const FName ItemID, const ET66ItemRarity ItemRarity)
{
	if (ItemID.IsNone() || !Owner.PickupCardBox.IsValid())
	{
		return;
	}
	if (ActiveCrateOverlay.IsValid() || bChestRewardVisible || bPickupCardVisible || bWheelPanelOpen)
	{
		FQueuedPickupCard& QueuedPickup = QueuedPickupCards.AddDefaulted_GetRef();
		QueuedPickup.ItemID = ItemID;
		QueuedPickup.ItemRarity = ItemRarity;
		return;
	}

	UWorld* World = Owner.GetWorld();
	if (!World)
	{
		return;
	}

	UT66GameInstance* GI = Cast<UT66GameInstance>(Owner.GetGameInstance());
	UT66LocalizationSubsystem* Loc = Owner.GetGameInstance() ? Owner.GetGameInstance()->GetSubsystem<UT66LocalizationSubsystem>() : nullptr;
	UT66RunStateSubsystem* RunState = Owner.GetRunState();

	FItemData ItemData;
	const bool bHasData = GI && GI->GetItemData(ItemID, ItemData);

	if (Owner.PickupCardNameText.IsValid())
	{
		Owner.PickupCardNameText->SetText(Loc ? Loc->GetText_ItemDisplayNameForRarity(ItemID, ItemRarity) : FText::FromName(ItemID));
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
				if (ItemID == FName(TEXT("Item_GamblersToken")))
				{
					MainValue = RunState->GetActiveGamblersTokenLevel();
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
	const TSoftObjectPtr<UTexture2D> PickupIconSoft = bHasData ? ItemData.GetIconForRarity(ItemRarity) : TSoftObjectPtr<UTexture2D>();
	if (!PickupIconSoft.IsNull())
	{
		UT66UITexturePoolSubsystem* TexPool = Owner.GetGameInstance() ? Owner.GetGameInstance()->GetSubsystem<UT66UITexturePoolSubsystem>() : nullptr;
		if (TexPool)
		{
			T66SlateTexture::BindSharedBrushAsync(TexPool, PickupIconSoft, &Owner, Owner.PickupCardIconBrush, FName(TEXT("HUDPickupCard")), true);
		}
	}
	if (Owner.PickupCardIconImage.IsValid())
	{
		Owner.PickupCardIconImage->SetImage(Owner.PickupCardIconBrush.Get());
		Owner.PickupCardIconImage->SetVisibility(!PickupIconSoft.IsNull() ? EVisibility::Visible : EVisibility::Collapsed);
	}
	if (Owner.PickupCardTileBorder.IsValid())
	{
		const FLinearColor AccentColor = bHasData
			? (FItemData::GetItemRarityColor(ItemRarity) * 0.52f + FLinearColor(0.05f, 0.05f, 0.06f, 0.48f))
			: FT66Style::Tokens::Panel;
		Owner.PickupCardTileBorder->SetBorderBackgroundColor(AccentColor);
	}
	if (Owner.PickupCardIconBorder.IsValid())
	{
		Owner.PickupCardIconBorder->SetBorderBackgroundColor(FLinearColor(0.04f, 0.04f, 0.05f, 1.f));
	}
	if (Owner.PickupCardSkipText.IsValid())
	{
		const FText KeyText = GetActionKeyText(FName(TEXT("Interact")));
		Owner.PickupCardSkipText->SetText(
			KeyText.IsEmpty()
				? NSLOCTEXT("T66.Presentation", "PickupCardCloseInteract", "Interact to close")
				: FText::Format(NSLOCTEXT("T66.Presentation", "PickupCardCloseKey", "Close: {0}"), KeyText));
	}

	ActivePickupCardItemID = ItemID;
	ActivePickupCardRarity = ItemRarity;
	bPickupCardVisible = true;
	Owner.PickupCardBox->SetVisibility(EVisibility::Visible);
	Owner.PickupCardBox->SetRenderOpacity(1.f);
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
	}
	if (Owner.PickupCardSkipText.IsValid())
	{
		Owner.PickupCardSkipText->SetText(FText::GetEmpty());
	}
}
