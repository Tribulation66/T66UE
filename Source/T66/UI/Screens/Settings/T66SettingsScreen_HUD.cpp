// Copyright Tribulation 66. All Rights Reserved.

#include "UI/Screens/Settings/T66SettingsScreen_Private.h"

#include "UI/Style/T66FlatStyle.h"
#include "Widgets/Layout/SConstraintCanvas.h"
#include "Widgets/Layout/SScaleBox.h"

using namespace T66SettingsScreenPrivate;
TSharedRef<SWidget> UT66SettingsScreen::BuildHUDTab()
{
	UT66LocalizationSubsystem* Loc = GetLocSubsystem();
	UT66PlayerSettingsSubsystem* PS = GetPlayerSettings();
	InitializeUIScaleFromPlayerSettingsIfNeeded();
	FSettingsSliderRowParams UIScaleSliderParams;
	UIScaleSliderParams.Label = NSLOCTEXT("T66.Settings", "UIScale", "UI Scale");
	UIScaleSliderParams.InitialValue = (FMath::Clamp(PendingUIScale, 0.75f, 1.50f) - 0.75f) / 0.75f;
	UIScaleSliderParams.FormatValueText = [](float Value)
	{
		const float UIScale = FMath::Clamp(FMath::Lerp(0.75f, 1.50f, Value), 0.75f, 1.50f);
		return FText::FromString(FString::Printf(TEXT("%.2fx"), UIScale));
	};
	UIScaleSliderParams.OnValueChanged = [this](float Value)
	{
		PendingUIScale = FMath::Clamp(FMath::Lerp(0.75f, 1.50f, Value), 0.75f, 1.50f);
	};
	UIScaleSliderParams.OnMouseCaptureEnd = [this, PS]()
	{
		if (PS)
		{
			PS->SetUIScale(PendingUIScale);
			bUIScaleInitialized = false;
		}
	};
	UIScaleSliderParams.OnControllerCaptureEnd = UIScaleSliderParams.OnMouseCaptureEnd;
	UIScaleSliderParams.HelpText = NSLOCTEXT("T66.Settings", "UIScaleHelp", "Scales the entire HUD and menu UI on top of the automatic DPI scale.");

	return SNew(SScrollBox)
		.ScrollBarStyle(GetSettingsFlatScrollBarStyle())
		.ScrollBarVisibility(EVisibility::Visible)
		.ScrollBarThickness(FVector2D(14.f, 14.f))
		.ScrollBarPadding(FMargin(10.f, 0.f, 2.f, 0.f))
		+ SScrollBox::Slot()
		[
			SNew(SVerticalBox)
			+ SVerticalBox::Slot().AutoHeight().Padding(0.0f, 0.0f, 0.0f, 8.0f)
			[
				MakeSettingsSliderRow(UIScaleSliderParams)
			]
			+ SVerticalBox::Slot().AutoHeight().Padding(0.0f, 0.0f, 0.0f, 12.0f)
			[
				SNew(STextBlock)
				.Text(Loc ? Loc->GetText_SettingsHudToggleIntro() : NSLOCTEXT("T66.Settings", "HudToggleIntro", "When you press the HUD toggle key, the following elements will show or hide:"))
				.Font(SettingsRegularFont(18))
				.ColorAndOpacity(GetSettingsPageText())
				.AutoWrapText(true)
			]
			+ SVerticalBox::Slot().AutoHeight().Padding(0.0f, 0.0f, 0.0f, 8.0f)
			[
				MakeSettingsToggleRow(
					Loc,
					Loc ? Loc->GetText_SettingsHudInventory() : NSLOCTEXT("T66.Settings", "HudInventory", "Inventory"),
					[PS]() { return PS ? PS->GetHudToggleAffectsInventory() : true; },
					[PS](bool b) { if (PS) PS->SetHudToggleAffectsInventory(b); }
				)
			]
			+ SVerticalBox::Slot().AutoHeight().Padding(0.0f, 0.0f, 0.0f, 8.0f)
			[
				MakeSettingsToggleRow(
					Loc,
					Loc ? Loc->GetText_SettingsHudMinimap() : NSLOCTEXT("T66.Settings", "HudMinimap", "Minimap"),
					[PS]() { return PS ? PS->GetHudToggleAffectsMinimap() : true; },
					[PS](bool b) { if (PS) PS->SetHudToggleAffectsMinimap(b); }
				)
			]
			+ SVerticalBox::Slot().AutoHeight().Padding(0.0f, 0.0f, 0.0f, 8.0f)
			[
				MakeSettingsToggleRow(
					Loc,
					Loc ? Loc->GetText_SettingsHudIdolSlots() : NSLOCTEXT("T66.Settings", "HudIdolSlots", "Idol slots"),
					[PS]() { return PS ? PS->GetHudToggleAffectsIdolSlots() : true; },
					[PS](bool b) { if (PS) PS->SetHudToggleAffectsIdolSlots(b); }
				)
			]
			+ SVerticalBox::Slot().AutoHeight().Padding(0.0f, 0.0f, 0.0f, 8.0f)
			[
				MakeSettingsToggleRow(
					Loc,
					Loc ? Loc->GetText_SettingsHudPortraitStats() : NSLOCTEXT("T66.Settings", "HudPortraitStats", "Portrait & stats"),
					[PS]() { return PS ? PS->GetHudToggleAffectsPortraitStats() : true; },
					[PS](bool b) { if (PS) PS->SetHudToggleAffectsPortraitStats(b); }
				)
			]
		];
}

TSharedRef<SWidget> UT66SettingsScreen::BuildMediaViewerTab()
{
	UT66LocalizationSubsystem* Loc = GetLocSubsystem();
	UT66PlayerSettingsSubsystem* PS = GetPlayerSettings();
	struct FMediaSourceDefinition
	{
		FText Label;
		ET66MediaViewerSource Source = ET66MediaViewerSource::TikTok;
	};

	const TArray<FMediaSourceDefinition> SourceDefinitions = {
		{ NSLOCTEXT("T66.Settings", "MediaSourceTikTok", "TIKTOK"), ET66MediaViewerSource::TikTok },
		{ NSLOCTEXT("T66.Settings", "MediaSourceShorts", "YOUTUBE SHORTS"), ET66MediaViewerSource::Shorts },
		{ NSLOCTEXT("T66.Settings", "MediaSourceReels", "INSTAGRAM REELS"), ET66MediaViewerSource::Reels }
	};

	const TSharedRef<SHorizontalBox> SourceButtons = SNew(SHorizontalBox);
	for (int32 Index = 0; Index < SourceDefinitions.Num(); ++Index)
	{
		const FMediaSourceDefinition& SourceDefinition = SourceDefinitions[Index];
		SourceButtons->AddSlot().AutoWidth().Padding(Index + 1 < SourceDefinitions.Num() ? FMargin(0.f, 0.f, 6.f, 0.f) : FMargin(0.f))
		[
			MakeSelectableSettingsButton(
				FT66ButtonParams(
					SourceDefinition.Label,
					FOnClicked::CreateLambda([PS, Source = SourceDefinition.Source]()
					{
						if (PS)
						{
							PS->SetMediaViewerSource(Source);
						}
						return FReply::Handled();
					}),
					ET66ButtonType::Neutral)
				.SetMinWidth(152.f)
				.SetPadding(FMargin(10.f, 5.f)),
				[PS, Source = SourceDefinition.Source]()
				{
					return PS && PS->GetMediaViewerSource() == Source;
				})
		];
	}

	return SNew(SScrollBox)
		.ScrollBarStyle(GetSettingsFlatScrollBarStyle())
		.ScrollBarVisibility(EVisibility::Visible)
		.ScrollBarThickness(FVector2D(14.f, 14.f))
		.ScrollBarPadding(FMargin(10.f, 0.f, 2.f, 0.f))
		+ SScrollBox::Slot()
		[
			SNew(SVerticalBox)
			+ SVerticalBox::Slot().AutoHeight().Padding(0.0f, 0.0f, 0.0f, 12.0f)
			[
				SNew(STextBlock)
				.Text(Loc ? Loc->GetText_SettingsMediaViewerPrivacyBody() : NSLOCTEXT("T66.Settings", "MediaViewerPrivacyBody", "The Media Viewer runs only on your computer. We do not receive or store the videos you watch or any data from TikTok, YouTube, or Instagram. Choose which feed opens here, then use the key bound to \"Toggle Media Viewer\" in the Controls tab."))
				.Font(SettingsRegularFont(18))
				.ColorAndOpacity(GetSettingsPageText())
				.AutoWrapText(true)
			]
			+ SVerticalBox::Slot().AutoHeight().Padding(0.0f, 16.0f, 0.0f, 8.0f)
			[
				MakeSettingsToggleRow(
					Loc,
					Loc ? Loc->GetText_SettingsMediaViewerEnable() : NSLOCTEXT("T66.Settings", "MediaViewerEnable", "Enable Media Viewer"),
					[PS]() { return PS ? PS->GetMediaViewerEnabled() : true; },
					[PS](bool bEnabled)
					{
						if (PS)
						{
							PS->SetMediaViewerEnabled(bEnabled);
						}
					})
			]
			+ SVerticalBox::Slot().AutoHeight().Padding(0.0f, 0.0f, 0.0f, 8.0f)
			[
				MakeSettingsRow(
					SNew(SHorizontalBox)
					+ SHorizontalBox::Slot().FillWidth(0.32f).VAlign(VAlign_Center)
					[
						SNew(STextBlock)
						.Text(Loc ? Loc->GetText_SettingsMediaViewerSource() : NSLOCTEXT("T66.Settings", "MediaViewerSource", "Default Feed"))
						.Font(SettingsRegularFont(22))
						.ColorAndOpacity(GetSettingsPageText())
					]
					+ SHorizontalBox::Slot().FillWidth(0.68f).VAlign(VAlign_Center)
					[
						SourceButtons
					])
			]
		];
}

TSharedRef<SWidget> UT66SettingsScreen::BuildFlatHUDSettingsUI()
{
	constexpr float CanvasW = 1920.f;
	constexpr float CanvasH = 1080.f;

	auto DTag = [](const TCHAR* Text) -> FName
	{
		return FName(Text);
	};

	auto MakeMetadataRegion = [](const FName Tag, const FString& Role) -> TSharedRef<SWidget>
	{
		return FT66FlatStyle::AttachMetadata(SNew(SBox), Tag, Role, ET66FlatState::Default);
	};

	TSharedRef<SConstraintCanvas> Canvas = SNew(SConstraintCanvas);
	auto AddN = [&Canvas](const float X, const float Y, const float W, const float H, const TSharedRef<SWidget>& Widget)
	{
		Canvas->AddSlot()
		.Anchors(FAnchors(X, Y, X, Y))
		.Alignment(FVector2D::ZeroVector)
		.Offset(FMargin(0.f, 0.f, W * CanvasW, H * CanvasH))
		[
			Widget
		];
	};

	AddN(0.000f, 0.000f, 1.000f, 1.000f,
		FT66FlatStyle::AttachMetadata(
			SNew(SBorder)
			.BorderImage(FCoreStyle::Get().GetBrush("WhiteBrush"))
			.BorderBackgroundColor(FLinearColor::Black),
			DTag(TEXT("SettingsHUD.Background")),
			TEXT("Background"),
			ET66FlatState::Default));

	AddN(0.000f, 0.095f, 1.000f, 0.905f, MakeMetadataRegion(DTag(TEXT("SettingsHUD.Root")), TEXT("Root")));
	AddStableSettingsTabRow(Canvas, this, TEXT("SettingsHUD"), CurrentTab, CanvasW, CanvasH);
	AddN(0.002f, 0.223f, 0.978f, 0.724f,
		FT66FlatStyle::AttachMetadata(
			BuildHUDTab(),
			DTag(TEXT("SettingsHUD.ContentScroll")),
			TEXT("ScrollBox"),
			ET66FlatState::Default));

	return SNew(SScaleBox)
		.Stretch(EStretch::ScaleToFit)
		.StretchDirection(EStretchDirection::Both)
		[
			SNew(SBox)
			.WidthOverride(CanvasW)
			.HeightOverride(CanvasH)
			[
				Canvas
			]
		];
}

TSharedRef<SWidget> UT66SettingsScreen::BuildFlatMediaViewerSettingsUI()
{
	UT66LocalizationSubsystem* Loc = GetLocSubsystem();
	UT66PlayerSettingsSubsystem* PS = GetPlayerSettings();

	constexpr float CanvasW = 1920.f;
	constexpr float CanvasH = 1080.f;
	const FName SettingsTabsGroup(TEXT("SettingsTabs"));

	auto DTag = [](const TCHAR* Text) -> FName
	{
		return FName(Text);
	};

	auto ChildTag = [](const FName& BaseTag, const TCHAR* Suffix) -> FName
	{
		return FName(*(BaseTag.ToString() + FString(TEXT(".")) + Suffix));
	};

	auto MakeLabel = [](const FName Tag, const TAttribute<FText>& Text, const int32 FontSize, const FLinearColor& Color, const bool bBold = false, const ETextJustify::Type Justify = ETextJustify::Left, const bool bAutoWrap = true) -> TSharedRef<SWidget>
	{
		return FT66FlatStyle::AttachMetadata(
			SNew(STextBlock)
			.Text(Text)
			.Font(bBold ? FT66FlatStyle::MakeBoldFont(FontSize) : FT66FlatStyle::MakeFont(FontSize))
			.ColorAndOpacity(Color)
			.Justification(Justify)
			.AutoWrapText(bAutoWrap)
			.OverflowPolicy(ETextOverflowPolicy::Ellipsis),
			Tag,
			TEXT("Label"),
			ET66FlatState::Default,
			TOptional<FLinearColor>(),
			false,
			NAME_None,
			true);
	};

	auto MakeMetadataRegion = [](const FName Tag, const FString& Role) -> TSharedRef<SWidget>
	{
		return FT66FlatStyle::AttachMetadata(SNew(SBox), Tag, Role, ET66FlatState::Default);
	};

	auto MakeFlatTab = [this, &MakeLabel, &ChildTag, SettingsTabsGroup](const FName Tag, const ET66SettingsTab Tab, const ET66FlatState State, const FText& Text, const float Width, const int32 FontSize = 22) -> TSharedRef<SWidget>
	{
		return FT66FlatStyle::MakeFlatToggleGroupButton(
			State,
			SNew(SBox)
			.HAlign(HAlign_Center)
			.VAlign(VAlign_Center)
			[
				MakeLabel(ChildTag(Tag, TEXT("Label")), Text, FontSize, State == ET66FlatState::Selected ? FT66FlatStyle::SelectedText() : FT66FlatStyle::PrimaryText(), true, ETextJustify::Center, false)
			],
			FOnClicked::CreateLambda([this, Tab]()
			{
				SwitchToTab(Tab);
				ForceRebuildSlate();
				return FReply::Handled();
			}),
			FMargin(0.f),
			Width,
			0.079f * CanvasH,
			true,
			Tag,
			SettingsTabsGroup);
	};

	auto MakeToggleButton = [this, &MakeLabel, &ChildTag](const FName Tag, const FName ToggleGroup, const ET66FlatState State, const FText& Text, TFunction<void()> Action, const float Width = 128.f) -> TSharedRef<SWidget>
	{
		return FT66FlatStyle::MakeFlatToggleGroupButton(
			State,
			SNew(SBox).HAlign(HAlign_Center).VAlign(VAlign_Center)
			[
				MakeLabel(ChildTag(Tag, TEXT("Label")), Text, 22, State == ET66FlatState::Selected ? FT66FlatStyle::SelectedText() : FT66FlatStyle::PrimaryText(), true, ETextJustify::Center, false)
			],
			FOnClicked::CreateLambda([this, Action = MoveTemp(Action)]()
			{
				Action();
				ForceRebuildSlate();
				return FReply::Handled();
			}),
			FMargin(0.f),
			Width,
			64.f,
			true,
			Tag,
			ToggleGroup);
	};

	TSharedRef<SConstraintCanvas> Canvas = SNew(SConstraintCanvas);
	auto AddN = [&Canvas](const float X, const float Y, const float W, const float H, const TSharedRef<SWidget>& Widget)
	{
		Canvas->AddSlot()
		.Anchors(FAnchors(X, Y, X, Y))
		.Alignment(FVector2D::ZeroVector)
		.Offset(FMargin(0.f, 0.f, W * CanvasW, H * CanvasH))
		[
			Widget
		];
	};

	const bool bMediaViewerEnabled = PS ? PS->GetMediaViewerEnabled() : true;
	const ET66MediaViewerSource CurrentSource = PS ? PS->GetMediaViewerSource() : ET66MediaViewerSource::TikTok;

	AddN(0.000f, 0.000f, 1.000f, 1.000f,
		FT66FlatStyle::AttachMetadata(
			SNew(SBorder)
			.BorderImage(FCoreStyle::Get().GetBrush("WhiteBrush"))
			.BorderBackgroundColor(FLinearColor::Black),
			DTag(TEXT("SettingsMediaViewer.Background")),
			TEXT("Background"),
			ET66FlatState::Default));

	AddN(0.000f, 0.095f, 1.000f, 0.905f, MakeMetadataRegion(DTag(TEXT("SettingsMediaViewer.Root")), TEXT("Root")));
	AddN(0.003f, 0.123f, 0.994f, 0.079f, MakeMetadataRegion(DTag(TEXT("SettingsMediaViewer.SettingsTabs")), TEXT("ToggleGroup.SettingsTabs")));

	AddN(0.003f, 0.123f, 0.119f, 0.079f, MakeFlatTab(DTag(TEXT("SettingsMediaViewer.SettingsTabs.GameplayButton")), ET66SettingsTab::Gameplay, ET66FlatState::Default, NSLOCTEXT("T66.Settings", "TabGameplayFlatMediaViewer", "GAMEPLAY"), 0.119f * CanvasW));
	AddN(0.129f, 0.123f, 0.118f, 0.079f, MakeFlatTab(DTag(TEXT("SettingsMediaViewer.SettingsTabs.GraphicsButton")), ET66SettingsTab::Graphics, ET66FlatState::Default, NSLOCTEXT("T66.Settings", "TabGraphicsFlatMediaViewer", "GRAPHICS"), 0.118f * CanvasW));
	AddN(0.253f, 0.123f, 0.118f, 0.079f, MakeFlatTab(DTag(TEXT("SettingsMediaViewer.SettingsTabs.ControlsButton")), ET66SettingsTab::Controls, ET66FlatState::Default, NSLOCTEXT("T66.Settings", "TabControlsFlatMediaViewer", "CONTROLS"), 0.118f * CanvasW));
	AddN(0.379f, 0.123f, 0.118f, 0.079f, MakeFlatTab(DTag(TEXT("SettingsMediaViewer.SettingsTabs.HUDButton")), ET66SettingsTab::HUD, ET66FlatState::Default, NSLOCTEXT("T66.Settings", "TabHUDFlatMediaViewer", "HUD"), 0.118f * CanvasW));
	AddN(0.503f, 0.123f, 0.118f, 0.079f, MakeFlatTab(DTag(TEXT("SettingsMediaViewer.SettingsTabs.MediaViewerButton")), ET66SettingsTab::MediaViewer, ET66FlatState::Selected, NSLOCTEXT("T66.Settings", "TabMediaViewerFlatMediaViewer", "MEDIA VIEWER"), 0.118f * CanvasW, 18));
	AddN(0.628f, 0.123f, 0.118f, 0.079f, MakeFlatTab(DTag(TEXT("SettingsMediaViewer.SettingsTabs.AudioButton")), ET66SettingsTab::Audio, ET66FlatState::Default, NSLOCTEXT("T66.Settings", "TabAudioFlatMediaViewer", "AUDIO"), 0.118f * CanvasW));
	AddN(0.754f, 0.123f, 0.118f, 0.079f, MakeFlatTab(DTag(TEXT("SettingsMediaViewer.SettingsTabs.CrashingButton")), ET66SettingsTab::Crashing, ET66FlatState::Default, NSLOCTEXT("T66.Settings", "TabCrashingFlatMediaViewer", "CRASHING"), 0.118f * CanvasW, 20));

	AddN(0.002f, 0.231f, 0.978f, 0.062f,
		MakeLabel(
			DTag(TEXT("SettingsMediaViewer.PrivacyBody")),
			Loc ? Loc->GetText_SettingsMediaViewerPrivacyBody() : NSLOCTEXT("T66.Settings", "MediaViewerPrivacyBodyFlat", "The Media Viewer runs only on your computer. We do not receive or store the videos you watch or any data from TikTok, YouTube, or Instagram. Choose which feed opens here, then use the key bound to \"Toggle Media Viewer\" in the Controls tab."),
			22,
			FT66FlatStyle::PrimaryText(),
			true,
			ETextJustify::Left,
			true));

	AddN(0.002f, 0.322f, 0.978f, 0.096f,
		FT66FlatStyle::MakeFlatSubPanel(
			ET66FlatState::Default,
			FMargin(32.f, 0.f),
			SNew(SHorizontalBox)
			+ SHorizontalBox::Slot().FillWidth(1.f).VAlign(VAlign_Center)
			[
				MakeLabel(DTag(TEXT("SettingsMediaViewer.EnableRow.Label")), Loc ? Loc->GetText_SettingsMediaViewerEnable() : NSLOCTEXT("T66.Settings", "MediaViewerEnableFlat", "Enable Media Viewer"), 26, FT66FlatStyle::PrimaryText(), false, ETextJustify::Left, false)
			]
			+ SHorizontalBox::Slot().AutoWidth().VAlign(VAlign_Center)
			[
				MakeToggleButton(DTag(TEXT("SettingsMediaViewer.EnableRow.OnButton")), DTag(TEXT("SettingsMediaViewer.Enable")), bMediaViewerEnabled ? ET66FlatState::Selected : ET66FlatState::Default, GetSettingsOnText(Loc), [PS]() { if (PS) PS->SetMediaViewerEnabled(true); })
			]
			+ SHorizontalBox::Slot().AutoWidth().VAlign(VAlign_Center).Padding(10.f, 0.f, 0.f, 0.f)
			[
				MakeToggleButton(DTag(TEXT("SettingsMediaViewer.EnableRow.OffButton")), DTag(TEXT("SettingsMediaViewer.Enable")), bMediaViewerEnabled ? ET66FlatState::Default : ET66FlatState::Selected, GetSettingsOffText(Loc), [PS]() { if (PS) PS->SetMediaViewerEnabled(false); })
			],
			nullptr,
			DTag(TEXT("SettingsMediaViewer.EnableRow"))));

	AddN(0.002f, 0.429f, 0.978f, 0.126f,
		FT66FlatStyle::MakeFlatSubPanel(
			ET66FlatState::Default,
			FMargin(36.f, 0.f),
			SNew(SHorizontalBox)
			+ SHorizontalBox::Slot().FillWidth(0.32f).VAlign(VAlign_Center)
			[
				MakeLabel(DTag(TEXT("SettingsMediaViewer.SourceRow.Label")), Loc ? Loc->GetText_SettingsMediaViewerSource() : NSLOCTEXT("T66.Settings", "MediaViewerSourceFlat", "Default Feed"), 26, FT66FlatStyle::PrimaryText(), false, ETextJustify::Left, false)
			]
			+ SHorizontalBox::Slot().AutoWidth().VAlign(VAlign_Center)
			[
				MakeToggleButton(DTag(TEXT("SettingsMediaViewer.SourceRow.TikTokButton")), DTag(TEXT("SettingsMediaViewer.Source")), CurrentSource == ET66MediaViewerSource::TikTok ? ET66FlatState::Selected : ET66FlatState::Default, NSLOCTEXT("T66.Settings", "MediaSourceTikTokFlat", "TIKTOK"), [PS]() { if (PS) PS->SetMediaViewerSource(ET66MediaViewerSource::TikTok); }, 214.f)
			]
			+ SHorizontalBox::Slot().AutoWidth().VAlign(VAlign_Center).Padding(12.f, 0.f, 0.f, 0.f)
			[
				MakeToggleButton(DTag(TEXT("SettingsMediaViewer.SourceRow.ShortsButton")), DTag(TEXT("SettingsMediaViewer.Source")), CurrentSource == ET66MediaViewerSource::Shorts ? ET66FlatState::Selected : ET66FlatState::Default, NSLOCTEXT("T66.Settings", "MediaSourceShortsFlat", "YOUTUBE SHORTS"), [PS]() { if (PS) PS->SetMediaViewerSource(ET66MediaViewerSource::Shorts); }, 214.f)
			]
			+ SHorizontalBox::Slot().AutoWidth().VAlign(VAlign_Center).Padding(12.f, 0.f, 0.f, 0.f)
			[
				MakeToggleButton(DTag(TEXT("SettingsMediaViewer.SourceRow.ReelsButton")), DTag(TEXT("SettingsMediaViewer.Source")), CurrentSource == ET66MediaViewerSource::Reels ? ET66FlatState::Selected : ET66FlatState::Default, NSLOCTEXT("T66.Settings", "MediaSourceReelsFlat", "INSTAGRAM REELS"), [PS]() { if (PS) PS->SetMediaViewerSource(ET66MediaViewerSource::Reels); }, 214.f)
			],
			nullptr,
			DTag(TEXT("SettingsMediaViewer.SourceRow"))));

	return SNew(SScaleBox)
		.Stretch(EStretch::ScaleToFit)
		.StretchDirection(EStretchDirection::Both)
		[
			SNew(SBox)
			.WidthOverride(CanvasW)
			.HeightOverride(CanvasH)
			[
				Canvas
			]
		];
}

void UT66SettingsScreen::InitializeUIScaleFromPlayerSettingsIfNeeded()
{
	if (bUIScaleInitialized)
	{
		return;
	}

	if (UT66PlayerSettingsSubsystem* PS = GetPlayerSettings())
	{
		PendingUIScale = PS->GetUIScale();
	}
	else
	{
		PendingUIScale = 1.0f;
	}

	bUIScaleInitialized = true;
}


