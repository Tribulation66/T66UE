// Copyright Tribulation 66. All Rights Reserved.

#include "UI/HUD/T66GameplayHUDWidget_Private.h"

bool UT66GameplayHUDWidget::IsPausePresentationActive() const
{
	const AT66PlayerController* T66PC = Cast<AT66PlayerController>(GetOwningPlayer());
	if (!T66PC || !T66PC->IsPaused())
	{
		return false;
	}

	const UT66UIManager* LocalUIManager = T66PC->GetUIManager();
	return LocalUIManager
		&& LocalUIManager->IsModalActive()
		&& LocalUIManager->GetCurrentModalType() == ET66ScreenType::PauseMenu;
}


TSharedRef<SWidget> UT66GameplayHUDWidget::BuildPauseStatsPanel() const
{
	return T66StatsPanelSlate::MakeEssentialStatsPanel(
		GetRunState(),
		GetGameInstance() ? GetGameInstance()->GetSubsystem<UT66LocalizationSubsystem>() : nullptr,
		GT66MediaPanelW,
		true,
		-7,
		-6,
		true);
}


TSharedRef<SWidget> UT66GameplayHUDWidget::BuildPauseAchievementsPanel() const
{
	UGameInstance* GI = GetGameInstance();
	UT66AchievementsSubsystem* Achievements = GI ? GI->GetSubsystem<UT66AchievementsSubsystem>() : nullptr;
	UT66PlayerSettingsSubsystem* PlayerSettings = GI ? GI->GetSubsystem<UT66PlayerSettingsSubsystem>() : nullptr;

	TArray<FAchievementData> TrackedAchievements = Achievements ? Achievements->GetAllAchievements() : TArray<FAchievementData>{};
	TrackedAchievements = TrackedAchievements.FilterByPredicate([Achievements](const FAchievementData& Achievement)
	{
		const bool bClaimed = Achievements ? Achievements->IsAchievementClaimed(Achievement.AchievementID) : false;
		const int32 Progress = FMath::Clamp(Achievement.CurrentProgress, 0, Achievement.RequirementCount);
		return !bClaimed
			&& !Achievement.bIsUnlocked
			&& Achievement.RequirementCount > 0
			&& Progress < Achievement.RequirementCount;
	});

	TrackedAchievements.Sort([PlayerSettings](const FAchievementData& Left, const FAchievementData& Right)
	{
		const bool bLeftFavorite = PlayerSettings && PlayerSettings->IsFavoriteAchievement(Left.AchievementID);
		const bool bRightFavorite = PlayerSettings && PlayerSettings->IsFavoriteAchievement(Right.AchievementID);
		if (bLeftFavorite != bRightFavorite)
		{
			return bLeftFavorite && !bRightFavorite;
		}

		const float LeftProgress = GetAchievementProgress01(Left);
		const float RightProgress = GetAchievementProgress01(Right);
		if (!FMath::IsNearlyEqual(LeftProgress, RightProgress))
		{
			return LeftProgress > RightProgress;
		}

		const int32 LeftRemaining = GetAchievementRemainingCount(Left);
		const int32 RightRemaining = GetAchievementRemainingCount(Right);
		if (LeftRemaining != RightRemaining)
		{
			return LeftRemaining < RightRemaining;
		}

		const int32 LeftOrder = GetAchievementOrderKey(Left.AchievementID);
		const int32 RightOrder = GetAchievementOrderKey(Right.AchievementID);
		if (LeftOrder != RightOrder)
		{
			return LeftOrder < RightOrder;
		}

		return Left.AchievementID.LexicalLess(Right.AchievementID);
	});

	TSharedRef<SVerticalBox> Rows = SNew(SVerticalBox);
	const int32 MaxDisplayedAchievements = 7;
	const int32 DisplayCount = FMath::Min(TrackedAchievements.Num(), MaxDisplayedAchievements);

	if (DisplayCount <= 0)
	{
		Rows->AddSlot()
		.AutoHeight()
		[
			SNew(STextBlock)
			.Text(NSLOCTEXT("T66.PauseMenu", "NoTrackedAchievements", "No in-progress achievements right now."))
			.Font(FT66Style::Tokens::FontRegular(10))
			.ColorAndOpacity(FT66Style::Tokens::TextMuted)
			.AutoWrapText(true)
		];
	}
	else
	{
		for (int32 Index = 0; Index < DisplayCount; ++Index)
		{
			const FAchievementData& Achievement = TrackedAchievements[Index];
			const int32 Progress = FMath::Clamp(Achievement.CurrentProgress, 0, Achievement.RequirementCount);
			const int32 Requirement = FMath::Max(1, Achievement.RequirementCount);
			const float Progress01 = GetAchievementProgress01(Achievement);
			const bool bFavorited = PlayerSettings && PlayerSettings->IsFavoriteAchievement(Achievement.AchievementID);
			const FLinearColor ProgressColor = bFavorited
				? FLinearColor(0.94f, 0.78f, 0.28f, 1.0f)
				: FT66Style::Tokens::Success;
			const FText ProgressText = FText::Format(
				NSLOCTEXT("T66.PauseMenu", "PauseAchievementProgress", "{0}/{1}"),
				FText::AsNumber(Progress),
				FText::AsNumber(Requirement));

			Rows->AddSlot()
			.AutoHeight()
			.Padding(0.f, 0.f, 0.f, 6.f)
			[
				SNew(SBorder)
				.BorderImage(FCoreStyle::Get().GetBrush("WhiteBrush"))
				.BorderBackgroundColor(FLinearColor(0.03f, 0.04f, 0.06f, 0.96f))
				.ToolTipText(Achievement.Description)
				.Padding(FMargin(10.f, 6.f))
				[
					SNew(SVerticalBox)
					+ SVerticalBox::Slot()
					.AutoHeight()
					[
						SNew(SHorizontalBox)
						+ SHorizontalBox::Slot()
						.FillWidth(1.f)
						.VAlign(VAlign_Center)
						[
							SNew(STextBlock)
							.Text(Achievement.DisplayName)
							.Font(FT66Style::Tokens::FontBold(11))
							.ColorAndOpacity(FT66Style::Tokens::Text)
							.AutoWrapText(true)
						]
						+ SHorizontalBox::Slot()
						.AutoWidth()
						.VAlign(VAlign_Center)
						.Padding(8.f, 0.f, 0.f, 0.f)
						[
							SNew(STextBlock)
							.Text(FText::FromString(bFavorited ? TEXT("\u2605") : TEXT("")))
							.Font(FT66Style::Tokens::FontRegular(12))
							.ColorAndOpacity(ProgressColor)
						]
					]
					+ SVerticalBox::Slot()
					.AutoHeight()
					.Padding(0.f, 2.f, 0.f, 0.f)
					[
						SNew(STextBlock)
						.Text(Achievement.Description)
						.Font(FT66Style::Tokens::FontRegular(9))
						.ColorAndOpacity(FT66Style::Tokens::TextMuted)
						.AutoWrapText(true)
					]
					+ SVerticalBox::Slot()
					.AutoHeight()
					.Padding(0.f, 6.f, 0.f, 0.f)
					[
						SNew(SProgressBar)
						.Percent(Progress01)
						.FillColorAndOpacity(ProgressColor)
					]
					+ SVerticalBox::Slot()
					.AutoHeight()
					.Padding(0.f, 4.f, 0.f, 0.f)
					[
						SNew(SHorizontalBox)
						+ SHorizontalBox::Slot()
						.FillWidth(1.f)
						[
							SNew(STextBlock)
							.Text(ProgressText)
							.Font(FT66Style::Tokens::FontBold(10))
							.ColorAndOpacity(FT66Style::Tokens::Text)
						]
						+ SHorizontalBox::Slot()
						.AutoWidth()
						[
							SNew(STextBlock)
							.Text(FText::Format(
								NSLOCTEXT("T66.PauseMenu", "PauseAchievementRemaining", "{0} left"),
								FText::AsNumber(GetAchievementRemainingCount(Achievement))))
							.Font(FT66Style::Tokens::FontRegular(9))
							.ColorAndOpacity(FT66Style::Tokens::TextMuted)
						]
					]
				]
			];
		}
	}

	return SNew(SBox)
		.WidthOverride(BottomRightPauseAchievementPanelWidth)
		[
			FT66Style::MakePanel(
				SNew(SBox)
				.HeightOverride(420.f)
				[
					SNew(SScrollBox)
					+ SScrollBox::Slot()
					[
						Rows
					]
				],
				FT66PanelParams(ET66PanelType::Panel).SetPadding(FMargin(12.f, 12.f, 12.f, 10.f)))
		];
}


void UT66GameplayHUDWidget::RefreshPausePresentation()
{
	const bool bPausePresentationActive = IsPausePresentationActive();
	bLastPausePresentationActive = bPausePresentationActive;

	if (PauseBackdropBorder.IsValid())
	{
		PauseBackdropBorder->SetVisibility(bPausePresentationActive ? EVisibility::HitTestInvisible : EVisibility::Collapsed);
	}

	if (PauseStatsPanelBox.IsValid())
	{
		PauseStatsPanelBox->SetVisibility(bPausePresentationActive ? EVisibility::Visible : EVisibility::Collapsed);
		PauseStatsPanelBox->SetContent(bPausePresentationActive ? BuildPauseStatsPanel() : StaticCastSharedRef<SWidget>(SNew(SSpacer)));
	}

	if (PauseAchievementsPanelBox.IsValid())
	{
		PauseAchievementsPanelBox->SetVisibility(EVisibility::Collapsed);
		PauseAchievementsPanelBox->SetContent(StaticCastSharedRef<SWidget>(SNew(SSpacer)));
	}

	if (IsMediaViewerOpen())
	{
		RequestTikTokWebView2OverlaySync();
	}
}


bool UT66GameplayHUDWidget::IsTikTokPlaceholderVisible() const
{
	return IsMediaViewerOpen();
}


void UT66GameplayHUDWidget::ToggleTikTokPlaceholder()
{
	if (UGameInstance* GI = GetGameInstance())
	{
		if (UT66MediaViewerSubsystem* MV = GI->GetSubsystem<UT66MediaViewerSubsystem>())
		{
			MV->ToggleMediaViewer();
			return;
		}
	}
}


bool UT66GameplayHUDWidget::IsMediaViewerOpen() const
{
	if (UGameInstance* GI = GetGameInstance())
	{
		if (const UT66MediaViewerSubsystem* MV = GI->GetSubsystem<UT66MediaViewerSubsystem>())
		{
			return MV->IsMediaViewerOpen();
		}
	}
	return false;
}


void UT66GameplayHUDWidget::HandleMediaViewerOpenChanged(bool /*bIsOpen*/)
{
	UpdateTikTokVisibility(true);
}


void UT66GameplayHUDWidget::RequestTikTokWebView2OverlaySync()
{
#if PLATFORM_WINDOWS && T66_WITH_WEBVIEW2
	// Defer one tick so Slate has a valid cached geometry for the panel.
	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().ClearTimer(TikTokOverlaySyncHandle);
		World->GetTimerManager().SetTimerForNextTick(this, &UT66GameplayHUDWidget::SyncTikTokWebView2OverlayToPlaceholder);
	}
#endif
}


void UT66GameplayHUDWidget::SyncTikTokWebView2OverlayToPlaceholder()
{
#if PLATFORM_WINDOWS && T66_WITH_WEBVIEW2
	if (!IsMediaViewerOpen()) return;
	// Use the inner video box (below tabs) so the WebView2 overlay doesn't cover the tab buttons.
	TSharedPtr<SBox> SyncBox = MediaViewerVideoBox.IsValid() ? MediaViewerVideoBox : TikTokPlaceholderBox;
	if (!SyncBox.IsValid()) return;
	if (!FSlateApplication::IsInitialized()) return;

	// If the widget is collapsed, geometry may be invalid. Guard against 0-size.
	const FGeometry Geo = SyncBox->GetCachedGeometry();
	const FVector2D LocalSize = Geo.GetLocalSize();
	if (LocalSize.X < 4.f || LocalSize.Y < 4.f)
	{
		return;
	}

	const TSharedPtr<SWindow> Window = FSlateApplication::Get().FindWidgetWindow(SyncBox.ToSharedRef());
	if (!Window.IsValid())
	{
		return;
	}

	// Compute rect in window client coordinates (pixels). In practice, Slate's "screen space" aligns with window screen space.
	const FVector2D AbsTL = Geo.LocalToAbsolute(FVector2D::ZeroVector);
	const FVector2D AbsBR = Geo.LocalToAbsolute(LocalSize);

	// Treat Slate absolute space as desktop screen coordinates, then let Win32 do ScreenToClient against the real HWND.
	const int32 X0 = FMath::RoundToInt(AbsTL.X);
	const int32 Y0 = FMath::RoundToInt(AbsTL.Y);
	const int32 X1 = FMath::RoundToInt(AbsBR.X);
	const int32 Y1 = FMath::RoundToInt(AbsBR.Y);
	const FIntRect Rect(FIntPoint(X0, Y0), FIntPoint(X1, Y1));

	if (UGameInstance* GI = GetGameInstance())
	{
		if (UT66MediaViewerSubsystem* MV = GI->GetSubsystem<UT66MediaViewerSubsystem>())
		{
			MV->SetTikTokWebView2ScreenRect(Rect);
		}
	}
#endif
}


void UT66GameplayHUDWidget::UpdateTikTokVisibility(bool bForce)
{
	const bool bOpen = IsMediaViewerOpen();
	const bool bStateChanged = !bHasAppliedMediaViewerOpenState || (bLastAppliedMediaViewerOpenState != bOpen);
	if (!bForce && !bStateChanged)
	{
		return;
	}

	bHasAppliedMediaViewerOpenState = true;
	bLastAppliedMediaViewerOpenState = bOpen;

#if PLATFORM_WINDOWS && T66_WITH_WEBVIEW2
	// On Windows, TikTok UI is rendered by a native WebView2 overlay, but we keep the Slate panel visible
	// as a layout anchor so we can position the overlay correctly.
	const EVisibility Vis = bOpen ? EVisibility::Visible : EVisibility::Collapsed;
#else
	const EVisibility Vis = bOpen ? EVisibility::Visible : EVisibility::Collapsed;
#endif
	if (TikTokPlaceholderBox.IsValid())
	{
		TikTokPlaceholderBox->SetVisibility(Vis);
	}
	UE_LOG(LogT66HUD, Verbose, TEXT("[TIKTOK] Viewer %s"), bOpen ? TEXT("OPEN") : TEXT("CLOSED"));
	if (bOpen)
	{
#if PLATFORM_WINDOWS && T66_WITH_WEBVIEW2
		// Align native WebView2 overlay to the phone panel location (and keep it aligned if window DPI/position changes).
		SyncTikTokWebView2OverlayToPlaceholder();
		RequestTikTokWebView2OverlaySync();
		if (UWorld* World = GetWorld())
		{
			World->GetTimerManager().ClearTimer(TikTokOverlaySyncHandle);
			World->GetTimerManager().SetTimer(TikTokOverlaySyncHandle, this, &UT66GameplayHUDWidget::SyncTikTokWebView2OverlayToPlaceholder, 0.50f, true);
		}
#endif
	}
	else
	{
#if PLATFORM_WINDOWS && T66_WITH_WEBVIEW2
		if (UWorld* World = GetWorld())
		{
			World->GetTimerManager().ClearTimer(TikTokOverlaySyncHandle);
		}
#endif
	}
}


static void T66_ApplyWorldDialogueSelectionOverlay(
	const TArray<TSharedPtr<SBorder>>& OptionBorders,
	const TArray<TSharedPtr<STextBlock>>& OptionTexts,
	int32 SelectedIndex)
{
	for (int32 i = 0; i < OptionBorders.Num(); ++i)
	{
		const bool bSelected = (i == SelectedIndex);
		if (OptionBorders[i].IsValid())
		{
			OptionBorders[i]->SetBorderBackgroundColor(bSelected
				? (FT66Style::IsDotaTheme() ? FLinearColor(0.34f, 0.25f, 0.13f, 0.98f) : FLinearColor(0.18f, 0.18f, 0.26f, 0.95f))
				: (FT66Style::IsDotaTheme() ? FLinearColor(0.030f, 0.026f, 0.024f, 0.94f) : FLinearColor(0.06f, 0.06f, 0.08f, 0.85f)));
		}
		if (OptionTexts.IsValidIndex(i) && OptionTexts[i].IsValid())
		{
			OptionTexts[i]->SetColorAndOpacity(bSelected ? FT66Style::Tokens::Text : FT66Style::Tokens::TextMuted);
		}
	}
}

void UT66GameplayHUDWidget::ShowInteractionPrompt(AActor* SourceActor, const FText& TargetName)
{
	if (!SourceActor || TargetName.IsEmpty())
	{
		return;
	}

	InteractionPromptEntries.RemoveAll([SourceActor](const FT66HUDInteractionPromptEntry& Entry)
	{
		return !Entry.SourceActor.IsValid() || Entry.SourceActor.Get() == SourceActor;
	});

	FT66HUDInteractionPromptEntry Entry;
	Entry.SourceActor = SourceActor;
	Entry.TargetName = TargetName;
	InteractionPromptEntries.Add(MoveTemp(Entry));
	RefreshInteractionPromptWidget();
}

void UT66GameplayHUDWidget::HideInteractionPrompt(AActor* SourceActor)
{
	if (!SourceActor)
	{
		return;
	}

	InteractionPromptEntries.RemoveAll([SourceActor](const FT66HUDInteractionPromptEntry& Entry)
	{
		return !Entry.SourceActor.IsValid() || Entry.SourceActor.Get() == SourceActor;
	});
	RefreshInteractionPromptWidget();
}

void UT66GameplayHUDWidget::RefreshInteractionPromptWidget()
{
	InteractionPromptEntries.RemoveAll([](const FT66HUDInteractionPromptEntry& Entry)
	{
		return !Entry.SourceActor.IsValid() || Entry.TargetName.IsEmpty();
	});

	if (!InteractionPromptBox.IsValid() || !InteractionPromptTargetText.IsValid())
	{
		return;
	}

	if (InteractionPromptEntries.Num() <= 0)
	{
		InteractionPromptTargetText->SetText(FText::GetEmpty());
		InteractionPromptBox->SetVisibility(EVisibility::Collapsed);
		return;
	}

	const FT66HUDInteractionPromptEntry& ActiveEntry = InteractionPromptEntries.Last();
	FText InteractKeyText = GetActionKeyText(FName(TEXT("Interact")));
	if (InteractKeyText.IsEmpty())
	{
		InteractKeyText = NSLOCTEXT("T66.GameplayHUD", "InteractFallbackKey", "F");
	}

	InteractionPromptTargetText->SetText(FText::Format(
		NSLOCTEXT("T66.GameplayHUD", "InteractPromptSentence", "Press {0} to interact with {1}"),
		InteractKeyText,
		ActiveEntry.TargetName));
	InteractionPromptBox->SetVisibility(EVisibility::HitTestInvisible);
}


void UT66GameplayHUDWidget::ShowWorldDialogue(const TArray<FText>& Options, int32 SelectedIndex)
{
	if (!WorldDialogueBox.IsValid()) return;
	if (Options.Num() < 2) return;
	if (WorldDialogueOptionTexts.Num() < 3) return;

	for (int32 i = 0; i < 3; ++i)
	{
		const bool bHasOption = Options.IsValidIndex(i);
		if (WorldDialogueOptionTexts[i].IsValid())
		{
			WorldDialogueOptionTexts[i]->SetText(bHasOption ? Options[i] : FText::GetEmpty());
		}
		if (WorldDialogueOptionBorders.IsValidIndex(i) && WorldDialogueOptionBorders[i].IsValid())
		{
			WorldDialogueOptionBorders[i]->SetVisibility(bHasOption ? EVisibility::Visible : EVisibility::Collapsed);
		}
	}
	T66_ApplyWorldDialogueSelectionOverlay(WorldDialogueOptionBorders, WorldDialogueOptionTexts, SelectedIndex);
	WorldDialogueBox->SetVisibility(EVisibility::Visible);
}


void UT66GameplayHUDWidget::HideWorldDialogue()
{
	if (!WorldDialogueBox.IsValid()) return;
	WorldDialogueBox->SetVisibility(EVisibility::Collapsed);
}


void UT66GameplayHUDWidget::SetWorldDialogueSelection(int32 SelectedIndex)
{
	T66_ApplyWorldDialogueSelectionOverlay(WorldDialogueOptionBorders, WorldDialogueOptionTexts, SelectedIndex);
}


void UT66GameplayHUDWidget::SetWorldDialogueScreenPosition(const FVector2D& ScreenPos)
{
	if (!WorldDialogueBox.IsValid()) return;
	WorldDialogueBox->SetRenderTransform(FSlateRenderTransform(ScreenPos));
}


bool UT66GameplayHUDWidget::IsWorldDialogueVisible() const
{
	return WorldDialogueBox.IsValid() && WorldDialogueBox->GetVisibility() == EVisibility::Visible;
}


void UT66GameplayHUDWidget::SetInteractive(bool bInteractive)
{
	SetVisibility(bInteractive ? ESlateVisibility::Visible : ESlateVisibility::SelfHitTestInvisible);
}

