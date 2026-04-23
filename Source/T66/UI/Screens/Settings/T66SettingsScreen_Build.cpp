// Copyright Tribulation 66. All Rights Reserved.

#include "UI/Screens/Settings/T66SettingsScreen_Private.h"

using namespace T66SettingsScreenPrivate;
TSharedRef<SWidget> UT66SettingsScreen::BuildSlateUI()
{
	UT66LocalizationSubsystem* Loc = GetLocSubsystem();
	struct FSettingsTabDefinition
	{
		FText Label;
		ET66SettingsTab Tab = ET66SettingsTab::Gameplay;
	};

	const TArray<FSettingsTabDefinition> TabDefinitions = {
		{ Loc ? Loc->GetText_SettingsTabGameplay() : NSLOCTEXT("T66.Settings", "TabGameplay", "GAMEPLAY"), ET66SettingsTab::Gameplay },
		{ Loc ? Loc->GetText_SettingsTabGraphics() : NSLOCTEXT("T66.Settings", "TabGraphics", "GRAPHICS"), ET66SettingsTab::Graphics },
		{ Loc ? Loc->GetText_SettingsTabControls() : NSLOCTEXT("T66.Settings", "TabControls", "CONTROLS"), ET66SettingsTab::Controls },
		{ Loc ? Loc->GetText_SettingsTabHUD() : NSLOCTEXT("T66.Settings", "TabHUD", "HUD"), ET66SettingsTab::HUD },
		{ Loc ? Loc->GetText_SettingsTabMediaViewer() : NSLOCTEXT("T66.Settings", "TabMediaViewer", "MEDIA VIEWER"), ET66SettingsTab::MediaViewer },
		{ Loc ? Loc->GetText_SettingsTabAudio() : NSLOCTEXT("T66.Settings", "TabAudio", "AUDIO"), ET66SettingsTab::Audio },
		{ Loc ? Loc->GetText_SettingsTabCrashing() : NSLOCTEXT("T66.Settings", "TabCrashing", "CRASHING"), ET66SettingsTab::Crashing },
		{ NSLOCTEXT("T66.Settings", "TabRetroFX", "RETRO FX"), ET66SettingsTab::RetroFX }
	};

	const TSharedRef<SHorizontalBox> TabRow = SNew(SHorizontalBox);
	for (const FSettingsTabDefinition& TabDefinition : TabDefinitions)
	{
		TabRow->AddSlot().FillWidth(1.f)
		[
			SNew(SBox).Padding(FMargin(2.f))
			[
				MakeSelectableSettingsButton(
					FT66ButtonParams(TabDefinition.Label, FOnClicked::CreateUObject(this, &UT66SettingsScreen::HandleTabClicked, TabDefinition.Tab), ET66ButtonType::Neutral)
					.SetFontSize(16)
					.SetHeight(48.f)
					.SetMinWidth(0.f)
					.SetPadding(FMargin(6.f, 4.f))
					.SetContent(
						SNew(STextBlock)
						.Text(TabDefinition.Label)
						.Font(SettingsBoldFont(16))
						.ColorAndOpacity(FT66Style::Tokens::Text)),
					[this, Tab = TabDefinition.Tab]() { return CurrentTab == Tab; })
			]
		];
	}

	// Build the widget switcher with all tab content (stored as class member)
	const bool bModalPresentation = (UIManager && UIManager->GetCurrentModalType() == ScreenType) || (!UIManager && GetOwningPlayer() && GetOwningPlayer()->IsPaused());
	const float ResponsiveScale = FMath::Max(FT66Style::GetViewportResponsiveScale(), KINDA_SMALL_NUMBER);
	const float TopBarOverlapPx = 22.f;
	const float TopInset = bModalPresentation
		? 0.f
		: FMath::Max(0.f, ((UIManager ? UIManager->GetFrontendTopBarContentHeight() : 0.f) - TopBarOverlapPx) / ResponsiveScale);
	const FVector2D SafeFrameSize = FT66Style::GetSafeFrameSize();
	const float SurfaceW = FMath::Max(1.f, SafeFrameSize.X);
	const float SurfaceH = FMath::Max(1.f, SafeFrameSize.Y - TopInset);

	const TSharedRef<SWidget> SettingsContent =
		SNew(SVerticalBox)
		// Tab bar + close button (extra vertical padding so tab buttons are not clipped)
		+ SVerticalBox::Slot()
		.AutoHeight()
		.Padding(0.0f)
		[
			MakeSettingsPanel(
				SNew(SHorizontalBox)
				+ SHorizontalBox::Slot().FillWidth(1.f)
				[
					TabRow
				]
				+ SHorizontalBox::Slot().AutoWidth()
				.Padding(6.f, 0.f, 0.f, 0.f)
				[
					SNew(SBox)
					.Visibility(bModalPresentation ? EVisibility::Visible : EVisibility::Collapsed)
					[
						MakeSettingsButton(
							FT66ButtonParams(NSLOCTEXT("T66.Common", "CloseX", "X"), FOnClicked::CreateUObject(this, &UT66SettingsScreen::HandleCloseClicked), ET66ButtonType::Danger)
							.SetFontSize(18)
							.SetMinWidth(36.f).SetHeight(36.f)
							.SetPadding(FMargin(0.f))
							.SetColor(FT66Style::Tokens::Danger)
							.SetContent(SNew(STextBlock).Text(NSLOCTEXT("T66.Common", "CloseX", "X"))
								.Font(SettingsBoldFont(20))
								.ColorAndOpacity(FT66Style::Tokens::Text))
						)
					]
				],
				ET66PanelType::Panel,
				T66SettingsRowFill(),
				FMargin(10.0f, 10.0f)
			)
		]
		// Content area
		+ SVerticalBox::Slot()
		.FillHeight(1.0f)
		.Padding(8.0f)
		[
			SNew(SBorder)
			.BorderImage(FCoreStyle::Get().GetBrush("NoBrush"))
			.Clipping(EWidgetClipping::ClipToBounds)
			[
				SAssignNew(ContentSwitcher, SWidgetSwitcher)
				.WidgetIndex(static_cast<int32>(CurrentTab))
				+ SWidgetSwitcher::Slot()
				[
					BuildGameplayTab()
				]
				+ SWidgetSwitcher::Slot()
				[
					BuildGraphicsTab()
				]
				+ SWidgetSwitcher::Slot()
				[
					BuildControlsTab()
				]
				+ SWidgetSwitcher::Slot()
				[
					BuildHUDTab()
				]
				+ SWidgetSwitcher::Slot()
				[
					BuildMediaViewerTab()
				]
				+ SWidgetSwitcher::Slot()
				[
					BuildAudioTab()
				]
				+ SWidgetSwitcher::Slot()
				[
					BuildCrashingTab()
				]
				+ SWidgetSwitcher::Slot()
				[
					BuildRetroFXTab()
				]
			]
		];

	const TSharedRef<SWidget> SettingsSurface =
		MakeSettingsPanel(
			SNew(SBorder)
			.BorderImage(FCoreStyle::Get().GetBrush("WhiteBrush"))
			.BorderBackgroundColor(GetSettingsPageBackground())
			.Clipping(EWidgetClipping::ClipToBounds)
			.Padding(FMargin(8.f, 0.f, 8.f, 8.f))
			[
				SettingsContent
			],
			ET66PanelType::Panel,
			T66SettingsShellFill(),
			FMargin(0.f));

	TSharedRef<SOverlay> Root = SNew(SOverlay);

	if (bModalPresentation)
	{
		Root->AddSlot()
		.HAlign(HAlign_Fill)
		.VAlign(VAlign_Fill)
		[
			SNew(SBorder)
			.BorderImage(FCoreStyle::Get().GetBrush("WhiteBrush"))
			.BorderBackgroundColor(FT66Style::Tokens::Scrim)
		];
	}

	Root->AddSlot()
	.HAlign(HAlign_Fill)
	.VAlign(VAlign_Fill)
	.Padding(FMargin(0.f, TopInset, 0.f, 0.f))
	[
		SNew(SBox)
		.WidthOverride(SurfaceW)
		.HeightOverride(SurfaceH)
		[
			SettingsSurface
		]
	];

	return Root;
}

FReply UT66SettingsScreen::HandleTabClicked(ET66SettingsTab Tab)
{
	SwitchToTab(Tab);
	return FReply::Handled();
}


