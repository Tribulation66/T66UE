// Copyright Tribulation 66. All Rights Reserved.

#include "UI/Screens/Settings/T66SettingsScreen_Private.h"

using namespace T66SettingsScreenPrivate;
TSharedRef<SWidget> UT66SettingsScreen::BuildGraphicsTab()
{
	UT66LocalizationSubsystem* Loc = GetLocSubsystem();
	UT66PlayerSettingsSubsystem* PS = GetPlayerSettings();
	InitializeGraphicsFromUserSettingsIfNeeded();

	auto FpsCapToText = [Loc](int32 Index) -> FText
	{
		switch (Index)
		{
		case 0: return FText::AsNumber(30);
		case 1: return FText::AsNumber(60);
		case 2: return FText::AsNumber(90);
		case 3: return FText::AsNumber(120);
		default: return Loc ? Loc->GetText_Unlimited() : NSLOCTEXT("T66.Settings", "Unlimited", "Unlimited");
		}
	};

	auto WindowModeToText = [Loc](EWindowMode::Type Mode) -> FText
	{
		if (!Loc)
		{
			switch (Mode)
			{
			case EWindowMode::Windowed: return NSLOCTEXT("T66.Settings", "Windowed", "Windowed");
			case EWindowMode::WindowedFullscreen: return NSLOCTEXT("T66.Settings", "BorderlessWindowed", "Borderless Windowed");
			default: return NSLOCTEXT("T66.Settings", "Fullscreen", "Fullscreen");
			}
		}
		switch (Mode)
		{
		case EWindowMode::Windowed: return Loc->GetText_Windowed();
		case EWindowMode::WindowedFullscreen: return Loc->GetText_BorderlessWindowed();
		default: return Loc->GetText_Fullscreen();
		}
	};

	auto DisplayModeToText = [Loc](ET66DisplayMode Mode) -> FText
	{
		if (!Loc)
		{
			return Mode == ET66DisplayMode::Widescreen ? NSLOCTEXT("T66.Settings", "Widescreen", "Widescreen") : NSLOCTEXT("T66.Settings", "Standard", "Standard");
		}
		return Mode == ET66DisplayMode::Widescreen ? Loc->GetText_DisplayModeWidescreen() : Loc->GetText_DisplayModeStandard();
	};

	auto ResToText = [](FIntPoint R) -> FText
	{
		return FText::Format(NSLOCTEXT("T66.Settings", "ResolutionFormat", "{0} x {1}"), FText::AsNumber(R.X), FText::AsNumber(R.Y));
	};

	TSharedRef<SOverlay> Root =
		SNew(SOverlay)
		+ SOverlay::Slot()
		[
			SNew(SVerticalBox)
		+ SVerticalBox::Slot().AutoHeight().Padding(0.0f, 0.0f, 0.0f, 8.0f)
		[
			MakeSettingsDropdownRow(
				Loc ? Loc->GetText_Monitor() : NSLOCTEXT("T66.Settings", "Monitor", "Monitor"),
				[this, Loc]()
				{
					FDisplayMetrics Metrics;
					FSlateApplication::Get().GetCachedDisplayMetrics(Metrics);
					const int32 Idx = FMath::Clamp(PendingGraphics.MonitorIndex, 0, FMath::Max(0, Metrics.MonitorInfo.Num() - 1));
					if (Idx == 0) return Loc ? Loc->GetText_PrimaryMonitor() : NSLOCTEXT("T66.Settings", "PrimaryMonitor", "Primary Monitor");
					return FText::Format(NSLOCTEXT("T66.Settings", "MonitorN", "Monitor {0}"), FText::AsNumber(Idx + 1));
				},
				[this, Loc](const TSharedPtr<STextBlock>& CurrentValueText)
				{
					FDisplayMetrics Metrics;
					FSlateApplication::Get().GetCachedDisplayMetrics(Metrics);
					TSharedRef<SVerticalBox> Box = SNew(SVerticalBox);
					for (int32 i = 0; i < Metrics.MonitorInfo.Num(); ++i)
					{
						const FText Label = (i == 0) ? (Loc ? Loc->GetText_PrimaryMonitor() : NSLOCTEXT("T66.Settings", "PrimaryMonitor", "Primary Monitor"))
							: FText::Format(NSLOCTEXT("T66.Settings", "MonitorN", "Monitor {0}"), FText::AsNumber(i + 1));
						const int32 MonitorIdx = i;
						Box->AddSlot().AutoHeight()
						[
							FT66Style::MakeDropdownOptionButton(
								Label,
								FOnClicked::CreateLambda([this, MonitorIdx, CurrentValueText, Loc]()
								{
									PendingGraphics.MonitorIndex = MonitorIdx;
									PendingGraphics.bDirty = true;
									if (CurrentValueText.IsValid())
									{
										CurrentValueText->SetText(MonitorIdx == 0
											? (Loc ? Loc->GetText_PrimaryMonitor() : NSLOCTEXT("T66.Settings", "PrimaryMonitor", "Primary Monitor"))
											: FText::Format(NSLOCTEXT("T66.Settings", "MonitorN", "Monitor {0}"), FText::AsNumber(MonitorIdx + 1)));
									}
									FSlateApplication::Get().DismissAllMenus();
									return FReply::Handled();
								}),
								PendingGraphics.MonitorIndex == MonitorIdx,
								0.f,
								34.f,
								14)
						];
					}
					return Box;
				}
			)
		]
		+ SVerticalBox::Slot().AutoHeight().Padding(0.0f, 0.0f, 0.0f, 8.0f)
		[
			MakeSettingsDropdownRow(
				Loc ? Loc->GetText_Resolution() : NSLOCTEXT("T66.Settings.Fallback", "Resolution", "Resolution"),
				[this, ResToText]() { return ResToText(PendingGraphics.Resolution); },
				[this, ResToText](const TSharedPtr<STextBlock>& CurrentValueText)
				{
					static const FIntPoint ResList[] = {
						FIntPoint(1280, 720),
						FIntPoint(1600, 900),
						FIntPoint(1920, 1080),
						FIntPoint(2560, 1440),
						FIntPoint(3840, 2160)
					};
					TSharedRef<SVerticalBox> Box = SNew(SVerticalBox);
					for (const FIntPoint& R : ResList)
					{
						Box->AddSlot().AutoHeight()
						[
							FT66Style::MakeDropdownOptionButton(
								ResToText(R),
								FOnClicked::CreateLambda([this, R, CurrentValueText, ResToText]()
								{
									PendingGraphics.Resolution = R;
									PendingGraphics.bDirty = true;
									if (CurrentValueText.IsValid())
									{
										CurrentValueText->SetText(ResToText(R));
									}
									FSlateApplication::Get().DismissAllMenus();
									return FReply::Handled();
								}),
								PendingGraphics.Resolution == R,
								0.f,
								34.f,
								14)
						];
					}
					return Box;
				}
			)
		]
		+ SVerticalBox::Slot().AutoHeight().Padding(0.0f, 0.0f, 0.0f, 8.0f)
		[
			MakeSettingsDropdownRow(
				Loc ? Loc->GetText_WindowMode() : NSLOCTEXT("T66.Settings.Fallback", "Window Mode", "Window Mode"),
				[this, WindowModeToText]() { return WindowModeToText(PendingGraphics.WindowMode); },
				[this, WindowModeToText](const TSharedPtr<STextBlock>& CurrentValueText)
				{
					TSharedRef<SVerticalBox> Box = SNew(SVerticalBox);
					auto Add = [this, &Box, WindowModeToText, CurrentValueText](EWindowMode::Type Mode)
					{
						Box->AddSlot().AutoHeight()
						[
							FT66Style::MakeDropdownOptionButton(
								WindowModeToText(Mode),
								FOnClicked::CreateLambda([this, Mode, CurrentValueText, WindowModeToText]()
								{
									PendingGraphics.WindowMode = Mode;
									PendingGraphics.bDirty = true;
									if (CurrentValueText.IsValid())
									{
										CurrentValueText->SetText(WindowModeToText(Mode));
									}
									FSlateApplication::Get().DismissAllMenus();
									return FReply::Handled();
								}),
								PendingGraphics.WindowMode == Mode,
								0.f,
								34.f,
								14)
						];
					};
					Add(EWindowMode::Windowed);
					Add(EWindowMode::Fullscreen);
					Add(EWindowMode::WindowedFullscreen);
					return Box;
				}
			)
		]
		+ SVerticalBox::Slot().AutoHeight().Padding(0.0f, 0.0f, 0.0f, 8.0f)
		[
			MakeSettingsDropdownRow(
				Loc ? Loc->GetText_DisplayMode() : NSLOCTEXT("T66.Settings.Fallback", "Display Mode", "Display Mode"),
				[this, DisplayModeToText]() { return DisplayModeToText(PendingGraphics.DisplayMode); },
				[this, DisplayModeToText](const TSharedPtr<STextBlock>& CurrentValueText)
				{
					TSharedRef<SVerticalBox> Box = SNew(SVerticalBox);
					auto Add = [this, &Box, DisplayModeToText, CurrentValueText](ET66DisplayMode Mode)
					{
						Box->AddSlot().AutoHeight()
						[
							FT66Style::MakeDropdownOptionButton(
								DisplayModeToText(Mode),
								FOnClicked::CreateLambda([this, Mode, CurrentValueText, DisplayModeToText]()
								{
									PendingGraphics.DisplayMode = Mode;
									PendingGraphics.bDirty = true;
									if (CurrentValueText.IsValid())
									{
										CurrentValueText->SetText(DisplayModeToText(Mode));
									}
									FSlateApplication::Get().DismissAllMenus();
									return FReply::Handled();
								}),
								PendingGraphics.DisplayMode == Mode,
								0.f,
								34.f,
								14)
						];
					};
					Add(ET66DisplayMode::Standard);
					Add(ET66DisplayMode::Widescreen);
					return Box;
				}
			)
		]
		+ SVerticalBox::Slot().AutoHeight().Padding(0.0f, 0.0f, 0.0f, 8.0f)
		[
			SNew(SBorder)
			.BorderBackgroundColor(T66SettingsRowFill())
			.Padding(FMargin(15.0f, 12.0f))
			[
				SNew(SHorizontalBox)
				+ SHorizontalBox::Slot().FillWidth(1.0f).VAlign(VAlign_Center)
				[
					SNew(SBox).MinDesiredWidth(200.0f)
					[
						SNew(STextBlock)
						.Text(NSLOCTEXT("T66.Settings", "UiTheme", "UI Style"))
						.Font(SettingsRegularFont(22))
						.ColorAndOpacity(GetSettingsPageText())
					]
				]
				+ SHorizontalBox::Slot().AutoWidth().Padding(10.0f, 0.0f, 0.0f, 0.0f)
				[
					SNew(STextBlock)
					.Text(NSLOCTEXT("T66.Settings", "UiThemeUnified", "Unified"))
					.Font(SettingsRegularFont(18))
					.ColorAndOpacity(GetSettingsPageMuted())
				]
			]
		]
		// Quality slider
		+ SVerticalBox::Slot().AutoHeight().Padding(0.0f, 0.0f, 0.0f, 8.0f)
		[
			SNew(SBorder)
			.BorderBackgroundColor(T66SettingsRowFill())
			.Padding(FMargin(15.0f, 12.0f))
			[
				SNew(SVerticalBox)
				+ SVerticalBox::Slot().AutoHeight()
				[
					SNew(SHorizontalBox)
					+ SHorizontalBox::Slot().FillWidth(1.0f)
					[
						SNew(STextBlock).Text(Loc ? Loc->GetText_BestPerformance() : NSLOCTEXT("T66.Settings.Fallback", "Best Performance", "Best Performance"))
						.Font(SettingsRegularFont(16))
						.ColorAndOpacity(GetSettingsPageMuted())
					]
					+ SHorizontalBox::Slot().FillWidth(1.0f).HAlign(HAlign_Right)
					[
						SNew(STextBlock).Text(Loc ? Loc->GetText_BestQuality() : NSLOCTEXT("T66.Settings.Fallback", "Best Quality", "Best Quality"))
						.Font(SettingsRegularFont(16))
						.ColorAndOpacity(GetSettingsPageMuted())
					]
				]
				+ SVerticalBox::Slot().AutoHeight().Padding(0.0f, 8.0f, 0.0f, 0.0f)
				[
					SNew(SSlider)
					.Value_Lambda([this]() { return static_cast<float>(PendingGraphics.QualityNotch) / 3.0f; })
					.StepSize(1.0f / 3.0f)
					.OnValueChanged_Lambda([this](float V)
					{
						PendingGraphics.QualityNotch = FMath::Clamp(FMath::RoundToInt(V * 3.0f), 0, 3);
						PendingGraphics.bDirty = true;
					})
				]
			]
		]
		+ SVerticalBox::Slot().AutoHeight().Padding(0.0f, 0.0f, 0.0f, 8.0f)
		[
			MakeSettingsDropdownRow(
				Loc ? Loc->GetText_FpsCap() : NSLOCTEXT("T66.Settings.Fallback", "FPS Cap", "FPS Cap"),
				[this, FpsCapToText]() { return FpsCapToText(PendingGraphics.FpsCapIndex); },
				[this, FpsCapToText](const TSharedPtr<STextBlock>& CurrentValueText)
				{
					TSharedRef<SVerticalBox> Box = SNew(SVerticalBox);
					for (int32 i = 0; i < 5; ++i)
					{
						Box->AddSlot().AutoHeight()
						[
							FT66Style::MakeDropdownOptionButton(
								FpsCapToText(i),
								FOnClicked::CreateLambda([this, i, CurrentValueText, FpsCapToText]()
								{
									PendingGraphics.FpsCapIndex = i;
									PendingGraphics.bDirty = true;
									if (CurrentValueText.IsValid())
									{
										CurrentValueText->SetText(FpsCapToText(i));
									}
									FSlateApplication::Get().DismissAllMenus();
									return FReply::Handled();
								}),
								PendingGraphics.FpsCapIndex == i,
								0.f,
								34.f,
								14)
						];
					}
					return Box;
				}
			)
		]
		// Fog toggle
		+ SVerticalBox::Slot().AutoHeight().Padding(0.0f, 0.0f, 0.0f, 8.0f)
		[
			MakeSettingsToggleRow(
				Loc,
				Loc ? Loc->GetText_SettingsFog() : NSLOCTEXT("T66.Settings", "Fog", "Fog"),
				[PS]() { return PS ? PS->GetFogEnabled() : false; },
				[PS](bool bEnabled)
				{
					if (PS)
					{
						PS->SetFogEnabled(bEnabled);
					}
				},
				FText(),
				FSettingsBoolToggleStyle())
		]
		// Apply button
		+ SVerticalBox::Slot().AutoHeight().HAlign(HAlign_Right).Padding(0.0f, 15.0f, 0.0f, 0.0f)
		[
			MakeSettingsButton(
				FT66ButtonParams(Loc ? Loc->GetText_Apply() : NSLOCTEXT("T66.Settings.Fallback", "APPLY", "APPLY"),
					FOnClicked::CreateUObject(this, &UT66SettingsScreen::HandleApplyGraphicsClicked), ET66ButtonType::Success)
				.SetFontSize(20).SetMinWidth(110.f))
		]
	];

	// Confirm overlay (10s auto-revert)
	Root->AddSlot()
	.HAlign(HAlign_Center)
	.VAlign(VAlign_Center)
	[
		SAssignNew(VideoModeConfirmOverlay, SBorder)
		.BorderBackgroundColor(FLinearColor(0.f, 0.f, 0.f, 0.75f))
		.Visibility(bVideoModeConfirmActive ? EVisibility::Visible : EVisibility::Collapsed)
		.Padding(FMargin(30.f))
		[
			SNew(SBorder)
			.BorderBackgroundColor(T66SettingsRowFill())
			.Padding(FMargin(25.f))
			[
				SNew(SVerticalBox)
				+ SVerticalBox::Slot().AutoHeight().HAlign(HAlign_Center).Padding(0.f, 0.f, 0.f, 10.f)
				[
					SNew(STextBlock)
					.Text(Loc ? Loc->GetText_KeepTheseSettingsTitle() : NSLOCTEXT("T66.Settings.Fallback", "Keep these settings?", "Keep these settings?"))
					.Font(SettingsBoldFont(24))
					.ColorAndOpacity(GetSettingsPageText())
				]
				+ SVerticalBox::Slot().AutoHeight().HAlign(HAlign_Center).Padding(0.f, 0.f, 0.f, 15.f)
				[
					SAssignNew(VideoModeConfirmCountdownText, STextBlock)
					.Text(FText::GetEmpty())
					.Font(SettingsRegularFont(18))
					.ColorAndOpacity(GetSettingsPageMuted())
				]
				+ SVerticalBox::Slot().AutoHeight().HAlign(HAlign_Center)
				[
					SNew(SHorizontalBox)
					+ SHorizontalBox::Slot().AutoWidth().Padding(6.f)
					[
					MakeSettingsButton(Loc ? Loc->GetText_Keep() : NSLOCTEXT("T66.Settings.Fallback", "KEEP", "KEEP"),
						FOnClicked::CreateLambda([this]() { EndVideoModeConfirmPrompt(true); return FReply::Handled(); }),
						ET66ButtonType::Success, 100.f)
					]
					+ SHorizontalBox::Slot().AutoWidth().Padding(6.f)
					[
					MakeSettingsButton(Loc ? Loc->GetText_Revert() : NSLOCTEXT("T66.Settings.Fallback", "REVERT", "REVERT"),
						FOnClicked::CreateLambda([this]() { EndVideoModeConfirmPrompt(false); return FReply::Handled(); }),
						ET66ButtonType::Danger, 100.f)
					]
				]
			]
		]
	];

	return Root;
}

FReply UT66SettingsScreen::HandleApplyGraphicsClicked()
{
	OnApplyGraphicsClicked();
	return FReply::Handled();
}

void UT66SettingsScreen::OnApplyGraphicsClicked()
{
	ApplyPendingGraphics(false);
}

void UT66SettingsScreen::InitializeGraphicsFromUserSettingsIfNeeded()
{
	if (bGraphicsInitialized) return;
	bGraphicsInitialized = true;

	if (UGameUserSettings* GUS = GEngine ? GEngine->GetGameUserSettings() : nullptr)
	{
		PendingGraphics.Resolution = GUS->GetScreenResolution();
		PendingGraphics.WindowMode = GUS->GetFullscreenMode();
		PendingGraphics.QualityNotch = FMath::Clamp(GUS->GetOverallScalabilityLevel(), 0, 3);
		const float Limit = GUS->GetFrameRateLimit();
		if (Limit <= 0.0f) PendingGraphics.FpsCapIndex = 4;
		else if (Limit <= 30.0f) PendingGraphics.FpsCapIndex = 0;
		else if (Limit <= 60.0f) PendingGraphics.FpsCapIndex = 1;
		else if (Limit <= 90.0f) PendingGraphics.FpsCapIndex = 2;
		else PendingGraphics.FpsCapIndex = 3;
	}
	if (UT66PlayerSettingsSubsystem* PS = GetPlayerSettings())
	{
		PendingGraphics.DisplayMode = (PS->GetDisplayModeIndex() == 1) ? ET66DisplayMode::Widescreen : ET66DisplayMode::Standard;
		PendingGraphics.MonitorIndex = PS->GetMonitorIndex();
	}
	else
	{
		PendingGraphics.DisplayMode = ET66DisplayMode::Standard;
		PendingGraphics.MonitorIndex = 0;
	}
	PendingGraphics.bDirty = false;
}

void UT66SettingsScreen::ApplyPendingGraphics(bool bForceConfirmPrompt)
{
	if (!PendingGraphics.bDirty && !bForceConfirmPrompt) return;

	UGameUserSettings* GUS = GEngine ? GEngine->GetGameUserSettings() : nullptr;
	if (!GUS) return;

	const FIntPoint OldRes = GUS->GetScreenResolution();
	const EWindowMode::Type OldMode = GUS->GetFullscreenMode();

	GUS->SetScreenResolution(PendingGraphics.Resolution);
	GUS->SetFullscreenMode(PendingGraphics.WindowMode);
	GUS->SetOverallScalabilityLevel(FMath::Clamp(PendingGraphics.QualityNotch, 0, 3));

	float NewLimit = 60.0f;
	switch (PendingGraphics.FpsCapIndex)
	{
	case 0: NewLimit = 30.0f; break;
	case 1: NewLimit = 60.0f; break;
	case 2: NewLimit = 90.0f; break;
	case 3: NewLimit = 120.0f; break;
	default: NewLimit = 0.0f; break; // Unlimited
	}
	GUS->SetFrameRateLimit(NewLimit);

	GUS->ApplySettings(false);

	// Persist display mode (Standard/Widescreen) for future viewport/camera use.
	if (UT66PlayerSettingsSubsystem* PS = GetPlayerSettings())
	{
		PS->SetDisplayModeIndex(static_cast<int32>(PendingGraphics.DisplayMode));
		PS->SetMonitorIndex(PendingGraphics.MonitorIndex);
	}

	// Move game window to selected monitor when multiple monitors are available.
	FDisplayMetrics Metrics;
	FSlateApplication::Get().GetCachedDisplayMetrics(Metrics);
	if (Metrics.MonitorInfo.IsValidIndex(PendingGraphics.MonitorIndex))
	{
		const FMonitorInfo& Mon = Metrics.MonitorInfo[PendingGraphics.MonitorIndex];
		TArray<TSharedRef<SWindow>> TopLevelWindows = FSlateApplication::Get().GetTopLevelWindows();
		if (TopLevelWindows.Num() > 0)
		{
			TSharedRef<SWindow> GameWindow = TopLevelWindows[0];
			const FVector2D Pos(static_cast<float>(Mon.WorkArea.Left), static_cast<float>(Mon.WorkArea.Top));
			GameWindow->MoveWindowTo(Pos);
		}
	}

	const bool bNeedsConfirm = bForceConfirmPrompt || (OldRes != PendingGraphics.Resolution) || (OldMode != PendingGraphics.WindowMode);
	if (bNeedsConfirm)
	{
		StartVideoModeConfirmPrompt();
	}
	else
	{
		GUS->SaveSettings();
		PendingGraphics.bDirty = false;
	}
}

void UT66SettingsScreen::StartVideoModeConfirmPrompt()
{
	if (bVideoModeConfirmActive) return;
	bVideoModeConfirmActive = true;
	VideoModeConfirmSecondsRemaining = 10;
	if (VideoModeConfirmOverlay.IsValid())
	{
		VideoModeConfirmOverlay->SetVisibility(EVisibility::Visible);
	}

	if (UT66LocalizationSubsystem* Loc = GetLocSubsystem())
	{
		if (VideoModeConfirmCountdownText.IsValid())
		{
			VideoModeConfirmCountdownText->SetText(FText::Format(Loc->GetText_KeepTheseSettingsBodyFormat(), FText::AsNumber(VideoModeConfirmSecondsRemaining)));
		}
	}

	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().ClearTimer(VideoModeConfirmTimerHandle);
		FTimerDelegate TickDelegate;
		TickDelegate.BindLambda([this]()
		{
			VideoModeConfirmSecondsRemaining = FMath::Max(0, VideoModeConfirmSecondsRemaining - 1);
			if (UT66LocalizationSubsystem* Loc = GetLocSubsystem())
			{
				if (VideoModeConfirmCountdownText.IsValid())
				{
					VideoModeConfirmCountdownText->SetText(FText::Format(Loc->GetText_KeepTheseSettingsBodyFormat(), FText::AsNumber(VideoModeConfirmSecondsRemaining)));
				}
			}
			if (VideoModeConfirmSecondsRemaining <= 0)
			{
				EndVideoModeConfirmPrompt(false);
			}
		});
		World->GetTimerManager().SetTimer(VideoModeConfirmTimerHandle, TickDelegate, 1.0f, true);
	}
}

void UT66SettingsScreen::EndVideoModeConfirmPrompt(bool bKeepNewSettings)
{
	if (!bVideoModeConfirmActive) return;
	bVideoModeConfirmActive = false;
	if (VideoModeConfirmOverlay.IsValid())
	{
		VideoModeConfirmOverlay->SetVisibility(EVisibility::Collapsed);
	}

	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().ClearTimer(VideoModeConfirmTimerHandle);
	}

	if (UGameUserSettings* GUS = GEngine ? GEngine->GetGameUserSettings() : nullptr)
	{
		if (bKeepNewSettings)
		{
			GUS->ConfirmVideoMode();
			GUS->SaveSettings();
			PendingGraphics.bDirty = false;
		}
		else
		{
			GUS->RevertVideoMode();
			GUS->ApplySettings(false);
			// Sync pending back to current applied.
			PendingGraphics.Resolution = GUS->GetScreenResolution();
			PendingGraphics.WindowMode = GUS->GetFullscreenMode();
			PendingGraphics.bDirty = false;
		}
	}
}


